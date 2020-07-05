#include "Log.hpp"
#include "Script.hpp"

using namespace std::literals::string_literals;

static inline EWAN::Script* GetScript(as::asIScriptEngine* engine)
{
    return static_cast<EWAN::Script::UserData::Engine*>(engine->GetUserData(0))->Script;
}

//
// Script::Calback
//

// Send event back to Script, with a twist
int EWAN::Script::Callback::Include(const char* include, const char* from, as::CScriptBuilder* builder, void* data)
{
    EWAN::Script::Builder& scriptBuilder = static_cast<EWAN::Script::Builder&>(*builder);
    return GetScript(builder->GetEngine())->CallbackInclude(scriptBuilder, std::string(include), std::string(from), data);
}

// Send event back to Script, with a twist
int EWAN::Script::Callback::Pragma(const std::string& pragmaText, as::CScriptBuilder& builder, void* data)
{
    EWAN::Script::Builder& scriptBuilder = static_cast<EWAN::Script::Builder&>(builder);
    return GetScript(builder.GetEngine())->CallbackPragma(scriptBuilder, pragmaText, data);
}

void EWAN::Script::Callback::ContextUserDataCleanup(as::asIScriptContext* context)
{
    EWAN::Script::UserData::Context* contextData = static_cast<EWAN::Script::UserData::Context*>(context->SetUserData(nullptr, 0));
    delete contextData;
}

void EWAN::Script::Callback::EngineUserDataCleanup(as::asIScriptEngine* engine)
{
    EWAN::Script::UserData::Engine* engineData = static_cast<EWAN::Script::UserData::Engine*>(engine->SetUserData(nullptr, 0));
    delete engineData;
}

void EWAN::Script::Callback::FunctionUserDataCleanup(as::asIScriptFunction* function)
{
    EWAN::Script::UserData::Function* functionData = static_cast<EWAN::Script::UserData::Function*>(function->SetUserData(nullptr, 0));

    delete functionData;
}

void EWAN::Script::Callback::ModuleUserDataCleanup(as::asIScriptModule* module)
{
    EWAN::Script::UserData::Module* moduleData = static_cast<EWAN::Script::UserData::Module*>(module->SetUserData(nullptr, 0));

    as::asIScriptEngine* engine = module->GetEngine();

    if(moduleData->Debug)
        GetScript(engine)->WriteInfo(engine, "Module destroyed", module->GetName());

    delete moduleData;
}
