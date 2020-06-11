#pragma once

#include <string>
#include <vector>

namespace EWAN
{
    class Text
    {
    public:
        static bool IsBlank(const std::string& text);

        static std::string Replace(const std::string& text, const std::string& from, const std::string& to);
        static std::vector<std::string> Split(const std::string& text, const char& separator);
        static std::string Join(const std::vector<std::string>& text, const std::string& delimeter);
        static std::string Trim(const std::string& text);

        static std::string ToLower(const std::string& text);
        static std::string ToUpper(const std::string& text);

        static std::string ConvertPath(const std::string& text);
    };
}
