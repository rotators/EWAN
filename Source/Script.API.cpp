#include "Script.hpp"

#include "App.hpp"
#include "Content.hpp"
#include "Log.hpp"
#include "Text.hpp"

#include "Libs/SFML.hpp"

//#define SCRIPT_REGISTRATION_CHECK

using namespace std::literals::string_literals;

namespace
{
#if !defined(SCRIPT_REGISTRATION_CHECK)
    static inline void _(bool& ok, int r)
    {
        if(ok)
            ok = r >= 0;
    }
#else
#    define _(ok, expr)                                            \
        do                                                         \
        {                                                          \
            int r = expr;                                          \
            if(r < 0)                                              \
            {                                                      \
                Log::Raw(""s + #expr + " = " + std::to_string(r)); \
                ok = false;                                        \
            }                                                      \
        } while(0);
#endif

    int RegisterContentCache(as::asIScriptEngine* engine, const std::string& cacheName, const std::string& typeName = {})
    {
        using namespace EWAN;

        bool ok = true;

        if(typeName.empty())
        {
            _(ok, engine->RegisterObjectMethod(cacheName.c_str(), "bool New(const string&in id)", as::asMETHOD(Content::Cache, New), as::asCALL_THISCALL));
        }
        else
        {
            _(ok, engine->RegisterObjectMethod(cacheName.c_str(), (typeName + " New(const string&in id)").c_str(), as::asMETHOD(Content::Cache, New), as::asCALL_THISCALL));
        }

        _(ok, engine->RegisterObjectMethod(cacheName.c_str(), "bool   Delete(const string&in id)", as::asMETHOD(Content::Cache, Delete), as::asCALL_THISCALL));
        _(ok, engine->RegisterObjectMethod(cacheName.c_str(), "size_t DeleteAll()", as::asMETHOD(Content::Cache, DeleteAll), as::asCALL_THISCALL));
        _(ok, engine->RegisterObjectMethod(cacheName.c_str(), "size_t Size()", as::asMETHOD(Content::Cache, Size), as::asCALL_THISCALL));
        _(ok, engine->RegisterObjectMethod(cacheName.c_str(), "bool   Exists(const string&in id)", as::asMETHOD(Content::Cache, Exists), as::asCALL_THISCALL));

        if(!typeName.empty())
        {
            _(ok, engine->RegisterObjectMethod(cacheName.c_str(), (typeName + " Get(const string&in id, bool silent = true)").c_str(), as::asMETHOD(Content::Cache, Get), as::asCALL_THISCALL));
        }

        return ok ? 0 : -1;
    }
}

namespace EWAN
{
    namespace ScriptAPI
    {
        void AppLog(App*, std::string text)
        {
            as::asIScriptContext*  context  = as::asGetActiveContext();
            as::asIScriptFunction* function = context->GetFunction();

            std::string caller = std::string(function->GetModuleName()) + "(" + function->GetScriptSectionName() + ")::" + function->GetName();

            context->GetEngine()->WriteMessage(caller.c_str(), 0, 0, as::asMSGTYPE_INFORMATION, text.c_str());
        }
    }
}

bool EWAN::Script::InitAPI(App* app, as::asIScriptEngine* engine, const std::string& ns /*= "EWAN" */)
{
    bool ok = true;

    RegisterScriptArray(engine, true);
    RegisterStdString(engine);
    RegisterStdStringUtils(engine);

    //

    _(ok, engine->SetDefaultNamespace(ns.c_str()));

    //

#if !defined(SCRIPT_REGISTRATION_CHECK)
    if constexpr(sizeof(size_t) == 4)
        _(ok, engine->RegisterTypedef("size_t", "uint32"));
    else
        _(ok, engine->RegisterTypedef("size_t", "uint64"));
#else
    _(ok, engine->RegisterTypedef("size_t", "uint32"));
#endif

    // Forward declarations

    static const std::vector<const char*> zeroObjRefNoHandle = {
        "App",
        "Content",
        "ContentCache",
        "ContentSprite",
        "ContentTexture",
        "GameInfo",
        "Script",
        "Window",
        "WindowFPS"};

    static const std::vector<const char*> zeroObjRefNoCount = {
        "Sprite",
        "Texture"};

    for(const auto& obj : zeroObjRefNoHandle)
    {
        _(ok, engine->RegisterObjectType(obj, 0, as::asOBJ_REF | as::asOBJ_NOHANDLE));
    }

    for(const auto& obj : zeroObjRefNoCount)
    {
        _(ok, engine->RegisterObjectType(obj, 0, as::asOBJ_REF | as::asOBJ_NOCOUNT));
    }

    _(ok, RegisterContentCache(engine, "ContentCache"));
    _(ok, RegisterContentCache(engine, "ContentSprite", "Sprite@"));
    _(ok, RegisterContentCache(engine, "ContentTexture", "Texture@"));

    // NOTE: Methods marked with 'SFML' are binding directly to SFML

    _(ok, engine->RegisterObjectProperty("App", "      bool     Quit", asOFFSET(App, Quit)));
    _(ok, engine->RegisterObjectProperty("App", "      Content  Content", asOFFSET(App, Content)));
    _(ok, engine->RegisterObjectProperty("App", "const GameInfo GameInfo", asOFFSET(App, GameInfo)));
    _(ok, engine->RegisterObjectProperty("App", "const Script   Script", asOFFSET(App, Script)));
    _(ok, engine->RegisterObjectProperty("App", "      Window   Window", asOFFSET(App, Window)));

    _(ok, engine->RegisterObjectMethod("App", "void Log(const string&in text) const", as::asFUNCTION(ScriptAPI::AppLog), as::asCALL_CDECL_OBJFIRST));

    //

    _(ok, engine->RegisterObjectProperty("Content", "const string   RootDirectory", asOFFSET(Content, RootDirectory)));
    _(ok, engine->RegisterObjectProperty("Content", "ContentCache   Font", asOFFSET(Content, Font)));
    _(ok, engine->RegisterObjectProperty("Content", "ContentSprite  Sprite", asOFFSET(Content, Sprite)));
    _(ok, engine->RegisterObjectProperty("Content", "ContentTexture Texture", asOFFSET(Content, Texture)));

    _(ok, engine->RegisterObjectMethod("Content", "size_t DeleteAll()", as::asMETHOD(Content, DeleteAll), as::asCALL_THISCALL));
    _(ok, engine->RegisterObjectMethod("Content", "size_t Size()", as::asMETHOD(Content, Size), as::asCALL_THISCALL));
    _(ok, engine->RegisterObjectMethod("Content", "bool LoadFile(const string&in fileName, const string&in id)", as::asMETHOD(Content, LoadFile), as::asCALL_THISCALL));
    _(ok, engine->RegisterObjectMethod("Content", "size_t LoadDirectory(const string&in directory)", as::asMETHOD(Content, LoadDirectory), as::asCALL_THISCALL));

    //

    _(ok, engine->RegisterObjectProperty("GameInfo", "const string Path", asOFFSET(GameInfo, Path)));
    _(ok, engine->RegisterObjectProperty("GameInfo", "const string Type", asOFFSET(GameInfo, Type)));
    _(ok, engine->RegisterObjectProperty("GameInfo", "const string ScriptInit", asOFFSET(GameInfo, ScriptInit)));

    //

    _(ok, engine->RegisterObjectProperty("Script", "const string RootDirectory", asOFFSET(Script, RootDirectory)));

    _(ok, engine->RegisterObjectMethod("Script", "bool LoadModule(const string&in fileName, const string&in moduleName) const", as::asMETHOD(Script, LoadModule_Call), as::asCALL_THISCALL));
    _(ok, engine->RegisterObjectMethod("Script", "bool UnloadModule(const string&in moduleName) const", as::asMETHOD(Script, UnloadModule_Call), as::asCALL_THISCALL));

    //

    _(ok, engine->RegisterObjectMethod("Sprite", "void Move(float xOffset, float yOffset)", as::asMETHODPR(sf::Sprite, move, (float, float), void), as::asCALL_THISCALL));                    // SFML Transformable
    _(ok, engine->RegisterObjectMethod("Sprite", "void SetOrigin(float x, float y)", as::asMETHODPR(sf::Sprite, setOrigin, (float, float), void), as::asCALL_THISCALL));                      // SFML Transformable
    _(ok, engine->RegisterObjectMethod("Sprite", "void SetPosition(float x, float y)", as::asMETHODPR(sf::Sprite, setPosition, (float, float), void), as::asCALL_THISCALL));                  // SFML Transformable
    _(ok, engine->RegisterObjectMethod("Sprite", "void SetRotation(float angle)", as::asMETHOD(sf::Sprite, setRotation), as::asCALL_THISCALL));                                               // SFML Transformable
    _(ok, engine->RegisterObjectMethod("Sprite", "void SetScale(float factorX, float factorY)", as::asMETHODPR(sf::Sprite, setScale, (float, float), void), as::asCALL_THISCALL));            // SFML Transformable
    _(ok, engine->RegisterObjectMethod("Sprite", "bool SetTexture(const Texture& texture, bool resetRect = true) // deprecated", as::asMETHOD(sf::Sprite, setTexture), as::asCALL_THISCALL)); // SFML Sprite

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

    _(ok, engine->RegisterObjectMethod("Window", "void Open(uint32 width = 0, uint32 height = 0, uint8 bitsPerPixel = 32, string title = \"\", uint32 style = WindowStyle::Default)", as::asMETHOD(Window, Open_Call), as::asCALL_THISCALL));
    _(ok, engine->RegisterObjectMethod("Window", "void Close()", as::asMETHOD(Window, close), as::asCALL_THISCALL)); // SFML
    _(ok, engine->RegisterObjectMethod("Window", "bool DrawSprite(const Content&in content, const string&in spriteId)", as::asMETHOD(Window, DrawSprite), as::asCALL_THISCALL));

    //

    _(ok, engine->RegisterObjectProperty("WindowFPS", "const uint16 Count", asOFFSET(decltype(Window::FPS), Count)));
    _(ok, engine->RegisterObjectProperty("WindowFPS", "const uint16 Min", asOFFSET(decltype(Window::FPS), Min)));
    _(ok, engine->RegisterObjectProperty("WindowFPS", "const uint16 Max", asOFFSET(decltype(Window::FPS), Max)));
    _(ok, engine->RegisterObjectProperty("WindowFPS", "const uint16 Frame", asOFFSET(decltype(Window::FPS), Frame)));
    _(ok, engine->RegisterObjectProperty("WindowFPS", "      bool   Visible", asOFFSET(decltype(Window::FPS), Visible)));

    _(ok, engine->SetDefaultNamespace(""));

    _(ok, engine->RegisterGlobalProperty(Text::Join(std::vector<std::string> {ns, "::App App"}, "").c_str(), app));

    return ok;
}

// As G++ option -Wno-cast-function-type is enabled only for *this* file, function cannot live anywhere else (without failing build)
bool EWAN::Script::InitMessageCallback(as::asIScriptEngine* engine)
{
    return engine->SetMessageCallback(as::asMETHOD(Script, CallbackMessage), this, as::asCALL_THISCALL) >= 0;
}
