#include "CAdditiveAnimPlayback.hpp"
#include "CSegStatementSet.hpp"

namespace urde
{

CAdditiveAnimPlayback::CAdditiveAnimPlayback(const std::weak_ptr<CAnimTreeNode>& anim, float weight, bool a,
                                             const CAdditiveAnimationInfo& info, bool b)
: x0_info(info), x8_anim(anim), xc_weight(weight), x14_a(a)
{
    if (!a && b)
        x20_ = true;
}

void CAdditiveAnimPlayback::AddToSegStatementSet(const CSegIdList& list,
                                                 const CCharLayoutInfo&,
                                                 CSegStatementSet&) const
{
}

void CAdditiveAnimPlayback::Update(float dt)
{
}

void CAdditiveAnimPlayback::FadeOut()
{
}

void CAdditiveAnimPlayback::SetWeight(float w)
{
}

}
