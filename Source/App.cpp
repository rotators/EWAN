#include "App.hpp"
#include "Log.hpp"

#include <type_traits> // std::is_same

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

    Log::PrintInfo("END MAIN LOOP");
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

        static_assert(std::is_same<int8_t, sf::Int8>::value);
        static_assert(std::is_same<int16_t, sf::Int16>::value);
        static_assert(std::is_same<int32_t, sf::Int32>::value);
        // static_assert(std::is_same<int64_t, sf::Int64>::value);

        static_assert(std::is_same<uint8_t, sf::Uint8>::value);
        static_assert(std::is_same<uint16_t, sf::Uint16>::value);
        static_assert(std::is_same<uint32_t, sf::Uint32>::value);
        // static_assert(std::is_same<uint64_t, sf::Uint64>::value);

        static_assert(std::is_same<uint8_t, as::asBYTE>::value);
        static_assert(std::is_same<uint32_t, as::asUINT>::value);
    }
}
