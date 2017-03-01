#include "CAnimTreeTimeScale.hpp"

namespace urde
{

CAnimTreeTimeScale::CAnimTreeTimeScale(const std::weak_ptr<CAnimTreeNode>& node, float scale, const std::string& name)
    : CAnimTreeSingleChild(node, name)
    , x18_timeScale(new CConstantAnimationTimeScale(scale))
{
}

std::string CAnimTreeTimeScale::CreatePrimitiveName(const std::weak_ptr<CAnimTreeNode>&, float, const CCharAnimTime&, float)
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
            return x18_timeScale->TimeScaleIntegral(x20_, x20_ + tmp);
        else
        {
            CCharAnimTime integral = x18_timeScale->TimeScaleIntegral(x20_, x28_);

            if (integral > tmp)
                return x18_timeScale->FindUpperLimit(x20_, tmp) * x20_;
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
