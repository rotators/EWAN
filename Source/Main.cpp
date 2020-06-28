#include "App.hpp"
#include "Log.hpp"

#include <cstdlib>
#include <cstdio>

auto main() -> int
{
    // Disable console buffering
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    std::setvbuf(stderr, nullptr, _IONBF, 0);

    EWAN::Log::Raw("BEGIN");

    EWAN::App* app = new EWAN::App();

    EWAN::Log::Raw("RUN");
    app->Run();

    EWAN::Log::Raw("DELETE");
    delete app;
    app = nullptr;

    EWAN::Log::Raw("END");

    return EXIT_SUCCESS;
}
