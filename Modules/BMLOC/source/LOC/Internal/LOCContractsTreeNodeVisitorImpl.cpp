#include <BM/LOC/Internal/LOCTreeNodeVisitor.h>

namespace BM::LOC::Internal
{
    using Self = LOCTreeNodeVisitor<LOCSupportMode::Hitman_Contracts>;

    bool Self::Visit(LOCTreeNode* node, size_t bufferSize)
    {
        return false;
    }
}