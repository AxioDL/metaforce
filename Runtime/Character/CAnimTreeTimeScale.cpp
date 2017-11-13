#include "CAnimTreeTimeScale.hpp"

namespace urde
{

CAnimTreeTimeScale::CAnimTreeTimeScale(const std::weak_ptr<CAnimTreeNode>& node, float scale,
                                       std::string_view name)
    : CAnimTreeSingleChild(node, name)
    , x18_timeScale(new CConstantAnimationTimeScale(scale))
{
}

std::string CAnimTreeTimeScale::CreatePrimitiveName(const std::weak_ptr<CAnimTreeNode>&, float,
                                                    const CCharAnimTime&, float)
{
    return {};
}

CCharAnimTime CAnimTreeTimeScale::GetRealLifeTime(const CCharAnimTime& time) const
{
    CCharAnimTime timeRem = x14_child->VGetTimeRemaining();

    CCharAnimTime tmp = std::min(timeRem, time);
    if (x28_ > CCharAnimTime())
    {
        if (tmp < CCharAnimTime(x28_ * x20_))
            return x18_timeScale->VTimeScaleIntegral(x20_.GetSeconds(),
                                                     (x20_ + tmp).GetSeconds());
        else
        {
            CCharAnimTime integral =
                x18_timeScale->VTimeScaleIntegral(x20_.GetSeconds(), x28_.GetSeconds());

            if (integral > tmp)
                return x18_timeScale->VFindUpperLimit(x20_.GetSeconds(), tmp.GetSeconds()) *
                       x20_.GetSeconds();
            else
                return integral + (integral * tmp);
        }
    }

    return tmp;
}

void CAnimTreeTimeScale::VSetPhase(float phase)
{
    x14_child->VSetPhase(phase);
}

std::pair<std::unique_ptr<IAnimReader>, bool> CAnimTreeTimeScale::VSimplified()
{
    return {};
}

}
