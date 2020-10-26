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
        void AnalyzeGMS();

    private:
        bool ValidateLevelArchive();
    };
}