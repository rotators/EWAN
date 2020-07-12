#pragma once

#include <string>
#include <vector>

//

#if __has_include(<source_location>)
#    include <source_location>
namespace ns_source_location = std;
#elif __has_include(<experimental/source_location>)
#    include <experimental/source_location>
namespace ns_source_location = std::experimental;
#endif

//

namespace EWAN
{
    class Log
    {
    public:
#if __has_include(<source_location>) || __has_include(<experimental/source_location>)
        static void Raw(const std::string& message, const ns_source_location::source_location& src = ns_source_location::source_location::current());
#else
        static void Raw(const std::string& message);
#endif

        static void PrintInfo(const std::string& message);
        static void PrintWarning(const std::string& message);
        static void PrintError(const std::string& message);
    };
}
