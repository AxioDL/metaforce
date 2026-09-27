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
  int GetPatchDimensionX() const { return mPatchDimX; }
  int GetPatchDimensionY() const { return mPatchDimY; }
  void SetMorphing(const bool m);
  const CScriptWater* GetNextConnectedWater(const CStateManager&) const;
  // RenderSurface__12CScriptWaterFv

  CFluidPlaneCPU& FluidPlane() { return *mFluidPlane; }
  const CFluidPlaneCPU& GetFluidPlane() const { return *mFluidPlane; }
  CPlane GetWRSurfacePlane() const {
    return CPlane(GetSurfaceZ(), CUnitVector3f(0.f, 0.f, 1.f, CUnitVector3f::kN_Yes));
  }
  float GetSurfaceZ() const { return GetTriggerBoundsWR().GetMaxPoint().GetZ(); }
  const CColor& GetUnderwaterFogColor() const { return mInsideFogColor; }
  const rstl::optional_object< TLockedToken< CGenDescription > >& GetVisorRunoffEffect() const {
    return mVisorRunoffEffect;
  }
  ushort GetVisorRunoffSfx() const { return mVisorRunoffSfx; }
  const rstl::optional_object< TLockedToken< CGenDescription > >&
  GetUnmorphVisorRunoffEffect() const {
    return mUnmorphVisorRunoffEffect;
  }
  ushort GetUnmorphVisorRunoffSfx() const { return mUnmorphVisorRunoffSfx; }
  // GetFluidType__12CScriptWaterCFv
  bool IsMorphing() const { return mMorphing; }
  float GetMorphFactor() const { return mMorphFactor; }
  // GetFrustumPlanes__12CScriptWaterCFv
  int GetSplashIndex(float scale) const;
  const rstl::optional_object< TLockedToken< CGenDescription > >&
  GetSplashEffect(float scale) const;
  const ushort GetSplashSound(float scale) const;
  float GetSplashEffectScale(float scale) const;
  const CColor& GetSplashColor() const { return mSplashColor; }

  static const float kSplashScales[6];

  static int CalculateIndex(const int x, const int y, const int stride) { return y * stride + x; }
  bool GetTileIntersects(const int x, const int y) const {
    const int index = CalculateIndex(x, y, mGridDimX);
    return mTileIntersects.get()[CalculateIndex(x, y, mGridDimX)] != false;
  }

private:
  CFrustumPlanes mFrustum;
  rstl::single_ptr< CFluidPlaneCPU > mFluidPlane;
  CVector3f mPositionMorphed;
  CVector3f mExtentMorphed;
  float mMorphInTime;
  CVector3f mPositionOrig;
  CVector3f mExtentOrig;
  float mDamageOrig;
  float mDamageMorphed;
  float mMorphOutTime;
  float mMorphFactor;
  rstl::list< rstl::pair< TUniqueId, bool > > mWaterInhabitants;
  float mFogBias;
  float mFogMagnitude;
  float mOrigFogBias;
  float mOrigFogMagnitude;
  float mFogSpeed;
  CColor mFogColor;
  CAssetId mSplashParticle1Id;
  CAssetId mSplashParticle2Id;
  CAssetId mSplashParticle3Id;
  CAssetId mVisorRunoffParticleId;
  rstl::optional_object< TLockedToken< CGenDescription > > mVisorRunoffEffect;
  CAssetId mUnmorphVisorRunoffParticleId;
  rstl::optional_object< TLockedToken< CGenDescription > > mUnmorphVisorRunoffEffect;
  ushort mVisorRunoffSfx;
  ushort mUnmorphVisorRunoffSfx;
  rstl::reserved_vector< rstl::optional_object< TLockedToken< CGenDescription > >, 3 >
      mSplashEffects;
  rstl::reserved_vector< ushort, 3 > mSplashSounds;
  CColor mSplashColor;
  CColor mInsideFogColor;
  float mAlphaInTime;
  float mAlphaOutTime;
  float mAlphaInRecip;
  float mAlphaOutRecip;
  float mAlpha;
  float mTileSize;
  int mGridDimX;
  int mGridDimY;
  int mGridCellCount;
  int mPatchDimX;
  int mPatchDimY;
  rstl::single_ptr< char > mTileIntersects;
  rstl::single_ptr< bool > mVertIntersects;
  // 0: all clear, 1: all intersect, 2: partial intersect
  rstl::single_ptr< char > mPatchIntersects;
  int mComputedGridCellCount;
  bool mB4 : 1;
  bool mMorphIn : 1;
  bool mMorphing : 1;
  bool mAllowRender : 1;
  bool mRecomputeClipping : 1;
  bool mAlphaIn : 1;
  bool mAlphaOut : 1;
};

CHECK_SIZEOF(CScriptWater, (VERSION >= VERSION_GM8E_02 ? 0x300 : 0x2f0))

#endif // _CSCRIPTWATER
