#pragma once

#include <vector>
#include <variant>
#include <optional>

#include <PRP/PRPTypes.h>

namespace ReGlacier
{
    using RawData = std::vector<uint8_t>;
    using PRPDataBlock = std::variant<bool, char, int8_t, int16_t, int32_t, float, RawData>;

    class PRPTreeNode
    {
    private:
        // Tree Control Block
        PRPTreeNode* _parent {nullptr };
        std::vector<PRPTreeNode*> _children {};

        // Data
        PRP_ETag _tag { PRP_ETag::NO_TAG };
        std::optional<size_t> _inBufferOffset;
        PRPDataBlock _dataBlock;

    public:
        PRPTreeNode() = default;
        explicit PRPTreeNode(PRP_ETag tag);
        PRPTreeNode(PRPTreeNode* parent, PRP_ETag tag);
        ~PRPTreeNode();

        bool IsRoot() const;
        void AddChild(PRPTreeNode* node);
        void RemoveChild(PRPTreeNode* node);
        [[nodiscard]] const std::vector<PRPTreeNode*>& GetChildNodes() const;
        PRPTreeNode* GetParent();

        // Debug
        [[nodiscard]] bool HasRefToBuffer() const;
        [[nodiscard]] size_t GetRefToBuffer() const;
        void SetRefToBuffer(size_t offset);

        // Data
        [[nodiscard]] PRP_ETag GetTag() const;
        void SetTag(PRP_ETag tag);
    };
}