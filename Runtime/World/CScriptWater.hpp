#pragma once

#include <list>
#include <memory>
#include <optional>
#include <utility>

#include "Runtime/RetroTypes.hpp"
#include "Runtime/rstl.hpp"
#include "Runtime/World/CFluidPlaneCPU.hpp"
#include "Runtime/World/CScriptTrigger.hpp"

#include <zeus/CColor.hpp>
#include <zeus/CFrustum.hpp>
#include <zeus/CVector3f.hpp>

namespace urde {

class CDamageInfo;
class CFluidUVMotion;

class CScriptWater : public CScriptTrigger {
  zeus::CFrustum x150_frustum;
  std::unique_ptr<CFluidPlaneCPU> x1b4_fluidPlane;
  zeus::CVector3f x1b8_positionMorphed;
  zeus::CVector3f x1c4_extentMorphed;
  float x1d0_morphInTime;
  zeus::CVector3f x1d4_positionOrig;
  zeus::CVector3f x1e0_extentOrig;
  float x1ec_damageOrig;
  float x1f0_damageMorphed;
  float x1f4_morphOutTime;
  float x1f8_morphFactor = 0.f;
  std::list<std::pair<TUniqueId, bool>> x1fc_waterInhabitants;
  float x214_fogBias;
  float x218_fogMagnitude;
  float x21c_origFogBias;
  float x220_origFogMagnitude;
  float x224_fogSpeed;
  zeus::CColor x228_fogColor;
  CAssetId x22c_splashParticle1Id;
  CAssetId x230_splashParticle2Id;
  CAssetId x234_splashParticle3Id;
  CAssetId x238_visorRunoffParticleId;
  std::optional<TLockedToken<CGenDescription>> x23c_visorRunoffEffect;
  CAssetId x24c_unmorphVisorRunoffParticleId;
  std::optional<TLockedToken<CGenDescription>> x250_unmorphVisorRunoffEffect;
  u16 x260_visorRunoffSfx;
  u16 x262_unmorphVisorRunoffSfx;
  rstl::reserved_vector<std::optional<TLockedToken<CGenDescription>>, 3> x264_splashEffects;
  rstl::reserved_vector<u16, 3> x298_splashSounds;
  zeus::CColor x2a4_splashColor;
  zeus::CColor x2a8_insideFogColor;
  float x2ac_alphaInTime;
  float x2b0_alphaOutTime;
  float x2b4_alphaInRecip;
  float x2b8_alphaOutRecip;
  float x2bc_alpha;
  float x2c0_tileSize;
  int x2c4_gridDimX = 0;
  int x2c8_gridDimY = 0;
  int x2cc_gridCellCount = 0;
  int x2d0_patchDimX = 0;
  int x2d4_patchDimY = 0;
  std::unique_ptr<bool[]> x2d8_tileIntersects;
  std::unique_ptr<bool[]> x2dc_vertIntersects;
  std::unique_ptr<u8[]> x2e0_patchIntersects; // 0: all clear, 1: all intersect, 2: partial intersect
  int x2e4_computedGridCellCount = 0;
  bool x2e8_24_b4 : 1;
  bool x2e8_25_morphIn : 1 = false;
  bool x2e8_26_morphing : 1 = false;
  bool x2e8_27_allowRender : 1;
  bool x2e8_28_recomputeClipping : 1 = true;
  bool x2e8_29_alphaIn : 1 = false;
  bool x2e8_30_alphaOut : 1 = false;

  void SetupGrid(bool recomputeClipping);
  void SetupGridClipping(CStateManager& mgr, int computeVerts);
  void UpdateSplashInhabitants(CStateManager& mgr);

public:
  CScriptWater(CStateManager& mgr, TUniqueId uid, std::string_view name, const CEntityInfo& info,
               const zeus::CVector3f& pos, const zeus::CAABox& box, const urde::CDamageInfo& dInfo,
               zeus::CVector3f& orientedForce, ETriggerFlags triggerFlags, bool thermalCold, bool allowRender,
               CAssetId patternMap1, CAssetId patternMap2, CAssetId colorMap, CAssetId bumpMap, CAssetId envMap,
               CAssetId envBumpMap, CAssetId unusedMap, const zeus::CVector3f& bumpLightDir, float bumpScale,
               float morphInTime, float morphOutTime, bool active, EFluidType fluidType, bool b4, float alpha,
               const CFluidUVMotion& uvMot, float turbSpeed, float turbDistance, float turbFreqMax, float turbFreqMin,
               float turbPhaseMax, float turbPhaseMin, float turbAmplitudeMax, float turbAmplitudeMin,
               const zeus::CColor& splashColor, const zeus::CColor& insideFogColor, CAssetId splashParticle1,
               CAssetId splashParticle2, CAssetId splashParticle3, CAssetId visorRunoffParticle,
               CAssetId unmorphVisorRunoffparticle, s32 visorRunoffSfx, s32 unmorphVisorRunoffSfx, s32 splashSfx1,
               s32 splashSfx2, s32 splashSfx3, float tileSize, u32 tileSubdivisions, float specularMin,
               float specularMax, float reflectionSize, float rippleIntensity, float reflectionBlend, float fogBias,
               float fogMagnitude, float fogSpeed, const zeus::CColor& fogColor, CAssetId lightmapId,
               float unitsPerLightmapTexel, float alphaInTime, float alphaOutTime, u32, u32, bool, s32, s32,
               std::unique_ptr<u32[]>&& u32Arr);

  void Accept(IVisitor& visitor) override;
  void Think(float, CStateManager&) override;
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
  void PreRender(CStateManager&, const zeus::CFrustum&) override;
  void AddToRenderer(const zeus::CFrustum&, CStateManager&) override;
  void Render(CStateManager&) override;
  void Touch(CActor&, CStateManager&) override;
  void CalculateRenderBounds() override;
  zeus::CAABox GetSortingBounds(const CStateManager&) const override;
  EWeaponCollisionResponseTypes GetCollisionResponseType(const zeus::CVector3f&, const zeus::CVector3f&,
                                                         const CWeaponMode&, EProjectileAttrib) const override;

  u16 GetSplashSound(float) const;
  const std::optional<TLockedToken<CGenDescription>>& GetSplashEffect(float) const;
  float GetSplashEffectScale(float) const;
  u32 GetSplashIndex(float) const;
  CFluidPlaneCPU& FluidPlane() { return *x1b4_fluidPlane; }
  zeus::CPlane GetWRSurfacePlane() const;
  float GetSurfaceZ() const;
  bool IsMorphing() const { return x2e8_26_morphing; }
  void SetMorphing(bool);
  float GetMorphFactor() const { return x1f8_morphFactor; }
  zeus::CColor GetSplashColor() const { return x2a4_splashColor; }
  void SetFrustumPlanes(const zeus::CFrustum& frustum) { x150_frustum = frustum; }
  const zeus::CFrustum& GetFrustumPlanes() const { return x150_frustum; }
  CFluidPlaneCPU& GetFluidPlane() const { return *x1b4_fluidPlane; }
  const std::optional<TLockedToken<CGenDescription>>& GetVisorRunoffEffect() const {
    return x23c_visorRunoffEffect;
  }
  u16 GetVisorRunoffSfx() const { return x260_visorRunoffSfx; }
  const std::optional<TLockedToken<CGenDescription>>& GetUnmorphVisorRunoffEffect() const {
    return x250_unmorphVisorRunoffEffect;
  }
  u16 GetUnmorphVisorRunoffSfx() const { return x262_unmorphVisorRunoffSfx; }
  const CScriptWater* GetNextConnectedWater(const CStateManager& mgr) const;
  u8 GetPatchRenderFlags(int x, int y) const { return x2e0_patchIntersects[y * x2d0_patchDimX + x]; }
  int GetPatchDimensionX() const { return x2d0_patchDimX; }
  int GetPatchDimensionY() const { return x2d4_patchDimY; }
  bool CanRippleAtPoint(const zeus::CVector3f& point) const;
  const zeus::CColor& GetInsideFogColor() const { return x2a8_insideFogColor; }
};
} // namespace urde
