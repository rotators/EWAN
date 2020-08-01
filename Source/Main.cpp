#include "App.hpp"
#include "Log.hpp"

#include <cstdio>
#include <cstdlib>

auto main() -> int
{
    // Disable console buffering
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    std::setvbuf(stderr, nullptr, _IONBF, 0);

    bool restart = true;
    while(restart)
    {
        EWAN::Log::PrintInfo("BEGIN PROGRAM");

        EWAN::App* app = new EWAN::App;

        app->Run();
        restart = app->Restart;

        EWAN::Log::PrintInfo("END PROGRAM");
        delete app;
        app = nullptr;

        if(restart)
            EWAN::Log::PrintInfo("RESTART PROGRAM");
    }

    EWAN::Log::PrintInfo("GOODBYE");

    return EXIT_SUCCESS;
}
