#include "Script.hpp"

#include "App.hpp"
#include "Content.hpp"
#include "Log.hpp"
#include "Text.hpp"

#include "Libs/SFML.hpp"

#include <type_traits> // std::is_same

//#define SCRIPT_REGISTRATION_CHECK

using namespace std::literals::string_literals;

namespace
{
#if !defined(SCRIPT_REGISTRATION_CHECK)

    static inline void _(int& ok, int r)
    {
        if(ok >= 0)
            ok = (r >= 0) ? 0 : -1;
    }

#else

    #define _(ok, expr)                                        \
        if(ok >= 0)                                            \
        {                                                      \
            int r = expr;                                      \
            Log::Raw(""s + #expr + " = " + std::to_string(r)); \
            ok = r >= 0;                                       \
        }

#endif

    template<typename T>
    std::string TypenameToString()
    {
        std::string result;

        if constexpr(std::is_same_v<T, float>)
            result = "float";
        else if constexpr(std::is_same_v<T, int32_t>)
            result = "int32";
        else if constexpr(std::is_same_v<T, uint32_t>)
            result = "uint32";

        return result;
    }
}

/* static */ void EWAN::Script::API::AppLog(App*, std::string text)
{
    as::asIScriptContext*  context  = as::asGetActiveContext();
    if(!context)
        return;

    WriteInfo(context->GetEngine(), text, UserData::Get(context->GetEngine())->Script->GetContextFunctionDetails(context));
}

/* static */ bool EWAN::Script::API::Init(App* app, as::asIScriptEngine* engine, const std::string& ns /*= "EWAN" */)
{
    int ok = 0;

    if(Text::IsBlank(ns))
        return false;

    // Register vanilla addons

    RegisterScriptArray(engine, true);
    RegisterStdString(engine);
    RegisterStdStringUtils(engine);

    //

    _(ok, engine->SetDefaultNamespace(ns.c_str()));

    //

    if constexpr(sizeof(size_t) == 4)
    {
        _(ok, engine->RegisterTypedef("size_t", "uint32"));
    }
    else
    {
        _(ok, engine->RegisterTypedef("size_t", "uint64"));
    }

    // Forward declarations

    static const std::vector<const char*> zeroObjRefNoHandle = {
        "App",
        "Content",
        "ContentCache",
        "ContentSprite",
        "ContentTexture",
        "GameInfo",
        "Keyboard",
        "Script",
        "Window",
        "WindowFPS"
        //
    };

    static const std::vector<const char*> zeroObjRefNoCount = {
        "Sprite",
        "Texture"
        //
    };

    static const std::vector<const char*> zeroObjRefGcTemplate = {
        //
    };

    for(const auto& obj : zeroObjRefNoHandle)
    {
        _(ok, engine->RegisterObjectType(obj, 0, as::asOBJ_REF | as::asOBJ_NOHANDLE));
    }

    for(const auto& obj : zeroObjRefNoCount)
    {
        _(ok, engine->RegisterObjectType(obj, 0, as::asOBJ_REF | as::asOBJ_NOCOUNT));
    }

    for(const auto& obj : zeroObjRefGcTemplate)
    {
        _(ok, engine->RegisterObjectType(obj, 0, as::asOBJ_REF | as::asOBJ_GC | as::asOBJ_TEMPLATE));
    }

    // NOTE: Properties and methods marked with 'SFML' are binding directly to SFML
    //       In most cases asMETHODPR() needs to be used instead of asMETHOD()

    _(ok, engine->RegisterObjectProperty("App", "      bool     Quit", asOFFSET(App, Quit)));
    _(ok, engine->RegisterObjectProperty("App", "      bool     Restart", asOFFSET(App, Restart)));

    _(ok, engine->RegisterObjectProperty("App", "      Content  Content", asOFFSET(App, Content)));
    _(ok, engine->RegisterObjectProperty("App", "const GameInfo GameInfo", asOFFSET(App, GameInfo)));
    _(ok, engine->RegisterObjectProperty("App", "const Keyboard Keyboard", asOFFSET(App, Keyboard)));
    _(ok, engine->RegisterObjectProperty("App", "const Script   Script", asOFFSET(App, Script)));
    _(ok, engine->RegisterObjectProperty("App", "      Window   Window", asOFFSET(App, Window)));

    _(ok, engine->RegisterObjectMethod("App", "void Log(string text) const", as::asFUNCTION(AppLog), as::asCALL_CDECL_OBJFIRST));

    //

    _(ok, engine->RegisterObjectProperty("Content", "const string   RootDirectory", asOFFSET(Content, RootDirectory)));
    _(ok, engine->RegisterObjectProperty("Content", "ContentCache   Font", asOFFSET(Content, Font)));
    _(ok, engine->RegisterObjectProperty("Content", "ContentSprite  Sprite", asOFFSET(Content, Sprite)));
    _(ok, engine->RegisterObjectProperty("Content", "ContentTexture Texture", asOFFSET(Content, Texture)));

    _(ok, engine->RegisterObjectMethod("Content", "size_t DeleteAll()", as::asMETHOD(Content, DeleteAll), as::asCALL_THISCALL));
    _(ok, engine->RegisterObjectMethod("Content", "size_t Size()", as::asMETHOD(Content, Size), as::asCALL_THISCALL));
    _(ok, engine->RegisterObjectMethod("Content", "bool LoadFile(string&in fileName, string&in id)", as::asMETHOD(Content, LoadFile), as::asCALL_THISCALL));
    _(ok, engine->RegisterObjectMethod("Content", "size_t LoadDirectory(string&in directory)", as::asMETHOD(Content, LoadDirectory), as::asCALL_THISCALL));

    //

    _(ok, RegisterContentCache(engine, "ContentCache"));
    _(ok, RegisterContentCache(engine, "ContentSprite", "Sprite@"));
    _(ok, RegisterContentCache(engine, "ContentTexture", "Texture@"));

    //

    _(ok, engine->RegisterObjectProperty("GameInfo", "const string Path", asOFFSET(GameInfo, Path)));
    _(ok, engine->RegisterObjectProperty("GameInfo", "const string Type", asOFFSET(GameInfo, Type)));
    _(ok, engine->RegisterObjectProperty("GameInfo", "const string ScriptInit", asOFFSET(GameInfo, ScriptInit)));

    //

    _(ok, engine->RegisterEnum("Key"));
    _(ok, engine->RegisterEnumValue("Key", "Unknown", sf::Keyboard::Key::Unknown));
    _(ok, engine->RegisterEnumValue("Key", "A", sf::Keyboard::Key::A));
    _(ok, engine->RegisterEnumValue("Key", "B", sf::Keyboard::Key::B));
    _(ok, engine->RegisterEnumValue("Key", "C", sf::Keyboard::Key::C));
    _(ok, engine->RegisterEnumValue("Key", "D", sf::Keyboard::Key::D));
    _(ok, engine->RegisterEnumValue("Key", "E", sf::Keyboard::Key::E));
    _(ok, engine->RegisterEnumValue("Key", "F", sf::Keyboard::Key::F));
    _(ok, engine->RegisterEnumValue("Key", "G", sf::Keyboard::Key::G));
    _(ok, engine->RegisterEnumValue("Key", "H", sf::Keyboard::Key::H));
    _(ok, engine->RegisterEnumValue("Key", "I", sf::Keyboard::Key::I));
    _(ok, engine->RegisterEnumValue("Key", "J", sf::Keyboard::Key::J));
    _(ok, engine->RegisterEnumValue("Key", "K", sf::Keyboard::Key::K));
    _(ok, engine->RegisterEnumValue("Key", "L", sf::Keyboard::Key::L));
    _(ok, engine->RegisterEnumValue("Key", "M", sf::Keyboard::Key::M));
    _(ok, engine->RegisterEnumValue("Key", "N", sf::Keyboard::Key::N));
    _(ok, engine->RegisterEnumValue("Key", "O", sf::Keyboard::Key::O));
    _(ok, engine->RegisterEnumValue("Key", "P", sf::Keyboard::Key::P));
    _(ok, engine->RegisterEnumValue("Key", "Q", sf::Keyboard::Key::Q));
    _(ok, engine->RegisterEnumValue("Key", "R", sf::Keyboard::Key::R));
    _(ok, engine->RegisterEnumValue("Key", "S", sf::Keyboard::Key::S));
    _(ok, engine->RegisterEnumValue("Key", "T", sf::Keyboard::Key::T));
    _(ok, engine->RegisterEnumValue("Key", "U", sf::Keyboard::Key::U));
    _(ok, engine->RegisterEnumValue("Key", "V", sf::Keyboard::Key::V));
    _(ok, engine->RegisterEnumValue("Key", "W", sf::Keyboard::Key::W));
    _(ok, engine->RegisterEnumValue("Key", "X", sf::Keyboard::Key::X));
    _(ok, engine->RegisterEnumValue("Key", "Y", sf::Keyboard::Key::Y));
    _(ok, engine->RegisterEnumValue("Key", "Z", sf::Keyboard::Key::Z));
    _(ok, engine->RegisterEnumValue("Key", "Num0", sf::Keyboard::Key::Num0));
    _(ok, engine->RegisterEnumValue("Key", "Num1", sf::Keyboard::Key::Num1));
    _(ok, engine->RegisterEnumValue("Key", "Num2", sf::Keyboard::Key::Num2));
    _(ok, engine->RegisterEnumValue("Key", "Num3", sf::Keyboard::Key::Num3));
    _(ok, engine->RegisterEnumValue("Key", "Num4", sf::Keyboard::Key::Num4));
    _(ok, engine->RegisterEnumValue("Key", "Num5", sf::Keyboard::Key::Num5));
    _(ok, engine->RegisterEnumValue("Key", "Num6", sf::Keyboard::Key::Num6));
    _(ok, engine->RegisterEnumValue("Key", "Num7", sf::Keyboard::Key::Num7));
    _(ok, engine->RegisterEnumValue("Key", "Num8", sf::Keyboard::Key::Num8));
    _(ok, engine->RegisterEnumValue("Key", "Num9", sf::Keyboard::Key::Num9));
    _(ok, engine->RegisterEnumValue("Key", "Escape", sf::Keyboard::Key::Escape));
    _(ok, engine->RegisterEnumValue("Key", "LControl", sf::Keyboard::Key::LControl));
    _(ok, engine->RegisterEnumValue("Key", "LShift", sf::Keyboard::Key::LShift));
    _(ok, engine->RegisterEnumValue("Key", "LAlt", sf::Keyboard::Key::LAlt));
    _(ok, engine->RegisterEnumValue("Key", "LSystem", sf::Keyboard::Key::LSystem));
    _(ok, engine->RegisterEnumValue("Key", "RControl", sf::Keyboard::Key::RControl));
    _(ok, engine->RegisterEnumValue("Key", "RShift", sf::Keyboard::Key::RShift));
    _(ok, engine->RegisterEnumValue("Key", "RAlt", sf::Keyboard::Key::RAlt));
    _(ok, engine->RegisterEnumValue("Key", "RSystem", sf::Keyboard::Key::RSystem));
    _(ok, engine->RegisterEnumValue("Key", "Menu", sf::Keyboard::Key::Menu));
    _(ok, engine->RegisterEnumValue("Key", "LBracket", sf::Keyboard::Key::LBracket));
    _(ok, engine->RegisterEnumValue("Key", "RBracket", sf::Keyboard::Key::RBracket));
    _(ok, engine->RegisterEnumValue("Key", "Semicolon", sf::Keyboard::Key::Semicolon));
    _(ok, engine->RegisterEnumValue("Key", "Comma", sf::Keyboard::Key::Comma));
    _(ok, engine->RegisterEnumValue("Key", "Period", sf::Keyboard::Key::Period));
    _(ok, engine->RegisterEnumValue("Key", "Quote", sf::Keyboard::Key::Quote));
    _(ok, engine->RegisterEnumValue("Key", "Slash", sf::Keyboard::Key::Slash));
    _(ok, engine->RegisterEnumValue("Key", "Backslash", sf::Keyboard::Key::Backslash));
    _(ok, engine->RegisterEnumValue("Key", "Tilde", sf::Keyboard::Key::Tilde));
    _(ok, engine->RegisterEnumValue("Key", "Equal", sf::Keyboard::Key::Equal));
    _(ok, engine->RegisterEnumValue("Key", "Hyphen", sf::Keyboard::Key::Hyphen));
    _(ok, engine->RegisterEnumValue("Key", "Space", sf::Keyboard::Key::Space));
    _(ok, engine->RegisterEnumValue("Key", "Enter", sf::Keyboard::Key::Enter));
    _(ok, engine->RegisterEnumValue("Key", "Backspace", sf::Keyboard::Key::Backspace));
    _(ok, engine->RegisterEnumValue("Key", "Tab", sf::Keyboard::Key::Tab));
    _(ok, engine->RegisterEnumValue("Key", "PageUp", sf::Keyboard::Key::PageUp));
    _(ok, engine->RegisterEnumValue("Key", "PageDown", sf::Keyboard::Key::PageDown));
    _(ok, engine->RegisterEnumValue("Key", "End", sf::Keyboard::Key::End));
    _(ok, engine->RegisterEnumValue("Key", "Home", sf::Keyboard::Key::Home));
    _(ok, engine->RegisterEnumValue("Key", "Insert", sf::Keyboard::Key::Insert));
    _(ok, engine->RegisterEnumValue("Key", "Delete", sf::Keyboard::Key::Delete));
    _(ok, engine->RegisterEnumValue("Key", "Add", sf::Keyboard::Key::Add));
    _(ok, engine->RegisterEnumValue("Key", "Subtract", sf::Keyboard::Key::Subtract));
    _(ok, engine->RegisterEnumValue("Key", "Multiply", sf::Keyboard::Key::Multiply));
    _(ok, engine->RegisterEnumValue("Key", "Divide", sf::Keyboard::Key::Divide));
    _(ok, engine->RegisterEnumValue("Key", "Left", sf::Keyboard::Key::Left));
    _(ok, engine->RegisterEnumValue("Key", "Right", sf::Keyboard::Key::Right));
    _(ok, engine->RegisterEnumValue("Key", "Up", sf::Keyboard::Key::Up));
    _(ok, engine->RegisterEnumValue("Key", "Down", sf::Keyboard::Key::Down));
    _(ok, engine->RegisterEnumValue("Key", "Numpad0", sf::Keyboard::Key::Numpad0));
    _(ok, engine->RegisterEnumValue("Key", "Numpad1", sf::Keyboard::Key::Numpad1));
    _(ok, engine->RegisterEnumValue("Key", "Numpad2", sf::Keyboard::Key::Numpad2));
    _(ok, engine->RegisterEnumValue("Key", "Numpad3", sf::Keyboard::Key::Numpad3));
    _(ok, engine->RegisterEnumValue("Key", "Numpad4", sf::Keyboard::Key::Numpad4));
    _(ok, engine->RegisterEnumValue("Key", "Numpad5", sf::Keyboard::Key::Numpad5));
    _(ok, engine->RegisterEnumValue("Key", "Numpad6", sf::Keyboard::Key::Numpad6));
    _(ok, engine->RegisterEnumValue("Key", "Numpad7", sf::Keyboard::Key::Numpad7));
    _(ok, engine->RegisterEnumValue("Key", "Numpad8", sf::Keyboard::Key::Numpad8));
    _(ok, engine->RegisterEnumValue("Key", "Numpad9", sf::Keyboard::Key::Numpad9));
    _(ok, engine->RegisterEnumValue("Key", "F1", sf::Keyboard::Key::F1));
    _(ok, engine->RegisterEnumValue("Key", "F2", sf::Keyboard::Key::F2));
    _(ok, engine->RegisterEnumValue("Key", "F3", sf::Keyboard::Key::F3));
    _(ok, engine->RegisterEnumValue("Key", "F4", sf::Keyboard::Key::F4));
    _(ok, engine->RegisterEnumValue("Key", "F5", sf::Keyboard::Key::F5));
    _(ok, engine->RegisterEnumValue("Key", "F6", sf::Keyboard::Key::F6));
    _(ok, engine->RegisterEnumValue("Key", "F7", sf::Keyboard::Key::F7));
    _(ok, engine->RegisterEnumValue("Key", "F8", sf::Keyboard::Key::F8));
    _(ok, engine->RegisterEnumValue("Key", "F9", sf::Keyboard::Key::F9));
    _(ok, engine->RegisterEnumValue("Key", "F10", sf::Keyboard::Key::F10));
    _(ok, engine->RegisterEnumValue("Key", "F11", sf::Keyboard::Key::F11));
    _(ok, engine->RegisterEnumValue("Key", "F12", sf::Keyboard::Key::F12));
    _(ok, engine->RegisterEnumValue("Key", "F13", sf::Keyboard::Key::F13));
    _(ok, engine->RegisterEnumValue("Key", "F14", sf::Keyboard::Key::F14));
    _(ok, engine->RegisterEnumValue("Key", "F15", sf::Keyboard::Key::F15));
    _(ok, engine->RegisterEnumValue("Key", "Pause", sf::Keyboard::Key::Pause));

    _(ok, engine->RegisterObjectProperty("Keyboard", "const bool Alt", asOFFSET(sf::Event::KeyEvent, alt)));
    _(ok, engine->RegisterObjectProperty("Keyboard", "const bool Control", asOFFSET(sf::Event::KeyEvent, control)));
    _(ok, engine->RegisterObjectProperty("Keyboard", "const bool Shift", asOFFSET(sf::Event::KeyEvent, shift)));

    //

    _(ok, engine->RegisterObjectProperty("Script", "const string RootDirectory", asOFFSET(Script, RootDirectory)));

    _(ok, engine->RegisterObjectMethod("Script", "string get_CurrentEventName() const property", as::asMETHOD(Script, CurrentEventName_Call), as::asCALL_THISCALL));

    _(ok, engine->RegisterObjectMethod("Script", "bool LoadModule(string&in fileName, string&in moduleName) const", as::asMETHOD(Script, LoadModule_Call), as::asCALL_THISCALL));
    _(ok, engine->RegisterObjectMethod("Script", "bool UnloadModule(string&in moduleName) const", as::asMETHOD(Script, UnloadModule_Call), as::asCALL_THISCALL));
    _(ok, engine->RegisterObjectMethod("Script", "void Yield() const", as::asMETHOD(Script, Yield_Call), as::asCALL_THISCALL));

    //

    _(ok, engine->RegisterObjectMethod("Sprite", "void Move(float xOffset, float yOffset)", as::asMETHODPR(sf::Sprite, move, (float, float), void), as::asCALL_THISCALL));         // SFML Transformable
    _(ok, engine->RegisterObjectMethod("Sprite", "void SetOrigin(float x, float y)", as::asMETHODPR(sf::Sprite, setOrigin, (float, float), void), as::asCALL_THISCALL));           // SFML Transformable
    _(ok, engine->RegisterObjectMethod("Sprite", "void SetPosition(float x, float y)", as::asMETHODPR(sf::Sprite, setPosition, (float, float), void), as::asCALL_THISCALL));       // SFML Transformable
    _(ok, engine->RegisterObjectMethod("Sprite", "void SetRotation(float angle)", as::asMETHODPR(sf::Sprite, setRotation, (float), void), as::asCALL_THISCALL));                   // SFML Transformable
    _(ok, engine->RegisterObjectMethod("Sprite", "void SetScale(float factorX, float factorY)", as::asMETHODPR(sf::Sprite, setScale, (float, float), void), as::asCALL_THISCALL)); // SFML Transformable
    _(ok, engine->RegisterObjectMethod("Sprite", "bool SetTexture(const Texture& texture, bool resetRect = true)", as::asMETHOD(sf::Sprite, setTexture), as::asCALL_THISCALL));    // SFML Sprite

    //

    _(ok, engine->RegisterEnum("WindowStyle"));
    _(ok, engine->RegisterEnumValue("WindowStyle", "None", sf::Style::None));
    _(ok, engine->RegisterEnumValue("WindowStyle", "Titlebar", sf::Style::Titlebar));
    _(ok, engine->RegisterEnumValue("WindowStyle", "Resize", sf::Style::Resize));
    _(ok, engine->RegisterEnumValue("WindowStyle", "Close", sf::Style::Close));
    _(ok, engine->RegisterEnumValue("WindowStyle", "Fullscreen", sf::Style::Fullscreen));
    _(ok, engine->RegisterEnumValue("WindowStyle", "Default", sf::Style::Default));

    _(ok, engine->RegisterObjectProperty("Window", "WindowFPS FPS", asOFFSET(Window, FPS)));

    _(ok, engine->RegisterObjectMethod("Window", "bool get_IsOpen() property", as::asMETHOD(Window, isOpen), as::asCALL_THISCALL)); // SFML

    _(ok, engine->RegisterObjectMethod("Window", "void Open(uint32 width = 0, uint32 height = 0, uint8 bitsPerPixel = 32, string&in title = \"\", uint32 style = WindowStyle::Default)", as::asMETHOD(Window, Open_Call), as::asCALL_THISCALL));
    _(ok, engine->RegisterObjectMethod("Window", "void Close()", as::asMETHOD(Window, close), as::asCALL_THISCALL)); // SFML
    _(ok, engine->RegisterObjectMethod("Window", "bool Draw(Sprite@ sprite)", as::asMETHODPR(Window, Draw, (sf::Sprite*), bool), as::asCALL_THISCALL));
    _(ok, engine->RegisterObjectMethod("Window", "bool Draw(const Content&in content, string&in spriteId)", as::asMETHODPR(Window, Draw, (const Content&, const std::string&), bool), as::asCALL_THISCALL));

    //

    _(ok, engine->RegisterObjectProperty("WindowFPS", "const uint16 Count", asOFFSET(decltype(Window::FPS), Count)));
    _(ok, engine->RegisterObjectProperty("WindowFPS", "const uint16 Min", asOFFSET(decltype(Window::FPS), Min)));
    _(ok, engine->RegisterObjectProperty("WindowFPS", "const uint16 Max", asOFFSET(decltype(Window::FPS), Max)));
    _(ok, engine->RegisterObjectProperty("WindowFPS", "const uint16 Frame", asOFFSET(decltype(Window::FPS), Frame)));
    _(ok, engine->RegisterObjectProperty("WindowFPS", "const float  FrameTime", asOFFSET(decltype(Window::FPS), FrameTime)));
    _(ok, engine->RegisterObjectProperty("WindowFPS", "      bool   Visible", asOFFSET(decltype(Window::FPS), Visible)));

    //

    _(ok, engine->SetDefaultNamespace(""));

    _(ok, engine->RegisterGlobalProperty(Text::Join(std::vector<std::string> {ns, "::App App"}, "").c_str(), app));

    return ok >= 0;
}

// As G++ options -Wno-cast-function-type/-Wno-useless-cast are enabled only for *this* file,
// functions setting specific callback functions cannot live anywhere else (without failing whole build or making a mess)

/* static */ bool EWAN::Script::API::InitEngineCallback(Script* script, as::asIScriptEngine* engine)
{
    return engine->SetMessageCallback(as::asMETHOD(Script, CallbackMessage), script, as::asCALL_THISCALL) >= 0;
}

/* static */ bool EWAN::Script::API::InitContextCallback(Script* script, as::asIScriptContext* context)
{
    return context->SetLineCallback(as::asMETHOD(Script, CallbackContextLine), script, as::asCALL_THISCALL) >= 0;
}

//

/* static */ int EWAN::Script::API::RegisterContentCache(as::asIScriptEngine* engine, const std::string& type, const std::string& subtype /*= {} */)
{
    using namespace EWAN;

    int ok = 0;

    const std::string boolOrSubtype = (subtype.empty() ? "bool" : subtype);

    // Generic cache returns bool when creating new object
    // Custom cache returns newly created subtype
    _(ok, engine->RegisterObjectMethod(type.c_str(), (boolOrSubtype + " New(string&in id)").c_str(), as::asMETHOD(Content::Cache, New), as::asCALL_THISCALL));
    _(ok, engine->RegisterObjectMethod(type.c_str(), "bool   Delete(string&in id)", as::asMETHOD(Content::Cache, Delete), as::asCALL_THISCALL));
    _(ok, engine->RegisterObjectMethod(type.c_str(), "size_t DeleteAll()", as::asMETHOD(Content::Cache, DeleteAll), as::asCALL_THISCALL));
    _(ok, engine->RegisterObjectMethod(type.c_str(), "size_t Size()", as::asMETHOD(Content::Cache, Size), as::asCALL_THISCALL));
    _(ok, engine->RegisterObjectMethod(type.c_str(), "bool   Exists(string&in id)", as::asMETHOD(Content::Cache, Exists), as::asCALL_THISCALL));

    // Custom cache can return subtype directly
    if(!subtype.empty())
    {
        _(ok, engine->RegisterObjectMethod(type.c_str(), (subtype + " Get(string&in id, bool silent = true)").c_str(), as::asMETHOD(Content::Cache, Get), as::asCALL_THISCALL));
    }

    return ok;
}
