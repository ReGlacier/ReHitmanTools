#include <BM/LOC/LOCTreeFactory.h>
#include <BM/LOC/LOCTree.h>

#include <stdexcept>

namespace BM::LOC
{
    LOCTreeNode * LOCTreeFactory::Create()
    {
        auto newNode = new LOCTreeNode(nullptr, nullptr);
        newNode->nodeType = TreeNodeType::NODE_WITH_CHILDREN;

        return newNode;
    }

    LOCTreeNode* LOCTreeFactory::Create(const std::string& name, TreeNodeType nodeType, LOCTreeNode* parent)
    {
        if (name.empty() && (parent || nodeType != TreeNodeType::NODE_WITH_CHILDREN))
        {
            throw std::runtime_error { "LOC Compiler Error: Empty name allowed only for root node of type TreeNodeType::NODE_WITH_CHILDREN!" };
        }

        if (name.find('/') != std::string::npos)
        {
            throw std::runtime_error { "LOC Compiler Error: Bad reference '" + name + "'. Special characters are not allowed in keys!" };
        }

        auto newNode = new LOCTreeNode(parent, nullptr);
        newNode->nodeType = nodeType;
        if (!name.empty())
        {
            newNode->name = name;
        }

        return newNode;
    }

    LOCTreeNode* LOCTreeFactory::Create(const std::string& name, const std::string& value, LOCTreeNode* parent)
    {
        if (name.empty())
        {
            throw std::runtime_error { "LOC Compiler Error: Empty name not allowed for key-value node!" };
        }

        if (value.empty())
        {
            throw std::runtime_error { "LOC Compiler Error: Empty value not allowed!" };
        }

        if (name.find('/') != std::string::npos)
        {
            throw std::runtime_error { "LOC Compiler Error: Bad reference '" + name + "'. Special characters are not allowed in keys!" };
        }

        auto newNode = new LOCTreeNode(parent, nullptr);
        newNode->nodeType = TreeNodeType::VALUE_OR_DATA;
        newNode->name = name;
        newNode->value = value;

        return newNode;
    }
}