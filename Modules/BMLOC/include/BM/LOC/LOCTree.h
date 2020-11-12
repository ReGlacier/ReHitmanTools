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
        struct MemoryMarkup
        {
            uint32_t Offset { 0 };
            uint32_t EndsAt { 0 };

            MemoryMarkup() = default;
            MemoryMarkup(uint32_t o, uint32_t e) : Offset(o), EndsAt(e) {}
        };

        // Defs
        static constexpr const char* kNoName = "<NONAME>";
        static constexpr int kRootNodeDepth = 0;

        // Base info
        LOCTreeNode* parent {nullptr};
        char* currentBufferPtr {nullptr};
        std::string name{kNoName};
        std::string value; //Always string? Check it later
        int8_t currentOffset{0}; //Not used in import mode, used in export mode
        TreeNodeType nodeType{0}; //See TreeNodeType for details
        std::optional<uint8_t> originalTypeRawData; //Original raw data if it was overridden by decompiler (just for reconstruction)
        std::optional<MemoryMarkup> memoryMarkup; //Only for compiler for fast memory position search

        // Tree data
        size_t numChild {0};
        std::vector<LOCTreeNode*> children;

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

    private:
        /**
         * @fn Visit node by node and extract useful information
         * @param treeNode pointer to the root node
         */
        static void VisitNode(LOCTreeNode* treeNode);
    };
}