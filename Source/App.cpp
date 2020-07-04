#include "App.hpp"
#include "Log.hpp"

#if __has_include(<format>)
 #include <format>
#endif

EWAN::App::App() :
    Version{PROJECT_VERSION_MAJOR, PROJECT_VERSION_MINOR, PROJECT_VERSION_PATCH, PROJECT_VERSION_TWEAK}
{}

EWAN::App::~App()
{
    if(!Finished)
        Finish();
}

void EWAN::App::Run()
{
    if(!Init())
    {
        Log::Raw("Initialization failed");
        return;
    }

    MainLoop();

    Finish();
}

bool EWAN::App::Init()
{
    Log::Raw("Init");
    Finished = false;

    Log::Raw("GameInfo...");
    if(!GameInfo.Init())
        return false;

    Log::Raw("Content...");
    if(!Content.Init(GameInfo))
        return false;

    Log::Raw("Window...");
    if(!Window.Init(Content))
        return false;

    Log::Raw("Script...");
    if(!Script.Init(this))
        return false;

    return true;
}

void EWAN::App::Finish()
{
    Log::Raw("Finish");
    Finished = true;

    Log::Raw("Script...");
    Script.Finish();

    Log::Raw("Window...");
    Window.Finish();

    Log::Raw("Content...");
    Content.Finish();

    Log::Raw("GameInfo...");
    GameInfo.Finish();
}

//

void EWAN::App::MainLoop()
{
    Log::Raw("BEGIN");

    Window.FPS.Clock.restart();

    bool quit = false;
    while(!Quit)
    {
        if(Window.isOpen())
        {
            quit = !Window.Update(Script);
            Window.Render(Script);
        }

        // always last
        Window.UpdateFPS();

        if(quit)
            break;
    }

    Log::Raw("END");
}

//

namespace EWAN
{
    // Compile time results of sizeof/alignof as errors :)
    template<typename T>
    void info()
    {
        [[maybe_unused]] int (*x)[sizeof(T)][alignof(T)] = -1;
    }

    [[maybe_unused]] static void StaticAssert()
    {
        #if 0
        info<App>();
        info<Content>();
        info<Settings>();
        info<Window>();
        #endif

        #if 0
        info<sf::Clock>();
        info<sf::Text>();
        info<sf::Window>();
        #endif

        #if 0
        static_assert(sizeof(App)       == 1256);
        static_assert(sizeof(Content)   == 400);
        static_assert(sizeof(Settings)  == 8);
        static_assert(sizeof(Window)    == 840);
        #endif
    }
}
