#include "GameInfo.hpp"
#include "Log.hpp"

static const EWAN::JSON::Schema GameInfoSchema = {
    { "/name",  "string" },
    { "/type?", "string?" },
    { "/script", "object" },
    { "/script/init", "string"}
};

void EWAN::GameInfo::Clear(bool full /*= false */)
{
    if(full)
        Path.clear();

    Name.clear();
    Type.clear();
    ScriptInit.clear();
}

nl::json EWAN::GameInfo::ToJSON()
{
    nl::json json = {
        { "name", Name },
        { "type", Type },
        { "script",
            { "init", ScriptInit }
        }
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
    JSON::FromJSON(json, "/script/init", ScriptInit);

    Log::Raw(ToJSON().dump());

    return true;
}
