#include "Log.hpp"

#include "Text.hpp"

#include <SFML/System.hpp>

#include <filesystem>
#include <iostream>

#if defined(_WIN32) || defined(_WIN64)
    #define IS_WINDOWS
    #if defined(__MINGW32__) || defined(__MINGW64__)
        #define IS_MINGW
    #endif
#endif

// Prevent sorting includes
// clang-format off
#if defined(IS_WINDOWS) && !defined(IS_MINGW)

    #include <windows.h>
    #include <debugapi.h>

#endif
// clang-format on

namespace
{
    sf::Mutex PrintLock;

    void PrintInternal(const std::string& prefix, std::string message)
    {
        message = "[" + prefix + "]" + (message.front() != '[' ? " " : "") + message;

        sf::Lock lock(PrintLock);

#if defined(IS_WINDOWS) && !defined(IS_MINGW)

        // Play nice with debuggers known to WINAPI
        if(IsDebuggerPresent())
        {
            message += "\n";
            OutputDebugStringA(message.c_str());
        }
        else

#endif
            std::cout << message << std::endl;
    }
}

#if __has_include(<source_location>) || __has_include(<experimental/source_location>)

void EWAN::Log::Raw(const std::string& message, const ns_source_location::source_location& src /*= ns_source_location::source_location::current() */)
{
    PrintInternal(std::string() + src.file_name() + ":" + std::to_string(src.line()) + "][" + src.function_name(), message);
}

#else

void EWAN::Log::Raw(const std::string& message)
{
    PrintInternal("???", message);
}

#endif

void EWAN::Log::PrintInfo(const std::string& message)
{
    if(!message.empty())
        PrintInternal("INFO", message);
}

void EWAN::Log::PrintWarning(const std::string& message)
{
    if(!message.empty())
        PrintInternal("WARNING", message);
}

void EWAN::Log::PrintError(const std::string& message)
{
    if(!message.empty())
        PrintInternal("ERROR", message);
}
