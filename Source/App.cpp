#include "App.hpp"
#include "Log.hpp"
#include "Text.hpp"
#include "Utils.hpp"

#include "Libs/JSON.hpp"

#include <algorithm>
#include <filesystem>

#if __has_include(<format>)
 #include <format>
#endif

EWAN::App::App()
{}

EWAN::App::~App()
{
    if(!Finished)
        Finish();
}

void EWAN::App::Run()
{
    if(Init())
        MainLoop();
    else
        Log::Raw("Initialization failed");

    Finish();
}

bool EWAN::App::Init()
{
    Log::Raw("Init");
    Finished = false;

    Log::Raw("GameInfo...");
    if(!InitGameInfo())
        return false;

    Log::Raw("Window...");
    if(!Window.Init(Content, Settings))
        return false;

    return true;
}

void EWAN::App::Finish()
{
    Log::Raw("Finish");
    Finished = true;

    Log::Raw("Window...");
    Window.Finish();

    if(Content.Size())
    {
        Log::Raw("Content...");
        Content.DeleteAll();
    }

    Content.FontExtensions.clear();
    Content.SoundBufferExtensions.clear();
    Content.TextureExtensions.clear();
}

//

bool EWAN::App::InitGameInfo(const std::string& path /*= "." */, const std::string& id /*= std::string()*/)
{
    if(path.empty())
    {
        Log::Raw("Empty game path");
        return false;
    }
    else if(!std::filesystem::exists(path))
    {
        Log::Raw("Game path does not exists");
        return false;
    }
    else if(!std::filesystem::is_directory(path))
    {
        Log::Raw("Game path is not a directory");
        return false;
    }

    std::vector<std::string> files;

    for(const auto& file : std::filesystem::recursive_directory_iterator(path))
    {
        if(!std::filesystem::is_regular_file(file))
            continue;

        if(Text::ToLower(file.path().extension().string() ) != ".game")
            continue;

        files.emplace_back(std::filesystem::absolute(file.path()).make_preferred().string());

        /*
        scripts.push_back( file.path().string().substr( path.length(), file.path().string().length() - path.length() ) );
        scripts.back().erase( 0, scripts.back().find_first_not_of( "\\/" ) );  // trim left
        */
    }

    if(files.empty())
    {
        Log::Raw("GameInfo not found");
        return false;
    }

    std::sort(files.begin(), files.end());

    for(const auto& filename : files)
    {
        std::string gameName = GameInfo.Path = filename;

        nl::json json;

        if(JSON::ReadJSON(filename, json) && GameInfo.FromJSON(json))
        {
            gameName = GameInfo.Name + " -> " + gameName;

            if(id.empty() || id == GameInfo.Name)
            {
                Log::Raw("GameInfo selected : " + gameName);
                return true;
            }

            Log::Raw("GameInfo skipped : " + gameName);
        }
        else
        {
            if(!GameInfo.Name.empty())
                gameName = GameInfo.Name + " -> " + gameName;
            Log::Raw("GameInfo invalid : " + gameName);
            GameInfo.Clear();
        }
    }

    if(!id.empty())
        Log::Raw("GameInfo not found : " + id);

    return false;
}

bool EWAN::App::LoadContent() 
{
    sf::Clock timer;
    Log::Raw("BEGIN");

    Content.FontExtensions.emplace_back(".bdf");
    Content.FontExtensions.emplace_back(".pcf");
    Content.FontExtensions.emplace_back(".ttf");
    Content.TextureExtensions.emplace_back( ".png" );

    if(!Content.LoadDirectory("Game"))
    {
        Log::Raw("END No content");
        return false;
    }

    #if __has_include(<format>)
    Log::Raw(std::format("END {}ms", timer.getElapsedTime().asMilliseconds()));
    #else
    Log::Raw("END " + std::to_string(timer.getElapsedTime().asMilliseconds()) + "ms");
    #endif

    return true;
}

void EWAN::App::MainLoop()
{
    Log::Raw("BEGIN");

    Window.FPS.Clock.restart();

    bool quit = false;
    while(!Quit)
    {
        if(Window.isOpen())
        {
            quit = !Window.Update();
            Window.Render();
        }

        // always last
        Window.UpdateFPS();

        if(quit)
            break;
    }

    Log::Raw("END");
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

        #if 0
        static_assert(sizeof(App)       == 1256);
        static_assert(sizeof(Content)   == 400);
        static_assert(sizeof(Settings)  == 8);
        static_assert(sizeof(Window)    == 840);
        #endif
    }
}
