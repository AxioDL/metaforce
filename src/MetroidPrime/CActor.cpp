#include "MetroidPrime/CActor.hpp"

#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CActorLights.hpp"
#include "MetroidPrime/CActorModelParticles.hpp"
#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CSimpleShadow.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"

#include "Kyoto/Audio/CAudioSys.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CTimeProvider.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Math/CFrustumPlanes.hpp"
#include "Kyoto/Math/CMath.hpp"

#include "MetroidPrime/TGameTypes.hpp"
#include "rstl/math.hpp"

static CMaterialList MakeActorMaterialList(const CMaterialList& in,
                                           const CActorParameters& params) {
  CMaterialList ret = in;
  if (params.GetVisorParameters().GetBool1()) {
    ret.Add(kMT_Unknown46);
  }
  if (params.GetVisorParameters().GetScanPassthrough()) {
    ret.Add(kMT_ScanPassthrough);
  }
  return ret;
}

CActor::CActor(const TUniqueId uid, const bool active, const rstl::string& name,
               const CEntityInfo& info, const CTransform4f& xf, const CModelData& mData,
               const CMaterialList& list, const CActorParameters& params,
               const TUniqueId nextDrawNode)
: CEntity(uid, info, active, name)
, mTransform(xf)
, mModelData(mData.IsNull() ? nullptr : rs_new CModelData(mData))
, mMaterial(MakeActorMaterialList(list, params))
, mMaterialFilter(
      CMaterialFilter::MakeIncludeExclude(CMaterialList(SolidMaterial), CMaterialList()))
, mSfxId(InvalidSfxId)
, mActorLights(mData.IsNull() ? nullptr : params.GetLighting().MakeActorLights().release())
, mRenderBounds(CAABox::MakeMaxInvertedBox())
, mDrawFlags(CModelFlags::Normal())
, mTime(0.f)
, mPitchBend(8192)
, mFluidId(kInvalidUniqueId)
, mNextDrawNode(nextDrawNode)
, mDrawnToken(-1)
, mAddedToken(-1)
, mDamageMag(params.GetThermalMag())
, mMaxVol(CAudioSys::kMaxVolume)
#if VERSION >= VERSION_GM8E_02
, mFluidIds(kInvalidUniqueId)
#endif
, mNonLoopingSfxHandles(CSfxHandle())
, mNextNonLoopingSfxHandle(0)
, mNotInSortedLists(true)
, mTransformDirty(true)
, mActorLightsDirty(true)
, mOutOfFrustum(false)
, mCalculateLighting(true)
, mShadowEnabled(false)
, mShadowDirty(false)
, mMuted(false)
, mUseInSortedLists(true)
, mCallTouch(true)
, mGlobalTimeProvider(params.UseGlobalRenderTime())
, mRenderUnsorted(params.ForceRenderUnsorted())
, mPointGeneratorParticles(false)
, mFluidCounter(0)
, mThermalVisorFlags(params.IsHotInThermal() ? kTF_Hot : kTF_Cold)
, mRenderParticleDBInside(true)
, mEnablePitchBend(false)
, mTargetableVisorFlags(params.GetVisorParameters().GetMask())
, mEnableRender(true)
, mWorldLightingDirty(false)
, mDrawEnabled(active)
, mDoTargetDistanceTest(true)
, mTargetable(true) {
  if (!mModelData.null()) {
    if (params.GetXRay().first != 0) {
      mModelData->SetXRayModel(params.GetXRay());
    }
    if (params.GetInfra().first != 0) {
      mModelData->SetInfraModel(params.GetInfra());
    }
    const CLightParameters& lighting = params.GetLighting();
    if (!lighting.ShouldMakeLights() || lighting.GetMaxAreaLights() == 0) {
      mModelData->SetAmbientColor(lighting.GetAmbientColor());
    }
    mModelData->SetSortThermal(!params.NoSortThermal());
  }
  const CAssetId scanId = params.GetScannable().GetScannableObject0();
  if (scanId != kInvalidAssetId) {
    mScanObjectInfo = rs_new TCachedToken< CScannableObjectInfo >(
        gpSimplePool->GetObj(SObjectTag('SCAN', scanId)));
    mScanObjectInfo->Lock();
  }
}

CActor::~CActor() { RemoveEmitter(); }

CAdvancementDeltas CActor::UpdateAnimation(float dt, CStateManager& mgr, bool advTree) {
  CAdvancementDeltas result = ModelData()->AdvanceAnimation(dt, mgr, GetAreaId(), advTree);
  ModelData()->AdvanceParticles(GetTransform(), dt, mgr);
  UpdateSfxEmitters();
  if (HasAnimation()) {
    ushort maxVol = mMaxVol;
    const int aid = GetCurrentAreaId().Value();

    const CGameCamera& camera = mgr.GetCameraManager()->GetCurrentCamera(mgr);
    const CVector3f origin = GetTranslation();
    const CVector3f toCamera = camera.GetTranslation() - origin;

    const CInt32POINode* intNode;
    const CSoundPOINode* soundNode;
    const CParticlePOINode* particleNode;

    int soundNodeCount = 0;
    if (HasAnimation()) {
      soundNode = GetAnimationData()->GetSoundPOIList(soundNodeCount);
    } else {
      soundNode = nullptr;
    }
    if (soundNodeCount > 0 && soundNode != nullptr) {
      for (int i = 0; i < soundNodeCount; ++soundNode, ++i) {
        int charIdx = soundNode->GetCharacterIndex();
        if (soundNode->GetPoiType() != kPT_Sound || GetMuted())
          continue;
        if (charIdx != -1 && GetAnimationData()->GetCharacterIndex() != charIdx)
          continue;
        ProcessSoundEvent(soundNode->GetSoundId(), soundNode->GetWeight(), soundNode->GetFlags(),
                          soundNode->GetFallOff(), soundNode->GetMaxDistance(), 20, maxVol,
                          toCamera, origin, aid, mgr, true);
      }
    }

    int intNodeCount = 0;
    if (HasAnimation()) {
      intNode = GetAnimationData()->GetInt32POIList(intNodeCount);
    } else {
      intNode = nullptr;
    }
    if (intNodeCount > 0 && intNode != nullptr) {
      for (int i = 0; i < intNodeCount; ++intNode, ++i) {
        int charIdx = intNode->GetCharacterIndex();
        if (intNode->GetPoiType() == kPT_SoundInt32 && !GetMuted() &&
            (charIdx == -1 || GetAnimationData()->GetCharacterIndex() == charIdx)) {
          ProcessSoundEvent(intNode->GetValue(), intNode->GetWeight(), intNode->GetFlags(), 0.1f,
                            150.f, 20, maxVol, toCamera, origin, aid, mgr, true);
        } else if (intNode->GetPoiType() == kPT_UserEvent) {
          DoUserAnimEvent(mgr, *intNode, static_cast< EUserEventType >(intNode->GetValue()), dt);
        }
      }
    }

    int particleNodeCount = 0;
    if (HasAnimation()) {
      particleNode = GetAnimationData()->GetParticlePOIList(particleNodeCount);
    } else {
      particleNode = nullptr;
    }
    if (particleNodeCount > 0 && particleNode != nullptr) {
      for (int i = 0; i < particleNodeCount; ++particleNode, ++i) {
        int charIdx = particleNode->GetCharacterIndex();
        if (charIdx != -1 && GetAnimationData()->GetCharacterIndex() != charIdx)
          continue;
        AnimationData()->GetParticleDB().SetParticleEffectState(particleNode->GetString(), true, mgr);
      }
    }
  }
  return result;
}

void CActor::RemoveEmitter() {
  if (CSfxHandle handle = mLoopingSfxHandle) {
    CSfxManager::RemoveEmitter(handle);
    mSfxId = -1;
    mLoopingSfxHandle = CSfxHandle();
  }
}

void CActor::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                             float dt) {
  if (type == kUE_LoopedSoundStop) {
    RemoveEmitter();
  }
}

float CActor::GetAverageAnimVelocity(int anim) {
  return HasAnimation() ? GetAnimationData()->GetAverageVelocity(anim) : 0.f;
}

void CActor::CalculateRenderBounds() {
  if (HasModelData()) {
    SetRenderBounds(GetModelData()->GetBounds(GetTransform()));
  } else {
    const CVector3f origin = GetTranslation();
    SetRenderBounds(CAABox(origin, origin));
  }
}

void CActor::SetModelData(const CModelData& modelData) {
  mModelData = modelData.IsNull() ? nullptr : rs_new CModelData(modelData);
}

void CActor::PreRender(CStateManager& mgr, const CFrustumPlanes& planes) {
  if (HasModelData()) {
    SetPreRenderClipped(!planes.BoxInFrustumPlanes(mRenderBounds));
    if (!GetPreRenderClipped()) {
      bool lightsDirty = false;
      if (GetPreRenderHasMoved()) {
        SetPreRenderHasMoved(false);
        SetShadowDirty(true);
        lightsDirty = true;
      } else if (mWorldLightingDirty) {
        lightsDirty = true;
      } else if (HasActorLights() && GetActorLights()->GetNeedsRelight()) {
        lightsDirty = true;
      }

      // TODO why doesn't GetDrawShadow() work?
      if (GetShadowDirty() && mShadowEnabled && HasShadow()) {
        Shadow()->Calculate(GetModelData()->GetBounds(), GetTransform(), mgr);
        SetShadowDirty(false);
      }

      if (GetCalculateLighting()) {
        const CAABox bounds = GetModelData()->GetBounds(GetTransform());
        if (mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_Thermal) {
          ActorLights()->BuildConstantAmbientLighting();
        } else {
          if (lightsDirty == true && GetCurrentAreaId() != kInvalidAreaId) {
            const CWorld* world = mgr.GetWorld();
            if (world->IsAreaValid(GetCurrentAreaId()) &&
                ActorLights()->BuildAreaLightList(mgr, *world->GetArea(GetCurrentAreaId()), bounds)) {
              mWorldLightingDirty = false;
            }
          }
          ActorLights()->BuildDynamicLightList(mgr, bounds);
        }
      }

      CModelData* mData = ModelData();
      if (GetModelData()->HasAnimation()) {
        mData->AnimationData()->PreRender();
      }
    } else {
      if (GetPreRenderHasMoved()) {
        SetPreRenderHasMoved(false);
        SetShadowDirty(true);
      }
      // TODO why doesn't GetDrawShadow() work?
      if (GetShadowDirty() && mShadowEnabled && HasShadow()) {
        if (planes.BoxInFrustumPlanes(
                GetShadow()->GetMaxShadowBox(GetModelData()->GetBounds(GetTransform()))) == true) {
          Shadow()->Calculate(GetModelData()->GetBounds(), GetTransform(), mgr);
          SetShadowDirty(false);
        }
      }
    }
  }
}

void CActor::AddToRenderer(const CFrustumPlanes& planes, const CStateManager& mgr) const {
  if (HasModelData()) {
    if (GetRenderParticleDatabaseInside()) {
      GetModelData()->RenderParticles(planes);
    }

    if (!GetPreRenderClipped()) {
      if (CanRenderUnsorted(mgr)) {
        Render(mgr);
      } else {
        EnsureRendered(mgr);
      }
    }

    if (mgr.GetPlayerState()->GetActiveVisor(mgr) != CPlayerState::kPV_XRay &&
        mgr.GetPlayerState()->GetActiveVisor(mgr) != CPlayerState::kPV_Thermal && GetDrawShadow() &&
        GetShadow()->Valid() && planes.BoxInFrustumPlanes(GetShadow()->GetBounds()) == true) {
      gpRender->AddDrawable(GetShadow(), GetShadow()->GetTransform().GetTranslation(),
                            GetShadow()->GetBounds(), 1, IRenderer::kDS_SortedCallback);
    }
  }
}

void CActor::EnsureRendered(const CStateManager& mgr, const CVector3f& pos,
                            const CAABox& bounds) const {
  if (GetModelData()) {
    const CModelData::EWhichModel which = CModelData::GetRenderingModel(mgr);
    GetModelData()->RenderUnsortedParts(which, GetTransform(), GetActorLights(), GetModelFlags());
  }
  mgr.AddDrawableActor(*this, pos, bounds);
}

void CActor::EnsureRendered(const CStateManager& mgr) const {
  const CAABox bounds = GetSortingBounds(mgr);
  const CVector3f viewForward = CGraphics::GetViewMatrix().GetForward();
  const CVector3f pos = bounds.ClosestPointAlongVector(viewForward);
  EnsureRendered(mgr, pos, bounds);
}

void CActor::DrawTouchBounds() const {}

bool CActor::CanRenderUnsorted(const CStateManager& mgr) const {
  bool result = HasAnimation();
  if (result && GetAnimationData()->GetParticleDB().AreAnySystemsDrawnWithModel() &&
      GetRenderParticleDatabaseInside()) {
    result = false;
  } else {
    result = mRenderUnsorted || IsModelOpaque(mgr);
  }
  return result;
}

void CActor::Render(const CStateManager& mgr) const {
  if (GetModelData() && !NullModel()) {
    bool renderPrePostParticles = GetRenderParticleDatabaseInside() && HasAnimation();
    if (renderPrePostParticles) {
      GetAnimationData()->GetParticleDB().RenderSystemsToBeDrawnFirst();
    }

    if (mEnableRender) {
      if (mPointGeneratorParticles) {
        mgr.SetupParticleHook(*this);
      }
      if (mGlobalTimeProvider) {
        RenderInternal(mgr);
      } else {
        const float timeSince = CGraphics::GetSecondsMod900() - mTime;
        CTimeProvider tp(CMath::FastFmod(timeSince, 900.f));
        RenderInternal(mgr);
      }
      if (mPointGeneratorParticles) {
        CSkinnedModel::ClearPointGeneratorFunc();
        mgr.GetActorModelParticles()->Render(mgr, *this);
      }
    }

    if (renderPrePostParticles) {
      GetAnimationData()->GetParticleDB().RenderSystemsToBeDrawnLast();
    }
  }
  DrawTouchBounds();
}

void CActor::RenderInternal(const CStateManager& mgr) const {
  CModelData::EWhichModel which = CModelData::GetRenderingModel(mgr);
  if (which == CModelData::kWM_ThermalHot) {
    if (GetModelData()->GetSortThermal()) {
      const float alpha = mDrawFlags.GetColorRef().GetAlpha();
      uchar addMag;
      float mulMag;
      if (mDamageMag <= 1.f) {
        mulMag = mDamageMag;
        addMag = 0;
      } else if (mDamageMag < 2.f) {
        mulMag = 1.f;
        addMag = CCast::ToUint8((mDamageMag - 1.f) * 255.f);
      } else {
        mulMag = 1.f;
        addMag = 255;
      }

      const uchar rgb = CCast::ToUint8((255.f * mulMag) * alpha);
      CColor mulColor(rgb, rgb, rgb, mDrawFlags.GetColorRef().GetAlphau8());
      CColor addColor(addMag, addMag, addMag, mDrawFlags.GetColorRef().GetAlphau8() / 4);
      GetModelData()->RenderThermal(mTransform, mulColor, addColor, mDrawFlags);
      return;
    } else if (mgr.GetThermalColdScale2() > 0.0001f && mDrawFlags.GetTrans() == 0) {
      const float scale = rstl::max_val< float >(
          (mgr.GetThermalColdScale2() + mgr.GetThermalColdScale1()) * mgr.GetThermalColdScale2(),
          mgr.GetThermalColdScale2());
      const uchar rgb = CCast::ToUint8(CMath::Clamp(0.f, scale * 255.f, 255.f));
      CColor color(rgb, rgb, rgb, 255);
      CModelFlags flags = mDrawFlags;
      flags.mBlendMode = CModelFlags::kT_Two;
      flags.mColor = color;
      GetModelData()->Render(which, mTransform, mActorLights.get(), flags);
      return;
    }
  }
  GetModelData()->Render(which, mTransform, mActorLights.get(), mDrawFlags);
}

float CActor::GetYaw() const {
  float sq = sqrt(mTransform.Get11() * mTransform.Get11() +
                  mTransform.Get01() * mTransform.Get01());
  if (sq > 0.001f) {
    double ret = -atan2(mTransform.Get01(), mTransform.Get11());
    return ret;
  }
  return 0.f;
}

float CActor::GetPitch() const {
  float sq = CMath::SqrtF(mTransform.Get11() * mTransform.Get11() +
                          mTransform.Get01() * mTransform.Get01());
  double ret = -atan2(-mTransform.Get21(), sq);
  return ret;
}

CHealthInfo* CActor::HealthInfo(CStateManager& mgr) { return nullptr; }

const CDamageVulnerability* CActor::GetDamageVulnerability() const {
  return &CDamageVulnerability::NormalVulnerability();
}

const CDamageVulnerability* CActor::GetDamageVulnerability(const CVector3f&, const CVector3f&,
                                                           const CDamageInfo&) const {
  return GetDamageVulnerability();
}

rstl::optional_object< CAABox > CActor::GetTouchBounds() const {
  return rstl::optional_object_null();
}

void CActor::Touch(CActor&, CStateManager&) {}

bool CActor::GetUseInSortedLists() const { return mUseInSortedLists; }

void CActor::SetUseInSortedLists(bool use) { mUseInSortedLists = use; }

bool CActor::GetCallTouch() const { return mCallTouch; }

void CActor::SetCallTouch(bool value) { mCallTouch = value; }

void CActor::AddMaterial(EMaterialTypes mat1, CStateManager& mgr) {
  mMaterial.Add(mat1);
  mgr.UpdateObjectInLists(*this);
}

void CActor::AddMaterial(EMaterialTypes mat1, EMaterialTypes mat2, CStateManager& mgr) {
  mMaterial.Add(mat1);
  mMaterial.Add(mat2);
  mgr.UpdateObjectInLists(*this);
}

void CActor::AddMaterial(EMaterialTypes mat1, EMaterialTypes mat2, EMaterialTypes mat3,
                         CStateManager& mgr) {
  mMaterial.Add(mat1);
  mMaterial.Add(mat2);
  mMaterial.Add(mat3);
  mgr.UpdateObjectInLists(*this);
}

void CActor::AddMaterial(EMaterialTypes mat1, EMaterialTypes mat2, EMaterialTypes mat3,
                         EMaterialTypes mat4, CStateManager& mgr) {
  mMaterial.Add(mat1);
  mMaterial.Add(mat2);
  mMaterial.Add(mat3);
  mMaterial.Add(mat4);
  mgr.UpdateObjectInLists(*this);
}

void CActor::AddMaterial(EMaterialTypes mat1, EMaterialTypes mat2, EMaterialTypes mat3,
                         EMaterialTypes mat4, EMaterialTypes mat5, CStateManager& mgr) {
  mMaterial.Add(mat1);
  mMaterial.Add(mat2);
  mMaterial.Add(mat3);
  mMaterial.Add(mat4);
  mMaterial.Add(mat5);
  mgr.UpdateObjectInLists(*this);
}

void CActor::RemoveMaterial(EMaterialTypes mat1, CStateManager& mgr) {
  mMaterial.Remove(mat1);
  mgr.UpdateObjectInLists(*this);
}

void CActor::RemoveMaterial(EMaterialTypes mat1, EMaterialTypes mat2, CStateManager& mgr) {

  mMaterial.Remove(mat1);
  mMaterial.Remove(mat2);
  mgr.UpdateObjectInLists(*this);
}

void CActor::RemoveMaterial(EMaterialTypes mat1, EMaterialTypes mat2, EMaterialTypes mat3,
                            CStateManager& mgr) {

  mMaterial.Remove(mat1);
  mMaterial.Remove(mat2);
  mMaterial.Remove(mat3);
  mgr.UpdateObjectInLists(*this);
}

void CActor::RemoveMaterial(EMaterialTypes mat1, EMaterialTypes mat2, EMaterialTypes mat3,
                            EMaterialTypes mat4, CStateManager& mgr) {

  mMaterial.Remove(mat1);
  mMaterial.Remove(mat2);
  mMaterial.Remove(mat3);
  mMaterial.Remove(mat4);
  mgr.UpdateObjectInLists(*this);
}

EWeaponCollisionResponseTypes CActor::GetCollisionResponseType(const CVector3f&, const CVector3f&,
                                                               const CWeaponMode&, int) const {
  return kWCR_OtherProjectile;
}

CVector3f CActor::GetOrbitPosition(const CStateManager&) const {
  return mTransform.GetTranslation();
}

CVector3f CActor::GetAimPosition(const CStateManager&, float) const {
  return mTransform.GetTranslation();
}

CVector3f CActor::GetHomingPosition(const CStateManager& mgr, float f) const {
  return GetAimPosition(mgr, f);
}

CVector3f CActor::GetScanObjectIndicatorPosition(const CStateManager& mgr) const {
  const CGameCamera& cam = mgr.GetCameraManager()->GetCurrentCamera(mgr);
  CVector3f orbitPos = GetOrbitPosition(mgr);
  float mag = (cam.GetTranslation() - orbitPos).Magnitude();
  float max = rstl::max_val(rstl::max_val(mRenderBounds.GetDepth(), mRenderBounds.GetWidth()),
                           mRenderBounds.GetHeight()) * 0.5f;
  max = rstl::min_val(max, mag - cam.GetNearClipDistance() - 0.1f);
  return orbitPos - (orbitPos - cam.GetTranslation()).AsNormalized() * max;
}

bool CActor::IsModelOpaque(const CStateManager& mgr) const {
  if (mPointGeneratorParticles) {
    return false;
  } else if (!HasModelData()) {
    return true;
  } else if (static_cast< char >(mDrawFlags.GetTrans()) > 4) {
    return false;
  } else {
    CModelData::EWhichModel which = CModelData::GetRenderingModel(mgr);
    return mModelData->IsDefinitelyOpaque(which);
  }
}

void CActor::SetCalculateLighting(bool b) {
  if (mActorLights.null()) {
    mActorLights = rs_new CActorLights(8, CVector3f::Zero(), 4, 4);
  }
  mCalculateLighting = b;
}

void CActor::SetActorLights(rstl::auto_ptr< CActorLights > lights) {
  mActorLights = lights.release();
  mCalculateLighting = true;
}

const CMaterialFilter& CActor::GetMaterialFilter() const { return mMaterialFilter; }

void CActor::SetMaterialFilter(const CMaterialFilter& filter) { mMaterialFilter = filter; }

void CActor::SetActive(const bool active) {
  SetTransformDirty(true);
  SetTransformDirtySpare(true);
  SetPreRenderHasMoved(true);
  mDrawEnabled = active; // no setter?
  CEntity::SetActive(active);
}

void CActor::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  switch (msg) {
  case kSM_Activate: {
    if (!GetActive()) {
      mTime = CGraphics::GetSecondsMod900();
    }
    break;
  }
  case kSM_Deactivate: {
    RemoveEmitter();
    break;
  }
  case kSM_Deleted: {
    RemoveEmitter();
    if (HasModelData() && AnimationData() != nullptr) {
      AnimationData()->GetParticleDB().DeleteAllLights(mgr);
    }
    break;
  }
  case kSM_Registered: {
    if (!mScanObjectInfo.null()) {
      AddMaterial(kMT_Scannable, mgr);
    } else {
      RemoveMaterial(kMT_Scannable, mgr);
    }
    if (HasAnimation()) {
      AnimationData()->InitializeEffects(mgr, GetAreaId(), GetModelData()->ScaleCopy());
    }
    break;
  }
  case kSM_AddSplashInhabitant: {
    SetInFluid(true, uid);
    break;
  }
  case kSM_RemoveSplashInhabitant: {
#if VERSION >= VERSION_GM8E_02
    SetInFluid(false, uid);
#else
    SetInFluid(false, kInvalidUniqueId);
#endif
    break;
  }
  case kSM_InitializedInArea: {
    rstl::vector< SConnection >::const_iterator iter = GetConnectionList().begin();
    for (; iter != GetConnectionList().end(); ++iter) {
      if (iter->mState != kSS_Default) {
        continue;
      }
      CActor* act = TCastToPtr< CActor >(mgr.ObjectById(mgr.GetIdForScript(iter->mObjId)));
      if (act != nullptr && mNextDrawNode == kInvalidUniqueId) {
        mNextDrawNode = act->GetUniqueId();
      }
    }
    break;
  }
  default:
    break;
  }
  CEntity::AcceptScriptMsg(msg, uid, mgr);
}

CAABox CActor::GetSortingBounds(const CStateManager& mgr) const { return GetRenderBoundsCached(); }

void CActor::FluidFXThink(EFluidState, CScriptWater&, CStateManager&) {}

void CActor::OnScanStateChange(EScanState state, CStateManager& mgr) {
  switch (state) {
  case kSS_Start:
    SendScriptMsgs(kSS_ScanStart, mgr, kSM_None);
    break;
  case kSS_Processing:
    SendScriptMsgs(kSS_ScanProcessing, mgr, kSM_None);
    break;
  case kSS_Done:
    SendScriptMsgs(kSS_ScanDone, mgr, kSM_None);
    break;
  }
}

CScannableObjectInfo* CActor::GetScannableObjectInfo() const {
  if (mScanObjectInfo.null()) {
    return nullptr;
  }

  if (mScanObjectInfo->TryCache()) {
    return mScanObjectInfo->GetObject();
  }

  return nullptr;
}

void CActor::MoveScannableObjectInfoToActor(CActor* actor, CStateManager& mgr) {
  if (actor == nullptr) {
    return;
  }

  actor->mScanObjectInfo = mScanObjectInfo;
  actor->AddMaterial(kMT_Scannable, mgr);
  RemoveMaterial(kMT_Scannable, mgr);
}

void CActor::SetMuted(bool b) {
  mMuted = b;
  RemoveEmitter();
}

void CActor::SetVolume(const uchar volume) {
  if (CSfxHandle handle = mLoopingSfxHandle) {
    CSfxManager::UpdateEmitter(handle, GetTranslation(), CVector3f::Zero(), volume);
  }
  mMaxVol = volume;
}

void CActor::SetSoundEventPitchBend(int v) {
  mEnablePitchBend = true;
  mPitchBend = v;
  if (mLoopingSfxHandle) {
    CSfxManager::PitchBend(mLoopingSfxHandle, v);
  }
}

CSfxHandle CActor::GetSfxHandle() const { return mLoopingSfxHandle; }

void CActor::SetInFluid(bool in, TUniqueId uid) {
#if VERSION >= VERSION_GM8E_02
  if (in) {
    bool found = false;
    for (int i = 0; i < 4; ++i) {
      if (mFluidIds[i] == uid) {
        found = true;
        break;
      }
    }
    if (!found) {
      for (int i = 3; i > 0; --i) {
        mFluidIds[i] = mFluidIds[i - 1];
      }
      mFluidIds[0] = uid;
      if (mFluidCounter < 4) {
        ++mFluidCounter;
      }
    }
  } else {
    for (int i = 0; i < 4; ++i) {
      if (mFluidIds[i] == uid) {
        --mFluidCounter;
        for (int j = i + 1; j < 4; ++j) {
          mFluidIds[j - 1] = mFluidIds[j];
        }
        mFluidIds[3] = kInvalidUniqueId;
        break;
      }
    }
  }
  mFluidId = mFluidIds[0];
#else
  if (in) {
    mFluidCounter += 1;
    mFluidId = uid;
  } else if (mFluidCounter != 0) {
    mFluidCounter--;
    if (mFluidCounter == 0) {
      mFluidId = kInvalidUniqueId;
    }
  }
#endif
}

// TODO nonmatching
void CActor::ProcessSoundEvent(const int sfxId, const float weight, const int flags,
                               const float fallOff, const float maxDist, const uchar minVol,
                               const uchar maxVol, const CVector3f& toListener,
                               const CVector3f& position, const int aid, CStateManager& mgr,
                               const bool translateId) {
  if (toListener.MagSquared() < maxDist * maxDist) {
    const TSfxId id =
        translateId ? CSfxManager::TranslateSFXID(sfxId) : static_cast< TSfxId >(sfxId);

    const bool looping = (sfxId & 0x80000000) != 0;
    const bool nonEmitter = (sfxId & 0x40000000) != 0;
    const bool continuousUpdate = sfxId & 0x20000000;
    const bool useAcoustics = (flags & 0x80) == 0;

    uint musyxFlags = 0x1; // Continuous parameter update
    if (flags & 0x8) {
      musyxFlags |= 0x8; // Doppler FX
    }

    CAudioSys::C3DEmitterParmData parms(maxDist, fallOff, musyxFlags, maxVol, minVol);
    parms.mPos = position;
    parms.mDir = CVector3f::Zero();
    parms.mSfxId = id;

    if (mgr.Random()->Float() <= weight) {
      if (looping) {
        const CSfxHandle curHandle = mLoopingSfxHandle;
        const TSfxId curId = mSfxId;
        if (!curHandle) {
          CSfxHandle handle;
          if (nonEmitter) {
            handle =
                CSfxManager::SfxStart(id, 127, 64, true, CSfxManager::kMedPriority, true, aid);
          } else {
            handle =
                CSfxManager::AddEmitter(parms, useAcoustics, CSfxManager::kMedPriority, true, aid);
          }
          if (handle) {
            mSfxId = id;
            mLoopingSfxHandle = handle;
            if (mEnablePitchBend) {
              CSfxManager::PitchBend(handle, mPitchBend);
            }
          }
        } else if (curId == id) {
          CSfxManager::UpdateEmitter(curHandle, parms.mPos, parms.mDir, maxVol);
        } else if (flags & 0x4) {
          CSfxManager::RemoveEmitter(curHandle);
          CSfxHandle handle =
              CSfxManager::AddEmitter(parms, useAcoustics, CSfxManager::kMedPriority, true, aid);
          if (handle) {
            mSfxId = id;
            mLoopingSfxHandle = handle;
            if (mEnablePitchBend && handle) {
              CSfxManager::PitchBend(handle, mPitchBend);
            }
          }
        }
      } else {
        CSfxHandle handle;
        if (nonEmitter) {
          handle = CSfxManager::SfxStart(id, 127, 64, useAcoustics, CSfxManager::kMedPriority,
                                         false, aid);
        } else {
          handle =
              CSfxManager::AddEmitter(parms, useAcoustics, CSfxManager::kMedPriority, false, aid);
        }
        if (continuousUpdate) {
          mNonLoopingSfxHandles[mNextNonLoopingSfxHandle] = handle;
          mNextNonLoopingSfxHandle =
              (mNextNonLoopingSfxHandle + 1) % mNonLoopingSfxHandles.size();
        }

        if (mEnablePitchBend) {
          CSfxManager::PitchBend(handle, mPitchBend);
        }
      }
    }
  }
}

CTransform4f CActor::GetLocatorTransform(const rstl::string& segName) const {
  return GetModelData()->GetLocatorTransform(segName);
}

CTransform4f CActor::GetScaledLocatorTransform(const rstl::string& segName) const {
  return GetModelData()->GetScaledLocatorTransform(segName);
}

void CActor::SetTranslation(const CVector3f& vec) {
  mTransform.SetTranslation(vec);
  SetTransformDirty(true);
  SetTransformDirtySpare(true);
  SetPreRenderHasMoved(true);
}

void CActor::CreateShadowIfNeeded() {
  if (!mSimpleShadow.null()) {
    return;
  }

  if (HasModelData()) {
    mSimpleShadow = rs_new CSimpleShadow(1.f, 1.f, 20.f, 0.05f);
  }
}

void CActor::SetDrawShadow(bool enabled) {
  if (enabled) {
    CreateShadowIfNeeded();
    if (!mShadowEnabled && !mSimpleShadow.null()) {
      mShadowDirty = true;
    }
  }
  mShadowEnabled = enabled;
}

bool CActor::CanDrawStatic() const {
  if (!GetActive() || !HasModelData() || static_cast< char >(mDrawFlags.GetBlendMode()) > 4) {
    return false;
  }
  const CModelData* modelData = GetModelData();
  if (modelData->IsNull() || modelData->HasAnimation()) {
    return false;
  }
  return true;
}

void CActor::UpdateSfxEmitters() {
  uint count = mNonLoopingSfxHandles.size();
  for (uint i = 0; i < count; ++i) {
    CSfxManager::UpdateEmitter(mNonLoopingSfxHandles[i], GetTranslation(), CVector3f::Zero(),
                               mMaxVol);
  }
}
