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
    zeus::CFrustum x150_frustum;
    std::unique_ptr<CFluidPlaneCPU> x1b4_fluidPlane;
    zeus::CVector3f x1b8_position;
    zeus::CVector3f x1c4_extent;
    float x1d0_f2;
    zeus::CVector3f x1d4_position2;
    zeus::CVector3f x1e0_extent2;
    float x1ec_damage;
    float x1f0_damage2;
    float x1f4_;
    float x1f8_lightmapDoubleBlendFactor = 0.f;
    std::list<std::pair<TUniqueId, bool>> x1fc_waterInhabitants;
    u32 x210_;
    float x214_;
    float x218_;
    float x21c_;
    float x220_;
    float x224_;
    zeus::CColor x228_;
    ResId x22c_splashParticle1Id;
    ResId x230_splashParticle2Id;
    ResId x234_splashParticle3Id;
    ResId x238_particle4Id;
    std::experimental::optional<TLockedToken<CGenDescription>> x23c_;
    ResId x24c_particle5Id;
    std::experimental::optional<TLockedToken<CGenDescription>> x250_visorRunoffEffect;
    u16 x260_;
    u16 x262_visorRunoffSfx;
    rstl::reserved_vector<std::experimental::optional<TLockedToken<CGenDescription>>, 3> x264_splashEffects;
    rstl::reserved_vector<u16, 3> x298_splashSounds;
    zeus::CColor x2a4_c1;
    zeus::CColor x2a8_c2;
    float x2ac_lf2;
    float x2b0_lf3;
    float x2b4_;
    float x2b8_;
    float x2bc_alpha;
    float x2c0_tileSize;
    u32 x2c4_gridDimX = 0;
    u32 x2c8_gridDimY = 0;
    u32 x2cc_gridCellCount = 0;
    u32 x2d0_patchDimX = 0;
    u32 x2d4_patchDimY = 0;
    std::unique_ptr<bool[]> x2d8_gridFlags;
    std::unique_ptr<bool[]> x2dc_;
    std::unique_ptr<bool[]> x2e0_patchFlags;
    u32 x2e4_gridCellCount2 = 0;
    union
    {
        struct
        {
            bool x2e8_24_b4 : 1;
            bool x2e8_25 : 1;
            bool x2e8_26 : 1;
            bool x2e8_27_b2 : 1;
            bool x2e8_28 : 1;
            bool x2e8_29 : 1;
            bool x2e8_30 : 1;
        };
        u32 _dummy = 0;
    };
    void SetupGrid(bool b);
public:
    CScriptWater(CStateManager& mgr, TUniqueId uid, const std::string& name, const CEntityInfo& info,
                 const zeus::CVector3f& pos, const zeus::CAABox& box, const urde::CDamageInfo& dInfo,
                 zeus::CVector3f& orientedForce, ETriggerFlags triggerFlags, bool b1, bool b2,
                 ResId patternMap1, ResId patternMap2, ResId colorMap, ResId bumpMap, ResId envMap,
                 ResId envBumpMap, ResId unusedMap, const zeus::CVector3f& bumpLightDir, float bumpScale,
                 float f2, float f3, bool active, CFluidPlane::EFluidType fluidType, bool b4, float alpha,
                 const CFluidUVMotion& uvMot, float turbSpeed, float turbDistance, float turbFreqMax,
                 float turbFreqMin, float turbPhaseMax, float turbPhaseMin, float turbAmplitudeMax,
                 float turbAmplitudeMin, const zeus::CColor& c1, const zeus::CColor& c2, ResId splashParticle1,
                 ResId splashParticle2, ResId splashParticle3, ResId particle4, ResId particle5, s32 i1,
                 s32 visorRunoffSfx, s32 splashSfx1, s32 splashSfx2, s32 splashSfx3, float tileSize,
                 u32 tileSubdivisions, float specularMin, float specularMax, float reflectionSize,
                 float fluidPlaneF2, float reflectionBlend, float slF6, float slF7, float slF8,
                 const zeus::CColor& c3, ResId lightmapId, float unitsPerLightmapTexel, float lF2, float lF3,
                 u32, u32, bool, s32, s32, std::unique_ptr<u32[]>&& u32Arr);

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
    const std::experimental::optional<TLockedToken<CGenDescription>>& GetSplashEffect(float) const;
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
    const std::experimental::optional<TLockedToken<CGenDescription>>& GetVisorRunoffEffect() const
    { return x250_visorRunoffEffect; }
    u16 GetVisorRunoffSfx() const { return x262_visorRunoffSfx; }
    const CScriptWater* GetNextConnectedWater(const CStateManager& mgr) const;
    float GetLightmapDoubleBlendFactor() const { return x1f8_lightmapDoubleBlendFactor; }
    bool CanRenderPatch(int x, int y) const { return x2e0_patchFlags[y * x2d0_patchDimX + x]; }
    u32 GetPatchDimensionX() const { return x2d0_patchDimX; }
    u32 GetPatchDimensionY() const { return x2d4_patchDimY; }
};
}

#endif // __URDE_CSCRIPTWATER_HPP__
