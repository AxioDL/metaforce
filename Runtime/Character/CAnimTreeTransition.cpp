#include "CAnimTreeTransition.hpp"

namespace urde
{

std::string CAnimTreeTransition::CreatePrimitiveName(const std::weak_ptr<CAnimTreeNode>&,
                                                     const std::weak_ptr<CAnimTreeNode>&, float)
{
    return {};
}

CAnimTreeTransition::CAnimTreeTransition(bool b1, const std::weak_ptr<CAnimTreeNode>& a,
                                         const std::weak_ptr<CAnimTreeNode>& b, const CCharAnimTime& transDur,
                                         const CCharAnimTime& timeInTrans, bool runA, bool loopA, int flags,
                                         std::string_view name, bool initialized)
: CAnimTreeTweenBase(b1, a, b, flags, name), x24_transDur(transDur), x2c_timeInTrans(timeInTrans),
  x34_runA(runA), x35_loopA(loopA), x36_initialized(initialized)
{
}

CAnimTreeTransition::CAnimTreeTransition(bool b1, const std::weak_ptr<CAnimTreeNode>& a,
                                         const std::weak_ptr<CAnimTreeNode>& b,
                                         const CCharAnimTime& transDur, bool runA,
                                         int flags, std::string_view name)
: CAnimTreeTweenBase(b1, a, b, flags, name), x24_transDur(transDur), x34_runA(runA),
  x35_loopA(a.lock()->VGetBoolPOIState("Loop"))
{
}

std::shared_ptr<IAnimReader> CAnimTreeTransition::VGetBestUnblendedChild() const
{
    std::shared_ptr<IAnimReader> child = x18_b->GetBestUnblendedChild();
    return (child ? child : x18_b);
}

CCharAnimTime CAnimTreeTransition::VGetTimeRemaining() const
{
    CCharAnimTime transTimeRem = x24_transDur - x2c_timeInTrans;
    CCharAnimTime rightTimeRem = x18_b->VGetTimeRemaining();
    return (rightTimeRem < transTimeRem) ? transTimeRem : rightTimeRem;
}

CSteadyStateAnimInfo CAnimTreeTransition::VGetSteadyStateAnimInfo() const
{
    CSteadyStateAnimInfo bInfo = x18_b->VGetSteadyStateAnimInfo();

    if (x24_transDur < bInfo.GetDuration())
        return CSteadyStateAnimInfo(bInfo.IsLooping(), bInfo.GetDuration(), bInfo.GetOffset());
    return CSteadyStateAnimInfo(bInfo.IsLooping(), x24_transDur, bInfo.GetOffset());
}

std::unique_ptr<IAnimReader> CAnimTreeTransition::VClone() const
{
    return std::make_unique<CAnimTreeTransition>(x20_24_b1, std::static_pointer_cast<CAnimTreeNode>(
                                                     std::shared_ptr<IAnimReader>(x14_a->Clone())),
                                                 std::static_pointer_cast<CAnimTreeNode>(
                                                     std::shared_ptr<IAnimReader>(x18_b->Clone())),
                                                 x24_transDur, x2c_timeInTrans, x34_runA,
                                                 x35_loopA, x1c_flags, x4_name, x36_initialized);
}

std::experimental::optional<std::unique_ptr<IAnimReader>> CAnimTreeTransition::VSimplified()
{
    if (zeus::close_enough(GetBlendingWeight(), 1.f))
    {
        if (auto simp = x18_b->Simplified())
            return simp;
        return {x18_b->Clone()};
    }
    return CAnimTreeTweenBase::VSimplified();
}

std::experimental::optional<std::unique_ptr<IAnimReader>> CAnimTreeTransition::VReverseSimplified()
{
    if (zeus::close_enough(GetBlendingWeight(), 0.f))
        return {x14_a->Clone()};
    return CAnimTreeTweenBase::VReverseSimplified();
}

SAdvancementResults CAnimTreeTransition::AdvanceViewForTransitionalPeriod(const CCharAnimTime& time)
{
    IncAdvancementDepth();
    CDoubleChildAdvancementResult res = AdvanceViewBothChildren(time, x34_runA, x35_loopA);
    DecAdvancementDepth();
    if (res.GetTrueAdvancement().EqualsZero())
        return {};

    float oldWeight = GetBlendingWeight();
    x2c_timeInTrans += res.GetTrueAdvancement();
    float newWeight = GetBlendingWeight();

    if (ShouldCullTree())
    {
        if (newWeight < 0.5f)
            x20_25_cullSelector = 1;
        else
            x20_25_cullSelector = 2;
    }

    if (x1c_flags & 0x1)
    {
        return {
            res.GetTrueAdvancement(),
            SAdvancementDeltas::Interpolate(res.GetLeftAdvancementDeltas(),
                                            res.GetRightAdvancementDeltas(),
                                            oldWeight, newWeight)
        };
    }

    return {
        res.GetTrueAdvancement(),
        res.GetRightAdvancementDeltas()
    };
}

SAdvancementResults CAnimTreeTransition::VAdvanceView(const CCharAnimTime& time)
{
    if (time.EqualsZero())
    {
        IncAdvancementDepth();
        x18_b->VAdvanceView(time);
        if (x34_runA)
            x14_a->VAdvanceView(time);
        DecAdvancementDepth();
        if (ShouldCullTree())
            x20_25_cullSelector = 1;
        return {};
    }

    if (!x36_initialized)
        x36_initialized = true;

    if (x2c_timeInTrans + time < x24_transDur)
    {
        SAdvancementResults res = AdvanceViewForTransitionalPeriod(time);
        res.x0_remTime = time - res.x0_remTime;
        return res;
    }

    CCharAnimTime transTimeRem = x24_transDur - x2c_timeInTrans;
    SAdvancementResults res;
    if (transTimeRem.GreaterThanZero())
    {
        res = AdvanceViewForTransitionalPeriod(transTimeRem);
        if (res.x0_remTime != transTimeRem)
            return res;
    }

    res.x0_remTime = time - transTimeRem;
    return res;
}

void CAnimTreeTransition::SetBlendingWeight(float w)
{
    std::static_pointer_cast<CAnimTreeTweenBase>(x18_b)->SetBlendingWeight(w);
}

float CAnimTreeTransition::VGetBlendingWeight() const
{
    if (x24_transDur.GreaterThanZero())
        return x2c_timeInTrans.GetSeconds() / x24_transDur.GetSeconds();
    return 0.f;
}
}
