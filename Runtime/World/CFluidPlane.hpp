#ifndef __URDE_CFLUIDPLANE_HPP__
#define __URDE_CFLUIDPLANE_HPP__

#include "RetroTypes.hpp"
namespace urde
{
class CFluidUVMotion;
class CRippleManager;
class CScriptWater;
class CStateManager;
class CFluidPlane
{
public:
    enum class EFluidType
    {
        Zero,
        One,
        Two,
        Three,
        Four,
        Five
    };

private:
    u32 x4_;
    u32 x8_;
    u32 xc_;
    float x40_;
    EFluidType x44_fluidType;
    float x48_;
public:
    CFluidPlane(u32, u32, u32, EFluidType, float, const CFluidUVMotion&, float);

    virtual void Ripple(float mag, TUniqueId rippler, const zeus::CVector3f& pos,
                        CScriptWater& water, CStateManager& mgr);
    virtual void Update();
    float GetAlpha() const;
    EFluidType GetFluidType() const { return x44_fluidType; }
    const CFluidUVMotion& GetUVMotion() const;
    void GetColorTexture() const;
    bool HasColorTexture() const;
    void GetTexturePattern1() const;
    bool HasTexturePattern1() const;
    void GetTexturePattern2() const;
    bool HasTexturePattern2() const;
};
}

#endif // __URDE_CFLUIDPLANE_HPP__
