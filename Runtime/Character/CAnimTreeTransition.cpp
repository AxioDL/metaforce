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

CCharAnimTime CAnimTreeTransition::VGetTimeRemaining() const { return {}; }

CSteadyStateAnimInfo CAnimTreeTransition::VGetSteadyStateAnimInfo() const
{
    return {};
}

std::shared_ptr<IAnimReader> CAnimTreeTransition::VClone() const { return {}; }

void CAnimTreeTransition::SetBlendingWeight(float w)
{
    static_cast<CAnimTreeTweenBase*>(x18_b.get())->SetBlendingWeight(w);
}

float CAnimTreeTransition::VGetBlendingWeight() const
{
    if (x24_.GreaterThanZero())
        return (1.f / x24_) * x2c_;
    return 0.f;
}
}
