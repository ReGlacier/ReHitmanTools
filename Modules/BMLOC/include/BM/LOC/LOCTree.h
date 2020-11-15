#pragma once

#include <string_view>
#include <string>
#include <vector>
#include <memory>
#include <optional>

#include <cstdint>

#include <BM/LOC/LOCTypes.h>

namespace BM::LOC
{
    struct LOCTreeNode
    {
        // Editor defs
        /**
         * @struct MemoryMarkup
         * @brief In-memory location of the node. This value used only in the compiler!
         */
        struct MemoryMarkup
        {
            uint32_t StartsAt {0 }; ///< Node starts at
            uint32_t EndsAt { 0 }; ///< Node ends at

            MemoryMarkup() = default;
            MemoryMarkup(uint32_t o, uint32_t e) : StartsAt(o), EndsAt(e) {}
        };

        // Defs
        static constexpr const char* kNoName = "<NONAME>";

        // Base info
        LOCTreeNode* parent {nullptr}; //Pointer to parent node
        char* currentBufferPtr {nullptr}; //Available only in decompiler (be aware, if original buffer deallocated this pointer will be broken)
        std::string name{kNoName}; //Name of node (could be empty for ROOT node)
        std::string value; //Always string? Check it later
        TreeNodeType nodeType{0}; //See TreeNodeType for details
        std::optional<uint8_t> originalTypeRawData; //Original raw data if it was overridden by decompiler (just for reconstruction)
        std::optional<MemoryMarkup> memoryMarkup; //Only for compiler for fast memory position search

        // Tree data
        size_t numChild {0}; //Number of children nodes
        std::vector<LOCTreeNode*> children {}; //List of children nodes (please, add node through AddChild!)

        // Methods
        LOCTreeNode(LOCTreeNode* p, char* b);
        ~LOCTreeNode();
        void AddChild(LOCTreeNode* node);
        void RemoveChild(LOCTreeNode* node);
        [[nodiscard]] bool IsRoot() const;
        [[nodiscard]] bool IsEmpty() const;
        [[nodiscard]] bool IsData() const;
        [[nodiscard]] bool IsContainer() const;

        // Parser
        static LOCTreeNode* ReadFromMemory(char* buffer, size_t bufferSize);

        // Compiler
        static bool Compile(LOCTreeNode* root, std::vector<uint8_t>& compiledBuffer);

        // Serializer
        static void CompileAndSave(LOCTreeNode* root, std::string_view pathToFile);

        // Comparator
        static bool Compare(LOCTreeNode* a, LOCTreeNode* b);

    private:
        /**
         * @fn Visit node by node and extract useful information
         * @param treeNode pointer to the root node
         */
        static void VisitNode(LOCTreeNode* treeNode);

        /**
         * @fn SortKeys
         * @brief Should be called after AddChild or RemoveChild!
         */
        void SortKeys();
    };
}