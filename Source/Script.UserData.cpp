#include "Script.hpp"

/* static */ EWAN::Script::UserData::Context* EWAN::Script::UserData::Get(as::asIScriptContext* context)
{
    return static_cast<Context*>(context->GetUserData(UserData::IDX));
}

/* static */ EWAN::Script::UserData::Engine* EWAN::Script::UserData::Get(as::asIScriptEngine* engine)
{
    return static_cast<Engine*>(engine->GetUserData(UserData::IDX));
}

/* static */ EWAN::Script::UserData::Function* EWAN::Script::UserData::Get(as::asIScriptFunction* function)
{
    return static_cast<Function*>(function->GetUserData(UserData::IDX));
}

/* static */ EWAN::Script::UserData::Module* EWAN::Script::UserData::Get(as::asIScriptModule* module)
{
    return static_cast<Module*>(module->GetUserData(UserData::IDX));
}
