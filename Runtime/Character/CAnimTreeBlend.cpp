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
                               float blendWeight, std::string_view name)
: CAnimTreeTweenBase(b1, a, b, 1 | 2, name), x24_blendWeight(blendWeight)
{

}

SAdvancementResults CAnimTreeBlend::VAdvanceView(const CCharAnimTime& a)
{
    return {};
}

CCharAnimTime CAnimTreeBlend::VGetTimeRemaining() const
{
    return {};
}

CSteadyStateAnimInfo CAnimTreeBlend::VGetSteadyStateAnimInfo() const
{
    return {};
}

std::unique_ptr<IAnimReader> CAnimTreeBlend::VClone() const
{
    return {};
}

void CAnimTreeBlend::SetBlendingWeight(float w)
{
}

float CAnimTreeBlend::VGetBlendingWeight() const
{
    return 0.f;
}

}
