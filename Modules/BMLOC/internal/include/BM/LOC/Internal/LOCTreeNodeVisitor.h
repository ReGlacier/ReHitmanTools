#pragma once

#include <BM/LOC/LOCSupportMode.h>
#include <BM/LOC/LOCTree.h>
#include <set>

namespace BM::LOC::Internal
{
    template <LOCSupportMode supportMode>
    struct LOCTreeNodeVisitor
    {
        static bool Visit(LOCTreeNode* node, size_t bufferSize);
        static size_t Markup(LOCTreeNode* node, size_t startPosition);
    };

    // Implementations
    template <>
    struct LOCTreeNodeVisitor<LOCSupportMode::Hitman_BloodMoney>
    {
        static bool Visit(LOCTreeNode* treeNode, size_t bufferSize);
    };

    template <>
    struct LOCTreeNodeVisitor<LOCSupportMode::Hitman_Contracts>
    {
        static bool Visit(LOCTreeNode* node, size_t bufferSize);
    };
}