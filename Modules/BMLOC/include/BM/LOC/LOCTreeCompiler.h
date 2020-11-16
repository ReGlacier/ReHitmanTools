#pragma once

#include <BM/LOC/LOCTree.h>
#include <BM/LOC/LOCSupportMode.h>

#include <vector>

#include <cstdint>

namespace BM::LOC
{
    class LOCTreeCompiler
    {
    public:
        using Buffer = std::vector<uint8_t>;

        static bool Compile(Buffer& buffer, LOCTreeNode* rootNode, LOCSupportMode supportMode = LOCSupportMode::Generic);
        static void MarkupTree(LOCTreeNode* rootNode);
    };
}