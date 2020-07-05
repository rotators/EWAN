#include "GameInfo.hpp"
#include "Log.hpp"
#include "Text.hpp"

#include <filesystem>

static const EWAN::JSON::Schema GameInfoSchema = {
    { "/name",  "string" },
    { "/type?", "string?" },
    { "/script", "object" },
    { "/script/init", "string"}
};

bool EWAN::GameInfo::Init(const std::string& path /*= "." */, const std::string& id /*= {}*/)
{
    if(path.empty())
    {
        Log::Raw("Empty game path");
        return false;
    }
    else if(!std::filesystem::exists(path))
    {
        Log::Raw("Game path does not exists");
        return false;
    }
    else if(!std::filesystem::is_directory(path))
    {
        Log::Raw("Game path is not a directory");
        return false;
    }

    std::vector<std::string> files;

    for(const auto& file : std::filesystem::recursive_directory_iterator(path))
    {
        if(!std::filesystem::is_regular_file(file))
            continue;

        if(Text::ToLower(file.path().extension().string() ) != ".game")
            continue;

        files.emplace_back(std::filesystem::absolute(file.path()).make_preferred().string());
    }

    if(files.empty())
    {
        Log::Raw("GameInfo not found");
        return false;
    }

    std::sort(files.begin(), files.end());

    for(const auto& filename : files)
    {
        Clear(true);
        Path = filename;

        nl::json json;

        if(JSON::ReadJSON(filename, json) && FromJSON(json))
        {
            if(id.empty() || id == Name)
            {
                Log::PrintInfo("Game manifest filename... " + Path + " (" + Name + ")");
                return true;
            }

            Log::Raw("GameInfo skipped : " + Path);
        }
        else
        {
            Log::Raw("GameInfo invalid : " + Path);
        }
    }

    if(!id.empty())
        Log::Raw("GameInfo not found : " + id);

    return false;
}

void EWAN::GameInfo::Finish()
{
    Clear(true);
}

//

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

    return true;
}
