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
    float x0_a = 0.f;
    float x4_b = 0.f;
public:
    void read(CInputStream& in)
    {
        x0_a = in.readFloatBig();
        x4_b = in.readFloatBig();
    }
    CAdditiveAnimationInfo() = default;
    CAdditiveAnimationInfo(CInputStream& in) {read(in);}
};

class CAdditiveAnimPlayback
{
public:
    enum class EAdditivePlaybackState
    {
        Zero,
        One,
        Two,
        Three,
        Four
    };
private:
    CAdditiveAnimationInfo x0_info;
    std::weak_ptr<CAnimTreeNode> x8_anim;
    float xc_weight;
    float x10_ = 0.f;
    bool x14_a;
    float x18_ = 0.f;
    EAdditivePlaybackState x1c_ = EAdditivePlaybackState::One;
    bool x20_ = false;
public:
    CAdditiveAnimPlayback(const std::weak_ptr<CAnimTreeNode>& anim, float weight, bool a,
                          const CAdditiveAnimationInfo& info, bool b);

    void AddToSegStatementSet(const CSegIdList& list, const CCharLayoutInfo&, CSegStatementSet&) const;
    void Update(float dt);
    void FadeOut();
    void SetWeight(float w);
};

}

#endif // __URDE_CADDITIVEANIMPLAYBACK_HPP__
