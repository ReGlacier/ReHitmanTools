#include <TypesDataBase.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <fstream>

namespace ReGlacier
{
    TypesDataBase* g_typesDataBase { nullptr };

    TypesDataBase& TypesDataBase::GetInstance()
    {
        if (!g_typesDataBase)
        {
            g_typesDataBase = new TypesDataBase();
        }

        return *g_typesDataBase;
    }

    void TypesDataBase::Release()
    {
        if (g_typesDataBase)
        {
            delete g_typesDataBase;
            g_typesDataBase = nullptr;
        }
    }

    bool TypesDataBase::Load(const std::string& path)
    {
        m_db.clear();

        std::ifstream fileStream(path, std::ifstream::binary);
        if (!fileStream) {
            spdlog::error("Failed to open types data base file {}", path);
            return false;
        }

        try {
            auto db = nlohmann::json::parse(fileStream);
            nlohmann::adl_serializer<decltype(m_db)>::from_json(db, m_db);
            spdlog::info("Type information DB loaded\n");
        } catch (nlohmann::json::exception& ex) {
            spdlog::error("Invalid types data base file {}. Parse error: {}", path, ex.what());
            return false;
        }

        fileStream.close();

        return true;
    }

    bool TypesDataBase::HasDefinitionForEntityTypeId(unsigned int entityTypeId) const
    {
        std::string value = fmt::format("0x{:X}", entityTypeId);
        auto iter = m_db.find(value);
        return iter != std::end(m_db);
    }

    std::string TypesDataBase::GetEntityTypeById(unsigned int entityTypeId) const
    {
        std::string value = fmt::format("0x{:X}", entityTypeId);
        auto iter = m_db.find(value);
        if (iter != std::end(m_db))
        {
            return fmt::format("{} : {} (0x{:X})", iter->second[0], iter->second[1], entityTypeId);
        }

        return fmt::format("NOT FOUND 0x{:X}", entityTypeId);
    }
}