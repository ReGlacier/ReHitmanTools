#include <LevelDescription.h>
#include <LevelContainer.h>
#include <LevelAssets.h>

#include <GameEntityFactory.h>

#include <ANM/ANM.h>
#include <GMS/GMS.h>
#include <PRM/PRM.h>
#include <TEX/TEX.h>
#include <SND/SND.h>
#include <LOC/LOC.h>

#include <spdlog/spdlog.h>

#include <algorithm>
#include <array>

extern "C" {
#include <unzip.h>
}

namespace ReGlacier
{
    enum IgnoreFlags : int {
        IgnoreGMS = 0,
        IgnoreANM = 1,
        IgnoreLOC = 2,
        IgnorePRM = 3,
        IgnorePRP = 4,
        IgnoreTEX = 5,
        IgnoreSND = 6
    };

    static constexpr size_t kTotalFlags = 7;

    struct LevelDescription::Context
    {
        std::string ArchivePath;
        LevelAssets Assets;
        LevelContainer::Ptr Container;

        ANM::Ptr ANMInstance;
        GMS::Ptr GMSInstance;
        PRM::Ptr PRMInstance;
        TEX::Ptr TEXInstance;
        SND::Ptr SNDInstance;
        LOC::Ptr LOCInstance;

        std::array<bool, kTotalFlags> Flags {};

        unzFile Zip { nullptr };

        Context()
        {
            Flags[IgnoreFlags::IgnoreGMS] = false;
            Flags[IgnoreFlags::IgnoreANM] = false;
            Flags[IgnoreFlags::IgnoreLOC] = false;
            Flags[IgnoreFlags::IgnorePRM] = false;
            Flags[IgnoreFlags::IgnorePRP] = false;
            Flags[IgnoreFlags::IgnoreTEX] = false;
            Flags[IgnoreFlags::IgnoreSND] = false;
        }

        ~Context()
        {
            if (Zip)
            {
                unzClose(Zip);
                Zip = nullptr;
            }
        }
    };

    LevelDescription::LevelDescription(const std::string& pathToLevelArchive)
    {
        m_context = new Context();
        m_context->ArchivePath = pathToLevelArchive;
    }

    LevelDescription::~LevelDescription()
    {
        if (m_context)
        {
            delete m_context;
            m_context = nullptr;
        }
    }

    bool LevelDescription::IsMain() const
    {
        return m_context != nullptr && (m_context->ArchivePath.find("_main") != std::string::npos);
    }

    bool LevelDescription::Open()
    {
        if (!m_context)
        {
            spdlog::error("Unable to open level. Context not inited!");
            return false;
        }

        m_context->Zip = unzOpen(m_context->ArchivePath.c_str());
        if (!m_context->Zip)
        {
            spdlog::error("Unable to open level archive {}", m_context->ArchivePath);
            return false;
        }

        return ValidateLevelArchive();
    }

    void LevelDescription::LoadAndAnalyze()
    {
        if (!m_context)
        {
            spdlog::error("LevelDescription::LoadAndAnalyze| Unable to analyze level. Context not inited!");
            return;
        }

        m_context->Container = std::make_unique<LevelContainer>(m_context->Zip);

        if (!m_context->Flags[IgnoreFlags::IgnoreLOC]) {
            m_context->LOCInstance = GameEntityFactory::Create<LOC>(m_context->Assets.LOC, m_context);
            if (!m_context->LOCInstance->Load())
            {
                spdlog::error("LevelDescription::Analyze| Failed to load LOC to analyze!");
            }
        } else spdlog::info(" * LOC ignored by user");

        if (!m_context->Flags[IgnoreFlags::IgnoreANM]) {
            m_context->ANMInstance = GameEntityFactory::Create<ANM>(m_context->Assets.ANM, m_context);
            if (!m_context->ANMInstance->Load()) {
                spdlog::error("LevelDescription::Analyze| Failed to load ANM to analyze!");
            }
        } else spdlog::info(" * ANM ignored by user");

        if (!m_context->Flags[IgnoreFlags::IgnoreSND]) {
            m_context->SNDInstance = GameEntityFactory::Create<SND>(m_context->Assets.SND, m_context);
            if (!m_context->SNDInstance->Load()) {
                spdlog::error("LevelDescription::Analyze| Failed to load SND to analyze!");
            }
        } else spdlog::info(" * SND ignored by user");

        if (!m_context->Flags[IgnoreFlags::IgnoreTEX]) {
            m_context->TEXInstance = GameEntityFactory::Create<TEX>(m_context->Assets.TEX, m_context);
            if (!m_context->TEXInstance->Load()) {
                spdlog::error("LevelDescription::Analyze| Failed to load TEX to analyze!");
            }
        } else spdlog::info(" * TEX ignored by user");

        if (!m_context->Flags[IgnoreFlags::IgnorePRM]) {
            m_context->PRMInstance = GameEntityFactory::Create<PRM>(m_context->Assets.PRM, m_context);
            if (!m_context->PRMInstance->Load()) {
                spdlog::error("LevelDescription::Analyze| Failed to load PRM to analyze!");
            }
        } else spdlog::info(" * PRM ignored by user");

        if (!m_context->Flags[IgnoreFlags::IgnoreGMS]) {
            m_context->GMSInstance = GameEntityFactory::Create<GMS>(m_context->Assets.GMS, m_context);
            if (!m_context->GMSInstance->Load()) {
                spdlog::error("LevelDescription::Analyze| Failed to load GMS to analyze!");
            }
        } else spdlog::info(" * GMS ignored by user");
    }

    void LevelDescription::PrintInfo()
    {
        if (m_context)
        {
            if (m_context->GMSInstance)
            {
                m_context->GMSInstance->PrintInfo();
            }
        }
    }

    void LevelDescription::ExportUncompressedGMS(const std::string& path)
    {
        if (!m_context)
        {
            spdlog::error("LevelDescription::ExportUncompressedGMS| No available context. Fatal error.");
            return;
        }

        if (m_context->Flags[IgnoreFlags::IgnoreGMS]) return;

        if (m_context->GMSInstance->SaveUncompressed(path))
        {
            spdlog::info("Raw GMS file exported to {}", path);
        } else {
            spdlog::error("Failed to save raw GMS file to {}. Please, check the log", path);
        }
    }

    bool LevelDescription::ExportLocalizationToJson(std::string_view path)
    {
        if (!m_context)
        {
            spdlog::error("LevelDescription::ExportLocalizationToJson| No available context. Fatal error.");
            return false;
        }

        if (m_context->Flags[IgnoreFlags::IgnoreLOC])
        {
            spdlog::warn("LevelDescription::ExportLocalizationToJson| Unable to export LOC file because it's ignored by user");
            return false;
        }

        if (!m_context->LOCInstance)
        {
            spdlog::error("LevelDescription::ExportLocalizationToJson| Call LoadAndAnalyze() before!");
            return false;
        }

        return m_context->LOCInstance->SaveAsJson(path);
    }

    void LevelDescription::SetIgnoreGMSFlag(bool flag) { m_context->Flags[IgnoreFlags::IgnoreGMS] = flag; }
    void LevelDescription::SetIgnoreANMFlag(bool flag) { m_context->Flags[IgnoreFlags::IgnoreANM] = flag; }
    void LevelDescription::SetIgnoreLOCFlag(bool flag) { m_context->Flags[IgnoreFlags::IgnoreLOC] = flag; }
    void LevelDescription::SetIgnorePRMFlag(bool flag) { m_context->Flags[IgnoreFlags::IgnorePRM] = flag; }
    void LevelDescription::SetIgnorePRPFlag(bool flag) { m_context->Flags[IgnoreFlags::IgnorePRP] = flag; }
    void LevelDescription::SetIgnoreTEXFlag(bool flag) { m_context->Flags[IgnoreFlags::IgnoreTEX] = flag; }
    void LevelDescription::SetIgnoreSNDFlag(bool flag) { m_context->Flags[IgnoreFlags::IgnoreSND] = flag; }

    bool LevelDescription::ValidateLevelArchive()
    {
        if (!m_context || !m_context->Zip)
        {
            spdlog::error("Level validation failed! Wrong call");
            return false;
        }

        int ret = unzGoToFirstFile(m_context->Zip);
        if (ret != UNZ_OK)
        {
            spdlog::error("ValidateLevelArchive| unzGoToFirstFile() failed with code {}", ret);
            return false;
        }

        // We need to locate folder
        bool allAssetsExplored = false;

        do {
            static constexpr int kMaxFileNameLength = 256;
            char fileName[kMaxFileNameLength] = { 0 };
            unz_file_info fileInfo;

            ret = unzGetCurrentFileInfo(m_context->Zip, &fileInfo, fileName, kMaxFileNameLength, nullptr, 0, nullptr, 0);
            if (ret != UNZ_OK)
            {
                spdlog::error("unzGetCurrentFileInfo() failed with error code {}", ret);
                return false;
            }

            std::string name { fileName };
            m_context->Assets.TryResolve(name);

            // Check
            allAssetsExplored = m_context->Assets.AllResolved();

            if (!allAssetsExplored && unzGoToNextFile(m_context->Zip) != UNZ_OK)
            {
                spdlog::warn("No more files to analyze. End of archive {}", m_context->ArchivePath);
                break;
            }
        } while (!allAssetsExplored);

        if (allAssetsExplored)
        {
            spdlog::info("Level structure resolved!");
        }

        unzGoToFirstFile(m_context->Zip);
        return true;
    }
}