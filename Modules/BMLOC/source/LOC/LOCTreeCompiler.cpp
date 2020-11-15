#include <BM/LOC/LOCTreeCompiler.h>
#include <stdexcept>

namespace BM::LOC
{
    static size_t GetStringAlignedLength(std::string_view str)
    {
        return str.length() + 1 + 4; // +1 - zero terminator, +4 alignment
    }

    /**
     * @return next caret position after aligned string written with zeros on align pos
     */
    static size_t WriteZStringAligned(LOCTreeCompiler::Buffer& buffer, size_t offset, std::string_view str)
    {
        if (offset > buffer.size())
        {
            throw std::out_of_range {
                "WriteZStringAligned: Out of range! Offset " + std::to_string(offset) +
                " not included in " + std::to_string(buffer.size() - 1) };
        }

        size_t strLen = GetStringAlignedLength(str);

        if (offset + strLen > buffer.size())
        {
            throw std::out_of_range {
                "WriteZStringAligned: Out of range! String buffer at " + std::to_string(offset) +
                " with length " + std::to_string(strLen) + " not included in " + std::to_string(buffer.size() - 1)
            };
        }

        std::memset(&buffer[offset], 0x0, strLen);
        std::memcpy(&buffer[offset], str.data(), str.length());

        return offset + strLen;
    }

    static size_t WriteZString(LOCTreeCompiler::Buffer& buffer, size_t offset, const std::string& str)
    {
        if (offset >= buffer.size())
        {
            throw std::out_of_range {
                "WriteZString: Out of range! Offset " + std::to_string(offset) +
                " not included in buffer " + std::to_string(buffer.size()) };
        }

        if (offset + str.length() + 1 >= buffer.size())
        {
            throw std::out_of_range {
                "WriteZString: Out of range! Offset " + std::to_string(offset) +
                " with length " + std::to_string(str.length() + 1) +
                " not included in " + std::to_string(buffer.size() - 1) };
            return 0;
        }

        std::memcpy(&buffer[offset], str.data(), str.length());

        return offset + str.length() + 1;
    }

    static void WriteByte(LOCTreeCompiler::Buffer& buffer, size_t offset, uint8_t b)
    {
        if (offset >= buffer.size())
        {
            throw std::out_of_range {
                    "WriteByte: Out of range! Offset " + std::to_string(offset) +
                    " with required space 1 (at " + std::to_string(offset + 1) +
                    ") not included in " + std::to_string(buffer.size() - 1)
            };
        }

        buffer[offset] = b;
    }

    static void WriteUInt32(LOCTreeCompiler::Buffer& buffer, size_t offset, uint32_t v)
    {
        if (offset >= buffer.size())
        {
            throw std::out_of_range {
                    "WriteUInt32: Out of range! Offset " + std::to_string(offset) +
                    " not included in " + std::to_string(buffer.size() - 1)
            };
        }

        if (offset + sizeof(uint32_t) >= buffer.size())
        {
            throw std::out_of_range {
                    "WriteUInt32: Out of range! Offset " + std::to_string(offset) +
                    " with required space 4 (at " + std::to_string(offset + sizeof(uint32_t)) +
                    ") not included in " + std::to_string(buffer.size() - 1)
            };
        }

        uint32_t bs = v;
        std::memcpy(&buffer[offset], &bs, sizeof(uint32_t));
    }

    static constexpr size_t kAlignment = 0x10;

    /**
     * Return last point in the virtual memory buffer (size)
     * @param node pointer to tree node
     * @param startPosition where node should be placed
     * @return end position of node location (node + sizeof(node))
     */
    static size_t CalculateUsedMemoryAndMarkLocations(LOCTreeNode* node, size_t startPosition)
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
                    endPosition = CalculateUsedMemoryAndMarkLocations(node->children[0], endPosition);
                    for (int i = 1; i < node->numChild; i++)
                    {
                        endPosition = CalculateUsedMemoryAndMarkLocations(node->children[i], endPosition);
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
                    endPosition = CalculateUsedMemoryAndMarkLocations(node->children[0], endPosition);
                    for (int i = 1; i < node->numChild; i++)
                    {
                        endPosition = CalculateUsedMemoryAndMarkLocations(node->children[i], endPosition);
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
            endPosition += GetStringAlignedLength(node->value);
            node->memoryMarkup = LOCTreeNode::MemoryMarkup { startPosition, endPosition };
        }
        else
        {
            throw std::exception { "CalculateUsedMemoryAndMarkLocations: Unknown node type" };
        }

        return node->memoryMarkup.value().EndsAt;
    }

    /**
     * @brief This function compiles node into the allocated buffer (no reallocations, the buffer must be resized already)
     * @note This function not checking an incoming node for recursive references. Be aware of that.
     * @param buffer allocated bytes buffer
     * @param node pointer to already marked up node
     */
    static void WriteTreeNodesIntoMarkedUpMemory(LOCTreeCompiler::Buffer& buffer, LOCTreeNode* node) // NOLINT(misc-no-recursion)
    {
        if (!node) throw std::exception { "WriteTreeNodesIntoMarkedUpMemory: Bad node pointer" };
        if (!node->memoryMarkup.has_value()) throw std::exception { "WriteTreeNodesIntoMarkedUpMemory: Node was not marked up!" };

        const auto& markup = node->memoryMarkup.value();

        if (node->IsContainer())
        {
            if (node->IsRoot())
            {
                WriteByte(buffer, markup.StartsAt, node->numChild); //Count of child nodes

                for (int i = 1; i < node->numChild; i++) //Write offsets from #1 to last node
                {
                    size_t offsetsTableStartPosition = markup.StartsAt + 1; // skip first byte (allocated for num childs)
                    offsetsTableStartPosition += ((i - 1) * sizeof(uint32_t));
                    WriteUInt32(
                            buffer,
                            offsetsTableStartPosition,
                            node->children[i]->memoryMarkup.value().StartsAt - node->children[0]->memoryMarkup.value().StartsAt
                    );
                }

                // Write children nodes
                for (int i = 0; i < node->numChild; i++)
                {
                    WriteTreeNodesIntoMarkedUpMemory(buffer, node->children[i]);
                }
            }
            else
            {
                size_t positionAfterString = WriteZString(buffer, markup.StartsAt, node->name); //Write name
                uint8_t typeByte = node->originalTypeRawData.has_value() ? node->originalTypeRawData.value() : static_cast<uint8_t>(node->nodeType);
                WriteByte(buffer, positionAfterString, typeByte); //Write type byte
                WriteByte(buffer, positionAfterString + 1, node->numChild); //Write num of child nodes

                for (int i = 1; i < node->numChild; i++)
                {
                    //Write offsets table from #1 to last node
                    WriteUInt32(
                            buffer,
                            (positionAfterString + 2) + ((i - 1) * sizeof(uint32_t)),
                            node->children[i]->memoryMarkup.value().StartsAt - node->children[0]->memoryMarkup.value().StartsAt
                    );
                }

                // Write children nodes
                for (int i = 0; i < node->numChild; i++)
                {
                    WriteTreeNodesIntoMarkedUpMemory(buffer, node->children[i]);
                }
            }
        } else if (node->IsData())
        {
            size_t positionAfterString = WriteZString(buffer, markup.StartsAt, node->name); //Write full string
            WriteByte(buffer, positionAfterString++, node->originalTypeRawData.has_value() ? node->originalTypeRawData.value() : static_cast<uint8_t>(node->nodeType)); //Write leading byte
            WriteZStringAligned(buffer, positionAfterString, node->value); // Write aligned value
        }
    }

    bool LOCTreeCompiler::Compile(Buffer& buffer, LOCTreeNode* rootNode)
    {
        // Markup memory layouts
        MarkupTree(rootNode);
        const auto& memMarkup = rootNode->memoryMarkup.value();
        buffer.resize(memMarkup.EndsAt);

        // Write memory layouts
        WriteTreeNodesIntoMarkedUpMemory(buffer, rootNode);

        return true;
    }

    void LOCTreeCompiler::MarkupTree(LOCTreeNode* rootNode)
    {
        CalculateUsedMemoryAndMarkLocations(rootNode, 0);
    }
}