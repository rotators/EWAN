#include "Log.hpp"
#include "Script.hpp"

EWAN::Script::Builder::Builder() :
    as::CScriptBuilder()
{
    SetIncludeCallback(Callback::Include, nullptr); // redirects to Script::CallbackInclude
    SetPragmaCallback(Callback::Pragma, nullptr);   // redirects to Script::CallbackPragma
}

int EWAN::Script::Builder::StartNewModule(as::asIScriptEngine* inEngine, const char* moduleName)
{
    int r = as::CScriptBuilder::StartNewModule(inEngine, moduleName);
    if(r < 0)
        return r;

    module->SetUserData(new UserData::Module, 0);

    return 0;
}

std::map<int, std::vector<std::string>>& EWAN::Script::Builder::GetAllMetadataForType()
{
    return typeMetadataMap;
}

std::map<int, std::vector<std::string>>& EWAN::Script::Builder::GetAllMetadataForFunc()
{
    return funcMetadataMap;
}

std::map<int, std::vector<std::string>>& EWAN::Script::Builder::GetAllMetadataForVar()
{
    return varMetadataMap;
}

// From AngelScript scriptbuilder.cpp GetAbsolutePath()
std::string EWAN::Script::Builder::NormalizePath(const std::string& path)
{
    std::string str = path;

    // Replace backslashes for forward slashes
    size_t pos = 0;
    while((pos = str.find("\\", pos)) != std::string::npos)
        str[pos] = '/';

    // Replace /./ with /
    pos = 0;
    while((pos = str.find("/./", pos)) != std::string::npos)
        str.erase(pos + 1, 2);

    // For each /../ remove the parent dir and the /../
    pos = 0;
    while((pos = str.find("/../")) != std::string::npos)
    {
        size_t pos2 = str.rfind("/", pos - 1);
        if(pos2 != std::string::npos)
            str.erase(pos2, pos + 3 - pos2);
        else
        {
            // The path is invalid
            break;
        }
    }

    return str;
}
