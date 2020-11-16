#include <BM/LOC/Internal/LOCTreeNodeVisitor.h>

namespace BM::LOC::Internal
{
    using Self = LOCTreeNodeVisitor<LOCSupportMode::Hitman_BloodMoney>;

    bool Self::Visit(LOCTreeNode* treeNode, size_t bufferSize)
    {
        if (treeNode->parent)
        {
            treeNode->nodeType = static_cast<TreeNodeType>(treeNode->currentBufferPtr[0]);
            treeNode->currentBufferPtr = (char*)&treeNode->currentBufferPtr[1]; // Move caret if we not a root
        }
        else
        {
            treeNode->nodeType = TreeNodeType::NODE_WITH_CHILDREN; // Root always contains children nodes, no data inside
        }

        const bool isOkNode = treeNode->IsData() || treeNode->IsContainer();
        if (!isOkNode)
        {
            treeNode->originalTypeRawData = static_cast<uint8_t>(treeNode->nodeType);
            treeNode->nodeType = TreeNodeType::VALUE_OR_DATA; // We can override type because we save that before this.
        }

        if (treeNode->IsData())
        {
            if (treeNode->currentBufferPtr)
            {
                treeNode->value = treeNode->currentBufferPtr;
            }
            return true; // Do not look for any child here
        }

        auto countOfChildNodes = static_cast<uint8_t>(*treeNode->currentBufferPtr);
        if (countOfChildNodes == 0)
            return true; // Orphaned or broken node, not interested for us

        char* offsetsPtr = (char*)&treeNode->currentBufferPtr[1];
        std::vector<uint32_t> offsets;
        offsets.reserve(countOfChildNodes);

        offsets.push_back(0); // Always our first entity located at +0x0, other located on their own offsets
        for (int i = 1; i < countOfChildNodes; i++)
        {
            uint32_t offset = *reinterpret_cast<uint32_t*>(&offsetsPtr[0]);
            if (offset >= bufferSize)
            {
                return false; // Out of bounds
            }

            offsets.push_back(*reinterpret_cast<uint32_t*>(&offsetsPtr[0]));
            offsetsPtr += sizeof(uint32_t);
        }

        treeNode->numChild = static_cast<uint8_t>(countOfChildNodes);

        char* baseAddr = treeNode->currentBufferPtr + (sizeof(uint32_t) * (countOfChildNodes - 1)) + 1;

        for (const auto& offset : offsets)
        {
            if (offset >= bufferSize)
            {
                return false;
            }

            std::string_view name { baseAddr + offset };

            if (offset + name.length() + 1 >= bufferSize)
            {
                return false;
            }

            auto child = new LOCTreeNode(treeNode, baseAddr + offset + name.length() + 1);
            child->name = name;
            treeNode->AddChild(child);

            if (!Internal::LOCTreeNodeVisitor<LOCSupportMode::Hitman_BloodMoney>::Visit(child, bufferSize))
            {
                return false;
            }
        }

        return true;
    }
}