#include "Text.hpp"

#include <algorithm>
#include <cctype>
#include <regex>
#include <sstream>

static std::regex reIsBlank("^[\\t\\ ]*$");

bool EWAN::Text::IsBlank(const std::string& text)
{
    return std::regex_match(text, reIsBlank);
}

std::string EWAN::Text::Replace(const std::string& text, const std::string& from, const std::string& to)
{
    std::string result;

    std::string::const_iterator current = text.begin();
    std::string::const_iterator end     = text.end();
    std::string::const_iterator next    = std::search(current, end, from.begin(), from.end());

    while(next != end)
    {
        result.append(current, next);
        result.append(to);
        current = next + from.size();
        next    = std::search(current, end, from.begin(), from.end());
    }

    result.append(current, next);

    return result;
}

std::vector<std::string> EWAN::Text::Split(const std::string& text, const char& separator)
{
    std::vector<std::string> result;

    if(!text.empty())
    {
        std::string        tmp;
        std::istringstream f(text);
        while(std::getline(f, tmp, separator))
        {
            if(separator != ' ')
                tmp = Trim(tmp);

            result.push_back(tmp);
        }
    }

    return result;
}

std::string EWAN::Text::Join(const std::list<std::string>& text, const std::string& delimeter)
{
    static const std::string empty;
    switch(text.size())
    {
        case 0:
            return empty;
        case 1:
            return text.front();
        default:
            std::ostringstream oss;
            std::copy(text.begin(), --text.end(), std::ostream_iterator<std::string>(oss, delimeter.c_str()));
            oss << *text.rbegin();
            return oss.str();
    }
}

std::string EWAN::Text::Join(const std::vector<std::string>& text, const std::string& delimeter)
{
    static const std::string empty;

    switch(text.size())
    {
        case 0:
            return empty;
        case 1:
            return text[0];
        default:
            std::ostringstream oss;
            std::copy(text.begin(), --text.end(), std::ostream_iterator<std::string>(oss, delimeter.c_str()));
            oss << *text.rbegin();
            return oss.str();
    }
}

std::string EWAN::Text::Trim(const std::string& text)
{
    return TrimLeft(TrimRight(text));
}

std::string EWAN::Text::TrimLeft(const std::string& text)
{
    std::string result = text;

    result.erase(result.begin(), std::find_if(result.begin(), result.end(), [](int ch) { return !std::isspace(ch); }));

    return result;
}

std::string EWAN::Text::TrimRight(const std::string& text)
{
    std::string result = text;

    result.erase(std::find_if(result.rbegin(), result.rend(), [](int ch) { return !std::isspace(ch); }).base(), result.end());

    return result;
}

std::string EWAN::Text::ToLower(const std::string& text)
{
    std::string result = text;

    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) { return std::tolower(c); });

    return result;
}

std::string EWAN::Text::ToUpper(const std::string& text)
{
    std::string result = text;

    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) { return std::toupper(c); });

    return result;
}

std::string EWAN::Text::ConvertPath(const std::string& text)
{
    return ToLower(Replace(text, "\\", "/"));
}
