#include "Script.hpp"

#include "Log.hpp"

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

// Send event back to Script
as::asIScriptContext* EWAN::Script::Callback::ContextRequest(as::asIScriptEngine* engine, void* data)
{
    return GetScript(engine)->CallbackContextRequest(engine, data);
}

// Send event back to Script
void EWAN::Script::Callback::ContextReturn(as::asIScriptEngine* engine, as::asIScriptContext* context, void* data)
{
    GetScript(engine)->CallbackContextReturn(engine, context, data);
}

void EWAN::Script::Callback::ContextUserDataCleanup(as::asIScriptContext* context)
{
    UserData::Context* contextData = static_cast<UserData::Context*>(context->SetUserData(nullptr, 0));
    delete contextData;
}

void EWAN::Script::Callback::EngineUserDataCleanup(as::asIScriptEngine* engine)
{
    UserData::Engine* engineData = static_cast<UserData::Engine*>(engine->SetUserData(nullptr, 0));
    delete engineData;
}

void EWAN::Script::Callback::FunctionUserDataCleanup(as::asIScriptFunction* function)
{
    UserData::Function* functionData = static_cast<UserData::Function*>(function->SetUserData(nullptr, 0));
    delete functionData;
}

void EWAN::Script::Callback::ModuleUserDataCleanup(as::asIScriptModule* module)
{
    UserData::Module* moduleData = static_cast<UserData::Module*>(module->SetUserData(nullptr, 0));
    delete moduleData;
}
