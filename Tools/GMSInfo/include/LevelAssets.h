#pragma once

#include <string>

namespace ReGlacier
{
    struct LevelAssets
    {
        std::string ANM;
        std::string BUF;
        std::string GMS;
        std::string LOC;
        std::string MAT;
        std::string OCT;
        std::string PRP;
        std::string PRM;
        std::string RMC;
        std::string RMI;
        std::string SGD;
        std::string SGP;
        std::string SND;
        std::string SUP;
        std::string TEX;
        std::string ZGF;

        [[nodiscard]] bool AllResolved() const;
        void TryResolve(const std::string& path);
    };
}