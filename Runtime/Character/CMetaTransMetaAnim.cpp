#include "CMetaTransMetaAnim.hpp"
#include "CMetaAnimFactory.hpp"

namespace urde
{

CMetaTransMetaAnim::CMetaTransMetaAnim(CInputStream& in)
: x4_metaAnim(CMetaAnimFactory::CreateMetaAnim(in)) {}

std::shared_ptr<CAnimTreeNode>
CMetaTransMetaAnim::VGetTransitionTree(const std::weak_ptr<CAnimTreeNode>& a,
                                       const std::weak_ptr<CAnimTreeNode>& b,
                                       const CAnimSysContext& animSys) const
{
}

}
