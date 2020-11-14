#include <BM/LOC/LOCTreeCompiler.h>
#include <BM/LOC/LOCTree.h>

#include <fstream>
#include <cassert>
#include <vector>

namespace BM::LOC
{
    LOCTreeNode::LOCTreeNode(LOCTreeNode* p, char* b) : parent(p), currentBufferPtr(b) {}

    LOCTreeNode::~LOCTreeNode()
    {
        if (numChild)
        {
            for (int i = 0; i < numChild; i++)
            {
                delete children[i];
            }

            numChild = 0;
        }
    }

    void LOCTreeNode::AddChild(LOCTreeNode* node)
    {
        if (!node)
        {
            assert(false); // Bad node here
            return;
        }

        // Setup us as the parent or don't set up anything into the parent field.
        assert(node->parent == this || node->parent == nullptr);
        if (node->parent != this)
        {
            node->parent = this;
        }

        children.push_back(node);
        numChild = children.size();
    }

    void LOCTreeNode::RemoveChild(LOCTreeNode* node)
    {
        if (!node)
        {
            assert(false); // Bad node here
            return;
        }

        node->parent = nullptr; // Remove us from parent field
        // Remove us from vector
        auto it = std::find(std::begin(children), std::end(children), node);
        if (it != std::end(children))
        {
            children.erase(it);
            numChild = children.size();
        }
    }

    bool LOCTreeNode::IsRoot() const
    {
        return parent == nullptr;
    }

    bool LOCTreeNode::IsEmpty() const
    {
        if (nodeType == TreeNodeType::VALUE_OR_DATA)
            return value.empty();

        if (nodeType == TreeNodeType::NODE_WITH_CHILDREN)
            return numChild == 0;

        // Empty & not inited tree node
        return true;
    }

    bool LOCTreeNode::IsData() const
    {
        return nodeType == TreeNodeType::VALUE_OR_DATA;
    }

    bool LOCTreeNode::IsContainer() const
    {
        return nodeType == TreeNodeType::NODE_WITH_CHILDREN;
    }

    LOCTreeNode* LOCTreeNode::ReadFromMemory(char* buffer, size_t bufferSize)
    {
        auto root = new LOCTreeNode(nullptr, buffer);
        LOCTreeNode::VisitNode(root);
        return root;
    }

    bool LOCTreeNode::Compile(LOCTreeNode* root, std::vector<uint8_t>& compiledBuffer)
    {
        if (!root || !root->IsRoot() || root->IsEmpty()) throw std::exception { "Bad root node! It's actually not root or empty" };
        if (root->numChild < 0 || root->numChild > 0xFF) throw std::exception { "To many root nodes! Allowed only 255 (0xFF) max!" };

        return LOCTreeCompiler::Compile(compiledBuffer, root);
    }

    void LOCTreeNode::CompileAndSave(LOCTreeNode* root, std::string_view pathToFile)
    {
        if (!root || !root->IsRoot() || root->IsEmpty()) throw std::exception { "LOCTreeNode::Save| Bad root node! Unable to serialize it!" };
        if (root->numChild < 0 || root->numChild > 0xFF) throw std::exception { "LOCTreeNode::Save| Too many root nodes! Max allowed 255, min 1" };

        std::ofstream file(pathToFile.data(), std::ofstream::trunc | std::ofstream::out);
        LOCTreeCompiler::Buffer compiledBuffer {};

        if (!file)
        {
            throw std::exception { "LOCTreeNode::Save| Unable to create output file!" };
        }

        try
        {
            bool compileResult = LOCTreeNode::Compile(root, compiledBuffer);
            if (!compileResult)
            {
                file.close();
                throw std::exception { "LOCTreeNode::Save| Unable to compile tree" };
            }

            file.write((char*)compiledBuffer.data(), compiledBuffer.size());
            file.close();
        } catch (...)
        {
            file.close();
            throw;
        }
    }

    void LOCTreeNode::VisitNode(LOCTreeNode* treeNode)
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
            //Determine which format should be used for it (string or bytes array? for bytes array we should know length)
            //FIXME
            if (treeNode->currentBufferPtr)
            {
                treeNode->value = treeNode->currentBufferPtr;
            }
            return; // Do not look for any child here
        }

        auto countOfChildNodes = static_cast<int8_t>(*treeNode->currentBufferPtr);
        if (countOfChildNodes <= 0)
            return; // Orphaned or broken node, not interested for us

        char* offsetsPtr = (char*)&treeNode->currentBufferPtr[1];
        std::vector<uint32_t> offsets;
        offsets.reserve(countOfChildNodes);

        offsets.push_back(0); // Always our first entity located at +0x0, other located on their own offsets
        for (int i = 1; i < countOfChildNodes; i++)
        {
            offsets.push_back(*reinterpret_cast<uint32_t*>(&offsetsPtr[0]));
            offsetsPtr += sizeof(uint32_t);
        }

        treeNode->numChild = static_cast<uint8_t>(countOfChildNodes);

        char* baseAddr = treeNode->currentBufferPtr + (sizeof(uint32_t) * (countOfChildNodes - 1)) + 1;

        for (const auto& offset : offsets)
        {
            std::string_view name { baseAddr + offset };
            auto child = new LOCTreeNode(treeNode, baseAddr + offset + name.length() + 1);
            child->name = name;
            treeNode->AddChild(child);
            LOCTreeNode::VisitNode(child);
        }
    }
}