#pragma once

#include <IGameEntity.h>
#include <LOC/LOCTree.h>

#include <string>
#include <map>

namespace ReGlacier
{
    class LOC : public IGameEntity
    {
    public:
        using Ptr = std::unique_ptr<LOC>;

        LOC(const std::string& name, LevelContainer* levelContainer, LevelAssets* levelAssets);
        ~LOC();

        bool Load() override;

        bool SaveAsJson(std::string_view filePath);

    private:
        LOCTreeNode* m_root { nullptr };
        std::unique_ptr<uint8_t[]> m_currentBuffer { nullptr }; //< We are taking ownership of the LOC buffer. It's required until the tree not serialized into other structs.
        size_t m_currentBufferSize { 0 };

    private:
        void VisitTreeNode(LOCTreeNode* treeNode);

        /**
         * @brief Search passed key in contents buffer
         * @param key zero terminated key string
         * @param buffer LOC file memory mapped buffer
         * @return pointer to value or nullptr if value not found
         */
        static char* Lookup(char* key, char* buffer);
        /**
         * @brief Try to lookup for key in buffer, if key exists this function returns true, otherwise false
         * @param key
         * @param buffer
         * @return
         */
        static bool HasTextResource(char* key, char* buffer);
    };
}