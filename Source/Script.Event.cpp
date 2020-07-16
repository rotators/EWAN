#include "Script.hpp"

#include "Log.hpp"
#include "Text.hpp"

using namespace std::literals::string_literals;

//
// Script::Event
//

std::function<void(as::asIScriptContext* context)> EWAN::Script::Event::NOP = [](as::asIScriptContext*) {
};

EWAN::Script::Event::Event(std::string name, std::list<std::string> params) :
    Name(name),
    Params(params),
    Functions()
{}

EWAN::Script::Event::~Event()
{
    if(!Functions.empty())
        Log::PrintWarning("Script event not cleared : " + Name);
}

std::string EWAN::Script::Event::GetDeclaration(const std::string& name /*= "f" */) const
{
    std::list<std::string> params = Params;

    std::string declaration = params.front() + " " + name;

    declaration += "(";

    params.pop_front();
    declaration += Text::Join(params, ", ");

    declaration += ")";

    return declaration;
}

std::string EWAN::Script::Event::GetDeclaration(as::asIScriptFunction* function) const
{
    std::string name, ns = function->GetNamespace();

    if(!ns.empty())
    {
        name += ns;
        name += "::";
    }

    name += function->GetName();

    return GetDeclaration(name);
}

//
// Script::Event
//

/*
void EWAN::Script::Event::Run(const std::list<as::asIScriptFunction*>& functions)
{
    if(functions.empty())
        return;

    Log::Raw("RequestContext");
    as::asIScriptContext* ctx = functions.front()->GetEngine()->RequestContext();

    for(const auto& function : functions )
    {
        Log::Raw("Prepare");
        ctx->Prepare(function);

        Run(ctx);
    }

    ctx->GetEngine()->ReturnContext(ctx);
    Log::Raw("ReturnContext");
}
*/

void EWAN::Script::Event::Register(as::asIScriptFunction* function)
{
    // TODO Script::Event::Register() search for duplicates

    if(UserData::Get(function->GetModule())->Debug)
        WriteInfo(function->GetEngine(), "Registered event callback : "s + function->GetDeclaration(true, true, true) + " = " + Name + ";", function->GetModuleName());

    Functions.push_back(function);
}

/*
void EWAN::Script::Event::Unregister(as::asIScriptModule* module)
{
    Unregister(OnBuild, module, "OnBuild");
    Unregister(OnInit, module, "OnInit");
    Unregister(OnFinish, module, "OnFinish");

    Unregister(OnDraw, module, "OnDraw");
    Unregister(OnKeyDown, module, "OnKeyDown");
    Unregister(OnKeyUp, module, "OnKeyUp");
    Unregister(OnMouseMove, module, "OnMouseMove");
    Unregister(OnMouseDown, module, "OnMouseDown");
    Unregister(OnMouseUp, module, "OnMouseUp");
}
*/

void EWAN::Script::Event::Unregister(as::asIScriptEngine* engine)
{
    for(as::asUINT m = 0, mLen = engine->GetModuleCount(); m < mLen; m++)
    {
        Unregister(engine->GetModuleByIndex(m));
    }
}

void EWAN::Script::Event::Unregister(as::asIScriptModule* module)
{
    const bool debugModule = UserData::Get(module)->Debug;

    Functions.remove_if([this, debugModule, module](as::asIScriptFunction* function) -> bool {
        if(function->GetModule() == module)
        {
            if(debugModule)
                WriteInfo(module->GetEngine(), "Unregistered event callback : "s + function->GetDeclaration(true, true, true) + " = " + Name + ";", function->GetModuleName());

            return true;
        }

        return false;
    });
}

bool EWAN::Script::Event::Run()
{
    if(Functions.empty())
        return true;

    return Run(NOP, NOP);
}

bool EWAN::Script::Event::Run(float& arg0)
{
    if(Functions.empty())
        return true;

    auto init = [arg0](as::asIScriptContext* context) {
        context->SetArgFloat(0, arg0);
    };

    return Run(init, NOP);
}

bool EWAN::Script::Event::RunBool(bool& result)
{
    if(Functions.empty())
        return true;

    auto finish = [&result](as::asIScriptContext* context) {
        result = context->GetReturnByte();
    };

    return Run(NOP, finish);
}

bool EWAN::Script::Event::Run(std::function<void(as::asIScriptContext* context)> init, std::function<void(as::asIScriptContext* context)> finish)
{
    if(Functions.empty())
        return true;

    as::asIScriptContext* context = nullptr;

    std::list<as::asIScriptContext*> yield;

    for(const auto& function : Functions)
    {
        if(!context)
        {
            context                       = function->GetEngine()->RequestContext();
            UserData::Get(context)->Event = this;
        }

        int r = context->Prepare(function);
        if(r != 0)
        {
            context->Release();
            return false;
        }

        const bool debug = UserData::Get(function->GetModule())->Debug;

        if(debug)
            WriteInfo(context->GetEngine(), "Run script event : "s + function->GetDeclaration(true, true, true) + " = " + Name + ";", function->GetModuleName());

        init(context);
        Execute(context, yield, finish);
    }

    if(context)
    {
        context->GetEngine()->ReturnContext(context);
        context = nullptr;
    }

    while(!yield.empty())
    {
        context = yield.front();
        yield.pop_front();

        as::asIScriptModule* module = context->GetFunction()->GetModule();
        if(UserData::Get(module)->Debug)
            WriteInfo(module->GetEngine(), "Resume script event : "s + context->GetFunction()->GetDeclaration(true, true, true) + " = " + Name + ";", module->GetName());

        Execute(context, yield, finish);
        if(context)
            context->GetEngine()->ReturnContext(context);
    }

    return true;
}

bool EWAN::Script::Event::Execute(as::asIScriptContext*& context, std::list<as::asIScriptContext*>& yield, std::function<void(as::asIScriptContext* context)> finish)
{
    const int  r           = context->Execute();
    const bool debugModule = UserData::Get(context->GetFunction()->GetModule())->Debug;

    if(r == as::asEXECUTION_FINISHED)
    {
        finish(context);
        return true;
    }
    else if(r == as::asEXECUTION_SUSPENDED)
    {
        UserData::Context* contextData = UserData::Get(context);

        if(contextData->SuspendReason == SuspendReason::Yield)
        {
            if(debugModule)
                WriteInfo(context->GetEngine(), "Suspend script event : "s + context->GetFunction()->GetDeclaration(true, true, true) + " = " + Name + ";", context->GetFunction()->GetModuleName());

            contextData->SuspendReason = SuspendReason::Unknown;
            yield.push_back(context);
            context = nullptr;
        }
        else
        {
            WriteWarning(context->GetEngine(), "Unknown script event suspend reason : "s + context->GetFunction()->GetDeclaration(true, true, true) + " = " + Name + ";");
        }
    }
    else
    {
        WriteError(context->GetEngine(), "Cannot execute script event : "s + context->GetFunction()->GetDeclaration(true, true, true) + " = " + Name + ";");
        WriteError(context->GetEngine(), "Cannot execute script event : Execute() = "s + std::to_string(r));
        context->Release();
        context = nullptr;
        return false;
    }

    return true;
}

bool EWAN::Script::Event::RunOnBuild(as::asIScriptModule* module)
{
    if(Functions.empty())
        return true;

    // Make sure event runs only once per module
    Event self = *this;
    Functions.clear();
    bool result = self.Run();

    // That big boy over here exists only to provide feedback in log
    self.Unregister(module);

    return result;
}

bool EWAN::Script::Event::RunOnInit(as::asIScriptEngine* engine, as::asIScriptFunction*& falseFunction)
{
    if(Functions.empty())
        return true;

    bool result = true;
    auto finish = [&falseFunction, &result](as::asIScriptContext* context) {
        if(!context->GetReturnByte())
        {
            falseFunction = context->GetFunction();
            result        = false;
        }
    };

    if(!Run(NOP, finish))
        result = false;

    Unregister(engine);

    return result;
}
