#include "App.hpp"
#include "Log.hpp"
#include "Text.hpp"

#if __has_include("Embed.hpp")
 #include "Embed.hpp"
#endif

#if __has_include(<format>)
 #include <format>
#endif

EWAN::App::App()
{}

EWAN::App::~App()
{
    Finish();
}

void EWAN::App::Run()
{
    Log::Raw("BEGIN");

    Init();

    LoadContent();
    MainLoop();

    Finish();

    Log::Raw("END");
}

void EWAN::App::Init()
{
    sf::Clock timer;
    Log::Raw("BEGIN");

    #if __has_include("Embed.hpp")
    InitEmbed(Content);
    #endif

    Log::Raw("Window");
    Window.Init(Content, Settings);

    #if __has_include(<format>)
    Log::Raw(std::format("END {}ms", timer.getElapsedTime().asMilliseconds()));
    #else
    Log::Raw("END " + std::to_string(timer.getElapsedTime().asMilliseconds()) + "ms");
    #endif
}

void EWAN::App::Finish()
{
    Window.Finish();

    if(Content.Size())
    {
        Log::Raw("Content");
        Content.DeleteAll();
    }

    Content.FontExtensions.clear();
    Content.SoundBufferExtensions.clear();
    Content.TextureExtensions.clear();
}

bool EWAN::App::LoadContent() 
{
    sf::Clock timer;
    Log::Raw("BEGIN");

    Content.FontExtensions.emplace_back(".bdf");
    Content.FontExtensions.emplace_back(".pcf");
    Content.FontExtensions.emplace_back(".ttf");
    Content.TextureExtensions.emplace_back( ".png" );

    if(!Content.LoadDirectory("Game"))
    {
        Log::Raw("END No content");
        return false;
    }

    #if __has_include(<format>)
    Log::Raw(std::format("END {}ms", timer.getElapsedTime().asMilliseconds()));
    #else
    Log::Raw("END " + std::to_string(timer.getElapsedTime().asMilliseconds()) + "ms");
    #endif

    return true;
}

void EWAN::App::MainLoop()
{
    Log::Raw("BEGIN");

    Window.FPS.Clock.restart();

    bool quit = false;
    while(!Quit)
    {
        if(Window.isOpen())
        {
            quit = !Window.Update();
            Window.Render();
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
    // Compile time results of sizeof/alignof
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
