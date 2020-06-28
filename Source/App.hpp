#pragma once

#include "Content.hpp"
#include "GameInfo.hpp"
#include "Script.hpp"
#include "Settings.hpp"
#include "Window.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace EWAN
{
    class App
    {
    public:
        bool Finished = false;
        bool Quit = false;
        uint8_t Unused0[2];

        EWAN::Content  Content;
        EWAN::GameInfo GameInfo;
        EWAN::Script   Script;
        EWAN::Settings Settings;
        EWAN::Window   Window;

    public:
        App();
        virtual ~App();

        void Run();
        bool Init();
        void Finish();

        bool InitGameInfo(const std::string& path = ".", const std::string& id = {});
        bool LoadContent();
        void MainLoop();
        void ProcessEvents();
        void ProcessRender();
    };
}
