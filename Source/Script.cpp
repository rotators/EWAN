#include "App.hpp"
#include "Log.hpp"
#include "Script.hpp"
#include "Text.hpp"
#include "Utils.hpp"

#include <filesystem>
#include <unordered_map>

using namespace std::literals::string_literals;

//
// Script
//

EWAN::Script::Script() :
    AS(nullptr)
{}

EWAN::Script::~Script()
{
    Finish();
}

bool EWAN::Script::Init(App* app)
{
    RootDirectory = std::filesystem::path(app->GameInfo.Path).replace_filename(app->GameInfo.ScriptInit).remove_filename().make_preferred().string();

    Log::Print("[INFO] Script initialization... AngelScript v"s + as::asGetLibraryVersion() + as::asGetLibraryOptions());
    Log::Print("[INFO] Script root directory... " + RootDirectory);

    as::asIScriptEngine* engine = CreateEngine();
    if(!engine)
        return false;

    if(!InitAPI(app, engine))
    {
        DestroyEngine(engine);
        return false;
    }

    if(!LoadInitModule(app->GameInfo, engine))
    {
        DestroyEngine(engine);
        return false;
    }

    /*/ At this point init module should request and finish building of any other modules /*/

    if(!BindImportedFunctions(engine))
    {
        WriteError(engine, "Script initialization failed... cannot bind all imported functions");
        DestroyEngine(engine);
        return false;
    }

    // Run OnInit event; always clears Event.OnInit
    // `falseFunction` points to first script function which returned false (if any)
    {
        as::asIScriptFunction* falseFunction = nullptr;
        if(!Event.RunOnInit(engine, falseFunction))
        {
            WriteError(engine, "Script initialization failed... "s + falseFunction->GetDeclaration(true, true, true) + " = false", falseFunction->GetModuleName());
            DestroyEngine(engine);
            return false;
        }
    }

    // Unload modules which requested it with `#pragma module unload`
    {
        std::vector<as::asIScriptModule*> allModules;
        for(as::asUINT m=0, mLen=engine->GetModuleCount(); m<mLen; m++)
        {
            allModules.push_back(engine->GetModuleByIndex(m));
        }

        for(auto& module : allModules)
        {
            if(GetUserData(module)->Unload)
            {
                UnloadModule(module);
                continue;
            }
        }
    }

    WriteInfo(engine, "Script initialization complete");
    AS = engine;

    return true;
}

void EWAN::Script::Finish()
{
    if(AS)
        DestroyEngine(AS);
}

//

/* static */ EWAN::Script::UserData::Context* EWAN::Script::GetUserData(as::asIScriptContext* context)
{
    return static_cast<UserData::Context*>(context->GetUserData(0));
}

/* static */EWAN::Script::UserData::Engine* EWAN::Script::GetUserData(as::asIScriptEngine* engine)
{
    return static_cast<UserData::Engine*>(engine->GetUserData(0));
}

/* static */EWAN::Script::UserData::Function* EWAN::Script::GetUserData(as::asIScriptFunction* function)
{
    return static_cast<UserData::Function*>(function->GetUserData(0));
}

/* static */EWAN::Script::UserData::Module* EWAN::Script::GetUserData(as::asIScriptModule* module)
{
    return static_cast<UserData::Module*>(module->GetUserData(0));
}

//

bool EWAN::Script::LoadModule(as::asIScriptEngine* engine, const std::string& fileName, const std::string& moduleName)
{
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
    WriteInfo(engine, "Loading module", moduleName);

    // Creates module and assigns UserData::Module
    int r = builder.StartNewModule(engine, moduleName.c_str());
    if(r < 0)
        return false;

    module = builder.GetModule();

    // Using AddSectionFromMemory() instead of AddSectionFromFile() for custom script sections names
    std::string fileContent;
    if(!Utils::ReadFile(fileName, fileContent))
        return false;

    // Set script section name as script filename relative to RootDirectory, with enforced *NIX path separators
    std::filesystem::path sectionName = std::filesystem::relative(fileName, RootDirectory);

    r = builder.AddSectionFromMemory(sectionName.string().c_str(), fileContent.c_str());
    if(r < 0)
        return false;

    /*/ At this point all non-instant `#pragma module` directives are available as UserData::Module /*/

    // Optional modules does not inform caller about failure
    bool optional = GetUserData(module)->Optional;
    r = builder.BuildModule();
    if(r < 0)
        return optional;

    for(as::asUINT f=0, fLen=module->GetFunctionCount(); f<fLen; f++)
    {
        module->GetFunctionByIndex(f)->SetUserData(new UserData::Function, 0);
    }

    if(!LoadModuleMetadata(builder))
    {
        UnloadModule(module);
        return false;
    }

    // Run OnBuild event; always clears Event.OnBuild
    if(!Event.RunOnBuild(module))
    {
        UnloadModule(module);

        return optional;
    }

    return true;
}

bool EWAN::Script::LoadModule_Call(const std::string& fileName, const std::string& moduleName)
{
    as::asIScriptContext* context = as::asGetActiveContext();
    if(!context)
        return false;

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
        for(as::asUINT f = 0, fLen=context->GetCallstackSize(); f<fLen; f++)
        {
            if(context->GetFunction(f)->GetModule() == module)
            {
                WriteError(context->GetEngine(), "Cannot unload module (currently in use) : "s + module->GetName());
                return false;
            }
        }
    }

    WriteInfo(module->GetEngine(), "Unloading module", module->GetName());

    Event.Unregister(module);
 
    module->UnbindAllImportedFunctions();
    module->Discard();
    module = nullptr;

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
    // {"OnExample", {{"void"}, Event.Example}} ..................... [OnExample] void f();
    // {"OnHappens", {{"bool"}, Event.Happens}} ..................... [OnHappens] bool f();
    // {"OnTrigger", {{"float", "string", "int"}, Event.Trigger}} ... [OnTrigger] float f(string, int);
    //
    // Note that while single function can handle any amount of engine events (as long their signatures are compatibile),
    // it's currently not possible to detect which event called script function.
    //

    static const std::unordered_map<std::string, Event::Data> eventData = {
        {"OnBuild", {{"void"}, Event.OnBuild}},
        {"OnInit", {{"bool"}, Event.OnInit}},

        {"OnDraw", {{"void"}, Event.OnDraw}}
    };

    as::asIScriptEngine* engine = builder.GetEngine();

    // Process only functions with metadata
    for(const auto& metadata : builder.GetAllMetadataForFunc())
    {
        // Check metadata against engine events list
        for(const auto& event : eventData)
        {
            // Unknown metadata is silently ignored
            if(std::find(metadata.second.begin(), metadata.second.end(), event.first) != metadata.second.end())
            {
                // This is kind of silly way of validating script function signature, but it works, OK?
                as::asIScriptFunction* function = engine->GetFunctionById(metadata.first);
                std::string expectedDeclaration = event.second.GetDeclaration(function);
                as::asIScriptFunction* sameFunction = function->GetModule()->GetFunctionByDecl(expectedDeclaration.c_str());
                if(function != sameFunction)
                {
                    WriteError(engine, "Invalid function signature for engine event : " + event.first +"\nExpected:\n  " + expectedDeclaration + ";\nFound:\n  " + function->GetDeclaration(true, true, false) + ";");
                    return false;
                }

                Event.Register(event.second.List, function, event.first);
            }
        }
    }

    return true;
}

//

bool EWAN::Script::BindImportedFunctions(as::asIScriptEngine* engine)
{
    for(as::asUINT m=0, mLen = engine->GetModuleCount(); m<mLen; m++)
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
    for(as::asUINT f=0, fLen=module->GetImportedFunctionCount(); f<fLen; f++)
    {
        const char* importModuleName = module->GetImportedFunctionSourceModule(f);
        const char* importFunctionDeclaration = module->GetImportedFunctionDeclaration(f);
        std::string importString = "import "s + importFunctionDeclaration + " from \"" + importModuleName + "\"";

        as::asIScriptModule* importModule = engine->GetModule(importModuleName, as::asGM_ONLY_IF_EXISTS);
        if(!importModule)
        {
            WriteError(engine, "Cannot import function (source module does not exists) : "s + importString, module->GetName());
            result = false;
            continue;
        }

        if(GetUserData(importModule)->Poison)
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

        if( module->BindImportedFunction(f, importFunction) < 0 )
        {
            WriteError(engine, "Cannot import function : "s + importString, module->GetName());
            result = false;
            continue;
        }
    }

    return result;
}

as::asIScriptContext* EWAN::Script::CreateContext(as::asIScriptEngine*)
{
    Log::Raw("");

    return nullptr;
}

as::asIScriptEngine* EWAN::Script::CreateEngine()
{
    Log::Print("[INFO] Creating script engine...");

    as::asIScriptEngine* engine = as::asCreateScriptEngine();
    if(!engine)
    {
        Log::Print("[ERROR] Cannot create script engine");

        return nullptr;
    }

    if(!InitMessageCallback(engine))
    {
        WriteError(engine, "Cannot set script engine message callback");
        DestroyEngine(engine);

        return nullptr;
    }

    static const std::unordered_map<as::asEEngineProp,as::asPWORD> properties = {
        { as::asEP_COMPILER_WARNINGS, 2 }, // -Werror for scripts, woohoo!
        { as::asEP_DISALLOW_EMPTY_LIST_ELEMENTS, true },
        { as::asEP_DISALLOW_GLOBAL_VARS, true },
        { as::asEP_OPTIMIZE_BYTECODE, true },
        { as::asEP_REQUIRE_ENUM_SCOPE, true }
    };

    int r = 0;

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

    engine->SetContextUserDataCleanupCallback(Callback::ContextUserDataCleanup);
    engine->SetEngineUserDataCleanupCallback(Callback::EngineUserDataCleanup);
    engine->SetFunctionUserDataCleanupCallback(Callback::FunctionUserDataCleanup);
    engine->SetModuleUserDataCleanupCallback(Callback::ModuleUserDataCleanup);

    engine->SetUserData(new UserData::Engine, 0);
    GetUserData(engine)->Script = this;

    return engine;
}

void EWAN::Script::DestroyEngine(as::asIScriptEngine*& engine)
{
    WriteInfo(engine, "Destroying script engine...");

    for(as::asUINT m=0, mLen=engine->GetModuleCount(); m<mLen; m++)
    {
        Event.Unregister(engine->GetModuleByIndex(m));
    }

    engine->ShutDownAndRelease();
    engine = nullptr;

    Log::Print("[INFO] Destroying script engine complete");
}

//

void EWAN::Script::WriteInfo(as::asIScriptEngine* engine, const std::string& message, const std::string& section /*= {} */, int row /*= 0 */, int col /*= 0 */)
{
    engine->WriteMessage(section.c_str(), row, col, as::asMSGTYPE_INFORMATION, message.c_str());
}

void EWAN::Script::WriteWarning(as::asIScriptEngine* engine, const std::string& message, const std::string& section /*= {} */, int row /*= 0 */, int col /*= 0 */)
{
    engine->WriteMessage(section.c_str(), row, col, as::asMSGTYPE_WARNING, message.c_str());
}

void EWAN::Script::WriteError(as::asIScriptEngine* engine, const std::string& message, const std::string& section /*= {} */, int row /*= 0 */, int col /*= 0 */)
{
    engine->WriteMessage(section.c_str(), row, col, as::asMSGTYPE_ERROR, message.c_str());
}

//

int EWAN::Script::CallbackInclude(Builder& builder, const std::string& include, const std::string& fromSection, [[maybe_unused]] void* data)
{
    // Path/To/Scripts/
    std::filesystem::path fileName = std::filesystem::path(RootDirectory);

    // Path/To/Scripts/SubDirectory/File.Name
    fileName += std::filesystem::path(fromSection);

    // Path/To/Scripts/SubDirectory/
    fileName.remove_filename();

    // Path/To/Scripts/SubDirectory/../File.Included
    fileName += std::filesystem::path(include);

    // Path/To/Script/File.Included
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
    static const std::string type[3] = { "ERROR", "WARNING", "INFO" };
    std::string log = "[" + type[msg.type] + "]", section = msg.section, message = msg.message;
    bool numbers = msg.row > 0 || msg.col > 0;

    if(!section.empty() || numbers)
    {
        log += "[";

        if(!section.empty())
            log += section + (numbers ? ":" : "");

        if(numbers)
            log += std::to_string(msg.row) + ":" + std::to_string(msg.col);

        log += "]";
    }

    if(!message.empty())
        log += " " +message;

    Log::Print(log);
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
    pragma = pragmargs.front();
    pragmargs.erase(pragmargs.begin());

    if(pragma == "module")
    {
        pragma = pragmargs.front();
        pragmargs.erase(pragmargs.begin());
    
        if(pragma == "debug" && pragmargs.empty())
        {
            UserData::Module* moduleData = GetUserData(module);
            if(!moduleData->Debug)
            {
                moduleData->Debug = true;
                WriteInfo(engine, "Module option : debug", module->GetName());
            }
        }
        else if(pragma == "name" && pragmargs.size() == 1)
        {
            if(engine->GetModule(pragmargs.front().c_str(), as::asGM_ONLY_IF_EXISTS) != nullptr)
            {
                WriteError(engine, "Module name already in use : " + pragmargs.front());
                return -1;
            }

            WriteInfo(engine, "Rename module : "s + pragmargs.front(), module->GetName());
            module->SetName(pragmargs.front().c_str());
        }
        else if(pragma == "optional" && pragmargs.empty())
        {
            UserData::Module* moduleData = GetUserData(module);
            if(!moduleData->Optional)
            {
                moduleData->Optional = true;
                WriteInfo(engine, "Module option : optional", module->GetName());
            }
        }
        else if(pragma == "poison" && pragmargs.empty())
        {
            UserData::Module* moduleData = GetUserData(module);
            if(!moduleData->Poison)
            {
                moduleData->Poison = true;
                WriteInfo(engine, "Module option : poisoned", module->GetName());
            }
        }
        else if(pragma == "unload" && pragmargs.empty())
        {
            UserData::Module* moduleData = GetUserData(module);
            if(!moduleData->Unload)
            {
                moduleData->Unload = true;
                WriteInfo(engine, "Module option : unloading", module->GetName());
                if(CallbackPragma(builder, "module poison", data) < 0)
                    return -1;
            }
        }
        else
        {
            // Mimic scriptbuilder error with little more details
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
