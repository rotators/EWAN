#include "Log.hpp"

#include "Text.hpp"

#include <SFML/System.hpp>

#include <filesystem>
#include <iostream>

static sf::Mutex COutLock;

#if __has_include(<source_location>) || __has_include(<experimental/source_location>)
void EWAN::Log::Raw(std::string_view message, const ns_source_location::source_location& src /*= ns_source_location::source_location::current() */)
{
    sf::Lock lock(COutLock);
    std::cout << "[" << src.file_name() << ":" << src.line() << "][" << src.function_name() << "]";

    if(!message.empty())
        std::cout << " " << message;

    std::cout << std::endl;
}
#else
void EWAN::Log::Raw(std::string_view message)
{
    sf::Lock lock(COutLock);
    std::cout << "[???] " << (message.front() != '[' ? " " : "") << message << std::endl;
}
#endif

void EWAN::Log::PrintInfo(std::string_view message)
{
    if(!message.empty())
    {
        sf::Lock lock(COutLock);
        std::cout << "[INFO]" << (message.front() != '[' ? " " : "") << message << std::endl;
    }
}

void EWAN::Log::PrintWarning(std::string_view message)
{
    if(!message.empty())
    {
        sf::Lock lock(COutLock);
        std::cout << "[WARNING]" << (message.front() != '[' ? " " : "") << message << std::endl;
    }
}

void EWAN::Log::PrintError(std::string_view message)
{
    if(!message.empty())
    {
        sf::Lock lock(COutLock);
        std::cout << "[ERROR]" << (message.front() != '[' ? " " : "") << message << std::endl;
    }
}
