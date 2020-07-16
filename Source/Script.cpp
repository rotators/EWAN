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
    // Event constructor                                  Script code
    // ------------------------------------------------------------------------------------
    // Event("OnExample", {"void"}) ..................... [OnExample] void  f();
    // Event("OnHappens", {"bool"}) ..................... [OnHappens] bool  f();
    // Event("OnTrigger", {"float", "string", "int"}) ... [OnTrigger] float f(string, int);
    //
    // Note that while single function can handle any amount of engine events (as long their signatures are compatibile),
    // it's currently not possible to detect which event called script function.
    //

    OnBuild("OnBuild", {"void"}),
    OnInit("OnInit", {"bool"}),
    OnFinish("OnFinish", {"void"}),
    OnDraw("OnDraw", {"void"}),

    AS(nullptr)
{}

EWAN::Script::~Script()
{
    Finish();
}

bool EWAN::Script::Init(App* app)
{
    RootDirectory = std::filesystem::path(app->GameInfo.Path).replace_filename(app->GameInfo.ScriptInit).remove_filename().make_preferred().string();

    Log::PrintInfo("Script root directory...  " + RootDirectory);
    Log::PrintInfo("Script initialization...  AngelScript v"s + as::asGetLibraryVersion() + as::asGetLibraryOptions());

    const std::string fail = "Script initialization failed... ";

    as::asIScriptEngine* engine = CreateEngine();
    if(!engine)
    {
        Log::PrintError(fail + "cannot create engine");
        return false;
    }

    if(!API::Init(app, engine))
    {
        WriteError(engine, fail + "cannot register API");
        DestroyEngine(engine);
        return false;
    }

    if(!LoadInitModule(app->GameInfo, engine))
    {
        WriteError(engine, fail + "cannot load init module");
        DestroyEngine(engine);
        return false;
    }

    /*/ At this point init module should request and finish building of any other modules /*/

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

    // There might be no modules left, if all modules are marked as optional
    if(!engine->GetModuleCount())
    {
        WriteError(engine, fail + "cannot find any modules (late check)");
        DestroyEngine(engine);
        return false;
    }

    if(engine->GarbageCollect(as::asGC_FULL_CYCLE) == 1)
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
        WriteError(engine, "Cannot load module (name already in use) : " + moduleName);
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

    // Set script section name as script filename relative to RootDirectory, with enforced *NIX path separators
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

    // All script functions must have UserData set
    for(as::asUINT f = 0, fLen = module->GetFunctionCount(); f < fLen; f++)
    {
        module->GetFunctionByIndex(f)->SetUserData(new UserData::Function, 0);
    }

    // [OnBuild] always clears Event.OnBuild
    if(!LoadModuleMetadata(builder) || !OnBuild.RunOnBuild(module))
    {
        WriteError(engine, fail, module->GetName());
        UnloadModule(module);

        return optional;
    }

    WriteInfo(engine, "Loading module complete", module->GetName());

    return true;
}

bool EWAN::Script::LoadModule_Call(const std::string& fileName, const std::string& moduleName)
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
    // Disallow unloading currently used module
    as::asIScriptContext* context = as::asGetActiveContext();
    if(context)
    {
        for(as::asUINT f = 0, fLen = context->GetCallstackSize(); f < fLen; f++)
        {
            if(context->GetFunction(f)->GetModule() == module)
            {
                WriteError(context->GetEngine(), "Cannot unload module (currently in use) : "s + module->GetName());
                return false;
            }
        }
    }

    // Cache data used for post-discard message
    as::asIScriptEngine* engine     = module->GetEngine();
    const std::string    moduleName = module->GetName();

    WriteInfo(engine, "Unloading module...", moduleName);

    OnBuild.Unregister(module);
    OnInit.Unregister(module);
    OnFinish.Unregister(module);

    OnDraw.Unregister(module);

    module->UnbindAllImportedFunctions();
    module->Discard();

    module = nullptr;

    WriteInfo(engine, "Unloading module complete", moduleName);

    return true;
}

bool EWAN::Script::UnloadModule_Call(const std::string& moduleName)
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
    //
    // Event name defines what kind of metadata script function need to have to become event callback
    // Function declaration is stored as string list, which holds return type and function parameters
    // Container is a reference to list of AngelScript functions, holding results of metadata parsing
    //
    // {event}, {{function, declaration, list}, container}
    //
    // Event definition                                               Script code
    // ------------------------------------------------------------------------------------------------
    // {"OnExample", {{"void"}, Event.Example}} ..................... [OnExample] void  f();
    // {"OnHappens", {{"bool"}, Event.Happens}} ..................... [OnHappens] bool  f();
    // {"OnTrigger", {{"float", "string", "int"}, Event.Trigger}} ... [OnTrigger] float f(string, int);
    //
    // Note that while single function can handle any amount of engine events (as long their signatures are compatibile),
    // it's currently not possible to detect which event called script function.
    //

    static const std::vector<Event*> eventData = {
        &OnBuild,
        &OnInit,
        &OnFinish,
        &OnDraw //
    };

    as::asIScriptEngine* engine = builder.GetEngine();

    // Process only functions with metadata
    for(const auto& metadata : builder.GetAllMetadataForFunc())
    {
        // Check metadata against engine events list
        for(const auto& event : eventData)
        {
            // Unknown metadata is silently ignored
            if(std::find(metadata.second.begin(), metadata.second.end(), event->Name) != metadata.second.end())
            {
                // This is kind of silly way of validating script function signature, but it works, OK?
                as::asIScriptFunction* function            = engine->GetFunctionById(metadata.first);
                std::string            expectedDeclaration = event->GetDeclaration(function);
                as::asIScriptFunction* sameFunction        = function->GetModule()->GetFunctionByDecl(expectedDeclaration.c_str());
                if(function != sameFunction)
                {
                    WriteError(engine, "Invalid function signature for engine event : " + event->Name + "\nExpected:\n  " + expectedDeclaration + ";\nFound:\n  " + function->GetDeclaration(true, true, false) + ";", function->GetModuleName());
                    return false;
                }

                event->Register(function);
            }
        }
    }

    return true;
}

//

void EWAN::Script::Yield_Call()
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
        {as::asEP_DISALLOW_GLOBAL_VARS, true},
        {as::asEP_OPTIMIZE_BYTECODE, true},
        {as::asEP_REQUIRE_ENUM_SCOPE, true} //
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

    engine->SetUserData(new UserData::Engine, 0);
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
    [[maybe_unused]] int         line, column;
    [[maybe_unused]] const char* sectionName;

    line = context->GetLineNumber(0, &column, &sectionName);

    // context->GetEngine()->WriteMessage(sectionName, line, column, as::asMSGTYPE_INFORMATION, context->GetFunction()->GetDeclaration(true, true, true));
}

as::asIScriptContext* EWAN::Script::CallbackContextRequest(as::asIScriptEngine* engine, [[maybe_unused]] void* data)
{
    as::asIScriptContext* context = nullptr;
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

        context->SetUserData(new UserData::Context, 0);
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
            log += section + (numbers ? ":" : "");

        if(numbers)
            log += std::to_string(msg.row) + ":" + std::to_string(msg.col);

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
