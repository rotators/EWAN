#pragma once

#include "Content.hpp"

#include "Libs/SFML.hpp"

#include <cstdint>
#include <limits>
#include <string>

namespace EWAN
{
    class App;
    class Script;

    class Window : public sf::RenderWindow
    {
    public:
        struct FPS
        {
            sf::Clock ClockFPS;
            sf::Clock ClockFrameTime;
            sf::Text  Text;

            float FrameTime = 0.0f;

            uint16_t Count = 0;
            uint16_t Min   = std::numeric_limits<uint16_t>::max();
            uint16_t Max   = 0;
            uint16_t Frame = 0;

            bool Visible = true;
        };

        EWAN::Window::FPS FPS;

        //        uint8_t Unused[4];

    public:
        Window();
        virtual ~Window();

    public:
        bool Init(Content& content);
        void Finish();

        void Open(sf::Uint32 width = 0, sf::Uint32 height = 0, sf::Uint32 bitsPerPixel = 32, const std::string& title = {}, sf::Uint32 style = sf::Style::Default);

        bool Draw(sf::Sprite* sprite);
        bool Draw(const Content& content, const std::string& id);

        void Update(App* app);
        void UpdateFPS();
        void Render(Script* script);
    };
}
