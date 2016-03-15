#ifndef __URDE_CGUIANIMBASE_HPP__
#define __URDE_CGUIANIMBASE_HPP__

#include "RetroTypes.hpp"
#include "CGuiRandomVar.hpp"

namespace urde
{
class CGuiAnimController;
class CGuiWidgetDrawParams;

enum class EGuiAnimType
{
    Rotation = 0,
    Translation = 1,
    Scaling = 2,
    Shearing = 3,
    Dissolve = 4,
    Keyframe = 5,
    Particles = 6,
    SFX = 9
};

class CGuiAnimBase
{
    friend class CGuiAnimSet;
    float x4_ = 0.f;
    float x8_ = 0.f;
    float xc_;
    CGuiRandomVar x10_randomVar;
    float x1c_ = 0.f;
    bool x20_isDone = false;
    s32 x24_ = -1;
    bool x28_;
public:
    virtual ~CGuiAnimBase() = default;
    CGuiAnimBase(float fval, const CGuiRandomVar& randVar, bool flag)
    : xc_(fval), x10_randomVar(randVar), x28_(flag) {}

    float GetFVal() const {return xc_;}
    const CGuiRandomVar& GetRandomVar() const {return x10_randomVar;}

    virtual void AnimInit(const CGuiAnimController* controller, float);
    virtual void AnimUpdate(CGuiAnimController* controller, float dt);
    virtual void AnimDraw(const CGuiWidgetDrawParams& params) const;
    virtual void CalcInitVelocity(const CGuiAnimController* controller);
    virtual void GetAnimType(const CGuiAnimController* controller) const;
    virtual bool GetIsFinishedLoading() const {return true;}
};

class CGuiAnimRotation : public CGuiAnimBase
{
};

class CGuiAnimTranslation : public CGuiAnimBase
{
};

class CGuiAnimScaling : public CGuiAnimBase
{
};

class CGuiAnimShearing : public CGuiAnimBase
{
};

class CGuiAnimDissolve : public CGuiAnimBase
{
};

class CGuiAnimKeyframe : public CGuiAnimBase
{
};

class CGuiAnimParticles : public CGuiAnimBase
{
};

class CGuiAnimSFX : public CGuiAnimBase
{
};

}

#endif // __URDE_CGUIANIMBASE_HPP__
