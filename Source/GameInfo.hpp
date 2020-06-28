#pragma once

#include "Libs/JSON.hpp"

#include <string>
#include <vector>

namespace EWAN
{
    class GameInfo : public JSON
    {
    public:
        std::string Path;

        std::string Name;
        std::string Type;
        std::string ScriptInit;

        void Clear(bool full = false);

    public:
        virtual nl::json ToJSON() override;
        virtual bool     FromJSON(const nl::json& json) override;
    };
}
