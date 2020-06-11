#include "App.hpp"
#include "Log.hpp"

#include <cstdlib>
#include <cstdio>
#include <string>

#if __has_include(<format>)
 #include <format>
#endif

auto main() -> int
{
    #if __has_include(<format>)
    EWAN::Log::Raw("VERSION {}.{}.{}.{} SFML {}.{}.{}", EWAN_VERSION_MAJOR, EWAN_VERSION_MINOR, EWAN_VERSION_PATCH, EWAN_VERSION_TWEAK, SFML_VERSION_MAJOR, SFML_VERSION_MINOR, SFML_VERSION_PATCH);
    #else
    EWAN::Log::Raw("VERSION " + std::to_string(EWAN_VERSION_MAJOR) + "." + std::to_string(EWAN_VERSION_MINOR) + "." + std::to_string(EWAN_VERSION_PATCH) + "." + std::to_string(EWAN_VERSION_TWEAK) + " SFML " + std::to_string(SFML_VERSION_MAJOR) + "." + std::to_string(SFML_VERSION_MINOR) + "." + std::to_string(SFML_VERSION_PATCH));
    #endif

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
