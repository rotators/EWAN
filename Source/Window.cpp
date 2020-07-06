#include "Window.hpp"

#include "Log.hpp"
#include "Script.hpp"

#include "Embed.hpp"

#include <algorithm> // std::clamp

EWAN::Window::Window() :
    sf::RenderWindow()
{}

EWAN::Window::~Window()
{
    Finish();
}

//

bool EWAN::Window::Init(EWAN::Content& content)
{
    content.Font.NewAs<sf::Font>("*embed/Window/FPS")->loadFromMemory(&Embed::Font::Monospace_Typewriter_ttf, Embed::Font::Monospace_Typewriter_ttf_l);
    FPS.Text.setFont(*content.Font.GetAs<sf::Font>("*embed/Window/FPS"));
    FPS.Text.setCharacterSize(14);

    return true;
}

void EWAN::Window::Finish()
{
    if(isOpen())
    {
        Log::PrintInfo("Window finalization...");

        close();

        Log::PrintInfo("Window finalization complete");
    }
}

//

void EWAN::Window::Open_Call(sf::Uint32 width /*= 0 */, sf::Uint32 height /*= 0 */, sf::Uint32 bitsPerPixel /*= 0 */, const std::string& title /*= {} */, sf::Uint32 style /*= sf::Style::Default */)
{
    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();

    // Use desktop resolution if width and height isn't set
    if(!width && !height)
    {
        width  = desktop.width;
        height = desktop.height;
    }
    else
    {
        width  = std::clamp<sf::Uint32>(width, 0, desktop.width);
        height = std::clamp<sf::Uint32>(height, 0, desktop.height);
    }

    if(!bitsPerPixel)
        bitsPerPixel = desktop.bitsPerPixel;
    else
        bitsPerPixel = std::clamp<sf::Uint32>(bitsPerPixel, 0, desktop.bitsPerPixel);

    // Center window by default
    const sf::Uint32 x = (desktop.width - width) / 2, y = (desktop.height - height) / 2;

    Log::Raw(std::to_string(width) + "x" + std::to_string(height) + ":" + std::to_string(bitsPerPixel) + " @ " + std::to_string(x) + "," + std::to_string(y));

    create(sf::VideoMode(0, 0, bitsPerPixel), title, style);
    setActive(true);

    setPosition(sf::Vector2i(x, y));
    setSize(sf::Vector2u(width, height));
}

//

bool EWAN::Window::DrawSprite(const Content& content, const std::string& id)
{
    sf::Sprite* sprite = content.Sprite.GetAs<sf::Sprite>(id);

    if(sprite)
    {
        draw(*sprite);
        return true;
    }

    return false;
}

bool EWAN::Window::Update(Script& script)
{
    bool result = true;

    sf::Event event;
    while(pollEvent(event))
    {
        if(event.type == sf::Event::Closed)
        {
            Log::Raw("Event::Closed");
            result = false;
        }
        else if(event.type == sf::Event::KeyPressed)
        {
            script.Event.Run(script.Event.OnKeyDown);

            if(event.key.shift && event.key.code == sf::Keyboard::Escape)
            {
                Log::Raw("Event::Keypressed Shift+Escape");
                result = false;
            }
        }
        else if(event.type == sf::Event::Resized)
        {
            sf::FloatRect visibleArea(0.0f, 0.0f, static_cast<float>(event.size.width), static_cast<float>(event.size.height));
            setView(sf::View(visibleArea));
        }
    }

    return result;
}

void EWAN::Window::UpdateFPS()
{
    if(FPS.Clock.getElapsedTime().asSeconds() >= 1.0f)
    {
        FPS.Clock.restart();
        FPS.Count = FPS.Frame;
        FPS.Frame = 0;

        FPS.Min = std::min(FPS.Count, FPS.Min);
        FPS.Max = std::max(FPS.Count, FPS.Max);
    }

    ++FPS.Frame;
}

void EWAN::Window::Render(Script& script)
{
    clear();

    float frameTime = FPS.FrameTime.restart().asSeconds();

    script.Event.Run(script.Event.OnDraw, frameTime);

    // always last

    if(FPS.Visible && FPS.Min != std::numeric_limits<uint16_t>::max())
    {
        FPS.Text.setString(std::to_string(FPS.Min) + " " + std::to_string(FPS.Count) + " " + std::to_string(FPS.Max) + " " + std::to_string(elapsed));
        draw(FPS.Text);
    }

    display();
}
