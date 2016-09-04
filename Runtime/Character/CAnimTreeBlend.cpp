#include "CAnimTreeBlend.hpp"

namespace urde
{

std::string CAnimTreeBlend::CreatePrimitiveName(const std::shared_ptr<CAnimTreeNode>& a,
                                                const std::shared_ptr<CAnimTreeNode>& b,
                                                float scale)
{
    return "";
}

CAnimTreeBlend::CAnimTreeBlend(bool b1,
                               const std::shared_ptr<CAnimTreeNode>& a,
                               const std::shared_ptr<CAnimTreeNode>& b,
                               float blendWeight, const std::string& name)
: CAnimTreeTweenBase(b1, a, b, 1 | 2, name), x24_blendWeight(blendWeight)
{

}

SAdvancementResults CAnimTreeBlend::VAdvanceView(const CCharAnimTime& a)
{
}

CCharAnimTime CAnimTreeBlend::VGetTimeRemaining() const
{
}

CSteadyStateAnimInfo CAnimTreeBlend::VGetSteadyStateAnimInfo() const
{
}

std::shared_ptr<IAnimReader> CAnimTreeBlend::VClone() const
{
}

void CAnimTreeBlend::SetBlendingWeight(float w)
{
}

float CAnimTreeBlend::VGetBlendingWeight() const
{
}

}
