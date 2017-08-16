#ifndef __URDE_CFLUIDPLANE_HPP__
#define __URDE_CFLUIDPLANE_HPP__

#include "RetroTypes.hpp"
#include "CToken.hpp"
#include "Graphics/CTexture.hpp"
#include "CFluidUVMotion.hpp"
#include "zeus/CAABox.hpp"
#include "zeus/CFrustum.hpp"

namespace urde
{
class CFluidUVMotion;
class CRippleManager;
class CScriptWater;
class CStateManager;
class CRipple;
class CFluidPlane
{
public:
    enum class EFluidType
    {
        NormalWater,
        PoisonWater,
        Lava,
        PhazonFluid,
        Four,
        ThickLava
    };

protected:
    CAssetId x4_texPattern1Id;
    CAssetId x8_texPattern2Id;
    CAssetId xc_texColorId;
    std::experimental::optional<TLockedToken<CTexture>> x10_texPattern1;
    std::experimental::optional<TLockedToken<CTexture>> x20_texPattern2;
    std::experimental::optional<TLockedToken<CTexture>> x30_texColor;
    float x40_alpha;
    EFluidType x44_fluidType;
    float x48_rippleIntensity;
    CFluidUVMotion x4c_uvMotion;
    float ProjectRippleVelocity(float baseI, float velDot) const;
    float CalculateRippleIntensity(float baseI) const;
public:
    CFluidPlane(CAssetId texPattern1, CAssetId texPattern2, CAssetId texColor, float alpha,
                EFluidType fluidType, float rippleIntensity, const CFluidUVMotion& motion);

    // Called by CPlayer, CMorphBall, CWeapon, CPuddleSpore, CMagdolite
    virtual void AddRipple(float mag, TUniqueId rippler, const zeus::CVector3f& center,
                           CScriptWater& water, CStateManager& mgr);

    // Called by CAi
    virtual void AddRipple(float intensity, TUniqueId rippler, const zeus::CVector3f& center,
                           const zeus::CVector3f& velocity, const CScriptWater& water, CStateManager& mgr,
                           const zeus::CVector3f& upVec);

    virtual void AddRipple(const CRipple& ripple, const CScriptWater& water, CStateManager& mgr);

    virtual void Render(const CStateManager& mgr, float alpha, const zeus::CAABox& aabb, const zeus::CTransform& xf,
                        const zeus::CTransform& areaXf, bool noNormals, const zeus::CFrustum& frustum,
                        const std::experimental::optional<CRippleManager>& rippleManager, TUniqueId waterId,
                        const bool* gridFlags, u32 gridDimX, u32 gridDimY, const zeus::CVector3f& areaCenter) const {}

    float GetAlpha() const { return x40_alpha; }
    EFluidType GetFluidType() const { return x44_fluidType; }
    const CFluidUVMotion& GetUVMotion() const { return x4c_uvMotion; }
    const CTexture& GetColorTexture() const { return **x30_texColor; }
    bool HasColorTexture() const { return x30_texColor.operator bool(); }
    const CTexture& GetTexturePattern1() const { return **x10_texPattern1; }
    bool HasTexturePattern1() const { return x10_texPattern1.operator bool(); }
    const CTexture& GetTexturePattern2() const { return **x20_texPattern2; }
    bool HasTexturePattern2() const { return x20_texPattern2.operator bool(); }
};
}

#endif // __URDE_CFLUIDPLANE_HPP__
