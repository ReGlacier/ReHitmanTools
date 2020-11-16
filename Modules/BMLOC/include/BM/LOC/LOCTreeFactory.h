#pragma once

#include <string>
#include <BM/LOC/LOCTypes.h>

namespace BM::LOC
{
    struct LOCTreeNode;

    class LOCTreeFactory
    {
    public:
        static LOCTreeNode* Create();
        static LOCTreeNode* Create(const std::string& name, TreeNodeType nodeType, LOCTreeNode* parent = nullptr);
        static LOCTreeNode* Create(const std::string& name, const std::string& value, LOCTreeNode* parent = nullptr);
    };
}