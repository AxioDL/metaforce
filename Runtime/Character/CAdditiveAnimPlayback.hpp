#ifndef __URDE_CADDITIVEANIMPLAYBACK_HPP__
#define __URDE_CADDITIVEANIMPLAYBACK_HPP__

#include "RetroTypes.hpp"

namespace urde
{
class CAnimTreeNode;
class CAdditiveAnimationInfo;
class CSegIdList;
class CCharLayoutInfo;
class CSegStatementSet;

class CAdditiveAnimationInfo
{
    float x0_fadeInDur = 0.f;
    float x4_fadeOutDur = 0.f;
public:
    void read(CInputStream& in)
    {
        x0_fadeInDur = in.readFloatBig();
        x4_fadeOutDur = in.readFloatBig();
    }
    CAdditiveAnimationInfo() = default;
    CAdditiveAnimationInfo(CInputStream& in) {read(in);}
    float GetFadeInDuration() const {return x0_fadeInDur;}
    float GetFadeOutDuration() const {return x4_fadeOutDur;}
};

class CAdditiveAnimPlayback
{
public:
    enum class EAdditivePlaybackPhase
    {
        None,
        FadingIn,
        FadingOut,
        FadedIn,
        FadedOut
    };
private:
    CAdditiveAnimationInfo x0_info;
    std::shared_ptr<CAnimTreeNode> x8_anim;
    float xc_targetWeight;
    float x10_curWeight = 0.f;
    bool x14_active;
    float x18_weightTimer = 0.f;
    EAdditivePlaybackPhase x1c_phase = EAdditivePlaybackPhase::FadingIn;
    bool x20_ = false;
public:
    CAdditiveAnimPlayback(const std::weak_ptr<CAnimTreeNode>& anim, float weight, bool active,
                          const CAdditiveAnimationInfo& info, bool b);

    void AddToSegStatementSet(const CSegIdList& list, const CCharLayoutInfo&, CSegStatementSet&) const;
    void Update(float dt);
    void FadeOut();
    void SetWeight(float w);
    float GetTargetWeight() const {return xc_targetWeight;}
    bool IsActive() const {return x14_active;}
    const std::shared_ptr<CAnimTreeNode>& GetAnim() const {return x8_anim;}
};

}

#endif // __URDE_CADDITIVEANIMPLAYBACK_HPP__
