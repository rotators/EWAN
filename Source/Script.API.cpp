#include "App.hpp"
#include "Content.hpp"
#include "Log.hpp"
#include "Script.hpp"
#include "Text.hpp"

static inline void _(bool& ok, int r)
{
    if(ok)
        ok = r >= 0;
}

static void AppLog(EWAN::App*, std::string text)
{
    as::asIScriptContext* context = as::asGetActiveContext();
    as::asIScriptFunction* function = context->GetFunction();

    std::string caller = std::string(function->GetModuleName()) + "(" + function->GetScriptSectionName() + ")::" + function->GetName();

    context->GetEngine()->WriteMessage(caller.c_str(), 0, 0, as::asMSGTYPE_INFORMATION, text.c_str());
}

bool EWAN::Script::InitAPI(App* app, as::asIScriptEngine* engine, const std::string& ns/*= "EWAN" */)
{
    bool ok = true;

    RegisterScriptArray(engine, true);
    RegisterStdString(engine);
    RegisterStdStringUtils(engine);

    _(ok, engine->SetDefaultNamespace(ns.c_str()));

    // Forward declarations
    static const std::vector<const char*> objRefNoHandle = {
        "App", "Content", "GameInfo", "Script", "Window",
        "WindowFPS"
    };

    for(const auto& obj : objRefNoHandle)
    {
        _(ok, engine->RegisterObjectType(obj, 0, as::asOBJ_REF | as::asOBJ_NOHANDLE));
    }

    //

    _(ok, engine->RegisterObjectProperty("App", "const Content  Content", asOFFSET(App, Content)));
    _(ok, engine->RegisterObjectProperty("App", "const GameInfo GameInfo", asOFFSET(App, GameInfo)));
    _(ok, engine->RegisterObjectProperty("App", "const Script   Script", asOFFSET(App, Script)));
    _(ok, engine->RegisterObjectProperty("App", "      Window   Window", asOFFSET(App, Window)));

    _(ok, engine->RegisterObjectMethod( "App", "void Log(const string text) const", as::asFUNCTION(AppLog), as::asCALL_CDECL_OBJFIRST));

    //

    _(ok, engine->RegisterObjectProperty("GameInfo", "const string Path", asOFFSET(GameInfo, Path)));
    _(ok, engine->RegisterObjectProperty("GameInfo", "const string Type", asOFFSET(GameInfo, Type)));
    _(ok, engine->RegisterObjectProperty("GameInfo", "const string ScriptInit", asOFFSET(GameInfo, ScriptInit)));

    //

    _(ok, engine->RegisterObjectProperty("Script", "const string RootDirectory", asOFFSET(Script, RootDirectory)));

    _(ok, engine->RegisterObjectMethod("Script", "bool LoadModule(const string fileName, const string moduleName) const", as::asMETHOD(Script,LoadModule_Call), as::asCALL_THISCALL));
    _(ok, engine->RegisterObjectMethod("Script", "bool UnloadModule(const string moduleName) const", as::asMETHOD(Script,UnloadModule_Call), as::asCALL_THISCALL));

    // NOTE: Functions marked with 'SFML' are binding directly to SFML methods

    _(ok, engine->RegisterEnum("WindowStyle"));
    _(ok, engine->RegisterEnumValue("WindowStyle", "None", sf::Style::None));
    _(ok, engine->RegisterEnumValue("WindowStyle", "Titlebar", sf::Style::Titlebar));
    _(ok, engine->RegisterEnumValue("WindowStyle", "Resize", sf::Style::Resize));
    _(ok, engine->RegisterEnumValue("WindowStyle", "Close", sf::Style::Close));
    _(ok, engine->RegisterEnumValue("WindowStyle", "Fullscreen", sf::Style::Fullscreen));
    _(ok, engine->RegisterEnumValue("WindowStyle", "Default", sf::Style::Default));

    _(ok, engine->RegisterObjectProperty("Window", "WindowFPS FPS", asOFFSET(Window, FPS)));

    _(ok, engine->RegisterObjectMethod("Window", "bool get_IsOpen() property", as::asMETHOD(Window, isOpen), as::asCALL_THISCALL)); // SFML

    _(ok, engine->RegisterObjectMethod("Window", "void Open(uint32 width = 0, uint32 height = 0, uint8 bitsPerPixel = 32, string title = \"\", uint32 style = WindowStyle::Default)", as::asMETHOD(Window, Open_Call), as::asCALL_THISCALL));
    _(ok, engine->RegisterObjectMethod("Window", "void Close()", as::asMETHOD(Window, close), as::asCALL_THISCALL)); // SFML

    //

    _(ok, engine->RegisterObjectProperty("WindowFPS", "const uint16 Count", asOFFSET(decltype(Window::FPS), Count)));
    _(ok, engine->RegisterObjectProperty("WindowFPS", "const uint16 Min", asOFFSET(decltype(Window::FPS), Min)));
    _(ok, engine->RegisterObjectProperty("WindowFPS", "const uint16 Max", asOFFSET(decltype(Window::FPS), Max)));
    _(ok, engine->RegisterObjectProperty("WindowFPS", "const uint16 Frame", asOFFSET(decltype(Window::FPS), Frame)));
    _(ok, engine->RegisterObjectProperty("WindowFPS", "      bool   Visible", asOFFSET(decltype(Window::FPS), Visible)));

    _(ok, engine->SetDefaultNamespace(""));

    _(ok, engine->RegisterGlobalProperty(Text::Join(std::vector<std::string>{ns, "::App App"}, "").c_str(), app));

    return ok;
}

// As G++ option -Wno-cast-function-type is enabled only for *this* file, function cannot live anywhere else (without failing build) 
bool EWAN::Script::InitMessageCallback(as::asIScriptEngine* engine)
{
    return engine->SetMessageCallback(as::asMETHOD(Script, CallbackMessage), this, as::asCALL_THISCALL) >= 0;
}
