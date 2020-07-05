#include "Libs/JSON.hpp"

#include "Log.hpp"
#include "Utils.hpp"

bool EWAN::JSON::ReadJSON(const std::string& filename, nl::json& json)
{
    std::string content;
    if(!Utils::ReadFile(filename, content) || content.empty())
        return false;

    try
    {
        json = nl::json::parse(content);
    }
    catch(nl::json::parse_error& e)
    {
        Log::Raw(e.what());

        return false;
    }

    return true;
}

bool EWAN::JSON::ValidateJSON(const nl::json& json, const EWAN::JSON::Schema& schema)
{
    for(const auto& part : schema)
    {
        std::string name = part.first, type = part.second;

        bool optional = false;

        if(name.empty() || type.empty())
        {
            Log::Raw("Invalid schema : empty " + std::string(name.empty() ? "name" : "type"));
            return false;
        }

        if(name.back() == '?')
        {
            name.pop_back();
            optional = true;
        }

        nl::json::json_pointer ptr(name);
        if(!json.contains(ptr))
        {
            if(optional)
            {
                Log::Raw("Incomplete json : '" + name + "' missing (optional)");
                continue;
            }

            Log::Raw("Invalid json : '" + name + "' missing (required)");
            return false;
        }

        auto value = json.at(ptr);
        bool ok    = true;
        optional   = false;
        if(type.back() == '?')
        {
            type.pop_back();
            optional = true;
        }

        if(type == "bool")
            ok = value.is_boolean();
        else if(type == "int")
            ok = value.is_number_integer();
        else if(type == "uint")
            ok = value.is_number_unsigned();
        else if(type == "float")
            ok = value.is_number_float();
        else if(type == "string")
            ok = value.is_string();
        else if(type == "array") // TODO "type[]"
            ok = value.is_array();
        else if(type == "object")
            ok = value.is_object();
        else
        {
            Log::Raw("Invalid schema : unknown type '" + type + "'");
            return false;
        }

        if(!ok && optional)
            ok = value.is_null();

        if(!ok)
        {
            Log::Raw("Invalid json : '" + name + "' != '" + type + "'" + (optional ? " || 'null" : ""));
            return false;
        }
    }

    return true;
}
