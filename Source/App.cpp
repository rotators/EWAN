#include "App.hpp"

#include "Log.hpp"

#include <type_traits> // std::is_same_v

#if __has_include(<format>)
    #include <format>
#endif

EWAN::App::App() :
    Version {PROJECT_VERSION_MAJOR, PROJECT_VERSION_MINOR, PROJECT_VERSION_PATCH, PROJECT_VERSION_TWEAK}
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
        Log::PrintError("Initialization failed");
        return;
    }

    MainLoop();

    Finish();
}

bool EWAN::App::Init()
{
    Finished = false;

    if(!GameInfo.Init())
        return false;

    if(!Content.Init(GameInfo))
        return false;

    if(!Window.Init(Content))
        return false;

    if(!Script.Init(this))
        return false;

    return true;
}

void EWAN::App::Finish()
{
    Finished = true;

    Script.Finish();
    Window.Finish();
    Content.Finish();
    GameInfo.Finish();
}

//

void EWAN::App::MainLoop()
{
    Log::PrintInfo("BEGIN MAIN LOOP");

    Window.FPS.ClockFPS.restart();
    Window.FPS.ClockFrameTime.restart();

    while(!Quit && !Restart)
    {
        if(Window.isOpen())
        {
            Window.Update(this);
            Window.Render(&Script);
        }

        // always last
        Window.UpdateFPS();
    }

    Log::PrintInfo("END MAIN LOOP");
}

//

namespace
{
    // Results of sizeof/alignof as compile-time errors :)
    template<typename T>
    void info()
    {
        [[maybe_unused]] int(*x)[sizeof(T)][alignof(T)] = -1;
    }

    [[maybe_unused]] static void StaticAssert()
    {
#if 0
        info<EWAN::App>();
        info<EWAN::Content>();
        info<EWAN::Script>();
        info<EWAN::Window>();
#endif

#if 0
        info<sf::Clock>();
        info<sf::Text>();
        info<sf::Window>();
#endif

        static_assert(std::is_same_v<int8_t, sf::Int8>);
        static_assert(std::is_same_v<int16_t, sf::Int16>);
        static_assert(std::is_same_v<int32_t, sf::Int32>);
        // static_assert(std::is_same_v<int64_t, sf::Int64>);

        static_assert(std::is_same_v<uint8_t, sf::Uint8>);
        static_assert(std::is_same_v<uint16_t, sf::Uint16>);
        static_assert(std::is_same_v<uint32_t, sf::Uint32>);
        // static_assert(std::is_same_v<uint64_t, sf::Uint64>);

        static_assert(std::is_same_v<uint8_t, as::asBYTE>);
        static_assert(std::is_same_v<uint32_t, as::asUINT>);
    }
}
