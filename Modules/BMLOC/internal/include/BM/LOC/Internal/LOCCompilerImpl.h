#pragma once

#include <BM/LOC/LOCSupportMode.h>
#include <BM/LOC/LOCTree.h>

#include <vector>

namespace BM::LOC::Internal
{
    template <LOCSupportMode supportMode>
    struct LOCCompilerImpl
    {
        static bool MarkupTree(LOCTreeNode* root);
        static bool Compile(std::vector<uint8_t>& outputBuffer, LOCTreeNode* root);
    };

    /// ---- IMPL FOR HITMAN BLOOD MONEY ----
    template <>
    struct LOCCompilerImpl<LOCSupportMode::Hitman_BloodMoney>
    {
        static bool MarkupTree(LOCTreeNode* root);
        static bool Compile(std::vector<uint8_t>& outputBuffer, LOCTreeNode* root);

    private:
        static size_t CalculateSizeForEntry(LOCTreeNode* node, size_t startPosition);
    };

    /// ---- IMPL FOR HITMAN CONTRACTS ----
    template <>
    struct LOCCompilerImpl<LOCSupportMode::Hitman_Contracts>
    {
        static bool MarkupTree(LOCTreeNode* root);
        static bool Compile(std::vector<uint8_t>& outputBuffer, LOCTreeNode* root);
    };
}