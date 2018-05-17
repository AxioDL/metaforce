#include "CAdditiveAnimPlayback.hpp"
#include "CSegStatementSet.hpp"
#include "CCharLayoutInfo.hpp"
#include "CAnimTreeNode.hpp"

namespace urde
{

CAdditiveAnimPlayback::CAdditiveAnimPlayback(const std::weak_ptr<CAnimTreeNode>& anim,
                                             float weight, bool active, const CAdditiveAnimationInfo& info,
                                             bool fadeOut)
: x0_info(info), x8_anim(anim.lock()), xc_targetWeight(weight), x14_active(active)
{
    if (!active && fadeOut)
        x20_needsFadeOut = true;
}

void CAdditiveAnimPlayback::AddToSegStatementSet(const CSegIdList& list,
                                                 const CCharLayoutInfo& layout,
                                                 CSegStatementSet& setOut) const
{
    CSegStatementSet stackSet;
    x8_anim->VGetSegStatementSet(list, stackSet);
    for (const CSegId& id : list.GetList())
    {
        CAnimPerSegmentData& data = stackSet.x4_segData[id];
        data.x10_offset = layout.GetFromParentUnrotated(id);
        data.x1c_hasOffset = true;
    }
    setOut.Add(list, layout, stackSet, x10_curWeight);
}

void CAdditiveAnimPlayback::Update(float dt)
{
    switch (x1c_phase)
    {
    case EAdditivePlaybackPhase::FadingIn:
    {
        float a = x0_info.GetFadeInDuration();
        float b = x18_weightTimer + dt;
        x18_weightTimer = std::min(b, a);
        if (a > 0.f)
            x10_curWeight = x18_weightTimer / a * xc_targetWeight;
        else
            x10_curWeight = xc_targetWeight;

        if (std::fabs(x10_curWeight - xc_targetWeight) < 0.00001f)
            x1c_phase = EAdditivePlaybackPhase::FadedIn;

        break;
    }
    case EAdditivePlaybackPhase::FadingOut:
    {
        float a = x18_weightTimer - dt;
        x18_weightTimer = std::max(a, 0.f);
        if (x0_info.GetFadeOutDuration() > 0.f)
            x10_curWeight = x18_weightTimer / x0_info.GetFadeOutDuration() * xc_targetWeight;
        else
            x10_curWeight = 0.f;

        if (std::fabs(x10_curWeight) < 0.00001f)
            x1c_phase = EAdditivePlaybackPhase::FadedOut;
    }
    default: break;
    }
}

void CAdditiveAnimPlayback::FadeOut()
{
    switch (x1c_phase)
    {
    case EAdditivePlaybackPhase::FadedOut:
    case EAdditivePlaybackPhase::FadedIn:
        x18_weightTimer = x0_info.GetFadeOutDuration();
        break;
    case EAdditivePlaybackPhase::FadingIn:
        x18_weightTimer = x18_weightTimer / x0_info.GetFadeInDuration() * x0_info.GetFadeOutDuration();
    default: break;
    }

    if (x0_info.GetFadeOutDuration() > 0.f)
        x1c_phase = EAdditivePlaybackPhase::FadingOut;
    else
        x1c_phase = EAdditivePlaybackPhase::FadedOut;
    x10_curWeight = 0.f;
}

void CAdditiveAnimPlayback::SetWeight(float w)
{
    xc_targetWeight = w;
    switch (x1c_phase)
    {
    case EAdditivePlaybackPhase::FadingIn:
    {
        if (x0_info.GetFadeInDuration() > 0.f)
            x10_curWeight = x18_weightTimer / x0_info.GetFadeInDuration() * xc_targetWeight;
        else
            x10_curWeight = xc_targetWeight;
        break;
    }
    case EAdditivePlaybackPhase::FadingOut:
    {
        if (x0_info.GetFadeOutDuration() > 0.f)
            x10_curWeight = x18_weightTimer / x0_info.GetFadeOutDuration() * xc_targetWeight;
        else
            x10_curWeight = xc_targetWeight;
        break;
    }
    default:
        x10_curWeight = xc_targetWeight;
        break;
    }
}

}
