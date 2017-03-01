#include "CAnimTreeTransition.hpp"

namespace urde
{

std::string CAnimTreeTransition::CreatePrimitiveName(const std::weak_ptr<CAnimTreeNode>&,
                                                     const std::weak_ptr<CAnimTreeNode>&, float)
{
    return {};
}

CAnimTreeTransition::CAnimTreeTransition(bool b1, const std::weak_ptr<CAnimTreeNode>& a,
                                         const std::weak_ptr<CAnimTreeNode>& b, const CCharAnimTime& time1,
                                         const CCharAnimTime& time2, bool b2, bool b3, int flags,
                                         const std::string& name, bool b4)
: CAnimTreeTweenBase(b1, a, b, flags, name), x24_(time1), x2c_(time2), x34_(b2), x35_(b3), x36_(b4)
{
}

CAnimTreeTransition::CAnimTreeTransition(bool b1, const std::weak_ptr<CAnimTreeNode>& a,
                                         const std::weak_ptr<CAnimTreeNode>& b, const CCharAnimTime& time, bool b2,
                                         int flags, const std::string& name)
: CAnimTreeTweenBase(b1, a, b, flags, name), x24_(time), x34_(b2), x35_(a.lock()->VGetBoolPOIState("Loop"))
{
}

std::shared_ptr<IAnimReader> CAnimTreeTransition::VGetBestUnblendedChild() const
{
    std::shared_ptr<IAnimReader> child = x18_b->GetBestUnblendedChild();
    return (child ? child : x18_b);
}

CCharAnimTime CAnimTreeTransition::VGetTimeRemaining() const
{
    CCharAnimTime time = x24_ * x2c_;
    CCharAnimTime bTimeRem = x18_b->VGetTimeRemaining();

    if (time < bTimeRem)
        return bTimeRem;
    return time;
}

CSteadyStateAnimInfo CAnimTreeTransition::VGetSteadyStateAnimInfo() const
{
    CSteadyStateAnimInfo bInfo = x18_b->VGetSteadyStateAnimInfo();

    if (x24_ < bInfo.GetDuration())
        return CSteadyStateAnimInfo(bInfo.IsLooping(), bInfo.GetDuration(), bInfo.GetOffset());
    return CSteadyStateAnimInfo(bInfo.IsLooping(), x24_, bInfo.GetOffset());
}

std::unique_ptr<IAnimReader> CAnimTreeTransition::VClone() const
{
    return std::make_unique<CAnimTreeTransition>(x20_31_b1, std::static_pointer_cast<CAnimTreeNode>(
                                                     std::shared_ptr<IAnimReader>(x14_a->Clone())),
                                                 std::static_pointer_cast<CAnimTreeNode>(
                                                     std::shared_ptr<IAnimReader>(x18_b->Clone())), x24_, x2c_,
                                                 x34_, x35_, x1c_flags, x4_name, x36_);
}

SAdvancementResults CAnimTreeTransition::VAdvanceView(const CCharAnimTime& time) const
{
    return {};
}

void CAnimTreeTransition::SetBlendingWeight(float w)
{
    std::static_pointer_cast<CAnimTreeTweenBase>(x18_b)->SetBlendingWeight(w);
}

float CAnimTreeTransition::VGetBlendingWeight() const
{
    if (x24_.GreaterThanZero())
        return (1.f / x24_) * x2c_;
    return 0.f;
}
}
