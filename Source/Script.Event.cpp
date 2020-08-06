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
        Log::PrintWarning("Event not cleared : " + Name);
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

void EWAN::Script::Event::Register(as::asIScriptFunction* function)
{
    if(UserData::Get(function)->Debug)
        WriteInfo(function->GetEngine(), "Registered event callback : "s + function->GetDeclaration(true, true, true) + " = " + Name + ";", function->GetModuleName());

    Functions.push_back(function);
}

void EWAN::Script::Event::Unregister(as::asIScriptEngine* engine)
{
    for(as::asUINT m = 0, mLen = engine->GetModuleCount(); m < mLen; m++)
    {
        Unregister(engine->GetModuleByIndex(m));
    }
}

void EWAN::Script::Event::Unregister(as::asIScriptModule* module)
{
    Functions.remove_if([this, module](as::asIScriptFunction* function) -> bool {
        if(function->GetModule() == module)
        {
            if(UserData::Get(function)->Debug)
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

bool EWAN::Script::Event::Run(const int32_t& arg0)
{
    if(Functions.empty())
        return true;

    auto init = [arg0](as::asIScriptContext* context) {
        context->SetArgDWord(0, arg0);
    };

    return Run(init, NOP);
}

bool EWAN::Script::Event::Run(const int32_t& arg0, const int32_t& arg1)
{
    if(Functions.empty())
        return true;

    auto init = [arg0, arg1](as::asIScriptContext* context) {
        context->SetArgDWord(0, arg0);
        context->SetArgDWord(1, arg1);
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

    // In perfect scenario, only one context is used for N functions
    as::asIScriptContext* context = nullptr;

    std::queue<as::asIScriptContext*> yield;

    // Primary run
    // Call functions which has been previously registered as event callbacks; if suspended, move them to secondary run block

    for(const auto& function : Functions)
    {
        if(!context)
        {
            // Request new context on first function and after yield

            context                       = function->GetEngine()->RequestContext();
            UserData::Get(context)->Event = this;
        }

        int r = context->Prepare(function);
        if(r != as::asSUCCESS)
        {
            WriteError(function->GetEngine(), "Cannot prepare context : "s + function->GetDeclaration(true, true, true) + " = " + std::to_string(r), function->GetModuleName());
            context->Release();
            return false;
        }

        if(UserData::Get(function)->Debug)
            WriteInfo(context->GetEngine(), "Run event : "s + function->GetDeclaration(true, true, true) + " = " + Name + ";", function->GetModuleName());

        init(context);
        Execute(context, yield, finish);
    }

    if(context)
    {
        context->GetEngine()->ReturnContext(context);
        context = nullptr;
    }

    // Secondary run
    // Call suspended functions (if any) in a loop, until there's none left; allows scripts to suspend functions indefinitely

    while(!yield.empty())
    {
        context = yield.front();
        yield.pop();

        as::asIScriptFunction* function = context->GetFunction();
        as::asIScriptModule*   module   = function->GetModule();

        if(UserData::Get(function)->Debug)
            WriteInfo(module->GetEngine(), "Resume event : "s + function->GetDeclaration(true, true, true) + " = " + Name + ";", module->GetName());

        Execute(context, yield, finish);
        if(context)
            context->GetEngine()->ReturnContext(context);
    }

    return true;
}

bool EWAN::Script::Event::Execute(as::asIScriptContext*& context, std::queue<as::asIScriptContext*>& yield, std::function<void(as::asIScriptContext* context)> finish)
{
    const int  r     = context->Execute();
    const bool debug = UserData::Get(context->GetFunction())->Debug;

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
            if(debug)
                WriteInfo(context->GetEngine(), "Suspend event : "s + context->GetFunction()->GetDeclaration(true, true, true) + " = " + Name + ";", context->GetFunction()->GetModuleName());

            // Move context to suspended functions container; as it holds function state it cannot be reused (obviously)
            contextData->SuspendReason = SuspendReason::Unknown;
            yield.push(context);
            context = nullptr;
        }
        else
        {
            WriteWarning(context->GetEngine(), "Unknown event suspend reason : "s + context->GetFunction()->GetDeclaration(true, true, true) + " = " + Name + ";");
        }
    }
    else
    {
        WriteError(context->GetEngine(), "Cannot execute event : "s + context->GetFunction()->GetDeclaration(true, true, true) + " = " + Name + ";");
        WriteError(context->GetEngine(), "Cannot execute event : Execute() = "s + std::to_string(r));

        // Do not reuse context which didn't finish properly
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
