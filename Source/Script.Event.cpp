#include "Script.hpp"
#include "Text.hpp"

using namespace std::literals::string_literals;

//
// Script::Event::Data
//

EWAN::Script::Event::Data::Data(std::list<std::string> params, std::list<as::asIScriptFunction*>& list) :
    Params(params),
    List(list)
{}

std::string EWAN::Script::Event::Data::GetDeclaration(const std::string& name /*= "f" */) const
{
    std::list<std::string> params = Params;

    std::string declaration = params.front() + " " + name;

    declaration += "(";

    params.pop_front();
    declaration += Text::Join(params, ", ");

    declaration += ")";

    return declaration;
}

std::string EWAN::Script::Event::Data::GetDeclaration(as::asIScriptFunction* function) const
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

void EWAN::Script::Event::Register(std::list<as::asIScriptFunction*>& functions, as::asIScriptFunction* function, const std::string& name)
{
    if(GetUserData(function->GetModule())->Debug)
        WriteInfo(function->GetEngine(), "Registered event callback : "s + function->GetDeclaration(true, true, true) + " = " + name + ";", function->GetModuleName());

    functions.push_back(function);
}

void EWAN::Script::Event::Unregister(as::asIScriptModule* module)
{
    Unregister(OnBuild, module, "OnBuild");
    Unregister(OnInit, module, "OnInit");
    Unregister(OnInit, module, "OnFinish");

    Unregister(OnDraw, module, "OnDraw");
    Unregister(OnKeyDown, module, "OnKeyDown");
    Unregister(OnKeyUp, module, "OnKeyUp");
    Unregister(OnMouseMove, module, "OnMouseMove");
    Unregister(OnMouseDown, module, "OnMouseDown");
    Unregister(OnMouseUp, module, "OnMouseUp");
}

void EWAN::Script::Event::Unregister(std::list<as::asIScriptFunction*>& functions, as::asIScriptEngine* engine, const std::string& name)
{
    for(as::asUINT m = 0, mLen = engine->GetModuleCount(); m < mLen; m++)
    {
        Unregister(functions, engine->GetModuleByIndex(m), name);
    }
}

void EWAN::Script::Event::Unregister(std::list<as::asIScriptFunction*>& functions, as::asIScriptModule* module, const std::string& name)
{
    const bool debug = GetUserData(module)->Debug;

    functions.remove_if([debug, module, name](as::asIScriptFunction* function) -> bool {
        if(function->GetModule() == module)
        {
            if(debug)
                WriteInfo(module->GetEngine(), "Unregistered event callback : "s + function->GetDeclaration(true, true, true) + " = " + name + ";", function->GetModuleName());

            return true;
        }

        return false;
    });
}

bool EWAN::Script::Event::Run(as::asIScriptContext* context)
{
    // TODO Validate context result

    context->Execute();

    return true;
}

bool EWAN::Script::Event::RunOnBuild(as::asIScriptModule* module)
{
    if(OnBuild.empty())
        return true;

    // Make sure event runs only once per module
    std::list<as::asIScriptFunction*> onBuild = OnBuild;
    OnBuild.clear();
    bool result = Run(onBuild);

    // That big boy over here exists only to provide feedback in log
    Unregister(onBuild, module, "OnBuild");

    return result;
}

bool EWAN::Script::Event::RunOnInit(as::asIScriptEngine* engine, as::asIScriptFunction*& falseFunction)
{
    as::asIScriptContext* context = engine->RequestContext();

    bool onInit = true;
    for(const auto& function : OnInit)
    {
        // TODO OnInit event is currently unsafe

        if(GetUserData(function->GetModule())->Debug)
            WriteInfo(engine, "Run event callback : "s + function->GetDeclaration(true, true, true) + " = OnInit;", function->GetModuleName());

        context->Prepare(function);
        context->Execute();

        onInit = context->GetReturnByte();

        if(!onInit)
        {
            falseFunction = function;
            break;
        }
    }

    engine->ReturnContext(context);

    Unregister(OnInit, engine, "OnInit");

    return onInit;
}

void EWAN::Script::Event::RunOnFinish(as::asIScriptEngine* engine)
{
    as::asIScriptContext* context = engine->RequestContext();

    for(const auto& function : OnFinish)
    {
        if(GetUserData(function->GetModule())->Debug)
            WriteInfo(engine, "Run event callback : "s + function->GetDeclaration(true, true, true) + " = OnFinish;", function->GetModuleName());

        // OnFinish event should ignore all errors during script execution, if possible

        context->Prepare(function);
        context->Execute();
    }

    engine->ReturnContext(context);

    Unregister(OnFinish, engine, "OnFinish");
}
