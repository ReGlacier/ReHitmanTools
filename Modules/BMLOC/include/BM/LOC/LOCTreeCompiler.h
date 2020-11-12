#pragma once

#include <BM/LOC/LOCTree.h>

#include <vector>

#include <cstdint>

namespace BM::LOC
{
    class LOCTreeCompiler
    {
    public:
        using Buffer = std::vector<uint8_t>;

        static bool Compile(Buffer& buffer, LOCTreeNode* rootNode);
    };
}