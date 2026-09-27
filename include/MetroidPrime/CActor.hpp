#ifndef _CACTOR
#define _CACTOR

#include "types.h"

#include "Collision/CMaterialFilter.hpp"
#include "Collision/CMaterialList.hpp"

#include "MetroidPrime/ActorCommon.hpp"
#include "MetroidPrime/CEntity.hpp"
#include "MetroidPrime/CModelData.hpp"

#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Graphics/CModelFlags.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/TToken.hpp"

#include "rstl/optional_object.hpp"
#include "rstl/pair.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"

class CActorLights;
class CActorParameters;
class CScannableObjectInfo;
class CSimpleShadow;

class CDamageInfo;
class CDamageVulnerability;
class CFrustumPlanes;
class CHealthInfo;
class CScriptWater;
class CWeaponMode;
class CInt32POINode;

class CActor : public CEntity {
public:
  enum EThermalFlags {
    kTF_None = 0,
    kTF_Cold = 1,
    kTF_Hot = 2,
  };
  enum EFluidState {
    kFS_EnteredFluid,
    kFS_InFluid,
    kFS_LeftFluid,
  };
  enum EScanState {
    kSS_Start,
    kSS_Processing,
    kSS_Done,
  };

  CActor(const TUniqueId uid, const bool active, const rstl::string& name, const CEntityInfo& info,
         const CTransform4f& xf, const CModelData& mData, const CMaterialList& list,
         const CActorParameters& params, TUniqueId nextDrawNode);
  ~CActor() override;
  DECLARE_TYPES_MATCH;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;
  void SetActive(const bool active) override;

  virtual void PreRender(CStateManager&, const CFrustumPlanes&);
  virtual void AddToRenderer(const CFrustumPlanes&, const CStateManager&) const;
  virtual void Render(const CStateManager&) const;
  virtual bool CanRenderUnsorted(const CStateManager&) const;
  virtual void CalculateRenderBounds();
  const CHealthInfo* GetHealthInfo(const CStateManager& mgr) const {
    return const_cast< CActor* >(this)->HealthInfo(const_cast< CStateManager& >(mgr));
  }
  virtual CHealthInfo* HealthInfo(CStateManager&);
  virtual const CDamageVulnerability* GetDamageVulnerability() const;
  virtual const CDamageVulnerability* GetDamageVulnerability(const CVector3f&, const CVector3f&,
                                                             const CDamageInfo&) const;
  virtual rstl::optional_object< CAABox > GetTouchBounds() const;
  virtual void Touch(CActor&, CStateManager&);
  virtual CVector3f GetOrbitPosition(const CStateManager&) const;
  virtual CVector3f GetAimPosition(const CStateManager&, float) const;
  virtual CVector3f GetHomingPosition(const CStateManager&, float) const;
  virtual CVector3f GetScanObjectIndicatorPosition(const CStateManager&) const;
  virtual EWeaponCollisionResponseTypes GetCollisionResponseType(const CVector3f&, const CVector3f&,
                                                                 const CWeaponMode&, int) const;
  virtual void FluidFXThink(EFluidState, CScriptWater&, CStateManager&);
  virtual void OnScanStateChange(EScanState, CStateManager&);
  virtual CAABox GetSortingBounds(const CStateManager&) const;
  virtual void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                               float dt);

  CAdvancementDeltas UpdateAnimation(float dt, CStateManager& mgr, bool advTree);

  void ProcessSoundEvent(const int sfxId, const float weight, const int flags, const float fallOff,
                         const float maxDist, const uchar minVol, const uchar maxVol,
                         const CVector3f& toListener, const CVector3f& position, const int aid,
                         CStateManager& mgr, const bool translateId);

  void UpdateSfxEmitters();
  void RemoveEmitter();
  void SetModelData(const CModelData& modelData);
  float GetAverageAnimVelocity(int anim);
  void EnsureRendered(const CStateManager& mgr) const;
  void EnsureRendered(const CStateManager& mgr, const CVector3f& pos, const CAABox& bounds) const;
  void DrawTouchBounds() const;
  bool IsModelOpaque(const CStateManager& mgr) const;
  void RenderInternal(const CStateManager& mgr) const;
  void CreateShadowIfNeeded();

  const CTransform4f& GetTransform() const { return mTransform; }
  CVector3f TransformWorldToLocalRotation(const CVector3f& v) const {
    return mTransform.TransposeRotate(v);
  }
  void SetTransform(const CTransform4f& xf) {
    mTransform = xf;
    SetTransformDirty(true);
    SetTransformDirtySpare(true);
    SetPreRenderHasMoved(true);
  }
  void SetRotation(const CQuaternion& rot) { SetTransform(rot.BuildTransform4f(GetTranslation())); }
  CQuaternion TransformLocalToWorldRotation(const CQuaternion& rot) const {
    return rot * CQuaternion::FromMatrix(GetTransform());
  }
  CQuaternion GetRotation() const { return CQuaternion::FromMatrix(GetTransform()); }
  CVector3f GetTranslation() const { return mTransform.GetTranslation(); }
  void SetTranslation(const CVector3f& vec);
  void GlobalMove(const CVector3f& delta) { SetTranslation(GetTranslation() + delta); }
  CTransform4f GetLocatorTransform(const rstl::string& segName) const;
  CTransform4f GetScaledLocatorTransform(const rstl::string& segName) const;
  float GetYaw() const;
  float GetPitch() const;
  void SetActorLights(rstl::auto_ptr< CActorLights > lights);
  void SetInFluid(bool b, TUniqueId uid);
  CScannableObjectInfo* GetScannableObjectInfo() const;
  void MoveScannableObjectInfoToActor(CActor* actor, CStateManager& mgr);

  /// ????
  bool NullModel() const { return !GetAnimationData() && !GetModelData()->HasNormalModel(); }

  bool HasModelData() const {
    return !mModelData.null() && (GetModelData()->HasAnimation() || GetModelData()->HasNormalModel());
  }
  CModelData* ModelData() { return mModelData.get(); }
  const CModelData* GetModelData() const { return mModelData.get(); }

  bool HasAnimation() const { return !mModelData.null() && GetModelData()->HasAnimation(); }
  CAnimData* AnimationData() {
    return ModelData()->AnimationData();
  }
  const CAnimData* GetAnimationData() const { return GetModelData()->GetAnimationData(); }

  CVector3f GetModelScale() const {
    const CModelData* modelData = GetModelData();
    return modelData->GetScale();
  }

  bool HasShadow() const { return GetShadow() != nullptr; }
  CSimpleShadow* Shadow() { return mSimpleShadow.get(); }
  const CSimpleShadow* GetShadow() const { return mSimpleShadow.get(); }

  bool HasActorLights() const { return !mActorLights.null(); }
  CActorLights* ActorLights() { return mActorLights.get(); }
  const CActorLights* GetActorLights() const { return mActorLights.get(); }

  const CModelFlags& GetModelFlags() const { return mDrawFlags; }
  void SetModelFlags(const CModelFlags& flags) { mDrawFlags = flags; }

  const CMaterialList& GetMaterialList() const { return mMaterial; }
  CMaterialList& MaterialList() { return mMaterial; }

  const CMaterialFilter& GetMaterialFilter() const;
  void SetMaterialFilter(const CMaterialFilter& filter);

  TUniqueId InFluidId() const { return mFluidId; }

  void SetDamageMag(const float d) { mDamageMag = d; }

  bool GetTransformDirty() const { return mNotInSortedLists; }
  bool GetTransformDirtySpare() const { return mTransformDirty; }
  bool GetPreRenderHasMoved() const { return mActorLightsDirty; }
  bool GetPreRenderClipped() const { return mOutOfFrustum; }
  bool GetCalculateLighting() const { return mCalculateLighting && HasActorLights(); }
  bool GetDrawShadow() const { return mShadowEnabled; }
  bool GetShadowDirty() const { return mShadowDirty; }
  bool GetMuted() const { return mMuted; }
  bool GetPointGeneratorParticles() const { return mPointGeneratorParticles; }
  void SetPointGeneratorParticles(bool active) { mPointGeneratorParticles = active; }
  bool IsInFluid() const { return mFluidCounter != 0; }
  EThermalFlags GetThermalFlags() const {
    return static_cast< EThermalFlags >(mThermalVisorFlags);
  }
  bool GetRenderParticleDatabaseInside() const { return mRenderParticleDBInside; }
  uchar GetTargetableVisorFlags() const { return mTargetableVisorFlags; }
  bool GetDoTargetDistanceTest() const { return mDoTargetDistanceTest; }
  bool GetTargetable() const { return mTargetable; }

  void SetTransformDirty(bool b) { mNotInSortedLists = b; }
  void SetTransformDirtySpare(bool b) { mTransformDirty = b; }
  void SetPreRenderHasMoved(bool b) { mActorLightsDirty = b; }
  void SetWorldLightingDirty(bool b) { mWorldLightingDirty = b; }
  void SetPreRenderClipped(bool b) { mOutOfFrustum = b; }
  void SetCalculateLighting(bool b);
  void SetDrawShadow(bool b);
  void SetShadowDirty(bool b) { mShadowDirty = b; }
  void SetMuted(bool b);
  void SetThermalFlags(EThermalFlags flags) { mThermalVisorFlags = flags; }
  void SetRenderParticleDatabaseInside(bool b) { mRenderParticleDBInside = b; }
  void SetDoTargetDistanceTest(const bool b) { mDoTargetDistanceTest = b; }
  void SetTargetable(bool b) { mTargetable = b; }

  void RemoveMaterial(EMaterialTypes, EMaterialTypes, EMaterialTypes, EMaterialTypes,
                      CStateManager&);
  void RemoveMaterial(EMaterialTypes, EMaterialTypes, EMaterialTypes, CStateManager&);
  void RemoveMaterial(EMaterialTypes, EMaterialTypes, CStateManager&);
  void RemoveMaterial(EMaterialTypes, CStateManager&);
  void AddMaterial(EMaterialTypes, EMaterialTypes, EMaterialTypes, EMaterialTypes, EMaterialTypes,
                   CStateManager&);
  void AddMaterial(EMaterialTypes, EMaterialTypes, EMaterialTypes, EMaterialTypes, CStateManager&);
  void AddMaterial(EMaterialTypes, EMaterialTypes, EMaterialTypes, CStateManager&);
  void AddMaterial(EMaterialTypes, EMaterialTypes, CStateManager&);
  void AddMaterial(EMaterialTypes, CStateManager&);
  void AddMaterial(const CMaterialList& l) { mMaterial.Add(l); }
  void SetMaterial(const CMaterialList& l) { mMaterial = l; }

  const CAABox& GetRenderBoundsCached() const { return mRenderBounds; }
  void SetRenderBounds(const CAABox& bounds) { mRenderBounds = bounds; }
  TUniqueId GetDrawParent() const { return mNextDrawNode; }
  void SetDrawParentId(TUniqueId id) { mNextDrawNode = id; }
  uint GetDrawToken() const { return mDrawnToken; }
  uint GetAddedToken() const { return mAddedToken; }
  void SetDrawToken(uint token) const { const_cast< CActor* >(this)->mDrawnToken = token; }
  void SetAddedToken(unsigned int token) const {
    const_cast< CActor* >(this)->mAddedToken = token;
  }
  bool IsDrawEnabled() const { return mDrawEnabled; }

  bool GetUseInSortedLists() const;
  void SetUseInSortedLists(bool use);
  bool GetCallTouch() const;
  void SetCallTouch(bool value);
  // GetOrbitDistanceCheck__6CActorCFv
  // GetCalculateLighting__6CActorCFv
  // GetDrawShadow__6CActorCFv
  // GetRenderParticleDatabaseInside__6CActorCFv
  // HasModelParticles__6CActorCFv
  void SetVolume(uchar volume);
  void SetSoundEventPitchBend(int);
  CSfxHandle GetSfxHandle() const;
  bool CanDrawStatic() const;
  void SetEnableRender(bool v) { mEnableRender = v; }

protected:
  void SetDrawEnabled(bool v) { mDrawEnabled = v; }

private:
  CTransform4f mTransform;
  rstl::single_ptr< CModelData > mModelData;
  CMaterialList mMaterial;
  CMaterialFilter mMaterialFilter;
  TSfxId mSfxId;
  CSfxHandle mLoopingSfxHandle;
  rstl::single_ptr< CActorLights > mActorLights;
  rstl::single_ptr< CSimpleShadow > mSimpleShadow;
  rstl::single_ptr< TCachedToken< CScannableObjectInfo > > mScanObjectInfo;
  CAABox mRenderBounds;
  CModelFlags mDrawFlags;
  float mTime;
  uint mPitchBend;
  TUniqueId mFluidId;
  TUniqueId mNextDrawNode;
  int mDrawnToken;
  int mAddedToken;
  float mDamageMag;
  uchar mMaxVol;
#if VERSION >= VERSION_GM8E_02
  rstl::reserved_vector< TUniqueId, 4 > mFluidIds;
#endif
  rstl::reserved_vector< CSfxHandle, 2 > mNonLoopingSfxHandles;
  uint mNextNonLoopingSfxHandle : 3;
  uint mNotInSortedLists : 1;
  uint mTransformDirty : 1;
  uint mActorLightsDirty : 1;
  uint mOutOfFrustum : 1;
  uint mCalculateLighting : 1;
  uint mShadowEnabled : 1;
  uint mShadowDirty : 1;
  uint mMuted : 1;
  uint mUseInSortedLists : 1;
  uint mCallTouch : 1;
  uint mGlobalTimeProvider : 1;
  uint mRenderUnsorted : 1;
  uint mPointGeneratorParticles : 1;
  uint mFluidCounter : 3;
  uint mThermalVisorFlags : 2;
  uint mRenderParticleDBInside : 1;
  uint mEnablePitchBend : 1;
  uint mTargetableVisorFlags : 4;
  uint mEnableRender : 1;
  uint mWorldLightingDirty : 1;
  uint mDrawEnabled : 1;
  uint mDoTargetDistanceTest : 1;
  uint mTargetable : 1;
};
CHECK_SIZEOF(CActor, (VERSION >= VERSION_GM8E_02 ? 0xf8 : 0xe8))

#endif // _CACTOR
