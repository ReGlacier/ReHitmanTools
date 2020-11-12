#pragma once

#include <string_view>
#include <string>
#include <vector>

#include <cstdint>

namespace ReGlacier
{
    enum TreeNodeType : int8_t {
        VALUE_OR_DATA      = 0x0,
        NODE_WITH_CHILDREN = 0x10
    };

    struct LOCTreeNode
    {
        // Defs
        static constexpr const char* kNoName = "<NONAME>";
        static constexpr int kRootNodeDepth = 0;

        // Base info
        LOCTreeNode* parent{nullptr};
        char* currentBufferPtr{nullptr};
        std::string_view name{kNoName};
        char* value { nullptr };
        size_t valueSize { 0 };
        int8_t currentOffset{0};
        TreeNodeType nodeType{0}; // 0x0 - end node with data, 0x16 - node without data

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
    };
}