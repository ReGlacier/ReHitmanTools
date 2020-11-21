#include <PRP/PRPTreeNode.h>

namespace ReGlacier
{
    PRPTreeNode::PRPTreeNode(PRP_ETag tag) : _tag(tag) {}

    PRPTreeNode::PRPTreeNode(PRPTreeNode* parent, PRP_ETag tag) : _parent(parent), _tag(tag) {}

    PRPTreeNode::~PRPTreeNode()
    {
        for (const auto& child : _children)
        {
            delete child;
        }
        _children.clear();
    }

    bool PRPTreeNode::IsRoot() const { return _parent == nullptr; }

    void PRPTreeNode::AddChild(PRPTreeNode* node)
    {
        if (!node) return;
        node->_parent = this;
        _children.push_back(node);
    }

    void PRPTreeNode::RemoveChild(PRPTreeNode* node)
    {
        if (!node) return;
        if (node->_parent != this) return;
        node->_parent = nullptr;
        auto it = std::find(std::begin(_children), std::end(_children), node);
        if (it != std::end(_children))
        {
            _children.erase(it);
        }
    }

    const std::vector<PRPTreeNode*>& PRPTreeNode::GetChildNodes() const
    {
        return _children;
    }

    PRPTreeNode * PRPTreeNode::GetParent()
    {
        return _parent;
    }

    bool PRPTreeNode::HasRefToBuffer() const
    {
        return _inBufferOffset.has_value();
    }

    size_t PRPTreeNode::GetRefToBuffer() const
    {
        return _inBufferOffset.has_value() ? _inBufferOffset.value() : 0;
    }

    void PRPTreeNode::SetRefToBuffer(size_t offset)
    {
        _inBufferOffset = offset;
    }

    PRP_ETag PRPTreeNode::GetTag() const
    {
        return _tag;
    }

    void PRPTreeNode::SetTag(PRP_ETag tag)
    {
        _tag = tag;
    }
}