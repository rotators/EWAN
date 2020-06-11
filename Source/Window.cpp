#include "Log.hpp"
#include "Window.hpp"

#include <algorithm>

EWAN::Window::Window() :
    sf::RenderWindow()
{}

EWAN::Window::Window(sf::VideoMode mode, const sf::String& title, sf::Uint32 style /*= sf::Style::Default */, const sf::ContextSettings& settings /*= sf::ContextSettings() */) :
    sf::RenderWindow(mode, title, style, settings)
{}

//

void EWAN::Window::Init([[maybe_unused]] const EWAN::Content& content, const EWAN::Settings& settings)
{
    #if __has_include("Embed.hpp")
    FPS.Text.setFont(*content.Font.Get("*embed/iii/monospace-typewriter.ttf"));
    FPS.Text.setCharacterSize(14);
    #endif

    sf::ContextSettings ctxSettings;

    create(sf::VideoMode(0, 0), std::string(), sf::Style::None, ctxSettings);
    setActive(true);

    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();

    #if __has_include(<format>)
    Log::Raw(std::format(" desktop = {}x{}", desktop.width, desktop.height));
    Log::Raw(std::format(" window  = {}x{} @ {},{}", settings.WindowWidth, settings.WindowHeight, settings.WindowLeft, settings.WindowTop))
    #else
    Log::Raw(" desktop = " + std::to_string(desktop.width) + "x" + std::to_string(desktop.height));
    Log::Raw(" window  = " + std::to_string(settings.WindowWidth) + "x" + std::to_string(settings.WindowHeight) + " @ " + std::to_string(settings.WindowLeft) + "," + std::to_string(settings.WindowTop));
    #endif

    setPosition(sf::Vector2i(settings.WindowLeft, settings.WindowTop));

    if(settings.WindowWidth && settings.WindowHeight)
        setSize(sf::Vector2u(settings.WindowWidth, settings.WindowHeight));
    else
        setSize(sf::Vector2u(desktop.width, desktop.height));

    Log::Raw("sf::Texture::getMaximumSize()=" + std::to_string(sf::Texture::getMaximumSize()));
}

void EWAN::Window::Finish()
{
    if(isOpen())
        close();
}

//

bool EWAN::Window::DrawSprite(const Content& content, const std::string& id)
{
    sf::Sprite* sprite = content.Sprite.Get(id);

    if(sprite)
    {
        draw(*sprite);
        return true;
    }

    return false;
}

bool EWAN::Window::Update()
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

void EWAN::Window::Render()
{
    clear();

    // always last

    #if __has_include("Embed.hpp")
    //if(FPS.Visible)
    {
        FPS.Text.setString(std::to_string(FPS.Min) + " " + std::to_string(FPS.Count) + " " + std::to_string(FPS.Max));
        draw(FPS.Text);
    }
    #endif

    display();
}
