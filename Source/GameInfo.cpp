#include "GameInfo.hpp"
#include "Log.hpp"

#include <map>

static const EWAN::JSON::Schema GameInfoSchema =
{
    { "/name", "string" },
    { "/type?", "string?" },
};

void EWAN::GameInfo::Clear()
{
    Path.clear();
    Name.clear();
}

nl::json EWAN::GameInfo::ToJSON()
{
    nl::json json =
    {
        { "name", Name },
        { "type", Type }
    };

    return json;
}

bool EWAN::GameInfo::FromJSON(const nl::json& json)
{
    if(!ValidateJSON(json, GameInfoSchema))
        return false;

    Clear();

    JSON::FromJSON(json, "/name", Name);
    JSON::FromJSON(json, "/type", Type, true);

    Log::Raw(ToJSON().dump());

    return true;
}
