#pragma once

#include "Content.hpp"
#include "GameInfo.hpp"
#include "Settings.hpp"
#include "Window.hpp"

#include "Libs/SFML.hpp"

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

        EWAN::Settings Settings;

        EWAN::Window    Window;
        EWAN::GameInfo  GameInfo;
        EWAN::Content   Content;

    public:
        App();
        virtual ~App();

        void Run();
        bool Init();
        void Finish();

        bool InitGameInfo(const std::string& path = ".", const std::string& id = std::string());
        bool LoadContent();
        void MainLoop();
        void ProcessEvents();
        void ProcessRender();

        bool ReadFile(const std::string& filename, std::string& content);
        bool ReadFile(const std::string& filename, std::vector<std::string>& lines);
    };
}
