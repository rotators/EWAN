#include "Script.hpp"

#include "App.hpp"
#include "Log.hpp"
#include "Text.hpp"
#include "Utils.hpp"

#include <filesystem>
#include <unordered_map>

using namespace std::literals::string_literals;

//
// Script
//

EWAN::Script::Script() :
    //
    // Event name defines what kind of metadata script function need to have to become event callback
    // Function declaration is stored as string list, which holds return type and function parameters
    //
    // Event name must start with "On"
    // Function declaration must use `?` character instead of root namespace name
    //
    // Event constructor                                  Script code
    // --------------------------------------------------------------------------------------------
    // Event("OnExample", {"void"}) ..................... [OnExample] void  f();
    // Event("OnHappens", {"bool"}) ..................... [OnHappens] bool  f();
    // Event("OnTrigger", {"float", "string", "int"}) ... [OnTrigger] float f(string, int);
    // Event("OnTragedy", {"void", "?::Thing"}) ......... [OnTragedy] void  f(RootNamespace::Thing);
    //
    // Note that single script function can handle any amount of events (as long their signatures are compatibile)
    //

    OnBuild("OnBuild", {"void"}),
    OnInit("OnInit", {"bool"}),
    OnFinish("OnFinish", {"void"}),
    OnDraw("OnDraw", {"void"}),
    OnKeyDown("OnKeyDown", {"void", "const ?::Key"}),
    OnKeyUp("OnKeyUp", {"void", "const ?::Key"}),
    OnMouseDown("OnMouseDown", {"void", "const ?::MouseButton"}),
    OnMouseUp("OnMouseUp", {"void", "const ?::MouseButton"}),
    OnMouseMove("OnMouseMove", {"void", "const int32", "const int32"})
{
    // Cache all events in single container, for easier mass-processing
    AllEvents.push_back(&OnBuild);
    AllEvents.push_back(&OnInit);
    AllEvents.push_back(&OnFinish);
    AllEvents.push_back(&OnDraw);
    AllEvents.push_back(&OnKeyDown);
    AllEvents.push_back(&OnKeyUp);
    AllEvents.push_back(&OnMouseDown);
    AllEvents.push_back(&OnMouseUp);
    AllEvents.push_back(&OnMouseMove);
}

EWAN::Script::~Script()
{
    Finish();
}

bool EWAN::Script::Init(App* app)
{
    static const std::string fail = "Script initialization failed... ";

    RootDirectory = std::filesystem::path(app->GameInfo.Path).replace_filename(app->GameInfo.ScriptInit).remove_filename().make_preferred().string();

    // Make sure root namespace is set to non-blank value, leaving detailed check to AngelScript during API registration

    if(app->GameInfo.ScriptNamespace.empty())
        app->GameInfo.ScriptNamespace = "EWAN";

    if(Text::IsBlank(app->GameInfo.ScriptNamespace))
    {
        Log::PrintError(fail + "root namespace is blank");
        return false;
    }

    // Display basic info about script system

    Log::PrintInfo("Script root directory...  " + RootDirectory);
    Log::PrintInfo("Script root namespace...  " + app->GameInfo.ScriptNamespace);
    Log::PrintInfo("Script initialization...  AngelScript v"s + as::asGetLibraryVersion() + as::asGetLibraryOptions());

    // API registration

    as::asIScriptEngine* engine = CreateEngine();
    if(!engine)
    {
        Log::PrintError(fail + "cannot create engine");
        return false;
    }

    if(!API::Init(app, engine, app->GameInfo.ScriptNamespace))
    {
        WriteError(engine, fail + "cannot register API");
        DestroyEngine(engine);
        return false;
    }

    // Replace root namespace placeholder with real value

    for(const auto& event : AllEvents)
    {
        for(auto& param : event->Params)
        {
            // "?" might need to be replaced with something else in future
            param = Text::Replace(param, "?", app->GameInfo.ScriptNamespace);
        }
    }

    // Load Init module; it is the only module loaded automagically by engine
    // Scripts are responsible for loading other modules using provided App::Script::LoadModule() function

    if(!LoadInitModule(app->GameInfo, engine))
    {
        WriteError(engine, fail + "cannot load init module");
        DestroyEngine(engine);
        return false;
    }

    // There might be no modules loaded, if init module is marked as optional and cannot be built
    if(!engine->GetModuleCount())
    {
        WriteError(engine, fail + "cannot find any modules (early check)");
        DestroyEngine(engine);
        return false;
    }

    if(!BindImportedFunctions(engine))
    {
        WriteError(engine, fail + "cannot bind all imported functions");
        DestroyEngine(engine);
        return false;
    }

    // [OnInit] always clears Event.OnInit
    // `falseFunction` points to first script function which returned false (if any)
    {
        as::asIScriptFunction* falseFunction = nullptr;
        if(!OnInit.RunOnInit(engine, falseFunction))
        {
            WriteError(engine, fail + falseFunction->GetDeclaration(true, true, true) + " = false", falseFunction->GetModuleName());
            DestroyEngine(engine);
            return false;
        }
    }

    // Unload modules which requested it with `#pragma module unload`
    {
        // Cache all modules first so they can be safely unloaded
        std::vector<as::asIScriptModule*> allModules;
        for(as::asUINT m = 0, mLen = engine->GetModuleCount(); m < mLen; m++)
        {
            allModules.push_back(engine->GetModuleByIndex(m));
        }

        for(auto& module : allModules)
        {
            if(UserData::Get(module)->Unload)
            {
                UnloadModule(module);
                continue;
            }
        }
    }

    // There might be no modules left, if all modules are marked as optional and cannot be built
    if(!engine->GetModuleCount())
    {
        WriteError(engine, fail + "cannot find any modules (late check)");
        DestroyEngine(engine);
        return false;
    }

    if(engine->GarbageCollect(as::asGC_FULL_CYCLE) != 0)
    {
        WriteError(engine, fail + "cannot finish garbage collection");
        DestroyEngine(engine);
        return false;
    }

    WriteInfo(engine, "Script initialization complete");
    AS = engine;

    return true;
}

void EWAN::Script::Finish()
{
    if(AS)
    {
        Log::PrintInfo("Script finalization...");

        OnFinish.IgnoreExecuteErrors = true;
        OnFinish.Run();
        OnFinish.Unregister(AS);
        DestroyEngine(AS);

        Log::PrintInfo("Script finalization complete");
    }
}

//

/* static */ void EWAN::Script::WriteInfo(as::asIScriptEngine* engine, const std::string& message, const std::string& section /*= {} */, int row /*= 0 */, int col /*= 0 */)
{
    engine->WriteMessage(section.c_str(), row, col, as::asMSGTYPE_INFORMATION, message.c_str());
}

/* static */ void EWAN::Script::WriteWarning(as::asIScriptEngine* engine, const std::string& message, const std::string& section /*= {} */, int row /*= 0 */, int col /*= 0 */)
{
    engine->WriteMessage(section.c_str(), row, col, as::asMSGTYPE_WARNING, message.c_str());
}

/* static */ void EWAN::Script::WriteError(as::asIScriptEngine* engine, const std::string& message, const std::string& section /*= {} */, int row /*= 0 */, int col /*= 0 */)
{
    engine->WriteMessage(section.c_str(), row, col, as::asMSGTYPE_ERROR, message.c_str());
}

//

bool EWAN::Script::LoadModule(as::asIScriptEngine* engine, const std::string& fileName, const std::string& moduleName)
{
    static const std::string fail = "Loading module failed";

    if(!std::filesystem::exists(fileName))
    {
        WriteError(engine, "Cannot load module (file does not exists) : " + fileName);
        return false;
    }

    as::asIScriptModule* module = engine->GetModule(moduleName.c_str(), as::asGM_ONLY_IF_EXISTS);
    if(module)
    {
        WriteError(engine, "Cannot load module (name already in use) : ", moduleName);
        return false;
    }

    Builder builder;
    WriteInfo(engine, "Loading module...", moduleName);

    // Creates module and assigns UserData::Module
    int r = builder.StartNewModule(engine, moduleName.c_str());
    if(r < 0)
        return false;

    module = builder.GetModule();

    // Using AddSectionFromMemory() instead of AddSectionFromFile() for custom script sections names
    std::string fileContent;
    if(!Utils::ReadFile(fileName, fileContent))
    {
        WriteInfo(engine, fail, module->GetName());
        UnloadModule(module);

        return false;
    }

    // Section name is set to script filename relative to RootDirectory, with enforced *NIX path separators
    const std::string sectionName = Text::Replace(std::filesystem::relative(fileName, RootDirectory).string(), "\\", "/");

    r = builder.AddSectionFromMemory(sectionName.c_str(), fileContent.c_str());
    if(r < 0)
    {
        WriteInfo(engine, fail, module->GetName());
        UnloadModule(module);

        return false;
    }

    /*/ At this point all non-instant `#pragma module` directives are available as UserData::Module /*/

    // Optional modules does not inform caller about failure
    bool optional = UserData::Get(module)->Optional;
    r             = builder.BuildModule();
    if(r < 0)
    {
        WriteError(engine, fail, module->GetName());
        UnloadModule(module);

        return optional;
    }

    // All script functions must have user data set
    for(as::asUINT f = 0, fLen = module->GetFunctionCount(); f < fLen; f++)
    {
        module->GetFunctionByIndex(f)->SetUserData(new UserData::Function, UserData::IDX);
    }

    // [OnBuild] always clears OnBuild.Functions
    if(!LoadModuleMetadata(builder) || !OnBuild.RunOnBuild(module))
    {
        WriteError(engine, fail, module->GetName());
        UnloadModule(module);

        return optional;
    }

    WriteInfo(engine, "Loading module complete", module->GetName());

    return true;
}

bool EWAN::Script::LoadModule_ScriptCall(const std::string& fileName, const std::string& moduleName)
{
    as::asIScriptContext* context = as::asGetActiveContext();
    if(!context)
    {
        Log::PrintError("LoadModule : no context");
        return false;
    }
    else if(Text::IsBlank(moduleName))
    {
        WriteError(context->GetEngine(), "LoadModule : moduleName is blank");
        return false;
    }

    return LoadModule(context->GetEngine(), RootDirectory + fileName, moduleName);
}

bool EWAN::Script::LoadInitModule(const GameInfo& game, as::asIScriptEngine* engine)
{
    std::string scriptFile = std::filesystem::path(game.Path).replace_filename(game.ScriptInit).make_preferred().string();

    return LoadModule(engine, scriptFile, "init");
}

bool EWAN::Script::UnloadModule(as::asIScriptModule*& module)
{
    // Disallow unloading module(s) in use
    as::asIScriptContext* context = as::asGetActiveContext();
    if(context)
    {
        for(as::asUINT f = 0, fLen = context->GetCallstackSize(); f < fLen; f++)
        {
            if(context->GetFunction(f)->GetModule() == module)
            {
                WriteError(context->GetEngine(), "Cannot unload module (currently in use)", module->GetName());
                return false;
            }
        }
    }

    // Cache data used by post-discard message
    as::asIScriptEngine* engine     = module->GetEngine();
    const std::string    moduleName = module->GetName();

    WriteInfo(engine, "Unloading module...", moduleName);

    for(const auto& event : AllEvents)
    {
        event->Unregister(module);
    }

    module->UnbindAllImportedFunctions();
    module->Discard();

    module = nullptr;

    WriteInfo(engine, "Unloading module complete", moduleName);

    return true;
}

bool EWAN::Script::UnloadModule_ScriptCall(const std::string& moduleName)
{
    as::asIScriptContext* context = as::asGetActiveContext();
    if(!context)
        return false;

    as::asIScriptModule* module = context->GetEngine()->GetModule(moduleName.c_str(), as::asGM_ONLY_IF_EXISTS);
    if(!module)
    {
        WriteError(context->GetEngine(), "Cannot unload module (does not exists) : "s + moduleName);
        return false;
    }

    return UnloadModule(module);
}

bool EWAN::Script::LoadModuleMetadata(Builder& builder)
{
    bool result = true;

    as::asIScriptEngine* engine = builder.GetEngine();
    as::asIScriptModule* module = builder.GetModule();

    // Process "slow" pragmas (requires module to be built first)

    if(UserData::Get(module)->Debug) // #pragma module debug
    {
        for(as::asUINT f = 0, fLen = module->GetFunctionCount(); f < fLen; f++)
        {
            as::asIScriptFunction* function = module->GetFunctionByIndex(f);
            UserData::Get(function)->Debug  = true;
        }
    }

    // Process functions with metadata

    for(const auto& metadata : builder.GetAllMetadataForFunc())
    {
        as::asIScriptFunction* function = engine->GetFunctionById(metadata.first);

        // Check [Debug *] metadata
        // This should be checked early, as other metadata might want to generate debug messages
        for(const auto& metadataText : metadata.second)
        {
            std::vector<std::string> text = Text::Split(metadataText, ' ');

            if(text.front() == "Debug" && text.size() == 2)
            {
                if(text.at(1) == "ON")
                    UserData::Get(function)->Debug = true;
                else if(text.at(1) == "OFF")
                    UserData::Get(function)->Debug = false;
            }
        }

        // Check [On*] metadata
        for(const auto& event : AllEvents)
        {
            if(event->Name.substr(0, 2) != "On")
            {
                WriteError(engine, "Invalid event name : "s + event->Name);
                return false;
            }

            if(std::find(metadata.second.begin(), metadata.second.end(), event->Name) != metadata.second.end())
            {
                // This is kind of silly way of validating script function signature, but it works, OK?
                std::string            expectedDeclaration = event->GetDeclaration(function);
                as::asIScriptFunction* sameFunction        = module->GetFunctionByDecl(expectedDeclaration.c_str());
                if(function != sameFunction)
                {
                    WriteError(engine, "Invalid function signature for event : " + event->Name + "\nExpected:\n  " + expectedDeclaration + ";\nFound:\n  " + function->GetDeclaration(true, true, false) + ";", module->GetName());
                    result = false;
                    continue;
                }

                event->Register(function);
            }
        }
    }

    return result;
}

//

std::string EWAN::Script::GetContextFunctionDetails(as::asIScriptContext* context, as::asUINT stackLevel /*= 0 */)
{
    int         line, column;
    const char* sectionName;

    line                            = context->GetLineNumber(stackLevel, &column, &sectionName);
    as::asIScriptFunction* function = context->GetFunction(stackLevel);

    std::string details;
    details += " "s + function->GetModuleName();
    details += " "s + sectionName;
    details += " "s + function->GetDeclaration(true, true, false);
    details += " "s + std::to_string(line) + "," + std::to_string(column);

    return details.substr(1);
}

//

std::string EWAN::Script::CurrentEventName_ScriptCall()
{
    as::asIScriptContext* context = as::asGetActiveContext();
    if(!context)
        return {};

    return UserData::Get(context)->Event->Name;
}

void EWAN::Script::Yield_ScriptCall()
{
    as::asIScriptContext* context = as::asGetActiveContext();
    if(!context)
        return;

    UserData::Get(context)->SuspendReason = SuspendReason::Yield;
    context->Suspend();
}

//

bool EWAN::Script::BindImportedFunctions(as::asIScriptEngine* engine)
{
    for(as::asUINT m = 0, mLen = engine->GetModuleCount(); m < mLen; m++)
    {
        if(!BindImportedFunctions(engine->GetModuleByIndex(m)))
            return false;
    }

    return true;
}

bool EWAN::Script::BindImportedFunctions(as::asIScriptModule* module)
{
    as::asIScriptEngine* engine = module->GetEngine();

    bool result = true;
    for(as::asUINT f = 0, fLen = module->GetImportedFunctionCount(); f < fLen; f++)
    {
        const char* importModuleName          = module->GetImportedFunctionSourceModule(f);
        const char* importFunctionDeclaration = module->GetImportedFunctionDeclaration(f);
        std::string importString              = "import "s + importFunctionDeclaration + " from \"" + importModuleName + "\"";

        as::asIScriptModule* importModule = engine->GetModule(importModuleName, as::asGM_ONLY_IF_EXISTS);
        if(!importModule)
        {
            WriteError(engine, "Cannot import function (source module does not exists) : "s + importString, module->GetName());
            result = false;
            continue;
        }

        if(UserData::Get(importModule)->Poison)
        {
            WriteError(engine, "Cannot import function (source module is poisoned) : "s + importString, module->GetName());
            result = false;
            continue;
        }

        as::asIScriptFunction* importFunction = importModule->GetFunctionByDecl(importFunctionDeclaration);
        if(!importFunction)
        {
            WriteError(engine, "Cannot import function (source function does not exists) : "s + importString, module->GetName());
            result = false;
            continue;
        }

        if(module->BindImportedFunction(f, importFunction) < 0)
        {
            WriteError(engine, "Cannot import function : "s + importString, module->GetName());
            result = false;
            continue;
        }
    }

    return result;
}

as::asIScriptEngine* EWAN::Script::CreateEngine()
{
    as::asIScriptEngine* engine = as::asCreateScriptEngine();
    if(!engine)
        return nullptr;

    int r;

    if(!API::InitEngineCallback(this, engine))
    {
        WriteError(engine, "Cannot set script engine message callback");
        DestroyEngine(engine);

        return nullptr;
    }

    static const std::unordered_map<as::asEEngineProp, as::asPWORD> properties = {
        {as::asEP_COMPILER_WARNINGS, 2}, // -Werror for scripts, woohoo!
        {as::asEP_DISALLOW_EMPTY_LIST_ELEMENTS, true},
        {as::asEP_OPTIMIZE_BYTECODE, true},
        {as::asEP_REQUIRE_ENUM_SCOPE, true}
        //
    };

    for(const auto& property : properties)
    {
        r = engine->SetEngineProperty(property.first, property.second);
        if(r < 0)
        {
            WriteError(engine, "Cannot set script engine property : " + std::to_string(property.first) + " = " + std::to_string(property.second));
            DestroyEngine(engine);

            return nullptr;
        }
    }

    engine->SetContextCallbacks(Callback::ContextRequest, Callback::ContextReturn, nullptr);
    engine->SetContextUserDataCleanupCallback(Callback::ContextUserDataCleanup);
    engine->SetEngineUserDataCleanupCallback(Callback::EngineUserDataCleanup);
    engine->SetFunctionUserDataCleanupCallback(Callback::FunctionUserDataCleanup);
    engine->SetModuleUserDataCleanupCallback(Callback::ModuleUserDataCleanup);

    engine->SetUserData(new UserData::Engine, UserData::IDX);
    UserData::Get(engine)->Script = this;

    return engine;
}

void EWAN::Script::DestroyEngine(as::asIScriptEngine*& engine)
{
    // Cache all modules first so they can be safely unloaded
    std::vector<as::asIScriptModule*> allModules;
    for(as::asUINT m = 0, mLen = engine->GetModuleCount(); m < mLen; m++)
    {
        allModules.push_back(engine->GetModuleByIndex(m));
    }

    for(auto& module : allModules)
    {
        UnloadModule(module);
    }
    allModules.clear();

    // Context cache must be cleared manually before shutting down script engine
    // Without that user data cleanup callback won't be called, thanks to reference counting
    UserData::Engine* engineData = UserData::Get(engine);
    if(!engineData->ContextCache.empty())
    {
        for(auto& context : engineData->ContextCache)
        {
            context->Release();
        }
        engineData->ContextCache.clear();
    }

    engine->ShutDownAndRelease();
    engine = nullptr;
}

//

void EWAN::Script::CallbackContextLine([[maybe_unused]] as::asIScriptContext* context)
{
    // Need explicit user data check here, as callback is also used by internal functions which doesn't have user data set
    UserData::Function* functionData = UserData::Get(context->GetFunction());

    if(functionData && functionData->Debug)
    {
#if 0
        std::string text = "?";
        as::asIScriptFunction* systemFunction = context->GetSystemFunction();
        if(systemFunction)
            text = "! "s + systemFunction->GetDeclaration(true, true, true);

        WriteInfo(context->GetEngine(), text, GetContextFunctionDetails(context));
#endif
    }
}

as::asIScriptContext* EWAN::Script::CallbackContextRequest(as::asIScriptEngine* engine, [[maybe_unused]] void* data)
{
    as::asIScriptContext* context    = nullptr;
    UserData::Engine*     engineData = UserData::Get(engine);

    if(engineData->ContextCache.empty())
    {
        // WriteInfo(engine, "Creating script context");

        context = engine->CreateContext();

        if(!API::InitContextCallback(this, context))
        {
            WriteError(engine, "Cannot set script context line callback");
            context->Release();
            return nullptr; // crash
        }

        context->SetUserData(new UserData::Context, UserData::IDX);
    }
    else
    {
        context = engineData->ContextCache.front();
        engineData->ContextCache.pop_front();
    }

    return context;
}

void EWAN::Script::CallbackContextReturn([[maybe_unused]] as::asIScriptEngine* engine, as::asIScriptContext* context, [[maybe_unused]] void* data)
{
    UserData::Engine*  engineData  = UserData::Get(engine);
    UserData::Context* contextData = UserData::Get(context);

    contextData->Reset();
    context->Unprepare();

    engineData->ContextCache.push_back(context);

    // WriteInfo(engine, "Caching script context : " + std::to_string(engineData->ContextCache.size()) + " total");
}

int EWAN::Script::CallbackInclude(Builder& builder, const std::string& include, const std::string& fromSection, [[maybe_unused]] void* data)
{
    // /Path/To/Scripts/
    std::filesystem::path fileName = std::filesystem::path(RootDirectory);

    // /Path/To/Scripts/SubDirectory/File.Main
    fileName += std::filesystem::path(fromSection);

    // /Path/To/Scripts/SubDirectory/
    fileName.remove_filename();

    // /Path/To/Scripts/SubDirectory/../File.Included
    fileName += std::filesystem::path(include);

    // /Path/To/Scripts/File.Included
    fileName = std::filesystem::path(builder.NormalizePath(fileName.string()));

    std::string sectionName = Text::Replace(std::filesystem::relative(fileName, RootDirectory).make_preferred().string(), "\\", "/");

    std::string fileContent;
    if(!Utils::ReadFile(fileName.make_preferred().string(), fileContent))
        return -1;

    if(builder.AddSectionFromMemory(sectionName.c_str(), fileContent.c_str()) < 0)
        return -1;

    return 0;
}

void EWAN::Script::CallbackMessage(const as::asSMessageInfo& msg)
{
    static const std::function<void(const std::string&)> functions[3] = {&Log::PrintError, &Log::PrintWarning, &Log::PrintInfo};
    std::function<void(const std::string&)>              function     = functions[msg.type];

    std::string log, section = msg.section;
    bool        numbers = msg.row > 0 || msg.col > 0;

    if(!section.empty() || numbers)
    {
        log += "[";

        if(!section.empty())
            log += section + (numbers ? " " : "");

        if(numbers)
            log += std::to_string(msg.row) + "," + std::to_string(msg.col);

        log += "]";
    }

    for(const auto& line : Text::Split(std::string(msg.message), '\n', false))
    {
        if(log.empty())
            function(line);
        else
            function(log + " " + line);
    }
}

int EWAN::Script::CallbackPragma(Builder& builder, const std::string& pragmaText, void* data)
{
    std::string pragma = pragmaText;

    pragma = Text::Replace(pragma, "\r", "");
    pragma = Text::Replace(pragma, "\n", "");
    pragma = Text::Replace(pragma, "\t", "    ");
    pragma = Text::Trim(pragma);

    if(pragma.empty())
        return 0;

    as::asIScriptEngine* engine = builder.GetEngine();
    as::asIScriptModule* module = builder.GetModule();

    const std::string pragmaString = pragma;

    std::vector<std::string> pragmargs = Text::Split(pragma, ' ');
    pragma                             = pragmargs.front();
    pragmargs.erase(pragmargs.begin());

    if(pragma == "module")
    {
        pragma = pragmargs.front();
        pragmargs.erase(pragmargs.begin());

        if(pragma == "debug" && pragmargs.empty())
        {
            UserData::Module* moduleData = UserData::Get(module);
            if(!moduleData->Debug)
            {
                // Pragmas cannot work on functions pointers, as module isn't built yet
                // This will be applied by LoadModuleMetadata()
                moduleData->Debug = true;
                WriteInfo(engine, "Module setup : debug", module->GetName());
            }
        }
        else if(pragma == "optional" && pragmargs.empty())
        {
            UserData::Module* moduleData = UserData::Get(module);
            if(!moduleData->Optional)
            {
                moduleData->Optional = true;
                WriteInfo(engine, "Module setup : optional", module->GetName());
            }
        }
        else if(pragma == "poison" && pragmargs.empty())
        {
            UserData::Module* moduleData = UserData::Get(module);
            if(!moduleData->Poison)
            {
                moduleData->Poison = true;
                WriteInfo(engine, "Module setup : poisoned", module->GetName());
            }
        }
        else if(pragma == "rename" && pragmargs.size() == 1)
        {
            if(engine->GetModule(pragmargs.front().c_str(), as::asGM_ONLY_IF_EXISTS) != nullptr)
            {
                WriteError(engine, "Module name already in use : " + pragmargs.front());
                return -1;
            }

            WriteInfo(engine, "Rename module : "s + pragmargs.front(), module->GetName());
            module->SetName(pragmargs.front().c_str());
        }
        else if(pragma == "unload" && pragmargs.empty())
        {
            UserData::Module* moduleData = UserData::Get(module);
            if(!moduleData->Unload)
            {
                moduleData->Unload = true;
                WriteInfo(engine, "Module setup : unload", module->GetName());
                if(CallbackPragma(builder, "module poison", data) < 0)
                    return -1;
            }
        }
        else
        {
            // Mimic as::CScriptBuilder error with little more details
            WriteError(engine, "Invalid #pragma directive : #pragma " + pragmaString);
            return -1;
        }
    }
    else
    {
        WriteError(engine, "Unknown #pragma directive : #pragma " + pragmaString);
        return -1;
    }

    return 0;
}

namespace
{
    [[maybe_unused]] static void StaticAssert()
    {
        static_assert(EWAN::Script::UserData::IDX < 1000 || EWAN::Script::UserData::IDX > 1999);
    }
}
