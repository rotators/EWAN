#pragma once

#include "Content.hpp"
#include "Script.hpp"
#include "Settings.hpp"

#include "Libs/SFML.hpp"

#include <cstdint>
#include <string>

namespace EWAN
{
    class Window : public sf::RenderWindow
    {
    public:
        struct FPS
        {
            sf::Clock Clock;
            sf::Text  Text;

            uint16_t  Count = 0;
            uint16_t  Min = uint16_t(-1);
            uint16_t  Max = 0;
            uint16_t  Frame = 0;

            bool      Visible = true;
        };

        EWAN::Window::FPS FPS;

//        uint8_t Unused[4];

    public:
        Window();
        Window(sf::VideoMode mode, const sf::String& title, sf::Uint32 style = sf::Style::Default, const sf::ContextSettings& settings = sf::ContextSettings());

    public:
        bool Init(Content& content, const Settings& settings);
        void Finish();

        bool DrawSprite(const Content& content, const std::string& id);

        bool Update(Script& script);
        void UpdateFPS();
        void Render(Script& script);
    };
}
