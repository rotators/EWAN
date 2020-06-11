#include <SFML/System.hpp>

#include "Log.hpp"
#include "Text.hpp"

#include <filesystem>
#include <iostream>

static sf::Mutex RawLock;

#if __has_include(<source_location>) || __has_include(<experimental/source_location>)
void EWAN::Log::Raw(std::string_view message, const ns_source_location::source_location& src /*= ns_source_location::source_location::current() */)
{
    sf::Lock lock(RawLock);
    std::cout << "[" << src.file_name() << ":" << src.line() << "][" << src.function_name() << "]";

    if(!message.empty())
        std::cout << " " << message;

    std::cout << std::endl;
}
#else
void EWAN::Log::Raw(std::string_view message)
{
    if(!message.empty())
    {
        sf::Lock lock(RawLock);
        std::cout << message << std::endl;
    }
}
#endif
