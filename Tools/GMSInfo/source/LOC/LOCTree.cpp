#include <LOC/LOCTree.h>

#include <cassert>

namespace ReGlacier
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
        }
    }

    bool LOCTreeNode::IsRoot() const
    {
        return parent == nullptr;
    }

    bool LOCTreeNode::IsEmpty() const
    {
        return numChild == 0 && nodeType == TreeNodeType::NODE_WITH_CHILDREN;
    }
}