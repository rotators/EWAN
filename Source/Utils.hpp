#pragma once

#include <fstream>
#include <string>
#include <vector>

namespace EWAN
{
    struct Utils
    {
        static bool ReadFile(const std::string& filename, std::ifstream& fstream);
        static bool ReadFile(const std::string& filename, std::string& content);
        static bool ReadFile(const std::string& filename, std::vector<std::string>& content);
    };
}
