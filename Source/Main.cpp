#include "App.hpp"
#include "Log.hpp"

#include <cstdlib>
#include <cstdio>

auto main() -> int
{
    // Disable console buffering
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    std::setvbuf(stderr, nullptr, _IONBF, 0);

    EWAN::Log::PrintInfo("BEGIN PROGRAM");

    EWAN::App* app = new EWAN::App();

    app->Run();

    EWAN::Log::PrintInfo("END PROGRAM");
    delete app;
    app = nullptr;

    EWAN::Log::PrintInfo("GOODBYE");

    return EXIT_SUCCESS;
}
