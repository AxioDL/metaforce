#ifndef _CSCRIPTWATER
#define _CSCRIPTWATER

#include "types.h"

#include "MetroidPrime/CFluidPlane.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTrigger.hpp"
#include "MetroidPrime/TGameTypes.hpp"

#include "Kyoto/Math/CFrustumPlanes.hpp"
#include "Kyoto/TToken.hpp"

#include "rstl/list.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/pair.hpp"
#include "rstl/single_ptr.hpp"
#include "rstl/string.hpp"

class CAABox;
class CColor;
class CDamageInfo;
class CFluidPlaneCPU;
class CFluidUVMotion;
class CGenDescription;
class CStateManager;
class CVector3f;

class CScriptWater : public CScriptTrigger {
public:
  CScriptWater(CStateManager&, TUniqueId, const rstl::string&, const CEntityInfo&, const CVector3f&,
               const CAABox&, const CDamageInfo&, const CVector3f&, uint, bool, bool, uint, uint,
               uint, uint, uint, uint, uint, const CVector3f&, float, float, float, bool,
               CFluidPlane::EFluidType, bool, float, const CFluidUVMotion&, float, float, float,
               float, float, float, float, float, const CColor&, const CColor&, uint, uint, uint,
               uint, uint, int, int, int, int, int, float, uint, float, float, float, float, float,
               float, float, float, const CColor&, uint, float, float, float, uint, uint, bool, int,
               int, const uint*);

  // CEntity
  ~CScriptWater() override;
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;

  // CActor
  void PreRender(CStateManager&, const CFrustumPlanes&) override;
  void AddToRenderer(const CFrustumPlanes&, const CStateManager&) const override;
  void Render(const CStateManager&) const override;
  void CalculateRenderBounds() override;
  void Touch(CActor&, CStateManager&) override;
  EWeaponCollisionResponseTypes GetCollisionResponseType(const CVector3f&, const CVector3f&,
                                                         const CWeaponMode&, int) const override;
  CAABox GetSortingBounds(const CStateManager&) const override;

  bool CanRippleAtPoint(const CVector3f&) const;
  void UpdateSplashInhabitants(CStateManager&);
  void SetupGrid(bool recomputeClipping);
  void SetupGridClipping(CStateManager&, int computeVerts);
  int GetPatchRenderFlags(int x, int y) const;
  int GetPatchDimensionX() const { return x2d0_patchDimX; }
  int GetPatchDimensionY() const { return x2d4_patchDimY; }
  void SetMorphing(const bool m);
  const CScriptWater* GetNextConnectedWater(const CStateManager&) const;
  // RenderSurface__12CScriptWaterFv

  CFluidPlaneCPU& FluidPlane() { return *x1b4_fluidPlane; }
  const CFluidPlaneCPU& GetFluidPlane() const { return *x1b4_fluidPlane; }
  CPlane GetWRSurfacePlane() const {
    return CPlane(GetSurfaceZ(), CUnitVector3f(0.f, 0.f, 1.f, CUnitVector3f::kN_Yes));
  }
  float GetSurfaceZ() const { return GetTriggerBoundsWR().GetMaxPoint().GetZ(); }
  const CColor& GetUnderwaterFogColor() const { return x2a8_insideFogColor; }
  const rstl::optional_object< TLockedToken< CGenDescription > >& GetVisorRunoffEffect() const {
    return x23c_visorRunoffEffect;
  }
  ushort GetVisorRunoffSfx() const { return x260_visorRunoffSfx; }
  const rstl::optional_object< TLockedToken< CGenDescription > >&
  GetUnmorphVisorRunoffEffect() const {
    return x250_unmorphVisorRunoffEffect;
  }
  ushort GetUnmorphVisorRunoffSfx() const { return x262_unmorphVisorRunoffSfx; }
  // GetFluidType__12CScriptWaterCFv
  bool IsMorphing() const { return x2e8_26_morphing; }
  float GetMorphFactor() const { return x1f8_morphFactor; }
  // GetFrustumPlanes__12CScriptWaterCFv
  int GetSplashIndex(float scale) const;
  const rstl::optional_object< TLockedToken< CGenDescription > >&
  GetSplashEffect(float scale) const;
  const ushort GetSplashSound(float scale) const;
  float GetSplashEffectScale(float scale) const;
  const CColor& GetSplashColor() const { return x2a4_splashColor; }

  static const float kSplashScales[6];

  static int CalculateIndex(const int x, const int y, const int stride) { return y * stride + x; }
  bool GetTileIntersects(const int x, const int y) const {
    const int index = CalculateIndex(x, y, x2c4_gridDimX);
    return x2d8_tileIntersects.get()[CalculateIndex(x, y, x2c4_gridDimX)] != false;
  }

private:
  CFrustumPlanes x150_frustum;
  rstl::single_ptr< CFluidPlaneCPU > x1b4_fluidPlane;
  CVector3f x1b8_positionMorphed;
  CVector3f x1c4_extentMorphed;
  float x1d0_morphInTime;
  CVector3f x1d4_positionOrig;
  CVector3f x1e0_extentOrig;
  float x1ec_damageOrig;
  float x1f0_damageMorphed;
  float x1f4_morphOutTime;
  float x1f8_morphFactor;
  rstl::list< rstl::pair< TUniqueId, bool > > x1fc_waterInhabitants;
  float x214_fogBias;
  float x218_fogMagnitude;
  float x21c_origFogBias;
  float x220_origFogMagnitude;
  float x224_fogSpeed;
  CColor x228_fogColor;
  CAssetId x22c_splashParticle1Id;
  CAssetId x230_splashParticle2Id;
  CAssetId x234_splashParticle3Id;
  CAssetId x238_visorRunoffParticleId;
  rstl::optional_object< TLockedToken< CGenDescription > > x23c_visorRunoffEffect;
  CAssetId x24c_unmorphVisorRunoffParticleId;
  rstl::optional_object< TLockedToken< CGenDescription > > x250_unmorphVisorRunoffEffect;
  ushort x260_visorRunoffSfx;
  ushort x262_unmorphVisorRunoffSfx;
  rstl::reserved_vector< rstl::optional_object< TLockedToken< CGenDescription > >, 3 >
      x264_splashEffects;
  rstl::reserved_vector< ushort, 3 > x298_splashSounds;
  CColor x2a4_splashColor;
  CColor x2a8_insideFogColor;
  float x2ac_alphaInTime;
  float x2b0_alphaOutTime;
  float x2b4_alphaInRecip;
  float x2b8_alphaOutRecip;
  float x2bc_alpha;
  float x2c0_tileSize;
  int x2c4_gridDimX;
  int x2c8_gridDimY;
  int x2cc_gridCellCount;
  int x2d0_patchDimX;
  int x2d4_patchDimY;
  rstl::single_ptr< char > x2d8_tileIntersects;
  rstl::single_ptr< bool > x2dc_vertIntersects;
  // 0: all clear, 1: all intersect, 2: partial intersect
  rstl::single_ptr< char > x2e0_patchIntersects;
  int x2e4_computedGridCellCount;
  bool x2e8_24_b4 : 1;
  bool x2e8_25_morphIn : 1;
  bool x2e8_26_morphing : 1;
  bool x2e8_27_allowRender : 1;
  bool x2e8_28_recomputeClipping : 1;
  bool x2e8_29_alphaIn : 1;
  bool x2e8_30_alphaOut : 1;
};

CHECK_SIZEOF(CScriptWater, (VERSION >= VERSION_GM8P_00 ? 0x300 : 0x2f0))

#endif // _CSCRIPTWATER
