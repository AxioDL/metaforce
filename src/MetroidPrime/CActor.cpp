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
, x34_transform(xf)
, x64_modelData(mData.IsNull() ? nullptr : rs_new CModelData(mData))
, x68_material(MakeActorMaterialList(list, params))
, x70_materialFilter(
      CMaterialFilter::MakeIncludeExclude(CMaterialList(SolidMaterial), CMaterialList()))
, x88_sfxId(InvalidSfxId)
, x90_actorLights(mData.IsNull() ? nullptr : params.GetLighting().MakeActorLights().release())
, x9c_renderBounds(CAABox::MakeMaxInvertedBox())
, xb4_drawFlags(CModelFlags::Normal())
, xbc_time(0.f)
, xc0_pitchBend(8192)
, xc4_fluidId(kInvalidUniqueId)
, xc6_nextDrawNode(nextDrawNode)
, xc8_drawnToken(-1)
, xcc_addedToken(-1)
, xd0_damageMag(params.GetThermalMag())
, xd4_maxVol(CAudioSys::kMaxVolume)
#if VERSION >= VERSION_GM8P_00
, xd8_fluidIds(kInvalidUniqueId)
#endif
, xd8_nonLoopingSfxHandles(CSfxHandle())
, xe4_24_nextNonLoopingSfxHandle(0)
, xe4_27_notInSortedLists(true)
, xe4_28_transformDirty(true)
, xe4_29_actorLightsDirty(true)
, xe4_30_outOfFrustum(false)
, xe4_31_calculateLighting(true)
, xe5_24_shadowEnabled(false)
, xe5_25_shadowDirty(false)
, xe5_26_muted(false)
, xe5_27_useInSortedLists(true)
, xe5_28_callTouch(true)
, xe5_29_globalTimeProvider(params.UseGlobalRenderTime())
, xe5_30_renderUnsorted(params.ForceRenderUnsorted())
, xe5_31_pointGeneratorParticles(false)
, xe6_24_fluidCounter(0)
, xe6_27_thermalVisorFlags(params.IsHotInThermal() ? kTF_Hot : kTF_Cold)
, xe6_29_renderParticleDBInside(true)
, xe6_30_enablePitchBend(false)
, xe6_31_targetableVisorFlags(params.GetVisorParameters().GetMask())
, xe7_27_enableRender(true)
, xe7_28_worldLightingDirty(false)
, xe7_29_drawEnabled(active)
, xe7_30_doTargetDistanceTest(true)
, xe7_31_targetable(true) {
  if (!x64_modelData.null()) {
    if (params.GetXRay().first != 0) {
      x64_modelData->SetXRayModel(params.GetXRay());
    }
    if (params.GetInfra().first != 0) {
      x64_modelData->SetInfraModel(params.GetInfra());
    }
    const CLightParameters& lighting = params.GetLighting();
    if (!lighting.ShouldMakeLights() || lighting.GetMaxAreaLights() == 0) {
      x64_modelData->SetAmbientColor(lighting.GetAmbientColor());
    }
    x64_modelData->SetSortThermal(!params.NoSortThermal());
  }
  const CAssetId scanId = params.GetScannable().GetScannableObject0();
  if (scanId != kInvalidAssetId) {
    x98_scanObjectInfo = rs_new TCachedToken< CScannableObjectInfo >(
        gpSimplePool->GetObj(SObjectTag('SCAN', scanId)));
    x98_scanObjectInfo->Lock();
  }
}

CActor::~CActor() { RemoveEmitter(); }

CAdvancementDeltas CActor::UpdateAnimation(float dt, CStateManager& mgr, bool advTree) {
  CAdvancementDeltas result = ModelData()->AdvanceAnimation(dt, mgr, GetAreaId(), advTree);
  ModelData()->AdvanceParticles(GetTransform(), dt, mgr);
  UpdateSfxEmitters();
  if (HasAnimation()) {
    ushort maxVol = xd4_maxVol;
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
  if (CSfxHandle handle = x8c_loopingSfxHandle) {
    CSfxManager::RemoveEmitter(handle);
    x88_sfxId = -1;
    x8c_loopingSfxHandle = CSfxHandle();
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
  x64_modelData = modelData.IsNull() ? nullptr : rs_new CModelData(modelData);
}

void CActor::PreRender(CStateManager& mgr, const CFrustumPlanes& planes) {
  if (HasModelData()) {
    SetPreRenderClipped(!planes.BoxInFrustumPlanes(x9c_renderBounds));
    if (!GetPreRenderClipped()) {
      bool lightsDirty = false;
      if (GetPreRenderHasMoved()) {
        SetPreRenderHasMoved(false);
        SetShadowDirty(true);
        lightsDirty = true;
      } else if (xe7_28_worldLightingDirty) {
        lightsDirty = true;
      } else if (HasActorLights() && GetActorLights()->GetNeedsRelight()) {
        lightsDirty = true;
      }

      // TODO why doesn't GetDrawShadow() work?
      if (GetShadowDirty() && xe5_24_shadowEnabled && HasShadow()) {
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
              xe7_28_worldLightingDirty = false;
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
      if (GetShadowDirty() && xe5_24_shadowEnabled && HasShadow()) {
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
    result = xe5_30_renderUnsorted || IsModelOpaque(mgr);
  }
  return result;
}

void CActor::Render(const CStateManager& mgr) const {
  if (GetModelData() && !NullModel()) {
    bool renderPrePostParticles = GetRenderParticleDatabaseInside() && HasAnimation();
    if (renderPrePostParticles) {
      GetAnimationData()->GetParticleDB().RenderSystemsToBeDrawnFirst();
    }

    if (xe7_27_enableRender) {
      if (xe5_31_pointGeneratorParticles) {
        mgr.SetupParticleHook(*this);
      }
      if (xe5_29_globalTimeProvider) {
        RenderInternal(mgr);
      } else {
        const float timeSince = CGraphics::GetSecondsMod900() - xbc_time;
        CTimeProvider tp(CMath::FastFmod(timeSince, 900.f));
        RenderInternal(mgr);
      }
      if (xe5_31_pointGeneratorParticles) {
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
      const float alpha = xb4_drawFlags.GetColorRef().GetAlpha();
      uchar addMag;
      float mulMag;
      if (xd0_damageMag <= 1.f) {
        mulMag = xd0_damageMag;
        addMag = 0;
      } else if (xd0_damageMag < 2.f) {
        mulMag = 1.f;
        addMag = CCast::ToUint8((xd0_damageMag - 1.f) * 255.f);
      } else {
        mulMag = 1.f;
        addMag = 255;
      }

      const uchar rgb = CCast::ToUint8((255.f * mulMag) * alpha);
      CColor mulColor(rgb, rgb, rgb, xb4_drawFlags.GetColorRef().GetAlphau8());
      CColor addColor(addMag, addMag, addMag, xb4_drawFlags.GetColorRef().GetAlphau8() / 4);
      GetModelData()->RenderThermal(x34_transform, mulColor, addColor, xb4_drawFlags);
      return;
    } else if (mgr.GetThermalColdScale2() > 0.0001f && xb4_drawFlags.GetTrans() == 0) {
      const float scale = rstl::max_val< float >(
          (mgr.GetThermalColdScale2() + mgr.GetThermalColdScale1()) * mgr.GetThermalColdScale2(),
          mgr.GetThermalColdScale2());
      const uchar rgb = CCast::ToUint8(CMath::Clamp(0.f, scale * 255.f, 255.f));
      CColor color(rgb, rgb, rgb, 255);
      CModelFlags flags = xb4_drawFlags;
      flags.x0_blendMode = CModelFlags::kT_Two;
      flags.x4_color = color;
      GetModelData()->Render(which, x34_transform, x90_actorLights.get(), flags);
      return;
    }
  }
  GetModelData()->Render(which, x34_transform, x90_actorLights.get(), xb4_drawFlags);
}

float CActor::GetYaw() const {
  float sq = sqrt(x34_transform.Get11() * x34_transform.Get11() +
                  x34_transform.Get01() * x34_transform.Get01());
  if (sq > 0.001f) {
    double ret = -atan2(x34_transform.Get01(), x34_transform.Get11());
    return ret;
  }
  return 0.f;
}

float CActor::GetPitch() const {
  float sq = CMath::SqrtF(x34_transform.Get11() * x34_transform.Get11() +
                          x34_transform.Get01() * x34_transform.Get01());
  double ret = -atan2(-x34_transform.Get21(), sq);
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

bool CActor::GetUseInSortedLists() const { return xe5_27_useInSortedLists; }

void CActor::SetUseInSortedLists(bool use) { xe5_27_useInSortedLists = use; }

bool CActor::GetCallTouch() const { return xe5_28_callTouch; }

void CActor::SetCallTouch(bool value) { xe5_28_callTouch = value; }

void CActor::AddMaterial(EMaterialTypes mat1, CStateManager& mgr) {
  x68_material.Add(mat1);
  mgr.UpdateObjectInLists(*this);
}

void CActor::AddMaterial(EMaterialTypes mat1, EMaterialTypes mat2, CStateManager& mgr) {
  x68_material.Add(mat1);
  x68_material.Add(mat2);
  mgr.UpdateObjectInLists(*this);
}

void CActor::AddMaterial(EMaterialTypes mat1, EMaterialTypes mat2, EMaterialTypes mat3,
                         CStateManager& mgr) {
  x68_material.Add(mat1);
  x68_material.Add(mat2);
  x68_material.Add(mat3);
  mgr.UpdateObjectInLists(*this);
}

void CActor::AddMaterial(EMaterialTypes mat1, EMaterialTypes mat2, EMaterialTypes mat3,
                         EMaterialTypes mat4, CStateManager& mgr) {
  x68_material.Add(mat1);
  x68_material.Add(mat2);
  x68_material.Add(mat3);
  x68_material.Add(mat4);
  mgr.UpdateObjectInLists(*this);
}

void CActor::AddMaterial(EMaterialTypes mat1, EMaterialTypes mat2, EMaterialTypes mat3,
                         EMaterialTypes mat4, EMaterialTypes mat5, CStateManager& mgr) {
  x68_material.Add(mat1);
  x68_material.Add(mat2);
  x68_material.Add(mat3);
  x68_material.Add(mat4);
  x68_material.Add(mat5);
  mgr.UpdateObjectInLists(*this);
}

void CActor::RemoveMaterial(EMaterialTypes mat1, CStateManager& mgr) {
  x68_material.Remove(mat1);
  mgr.UpdateObjectInLists(*this);
}

void CActor::RemoveMaterial(EMaterialTypes mat1, EMaterialTypes mat2, CStateManager& mgr) {

  x68_material.Remove(mat1);
  x68_material.Remove(mat2);
  mgr.UpdateObjectInLists(*this);
}

void CActor::RemoveMaterial(EMaterialTypes mat1, EMaterialTypes mat2, EMaterialTypes mat3,
                            CStateManager& mgr) {

  x68_material.Remove(mat1);
  x68_material.Remove(mat2);
  x68_material.Remove(mat3);
  mgr.UpdateObjectInLists(*this);
}

void CActor::RemoveMaterial(EMaterialTypes mat1, EMaterialTypes mat2, EMaterialTypes mat3,
                            EMaterialTypes mat4, CStateManager& mgr) {

  x68_material.Remove(mat1);
  x68_material.Remove(mat2);
  x68_material.Remove(mat3);
  x68_material.Remove(mat4);
  mgr.UpdateObjectInLists(*this);
}

EWeaponCollisionResponseTypes CActor::GetCollisionResponseType(const CVector3f&, const CVector3f&,
                                                               const CWeaponMode&, int) const {
  return kWCR_OtherProjectile;
}

CVector3f CActor::GetOrbitPosition(const CStateManager&) const {
  return x34_transform.GetTranslation();
}

CVector3f CActor::GetAimPosition(const CStateManager&, float) const {
  return x34_transform.GetTranslation();
}

CVector3f CActor::GetHomingPosition(const CStateManager& mgr, float f) const {
  return GetAimPosition(mgr, f);
}

CVector3f CActor::GetScanObjectIndicatorPosition(const CStateManager& mgr) const {
  const CGameCamera& cam = mgr.GetCameraManager()->GetCurrentCamera(mgr);
  CVector3f orbitPos = GetOrbitPosition(mgr);
  float mag = (cam.GetTranslation() - orbitPos).Magnitude();
  float max = rstl::max_val(rstl::max_val(x9c_renderBounds.GetDepth(), x9c_renderBounds.GetWidth()),
                           x9c_renderBounds.GetHeight()) * 0.5f;
  max = rstl::min_val(max, mag - cam.GetNearClipDistance() - 0.1f);
  return orbitPos - (orbitPos - cam.GetTranslation()).AsNormalized() * max;
}

bool CActor::IsModelOpaque(const CStateManager& mgr) const {
  if (xe5_31_pointGeneratorParticles) {
    return false;
  } else if (!HasModelData()) {
    return true;
  } else if (static_cast< char >(xb4_drawFlags.GetTrans()) > 4) {
    return false;
  } else {
    CModelData::EWhichModel which = CModelData::GetRenderingModel(mgr);
    return x64_modelData->IsDefinitelyOpaque(which);
  }
}

void CActor::SetCalculateLighting(bool b) {
  if (x90_actorLights.null()) {
    x90_actorLights = rs_new CActorLights(8, CVector3f::Zero(), 4, 4);
  }
  xe4_31_calculateLighting = b;
}

void CActor::SetActorLights(rstl::auto_ptr< CActorLights > lights) {
  x90_actorLights = lights.release();
  xe4_31_calculateLighting = true;
}

const CMaterialFilter& CActor::GetMaterialFilter() const { return x70_materialFilter; }

void CActor::SetMaterialFilter(const CMaterialFilter& filter) { x70_materialFilter = filter; }

void CActor::SetActive(const bool active) {
  SetTransformDirty(true);
  SetTransformDirtySpare(true);
  SetPreRenderHasMoved(true);
  xe7_29_drawEnabled = active; // no setter?
  CEntity::SetActive(active);
}

void CActor::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  switch (msg) {
  case kSM_Activate: {
    if (!GetActive()) {
      xbc_time = CGraphics::GetSecondsMod900();
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
    if (!x98_scanObjectInfo.null()) {
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
#if VERSION >= VERSION_GM8P_00
    SetInFluid(false, uid);
#else
    SetInFluid(false, kInvalidUniqueId);
#endif
    break;
  }
  case kSM_InitializedInArea: {
    rstl::vector< SConnection >::const_iterator iter = GetConnectionList().begin();
    for (; iter != GetConnectionList().end(); ++iter) {
      if (iter->x0_state != kSS_Default) {
        continue;
      }
      CActor* act = TCastToPtr< CActor >(mgr.ObjectById(mgr.GetIdForScript(iter->x8_objId)));
      if (act != nullptr && xc6_nextDrawNode == kInvalidUniqueId) {
        xc6_nextDrawNode = act->GetUniqueId();
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
  if (x98_scanObjectInfo.null()) {
    return nullptr;
  }

  if (x98_scanObjectInfo->TryCache()) {
    return x98_scanObjectInfo->GetObject();
  }

  return nullptr;
}

void CActor::MoveScannableObjectInfoToActor(CActor* actor, CStateManager& mgr) {
  if (actor == nullptr) {
    return;
  }

  actor->x98_scanObjectInfo = x98_scanObjectInfo;
  actor->AddMaterial(kMT_Scannable, mgr);
  RemoveMaterial(kMT_Scannable, mgr);
}

void CActor::SetMuted(bool b) {
  xe5_26_muted = b;
  RemoveEmitter();
}

void CActor::SetVolume(const uchar volume) {
  if (CSfxHandle handle = x8c_loopingSfxHandle) {
    CSfxManager::UpdateEmitter(handle, GetTranslation(), CVector3f::Zero(), volume);
  }
  xd4_maxVol = volume;
}

void CActor::SetSoundEventPitchBend(int v) {
  xe6_30_enablePitchBend = true;
  xc0_pitchBend = v;
  if (x8c_loopingSfxHandle) {
    CSfxManager::PitchBend(x8c_loopingSfxHandle, v);
  }
}

CSfxHandle CActor::GetSfxHandle() const { return x8c_loopingSfxHandle; }

void CActor::SetInFluid(bool in, TUniqueId uid) {
#if VERSION >= VERSION_GM8P_00
  if (in) {
    bool found = false;
    for (int i = 0; i < 4; ++i) {
      if (xd8_fluidIds[i] == uid) {
        found = true;
        break;
      }
    }
    if (!found) {
      for (int i = 3; i > 0; --i) {
        xd8_fluidIds[i] = xd8_fluidIds[i - 1];
      }
      xd8_fluidIds[0] = uid;
      if (xe6_24_fluidCounter < 4) {
        ++xe6_24_fluidCounter;
      }
    }
  } else {
    for (int i = 0; i < 4; ++i) {
      if (xd8_fluidIds[i] == uid) {
        --xe6_24_fluidCounter;
        for (int j = i + 1; j < 4; ++j) {
          xd8_fluidIds[j - 1] = xd8_fluidIds[j];
        }
        xd8_fluidIds[3] = kInvalidUniqueId;
        break;
      }
    }
  }
  xc4_fluidId = xd8_fluidIds[0];
#else
  if (in) {
    xe6_24_fluidCounter += 1;
    xc4_fluidId = uid;
  } else if (xe6_24_fluidCounter != 0) {
    xe6_24_fluidCounter--;
    if (xe6_24_fluidCounter == 0) {
      xc4_fluidId = kInvalidUniqueId;
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
    parms.x0_pos = position;
    parms.xc_dir = CVector3f::Zero();
    parms.x24_sfxId = id;

    if (mgr.Random()->Float() <= weight) {
      if (looping) {
        const CSfxHandle curHandle = x8c_loopingSfxHandle;
        const TSfxId curId = x88_sfxId;
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
            x88_sfxId = id;
            x8c_loopingSfxHandle = handle;
            if (xe6_30_enablePitchBend) {
              CSfxManager::PitchBend(handle, xc0_pitchBend);
            }
          }
        } else if (curId == id) {
          CSfxManager::UpdateEmitter(curHandle, parms.x0_pos, parms.xc_dir, maxVol);
        } else if (flags & 0x4) {
          CSfxManager::RemoveEmitter(curHandle);
          CSfxHandle handle =
              CSfxManager::AddEmitter(parms, useAcoustics, CSfxManager::kMedPriority, true, aid);
          if (handle) {
            x88_sfxId = id;
            x8c_loopingSfxHandle = handle;
            if (xe6_30_enablePitchBend && handle) {
              CSfxManager::PitchBend(handle, xc0_pitchBend);
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
          xd8_nonLoopingSfxHandles[xe4_24_nextNonLoopingSfxHandle] = handle;
          xe4_24_nextNonLoopingSfxHandle =
              (xe4_24_nextNonLoopingSfxHandle + 1) % xd8_nonLoopingSfxHandles.size();
        }

        if (xe6_30_enablePitchBend) {
          CSfxManager::PitchBend(handle, xc0_pitchBend);
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
  x34_transform.SetTranslation(vec);
  SetTransformDirty(true);
  SetTransformDirtySpare(true);
  SetPreRenderHasMoved(true);
}

void CActor::CreateShadowIfNeeded() {
  if (!x94_simpleShadow.null()) {
    return;
  }

  if (HasModelData()) {
    x94_simpleShadow = rs_new CSimpleShadow(1.f, 1.f, 20.f, 0.05f);
  }
}

void CActor::SetDrawShadow(bool enabled) {
  if (enabled) {
    CreateShadowIfNeeded();
    if (!xe5_24_shadowEnabled && !x94_simpleShadow.null()) {
      xe5_25_shadowDirty = true;
    }
  }
  xe5_24_shadowEnabled = enabled;
}

bool CActor::CanDrawStatic() const {
  if (!GetActive() || !HasModelData() || static_cast< char >(xb4_drawFlags.GetBlendMode()) > 4) {
    return false;
  }
  if (GetModelData()->IsNull() || GetModelData()->HasAnimation()) {
    return false;
  }
  return true;
}

void CActor::UpdateSfxEmitters() {
  uint count = xd8_nonLoopingSfxHandles.size();
  for (uint i = 0; i < count; ++i) {
    CSfxManager::UpdateEmitter(xd8_nonLoopingSfxHandles[i], GetTranslation(), CVector3f::Zero(),
                               xd4_maxVol);
  }
}
