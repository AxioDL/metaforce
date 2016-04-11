#include "CAnimTreeNode.hpp"

namespace urde
{

u32 CAnimTreeNode::GetNumChildren() const
{
    return VGetNumChildren();
}

std::shared_ptr<IAnimReader> CAnimTreeNode::GetBestUnblendedChild() const
{
    return VGetBestUnblendedChild();
}

}
