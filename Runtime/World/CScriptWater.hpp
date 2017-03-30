#ifndef __URDE_CSCRIPTWATER_HPP__
#define __URDE_CSCRIPTWATER_HPP__

#include "CScriptTrigger.hpp"
#include "CFluidPlaneCPU.hpp"
namespace urde
{

class CDamageInfo;
class CFluidUVMotion;

class CScriptWater : public CScriptTrigger
{
private:
    static const float kSplashScales[6];
    u32 x150_ = 0;
    std::unique_ptr<CFluidPlaneCPU> x1b4_fluidPlane;
    zeus::CVector3f x1b8_;
    float x1f4_;
    float x1f8_ = 0.f;
    zeus::CVector3f x1d4_;
    std::vector<std::pair<TUniqueId, bool>> x200_waterInhabitants;
    u32 x210_;
    float x214_;
    float x218_;
    float x21c_;
    float x220_;
    float x224_;
    zeus::CColor x228_;
    u32 x22c_;
    u32 x230_;
    u32 x234_;
    u32 x238_;
    u32 x248_;
    u32 x24c_;
    bool x25c_;
    s16 x260_;
    s16 x262_;
    u32 x264_ = 0;
    TLockedToken<CParticleGen> x268_splashEffects[5];
    s16 x29c_splashSounds[5];
public:
    CScriptWater(CStateManager&, TUniqueId, const std::string& name, const CEntityInfo&, const zeus::CVector3f&,
                 const zeus::CAABox&, CDamageInfo const&, zeus::CVector3f&, ETriggerFlags, bool, bool, ResId, ResId, ResId, ResId,
                 ResId, ResId, ResId, const zeus::CVector3f&, float, float, float, bool, CFluidPlane::EFluidType, bool,
                 float, const CFluidUVMotion&, float, float, float, float, float, float, float, float,
                 const zeus::CColor&, const zeus::CColor&, ResId, ResId, ResId, ResId, ResId, s32, s32, s32, s32, s32,
                 float, u32, float, float, float, float, float, float, float, float, const zeus::CColor&, ResId, float,
                 float, float, u32, u32, bool, s32, s32, const u32*);

    void Think(float, CStateManager&);
    void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);
    void PreRender(CStateManager &, const zeus::CFrustum &);
    void AddToRenderer(const zeus::CFrustum&, const CStateManager&) const;
    void Render(const CStateManager&) const;
    void Touch(CActor &, CStateManager &);
    void CalculateRenderBounds();
    zeus::CAABox GetSortingBounds(const CStateManager&) const;
    void RenderSurface();
    EWeaponCollisionResponseTypes GetCollisionResponseType(const zeus::CVector3f&, const zeus::CVector3f&, CWeaponMode&,
                                                           int);

    void UpdateSplashInhabitants(CStateManager&);
    s16 GetSplashSound(float) const;
    const TLockedToken<CParticleGen>& GetSplashEffect(float) const;
    float GetSplashEffectScale(float) const;
    u32 GetSplashIndex(float) const;
    void FluidPlane();
    zeus::CPlane GetWRSurfacePlane() const;
    float GetSurfaceZ() const;
    bool IsMorphing() const;
    void SetMorphing(bool);
    zeus::CColor GetSplashColor() const;
    void SetFrustumPlanes(const zeus::CFrustum& frustum);
    const zeus::CFrustum& GetFrustumPlanes() const;
    CFluidPlaneCPU& GetFluidPlane() const { return *x1b4_fluidPlane; }
};
}

#endif // __URDE_CSCRIPTWATER_HPP__
