#pragma once

#include <string>

namespace ReGlacier
{
    class LevelDescription
    {
    protected:
        struct Context;
        Context* m_context;

    public:
        explicit LevelDescription(const std::string& pathToLevelArchive);
        ~LevelDescription();

        bool Open();
        [[nodiscard]] bool IsMain() const;
        void LoadAndAnalyze();
        void PrintInfo();
        void ExportUncompressedGMS(const std::string& path);
        bool ExportLocalizationToJson(std::string_view path);
        bool GenerateGMSWithUncompressedBody(std::string_view path);

        void SetIgnoreGMSFlag(bool flag);
        void SetIgnoreANMFlag(bool flag);
        void SetIgnoreLOCFlag(bool flag);
        void SetIgnorePRMFlag(bool flag);
        void SetIgnorePRPFlag(bool flag);
        void SetIgnoreTEXFlag(bool flag);
        void SetIgnoreSNDFlag(bool flag);
    private:
        bool ValidateLevelArchive();
    };
}