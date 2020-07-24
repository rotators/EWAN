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
    as::asIScriptFunction* function = context->GetFunction();

    std::string caller = std::string(function->GetModuleName()) + "(" + function->GetScriptSectionName() + ")::" + function->GetName();

    WriteInfo(context->GetEngine(), text, caller);
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

    // NOTE: Methods marked with 'SFML' are binding directly to SFML
    //       In most cases asMETHODPR() needs to be used instead of asMETHOD()

    _(ok, engine->RegisterObjectProperty("App", "      bool     Quit", asOFFSET(App, Quit)));

    _(ok, engine->RegisterObjectProperty("App", "      Content  Content", asOFFSET(App, Content)));
    _(ok, engine->RegisterObjectProperty("App", "const GameInfo GameInfo", asOFFSET(App, GameInfo)));
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

    _(ok, engine->RegisterObjectProperty("Script", "const string RootDirectory", asOFFSET(Script, RootDirectory)));

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
