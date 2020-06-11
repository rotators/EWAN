#pragma once

#include "Content.hpp"
#include "Settings.hpp"
#include "Window.hpp"

#include "SFML.hpp"

#include <cstdint>
#include <string>

namespace EWAN
{
    class App
    {
    public:
        #if __has_include("Embed.hpp")
        static constexpr bool Embed = true;
        #else
        static constexpr bool Embed = false;
        #endif

        bool Quit = false;
        uint8_t Unused0[3];

        EWAN::Settings Settings;

        EWAN::Window    Window;
        EWAN::Content   Content;

    public:
        App();
        virtual ~App();

        void Run();
        void Init();
        void Finish();

        bool LoadContent();
        void MainLoop();
        void ProcessEvents();
        void ProcessRender();
    };
}
