#include <BM/LOC/LOCTreeCompiler.h>
#include <cassert>

#include <iostream>

namespace BM::LOC
{
    static size_t GetStringAlignedLength(std::string_view str, size_t pos, size_t al)
    {
        if (str.length() < al) return al;
        return str.length() + 1;
    }

    /**
     * @return next caret position after aligned string written with zeros on align pos
     */
    static size_t WriteZStringAligned(LOCTreeCompiler::Buffer& buffer, size_t offset, const std::string& str, size_t alignment)
    {
        if (offset >= buffer.size())
        {
            std::cout << "WriteZStringAligned| Bad offset! Got " << offset << " but max offset is " << (buffer.size() - 1) << '\n';
            assert(offset < buffer.size());
            return 0;
        }

        size_t strLen = GetStringAlignedLength(str, offset, alignment);

        if (offset + strLen >= buffer.size())
        {
            std::cout << "WriteZStringAligned| Bad string length! Got " << (offset + strLen) << " but max length is " << (buffer.size() - 1) << '\n';
            assert(false);
            return 0;
        }

        std::memset(&buffer[offset], 0x0, strLen);
        std::memcpy(&buffer[offset], str.data(), str.length());

        return offset + strLen;
    }

    static size_t WriteZString(LOCTreeCompiler::Buffer& buffer, size_t offset, const std::string& str)
    {
        if (offset >= buffer.size())
        {
            std::cout << "WriteZString| Bad offset! Got " << offset << " but max offset is " << (buffer.size() - 1) << '\n';
            assert(false);
            return 0;
        }

        if (offset + str.length() + 1 >= buffer.size())
        {
            std::cout << "WriteZString| Not enough space in buffer. Required " << (offset + str.length() + 1) << " but max offset is " << (buffer.size() - 1) << '\n';
            assert(false);
            return 0;
        }

        std::memcpy(&buffer[offset], str.data(), str.length());

        return offset + str.length() + 1;
    }

    static void WriteByte(LOCTreeCompiler::Buffer& buffer, size_t offset, uint8_t b)
    {
        assert(offset < buffer.size());
        if (offset >= buffer.size())
        {
            std::cout << "WriteByte| Bad offset! Got " << offset << " but max offset is " << (buffer.size() - 1) << '\n';
            assert(false);
            return;
        }

        buffer[offset] = b;
    }

    static void WriteUInt32(LOCTreeCompiler::Buffer& buffer, size_t offset, uint32_t v)
    {
        *((uint32_t*)&buffer[offset]) = v;
    }

    static constexpr size_t kTypeBytesCount = 1;
    static constexpr size_t kNumChildBytes = 1;
    static constexpr size_t kAlignment = 0x10;

    static size_t CalculateUsedMemoryAndMarkLocations(LOCTreeNode* node, size_t caretPosition)
    {
        if (node->IsData())
        {
            size_t entitySize = 0;

            entitySize += node->name.length() + 1;
            entitySize += kTypeBytesCount;
            entitySize += GetStringAlignedLength(node->value, caretPosition + entitySize, kAlignment) + 1;

            node->memoryMarkup = LOCTreeNode::MemoryMarkup { caretPosition, caretPosition + entitySize };
        }
        else if (node->IsContainer())
        {
            if (node->IsRoot())
            {
                size_t entitySize = 0;
                entitySize += 1;
                entitySize += sizeof(uint32_t) * (node->numChild - 1);

                size_t nextCaretPosition = entitySize + caretPosition;

                for (int i = 0; i < node->numChild; i++)
                {
                    size_t futureCaretPosition = CalculateUsedMemoryAndMarkLocations(node->children[i], nextCaretPosition);
                    nextCaretPosition = futureCaretPosition;
                }

                node->memoryMarkup = LOCTreeNode::MemoryMarkup { caretPosition, nextCaretPosition };
            }
            else
            {
                size_t entitySize = 0;

                entitySize += node->name.length() + 1;
                entitySize += 1;
                entitySize += sizeof(uint32_t) * (node->numChild - 1);

                size_t nextCaretPosition = entitySize + caretPosition;

                for (int i = 0; i < node->numChild; i++)
                {
                    size_t futureCaretPosition = CalculateUsedMemoryAndMarkLocations(node->children[i], nextCaretPosition);
                    nextCaretPosition = futureCaretPosition;
                }

                node->memoryMarkup = LOCTreeNode::MemoryMarkup { caretPosition, nextCaretPosition };
            }
        }
        else
        {
            assert(false);
        }

        return node->memoryMarkup.value().EndsAt;
    }

    static void WriteTreeNodesIntoMarkedUpMemory(LOCTreeCompiler::Buffer& buffer, LOCTreeNode* node)
    {
        assert(node);
        assert(node->memoryMarkup.has_value());

        const auto& markup = node->memoryMarkup.value();

        if (node->IsContainer())
        {
            if (node->IsRoot())
            {
                // Write markup
                WriteByte(buffer, markup.Offset, node->numChild);
                for (int i = 1; i < node->numChild; i++)
                {
                    WriteUInt32(buffer, (markup.Offset + 1) + ((i - 1) * sizeof(uint32_t)), node->children[i]->memoryMarkup.value().Offset);
                }

                // Write children nodes
                for (int i = 0; i < node->numChild; i++)
                {
                    WriteTreeNodesIntoMarkedUpMemory(buffer, node->children[i]);
                }
            }
            else
            {
                size_t positionAfterString = WriteZString(buffer, markup.Offset, node->name);
                uint8_t leadingByte = node->originalTypeRawData.has_value() ? node->originalTypeRawData.value() : static_cast<uint8_t>(node->nodeType);
                WriteByte(buffer, positionAfterString, leadingByte);
                WriteByte(buffer, positionAfterString + 1, node->numChild);
                for (int i = 1; i < node->numChild; i++)
                {
                    WriteUInt32(buffer, (positionAfterString + 2) + ((i - 1) * sizeof(uint32_t)), node->children[i]->memoryMarkup.value().Offset);
                }

                // Write children nodes
                for (int i = 0; i < node->numChild; i++)
                {
                    WriteTreeNodesIntoMarkedUpMemory(buffer, node->children[i]);
                }
            }
        } else if (node->IsData())
        {
            size_t positionAfterString = WriteZString(buffer, markup.Offset, node->name);
            WriteByte(buffer, positionAfterString++, node->originalTypeRawData.has_value() ? node->originalTypeRawData.value() : static_cast<uint8_t>(node->nodeType));
            WriteZStringAligned(buffer, positionAfterString, node->value, kAlignment);
        }
    }

    bool LOCTreeCompiler::Compile(Buffer& buffer, LOCTreeNode* rootNode)
    {
        const size_t totalSpaceForEverything = CalculateUsedMemoryAndMarkLocations(rootNode, 0);
        buffer.resize(totalSpaceForEverything);
        WriteTreeNodesIntoMarkedUpMemory(buffer, rootNode);

        return true;
    }
}