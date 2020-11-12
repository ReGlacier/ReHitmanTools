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

extern "C" {
#include <unzip.h>
}

namespace ReGlacier
{
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

        unzFile Zip { nullptr };

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

    void LevelDescription::Analyze()
    {
        if (!m_context)
        {
            spdlog::error("Unable to analyze level. Context not inited!");
            return;
        }

        m_context->Container = std::make_unique<LevelContainer>(m_context->Zip);

        m_context->LOCInstance = GameEntityFactory::Create<LOC>(m_context->Assets.LOC, m_context);
        if (!m_context->LOCInstance->Load())
        {
            spdlog::error("LevelDescription::Analyze| Failed to load LOC to analyze!");
        }

        m_context->ANMInstance = GameEntityFactory::Create<ANM>(m_context->Assets.ANM, m_context);
        if (!m_context->ANMInstance->Load())
        {
            spdlog::error("LevelDescription::Analyze| Failed to load ANM to analyze!");
        }

        m_context->SNDInstance = GameEntityFactory::Create<SND>(m_context->Assets.SND, m_context);
        if (!m_context->SNDInstance->Load())
        {
            spdlog::error("LevelDescription::Analyze| Failed to load SND to analyze!");
        }

        m_context->TEXInstance = GameEntityFactory::Create<TEX>(m_context->Assets.TEX, m_context);
        if (!m_context->TEXInstance->Load())
        {
            spdlog::error("LevelDescription::Analyze| Failed to load TEX to analyze!");
        }

        m_context->PRMInstance = GameEntityFactory::Create<PRM>(m_context->Assets.PRM, m_context);
        if (!m_context->PRMInstance->Load())
        {
            spdlog::error("LevelDescription::Analyze| Failed to load PRM to analyze!");
        }

        m_context->GMSInstance = GameEntityFactory::Create<GMS>(m_context->Assets.GMS, m_context);
        if (!m_context->GMSInstance->Load())
        {
            spdlog::error("LevelDescription::Analyze| Failed to load GMS to analyze!");
        }
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
        if (!m_context || !m_context->GMSInstance)
        {
            spdlog::error("LevelDescription::ExportUncompressedGMS| Call AnalyzeGMS() before!");
            return;
        }

        if (m_context->GMSInstance->SaveUncompressed(path))
        {
            spdlog::info("Raw GMS file exported to {}", path);
        } else {
            spdlog::error("Failed to save raw GMS file to {}. Please, check the log", path);
        }
    }

    bool LevelDescription::ExportLocalizationToJson(std::string_view path)
    {
        if (!m_context || !m_context->LOCInstance)
        {
            spdlog::error("LevelDescription::ExportLocalizationToJson| Call Analyze() before!");
            return false;
        }

        return m_context->LOCInstance->SaveAsJson(path);
    }

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