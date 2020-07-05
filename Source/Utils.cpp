#include "Utils.hpp"

#include "Log.hpp"
#include "Text.hpp"

#include <filesystem>

bool EWAN::Utils::ReadFile(const std::string& filename, std::ifstream& fstream)
{
    fstream.close();
    const std::string file = Text::Replace(filename, "\\", "/");
    if(!std::filesystem::exists(file))
    {
        Log::Raw("File not found : " + std::filesystem::path(file).make_preferred().string());
        return false;
    }

    // don't waste time on empty files
    // also, while(!std::ifstream::eof()) goes wild on empty files
    if(std::filesystem::file_size(file) == 0)
        return true;

    fstream.open(file, std::ios_base::in | std::ios_base::binary);

    bool result = fstream.is_open();
    if(result)
    {
        // skip bom
        char bom[3] = {0, 0, 0};
        fstream.read(bom, sizeof(bom));
        if(bom[0] != static_cast<char>(0xEF) || bom[1] != static_cast<char>(0xBB) || bom[2] != static_cast<char>(0xBF))
            fstream.seekg(0, std::ifstream::beg);
    }
    else
        Log::Raw("File cannot be read : " + std::filesystem::path(file).make_preferred().string());

    return result;
}

bool EWAN::Utils::ReadFile(const std::string& filename, std::string& content)
{
    content.clear();

    std::ifstream fstream;
    if(!ReadFile(filename, fstream))
        return false;

    std::string line;
    while(!fstream.eof())
    {
        std::getline(fstream, line, '\n');

        line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
        line.erase(std::remove(line.begin(), line.end(), '\n'), line.end());

        content += line + "\n";
    }

    return true;
}

bool EWAN::Utils::ReadFile(const std::string& filename, std::vector<std::string>& content)
{
    content.clear();

    std::ifstream fstream;
    if(!ReadFile(filename, fstream))
        return false;

    std::string line;
    while(!fstream.eof())
    {
        std::getline(fstream, line, '\n');

        line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
        line.erase(std::remove(line.begin(), line.end(), '\n'), line.end());

        content.push_back(line);
    }

    return true;
}