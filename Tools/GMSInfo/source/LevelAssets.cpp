#include <LevelAssets.h>
#include <algorithm>

#include <spdlog/spdlog.h>

namespace ReGlacier
{
    bool LevelAssets::AllResolved() const
    {
        return
                !ANM.empty() && !BUF.empty() && !GMS.empty() && !LOC.empty() &&
                !MAT.empty() && !OCT.empty() && !PRP.empty() && !PRM.empty() &&
                !RMC.empty() && !RMI.empty() && !SGD.empty() && !SGP.empty() &&
                !SND.empty() && !SUP.empty() && !TEX.empty() && !ZGF.empty();
    }

    void LevelAssets::TryResolve(const std::string& path)
    {
        std::string nameInLowerCase = path;
        std::for_each(std::begin(nameInLowerCase), std::end(nameInLowerCase), [](char& c) { c = static_cast<char>(std::tolower(c)); });

        if      (nameInLowerCase.ends_with("anm")) ANM = path;
        else if (nameInLowerCase.ends_with("buf")) BUF = path;
        else if (nameInLowerCase.ends_with("gms")) GMS = path;
        else if (nameInLowerCase.ends_with("loc")) LOC = path;
        else if (nameInLowerCase.ends_with("mat")) MAT = path;
        else if (nameInLowerCase.ends_with("oct")) OCT = path;
        else if (nameInLowerCase.ends_with("prp")) PRP = path;
        else if (nameInLowerCase.ends_with("prm")) PRM = path;
        else if (nameInLowerCase.ends_with("rmc")) RMC = path;
        else if (nameInLowerCase.ends_with("rmi")) RMI = path;
        else if (nameInLowerCase.ends_with("sgd")) SGD = path;
        else if (nameInLowerCase.ends_with("sgp")) SGP = path;
        else if (nameInLowerCase.ends_with("snd")) SND = path;
        else if (nameInLowerCase.ends_with("sup")) SUP = path;
        else if (nameInLowerCase.ends_with("tex")) TEX = path;
        else if (nameInLowerCase.ends_with("zgf")) ZGF = path;
        else spdlog::warn("Assets::TryResolve| File {} ignored", path);
    }
}