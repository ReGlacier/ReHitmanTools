#include <BM/LOC/Internal/LOCCompilerImpl.h>
#include <BM/LOC/Internal/CompilerUtils.h>

namespace BM::LOC::Internal
{
    using Self = LOCCompilerImpl<LOCSupportMode::Hitman_BloodMoney>;

    bool Self::MarkupTree(LOCTreeNode* root)
    {
        CalculateSizeForEntry(root, 0);
        return true;
    }

    bool Self::Compile(std::vector<uint8_t>& buffer, LOCTreeNode* node)
    {
        if (!node) throw std::exception { "WriteTreeNodesIntoMarkedUpMemory: Bad node pointer" };
        if (!node->memoryMarkup.has_value()) throw std::exception { "WriteTreeNodesIntoMarkedUpMemory: Node was not marked up!" };

        const auto& markup = node->memoryMarkup.value();

        if (node->IsContainer())
        {
            if (node->IsRoot())
            {
                CompilerUtils::WriteByte(buffer, markup.StartsAt, node->numChild); //Count of child nodes

                for (int i = 1; i < node->numChild; i++) //Write offsets from #1 to last node
                {
                    size_t offsetsTableStartPosition = markup.StartsAt + 1; // skip first byte (allocated for num childs)
                    offsetsTableStartPosition += ((i - 1) * sizeof(uint32_t));
                    CompilerUtils::WriteUInt32(
                            buffer,
                            offsetsTableStartPosition,
                            node->children[i]->memoryMarkup.value().StartsAt - node->children[0]->memoryMarkup.value().StartsAt
                    );
                }

                // Write children nodes
                for (int i = 0; i < node->numChild; i++)
                {
                    if (!Self::Compile(buffer, node->children[i]))
                    {
                        return false;
                    }
                }
            }
            else
            {
                size_t positionAfterString = CompilerUtils::WriteZString(buffer, markup.StartsAt, node->name); //Write name
                uint8_t typeByte = node->originalTypeRawData.has_value() ? node->originalTypeRawData.value() : static_cast<uint8_t>(node->nodeType);
                CompilerUtils::WriteByte(buffer, positionAfterString, typeByte); //Write type byte
                CompilerUtils::WriteByte(buffer, positionAfterString + 1, node->numChild); //Write num of child nodes

                for (int i = 1; i < node->numChild; i++)
                {
                    //Write offsets table from #1 to last node
                    CompilerUtils::WriteUInt32(
                            buffer,
                            (positionAfterString + 2) + ((i - 1) * sizeof(uint32_t)),
                            node->children[i]->memoryMarkup.value().StartsAt - node->children[0]->memoryMarkup.value().StartsAt
                    );
                }

                // Write children nodes
                for (int i = 0; i < node->numChild; i++)
                {
                    if (!Self::Compile(buffer, node->children[i]))
                    {
                        return false;
                    }
                }
            }
        } else if (node->IsData())
        {
            size_t positionAfterString = CompilerUtils::WriteZString(buffer, markup.StartsAt, node->name); //Write full string
            CompilerUtils::WriteByte(buffer, positionAfterString++, node->originalTypeRawData.has_value() ? node->originalTypeRawData.value() : static_cast<uint8_t>(node->nodeType)); //Write leading byte
            CompilerUtils::WriteZStringAligned(buffer, positionAfterString, node->value); // Write aligned value
        }

        return true;
    }

    size_t Self::CalculateSizeForEntry(LOCTreeNode* node, size_t startPosition)
    {
        if (node->IsContainer())
        {
            if (node->IsRoot())
            {
                /**
                 * Root node layout
                 *
                 *   Count of children     Offset table for #1..N chld
                 * [ 1 byte - child num ][ 4 * (child count - 1) bytes ]
                 *
                 * Child #0 stored after root layout
                 * Child #1 stored after #0 layout
                 * ...
                 * Child #(N-1) stored after #(N-2) layout
                 */
                size_t endPosition = startPosition;
                endPosition += 1; // For count of children
                if (node->numChild > 0)
                {
                    endPosition += (sizeof(uint32_t) * (node->numChild - 1)); // Space for offsets table without leading entity
                    endPosition = Self::CalculateSizeForEntry(node->children[0], endPosition);
                    for (int i = 1; i < node->numChild; i++)
                    {
                        endPosition = Self::CalculateSizeForEntry(node->children[i], endPosition);
                    }
                }
                node->memoryMarkup = LOCTreeNode::MemoryMarkup { startPosition, endPosition };
            }
            else
            {
                /**
                 * Generic container node layout
                 *
                 *  [ Name length + 1 ][ Type Byte ][ 4 * (children nodes count - 1) ]
                 *
                 * Child #0 stored after root layout
                 * Child #1 stored after #0 layout
                 * ...
                 * Child #(N-1) stored after #(N-2) layout
                 */
                size_t endPosition = startPosition;
                endPosition += node->name.length() + 1; // For name of the node
                endPosition += 1; // For type byte
                endPosition += 1; // For num child
                if (node->numChild > 0)
                {
                    endPosition += sizeof(uint32_t) * (node->numChild - 1); // For index table
                    endPosition = Self::CalculateSizeForEntry(node->children[0], endPosition);
                    for (int i = 1; i < node->numChild; i++)
                    {
                        endPosition = Self::CalculateSizeForEntry(node->children[i], endPosition);
                    }
                }
                node->memoryMarkup = LOCTreeNode::MemoryMarkup { startPosition, endPosition };
            }
        }
        else if (node->IsData())
        {
            /**
             * Data node format
             *
             * [ Name bytes + 1 ][ type byte ][ Value bytes + 1 ]
             */
            size_t endPosition = startPosition;
            endPosition += node->name.length() + 1; // For node name
            endPosition += 1; // For type byte
            endPosition += CompilerUtils::GetStringAlignedLength(node->value);
            node->memoryMarkup = LOCTreeNode::MemoryMarkup { startPosition, endPosition };
        }
        else
        {
            throw std::exception { "CalculateUsedMemoryAndMarkLocations: Unknown node type" };
        }

        return node->memoryMarkup.value().EndsAt;
    }
}