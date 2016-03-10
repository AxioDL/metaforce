#ifndef __URDE_CGUIANIMBASE_HPP__
#define __URDE_CGUIANIMBASE_HPP__

namespace urde
{
class CGuiAnimController;
class CGuiWidgetDrawParams;
class CGuiRandomVar;

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
    const CGuiRandomVar& x30_randomVar;
public:
    virtual ~CGuiAnimBase() = default;
    CGuiAnimBase(float, const CGuiRandomVar&, bool);

    virtual void AnimInit(const CGuiAnimController* controller, float);
    virtual void AnimUpdate(CGuiAnimController* controller, float dt);
    virtual void AnimDraw(const CGuiWidgetDrawParams& params) const;
    virtual void CalcInitVelocity(const CGuiAnimController* controller);
    virtual void GetAnimType(const CGuiAnimController* controller) const;
    virtual void GetItFinishedLoading() const {return true;}
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
