#pragma once

#if defined(__GNUC__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wpadded"
    #pragma GCC diagnostic ignored "-Wuseless-cast"
#endif

#include <nlohmann/json.hpp>

#if defined(__GNUC__)
    #pragma GCC diagnostic pop
#endif

#include <string>
#include <unordered_map>

namespace nl = nlohmann;

namespace EWAN
{
    class JSON
    {
    public:
        typedef std::unordered_map<std::string, std::string> Schema;

    public:
        virtual nl::json ToJSON()                       = 0;
        virtual bool     FromJSON(const nl::json& json) = 0;

        template<typename T>
        void FromJSON(const nl::json& json, const char* key, T& target, bool optional = false)
        {
            JSON::FromJSON(json, std::string(key), target, optional);
        }

        template<typename T>
        void FromJSON(const nl::json& json, const std::string& key, T& target, bool optional = false)
        {
            nl::json::json_pointer ptr(key);

            if(optional && !json.contains(ptr))
                return;

            if(json.at(ptr).is_null())
                return;

            json.at(ptr).get_to(target);
        }

    public:
        static bool ReadJSON(const std::string& filename, nl::json& json);
        static bool ValidateJSON(const nl::json& json, const JSON::Schema& schema);
    };
}