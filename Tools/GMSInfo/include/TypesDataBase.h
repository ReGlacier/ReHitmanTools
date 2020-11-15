#pragma once

#include <array>
#include <string>
#include <unordered_map>

namespace ReGlacier
{
    class TypesDataBase
    {
    private:
        std::unordered_map<std::string, std::array<std::string, 2>> m_db;
    public:
        static TypesDataBase& GetInstance();
        static void Release();

        bool Load(const std::string& path);

        [[nodiscard]] bool HasDefinitionForEntityTypeId(unsigned int entityTypeId) const;
        [[nodiscard]] std::string GetEntityTypeById(unsigned int entityTypeId) const;
    };
}