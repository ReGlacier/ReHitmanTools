#include <BM/LOC/Internal/LOCTreeNodeVisitor.h> // PRIVATE IMPL
#include <BM/LOC/LOCTreeCompiler.h>
#include <BM/LOC/LOCTreeFactory.h>
#include <BM/LOC/LOCTree.h>

#include <algorithm>
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

        SortKeys();
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

            SortKeys();
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

    void LOCTreeNode::SortKeys()
    {
        if (children.size() <= 1)
        {
            return; // Nothing to sort here
        }

        auto SortPred = [](const LOCTreeNode* first, const LOCTreeNode* second) -> bool {
            assert(!first->name.empty());
            assert(!second->name.empty());

            return first->name < second->name;
        };

        std::sort(std::begin(children), std::end(children), SortPred);
    }

    LOCTreeNode* LOCTreeNode::ReadFromMemory(char* buffer, size_t bufferSize, LOCSupportMode supportMode)
    {
        switch (supportMode)
        {
            case LOCSupportMode::Hitman_BloodMoney:
            {
                auto root = new LOCTreeNode(nullptr, buffer);
                if (!BM::LOC::Internal::LOCTreeNodeVisitor<LOCSupportMode::Hitman_BloodMoney>::Visit(root, bufferSize))
                {
                    delete root;
                    return nullptr;
                }
                return root;
            }
            break;
            case LOCSupportMode::Hitman_Contracts:
            {
                auto root = LOCTreeFactory::Create();
                if (!BM::LOC::Internal::LOCTreeNodeVisitor<LOCSupportMode::Hitman_Contracts>::Visit(root, bufferSize))
                {
                    delete root;
                    return nullptr;
                }
                return root;
            }
            break;
            // ---< NOT SUPPORTED YET >---
            case LOCSupportMode::Hitman_2SA:
            case LOCSupportMode::Hitman_A47:
            default:
                return nullptr;
        }
    }

    static void VisitAndMarkNode(LOCTreeNode* node, LOCTreeNode::CacheDataBase& cache, const std::string& currentKey) // NOLINT(misc-no-recursion)
    {
        if (node->IsData())
        {
            std::string finalKey = currentKey;

            if (finalKey[finalKey.length() - 1] != '/')
                finalKey += '/';

            finalKey += node->name;
            cache[finalKey] = node->value;
        }
        else if (node->IsContainer() && node->numChild > 0)
        {
            std::string finalKey = currentKey;

            if (!node->IsRoot())
            {
                if (finalKey[finalKey.length() - 1] != '/')
                    finalKey += '/';

                finalKey += node->name;
            }

            for (int i = 0; i < node->numChild; i++)
            {
                VisitAndMarkNode(node->children[i], cache, finalKey);
            }
        }
    }

    void LOCTreeNode::GenerateCacheDataBase(LOCTreeNode* root, LOCTreeNode::CacheDataBase& cache)
    {
        VisitAndMarkNode(root, cache, "/");
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

    bool LOCTreeNode::Compare(LOCTreeNode* a, LOCTreeNode* b)
    {
        if (!a || !b)
        {
            return false;
        }
        if (a->IsRoot() != b->IsRoot())
        {
            return false;
        }
        if (!a->IsRoot() && a->name != b->name)
        {
            return false;
        }
        if (a->IsData() != b->IsData())
        {
            return false;
        }
        if (a->IsData() && a->value != b->value)
        {
            return false;
        }
        if (a->IsContainer() != b->IsContainer())
        {
            return false;
        }
        if (a->originalTypeRawData.has_value() != b->originalTypeRawData.has_value())
        {
            return false;
        }
        if (a->originalTypeRawData.has_value())
        {
            const auto& aOrgType = a->originalTypeRawData.value();
            const auto& bOrgType = b->originalTypeRawData.value();
            if (aOrgType != bOrgType)
            {
                return false;
            }
        }
        if (a->IsContainer())
        {
            if (a->numChild != b->numChild) return false;
            assert(a->numChild == a->children.size());
            assert(b->numChild == b->children.size());

            for (int i = 0; i < a->numChild; i++)
            {
                if (!LOCTreeNode::Compare(a->children[i], b->children[i]))
                {
                    return false;
                }
            }
        }
        return true;
    }
}