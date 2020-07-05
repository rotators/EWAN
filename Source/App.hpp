#pragma once

#include "Content.hpp"
#include "GameInfo.hpp"
#include "Script.hpp"
#include "Window.hpp"

#include <cstdint>

namespace EWAN
{
    class App
    {
    public:
        const uint8_t Version[4];

        bool    Finished = false;
        bool    Quit     = false;
        uint8_t Unused0[2];

        EWAN::Content  Content;
        EWAN::GameInfo GameInfo;
        EWAN::Script   Script;
        EWAN::Window   Window;

    public:
        App();
        virtual ~App();

        void Run();
        bool Init();
        void Finish();

        void MainLoop();
    };
}
