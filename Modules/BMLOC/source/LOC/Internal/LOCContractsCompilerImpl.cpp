#include <BM/LOC/Internal/LOCCompilerImpl.h>

namespace BM::LOC::Internal
{
    using Self = LOCCompilerImpl<LOCSupportMode::Hitman_Contracts>;

    bool Self::Compile(std::vector<uint8_t>& outputBuffer, LOCTreeNode* root)
    {
        return false;
    }

    bool Self::MarkupTree(LOCTreeNode* root)
    {
        return false;
    }
}