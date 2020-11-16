#include <BM/LOC/Internal/LOCTreeNodeVisitor.h>

namespace BM::LOC::Internal
{
    using Self = LOCTreeNodeVisitor<LOCSupportMode::Hitman_Contracts>;

    bool Self::Visit(LOCTreeNode* treeNode, size_t bufferSize)
    {
        return false;
    }
}