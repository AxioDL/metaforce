#include "Runtime/World/CActor.hpp"

#include "Runtime/CSimplePool.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/CTimeProvider.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Audio/CSfxManager.hpp"
#include "Runtime/Camera/CGameCamera.hpp"
#include "Runtime/Character/CActorLights.hpp"
#include "Runtime/Character/IAnimReader.hpp"
#include "Runtime/Collision/CMaterialList.hpp"
#include "Runtime/Graphics/CBooRenderer.hpp"
#include "Runtime/Graphics/CSkinnedModel.hpp"
#include "Runtime/World/CActorParameters.hpp"
#include "Runtime/World/CWorld.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

#include <hecl/CVarManager.hpp>

namespace urde {
static CMaterialList MakeActorMaterialList(const CMaterialList& materialList, const CActorParameters& params) {
  CMaterialList ret = materialList;
  if (params.GetVisorParameters().x0_4_b1)
    ret.Add(EMaterialTypes::Unknown46);
  if (params.GetVisorParameters().x0_5_scanPassthrough)
    ret.Add(EMaterialTypes::ScanPassthrough);
  return ret;
}

CActor::CActor(TUniqueId uid, bool active, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
               CModelData&& mData, const CMaterialList& list, const CActorParameters& params, TUniqueId otherUid)
: CEntity(uid, info, active, name)
, x34_transform(xf)
, x68_material(MakeActorMaterialList(list, params))
, x70_materialFilter(CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {0ull}))
, xc6_nextDrawNode(otherUid)
, xe5_29_globalTimeProvider(params.x58_24_globalTimeProvider)
, xe5_30_renderUnsorted(params.x58_26_renderUnsorted)
, xe6_27_thermalVisorFlags(u8(params.x58_25_thermalHeat ? 2 : 1))
, xe6_31_targetableVisorFlags(params.GetVisorParameters().GetMask())
, xe7_29_drawEnabled(active) {
  x90_actorLights = mData.IsNull() ? nullptr : params.x0_lightParms.MakeActorLights();
  if (mData.x10_animData || mData.x1c_normalModel)
    x64_modelData = std::make_unique<CModelData>(std::move(mData));
  xd0_damageMag = params.x64_thermalMag;
  xd8_nonLoopingSfxHandles.resize(2);

  if (x64_modelData) {
    if (params.x44_xrayAssets.first.IsValid())
      x64_modelData->SetXRayModel(params.x44_xrayAssets);
    if (params.x4c_thermalAssets.first.IsValid())
      x64_modelData->SetInfraModel(params.x4c_thermalAssets);
    if (!params.x0_lightParms.x1c_makeLights || params.x0_lightParms.x3c_maxAreaLights == 0)
      x64_modelData->x18_ambientColor = params.x0_lightParms.x18_noLightsAmbient;
    x64_modelData->x14_25_sortThermal = !params.x58_27_noSortThermal;
  }

  if (params.x40_scanParms.GetScanId().IsValid())
    x98_scanObjectInfo = g_SimplePool->GetObj(SObjectTag{FOURCC('SCAN'), params.x40_scanParms.GetScanId()});
}

void CActor::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  switch (msg) {
  case EScriptObjectMessage::Activate: {
    if (!x30_24_active)
      xbc_time = CGraphics::GetSecondsMod900();
    break;
  }
  case EScriptObjectMessage::Deactivate:
    RemoveEmitter();
    break;
  case EScriptObjectMessage::Deleted: // 34
  {
    RemoveEmitter();
    if (HasModelData() && !x64_modelData->IsNull())
      if (CAnimData* aData = x64_modelData->GetAnimationData())
        aData->GetParticleDB().DeleteAllLights(mgr);
    break;
  }
  case EScriptObjectMessage::Registered: // 33
  {
    if (x98_scanObjectInfo)
      AddMaterial(EMaterialTypes::Scannable, mgr);
    else
      RemoveMaterial(EMaterialTypes::Scannable, mgr);

    if (HasModelData() && x64_modelData->HasAnimData()) {
      TAreaId aid = GetAreaId();
      x64_modelData->GetAnimationData()->InitializeEffects(mgr, aid, x64_modelData->GetScale());
    }
    break;
  }
  case EScriptObjectMessage::UpdateSplashInhabitant: // 37
    SetInFluid(true, uid);
    break;
  case EScriptObjectMessage::RemoveSplashInhabitant: // 39
    SetInFluid(false, kInvalidUniqueId);
    break;
  case EScriptObjectMessage::InitializedInArea: // 35
  {
    for (const SConnection& conn : x20_conns) {
      if (conn.x0_state != EScriptObjectState::Default)
        continue;

      const CActor* act = TCastToConstPtr<CActor>(mgr.GetObjectById(mgr.GetIdForScript(conn.x8_objId)));
      if (act && xc6_nextDrawNode == kInvalidUniqueId)
        xc6_nextDrawNode = act->GetUniqueId();
    }
    break;
  }
  default:
    break;
  }
  CEntity::AcceptScriptMsg(msg, uid, mgr);
}

void CActor::PreRender(CStateManager& mgr, const zeus::CFrustum& planes) {
  if (!x64_modelData || x64_modelData->IsNull())
    return;

  xe4_30_outOfFrustum = !planes.aabbFrustumTest(x9c_renderBounds);
  if (!xe4_30_outOfFrustum) {
    xe7_28_worldLightingDirty = true;

    bool lightsDirty = false;
    if (xe4_29_actorLightsDirty) {
      xe4_29_actorLightsDirty = false;
      lightsDirty = true;
      xe5_25_shadowDirty = true;
    } else if (xe7_28_worldLightingDirty) {
      lightsDirty = true;
    } else if (x90_actorLights && x90_actorLights->GetIsDirty()) {
      lightsDirty = true;
    }

    if (xe5_25_shadowDirty && xe5_24_shadowEnabled && x94_simpleShadow) {
      x94_simpleShadow->Calculate(x64_modelData->GetBounds(), x34_transform, mgr);
      xe5_25_shadowDirty = false;
    }

    if (xe4_31_calculateLighting && x90_actorLights) {
      zeus::CAABox bounds = x64_modelData->GetBounds(x34_transform);
      if (mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::EPlayerVisor::Thermal) {
        x90_actorLights->BuildConstantAmbientLighting();
      } else {
        if (lightsDirty && x4_areaId != kInvalidAreaId) {
          const CGameArea* area = mgr.GetWorld()->GetAreaAlways(x4_areaId);
          if (area->IsPostConstructed())
            if (x90_actorLights->BuildAreaLightList(mgr, *area, bounds))
              xe7_28_worldLightingDirty = false;
        }
        x90_actorLights->BuildDynamicLightList(mgr, bounds);
      }
    }

    if (x64_modelData->HasAnimData())
      x64_modelData->GetAnimationData()->PreRender();
  } else {
    if (xe4_29_actorLightsDirty) {
      xe4_29_actorLightsDirty = false;
      xe5_25_shadowDirty = true;
    }

    if (xe5_25_shadowDirty && xe5_24_shadowEnabled && x94_simpleShadow) {
      zeus::CAABox bounds = x64_modelData->GetBounds(x34_transform);
      if (planes.aabbFrustumTest(x94_simpleShadow->GetMaxShadowBox(bounds))) {
        x94_simpleShadow->Calculate(x64_modelData->GetBounds(), x34_transform, mgr);
        xe5_25_shadowDirty = false;
      }
    }
  }
}

void CActor::AddToRenderer(const zeus::CFrustum& planes, CStateManager& mgr) {
  if (!x64_modelData || x64_modelData->IsNull()) {
    return;
  }

  if (xe6_29_renderParticleDBInside) {
    x64_modelData->RenderParticles(planes);
  }

  if (!xe4_30_outOfFrustum) {
    if (CanRenderUnsorted(mgr)) {
      Render(mgr);
    } else {
      EnsureRendered(mgr);
    }
  }

  if (mgr.GetPlayerState()->GetActiveVisor(mgr) != CPlayerState::EPlayerVisor::XRay &&
      mgr.GetPlayerState()->GetActiveVisor(mgr) != CPlayerState::EPlayerVisor::Thermal && xe5_24_shadowEnabled &&
      x94_simpleShadow->Valid() && planes.aabbFrustumTest(x94_simpleShadow->GetBounds())) {
    g_Renderer->AddDrawable(x94_simpleShadow.get(), x94_simpleShadow->GetTransform().origin,
                            x94_simpleShadow->GetBounds(), 1, CBooRenderer::EDrawableSorting::SortedCallback);
  }
}

void CActor::DrawTouchBounds() const {
  // Empty
}

void CActor::RenderInternal(const CStateManager& mgr) const {
  SCOPED_GRAPHICS_DEBUG_GROUP(
      fmt::format(FMT_STRING("CActor::RenderInternal {} {} {}"), x8_uid, xc_editorId, x10_name).c_str(), zeus::skOrange);

  CModelData::EWhichModel which = CModelData::GetRenderingModel(mgr);
  if (which == CModelData::EWhichModel::ThermalHot) {
    if (x64_modelData->GetSortThermal()) {
      float addMag;
      float mulMag = 1.f;
      if (xd0_damageMag <= 1.f) {
        mulMag = xd0_damageMag;
        addMag = 0.f;
      } else if (xd0_damageMag < 2.f) {
        addMag = xd0_damageMag - 1.f;
      } else {
        addMag = 1.f;
      }

      zeus::CColor mulColor(mulMag * xb4_drawFlags.x4_color.a(), xb4_drawFlags.x4_color.a());
      zeus::CColor addColor(addMag, xb4_drawFlags.x4_color.a() / 4.f);
      x64_modelData->RenderThermal(x34_transform, mulColor, addColor, xb4_drawFlags);
      return;
    } else if (mgr.GetThermalColdScale2() > 0.00001f && !xb4_drawFlags.x0_blendMode) {
      zeus::CColor color(
          zeus::clamp(0.f,
                      std::min((mgr.GetThermalColdScale2() + mgr.GetThermalColdScale1()) * mgr.GetThermalColdScale2(),
                               mgr.GetThermalColdScale2()),
                      1.f),
          1.f);
      CModelFlags flags(2, xb4_drawFlags.x1_matSetIdx, xb4_drawFlags.x2_flags, color);
      x64_modelData->Render(mgr, x34_transform, x90_actorLights.get(), flags);
      return;
    }
  }
  x64_modelData->Render(which, x34_transform, x90_actorLights.get(), xb4_drawFlags);
}

bool CActor::IsModelOpaque(const CStateManager& mgr) const {
  if (xe5_31_pointGeneratorParticles)
    return false;
  if (!x64_modelData || x64_modelData->IsNull())
    return true;
  if (xb4_drawFlags.x0_blendMode > 4)
    return false;
  return x64_modelData->IsDefinitelyOpaque(CModelData::GetRenderingModel(mgr));
}

void CActor::Render(CStateManager& mgr) {
  if (x64_modelData && !x64_modelData->IsNull()) {
    bool renderPrePostParticles = xe6_29_renderParticleDBInside && x64_modelData && x64_modelData->HasAnimData();
    if (renderPrePostParticles)
      x64_modelData->GetAnimationData()->GetParticleDB().RenderSystemsToBeDrawnFirst();

    if (xe7_27_enableRender) {
      if (xe5_31_pointGeneratorParticles)
        mgr.SetupParticleHook(*this);
      if (xe5_29_globalTimeProvider) {
        RenderInternal(mgr);
      } else {
        const float timeSince = CGraphics::GetSecondsMod900() - xbc_time;
        const float tpTime = timeSince - std::floor(timeSince / 900.f) * 900.f;
        CTimeProvider tp(tpTime);
        RenderInternal(mgr);
      }
      if (xe5_31_pointGeneratorParticles) {
        CSkinnedModel::ClearPointGeneratorFunc();
        mgr.GetActorModelParticles()->Render(mgr, *this);
      }
    }

    if (renderPrePostParticles)
      x64_modelData->GetAnimationData()->GetParticleDB().RenderSystemsToBeDrawnLast();
  }
  DrawTouchBounds();
}

bool CActor::CanRenderUnsorted(const CStateManager& mgr) const {
  if (x64_modelData && x64_modelData->HasAnimData() &&
      x64_modelData->GetAnimationData()->GetParticleDB().AreAnySystemsDrawnWithModel() && xe6_29_renderParticleDBInside)
    return false;
  else if (xe5_30_renderUnsorted || IsModelOpaque(mgr))
    return true;
  return false;
}

void CActor::CalculateRenderBounds() {
  if (x64_modelData && (x64_modelData->GetAnimationData() || x64_modelData->GetNormalModel()))
    x9c_renderBounds = x64_modelData->GetBounds(x34_transform);
  else
    x9c_renderBounds = zeus::CAABox(x34_transform.origin, x34_transform.origin);
}

CHealthInfo* CActor::HealthInfo(CStateManager&) { return nullptr; }

const CDamageVulnerability* CActor::GetDamageVulnerability() const {
  return &CDamageVulnerability::NormalVulnerabilty();
}

const CDamageVulnerability* CActor::GetDamageVulnerability(const zeus::CVector3f&, const zeus::CVector3f&,
                                                           const CDamageInfo&) const {
  return GetDamageVulnerability();
}

std::optional<zeus::CAABox> CActor::GetTouchBounds() const { return {}; }

void CActor::Touch(CActor&, CStateManager&) {}

zeus::CVector3f CActor::GetOrbitPosition(const CStateManager&) const { return x34_transform.origin; }

zeus::CVector3f CActor::GetAimPosition(const CStateManager&, float) const { return x34_transform.origin; }

zeus::CVector3f CActor::GetHomingPosition(const CStateManager& mgr, float f) const { return GetAimPosition(mgr, f); }

zeus::CVector3f CActor::GetScanObjectIndicatorPosition(const CStateManager& mgr) const {
  const CGameCamera* cam = mgr.GetCameraManager()->GetCurrentCamera(mgr);
  zeus::CVector3f orbitPos = GetOrbitPosition(mgr);
  float camToOrbitPos = (cam->GetTranslation() - orbitPos).magnitude();
  const zeus::CVector3f boundsExtent = x9c_renderBounds.max - x9c_renderBounds.min;
  float distFac = std::min(std::max(boundsExtent.x(), std::max(boundsExtent.y(), boundsExtent.z())) * 0.5f,
                           camToOrbitPos - cam->GetNearClipDistance() - 0.1f);
  return orbitPos - (orbitPos - cam->GetTranslation()).normalized() * distFac;
}

void CActor::RemoveEmitter() {
  if (x8c_loopingSfxHandle) {
    CSfxManager::RemoveEmitter(x8c_loopingSfxHandle);
    x88_sfxId = -1;
    x8c_loopingSfxHandle.reset();
  }
}

void CActor::SetVolume(float vol) {
  if (x8c_loopingSfxHandle)
    CSfxManager::UpdateEmitter(x8c_loopingSfxHandle, GetTranslation(), zeus::skZero3f, vol);
  xd4_maxVol = vol;
}

zeus::CTransform CActor::GetScaledLocatorTransform(std::string_view segName) const {
  return x64_modelData->GetScaledLocatorTransform(segName);
}

zeus::CTransform CActor::GetLocatorTransform(std::string_view segName) const {
  return x64_modelData->GetLocatorTransform(segName);
}

EWeaponCollisionResponseTypes CActor::GetCollisionResponseType(const zeus::CVector3f&, const zeus::CVector3f&,
                                                               const CWeaponMode&, EProjectileAttrib) const {
  return EWeaponCollisionResponseTypes::OtherProjectile;
}

void CActor::FluidFXThink(CActor::EFluidState, CScriptWater&, CStateManager&) {}

void CActor::OnScanStateChanged(EScanState state, CStateManager& mgr) {
  if (state == EScanState::Start)
    SendScriptMsgs(EScriptObjectState::ScanStart, mgr, EScriptObjectMessage::None);
  else if (state == EScanState::Processing)
    SendScriptMsgs(EScriptObjectState::ScanProcessing, mgr, EScriptObjectMessage::None);
  else if (state == EScanState::Done)
    SendScriptMsgs(EScriptObjectState::ScanDone, mgr, EScriptObjectMessage::None);
}

zeus::CAABox CActor::GetSortingBounds(const CStateManager&) const { return x9c_renderBounds; }

void CActor::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType event, float dt) {
  if (event == EUserEventType::LoopedSoundStop)
    RemoveEmitter();
}

void CActor::RemoveMaterial(EMaterialTypes t1, EMaterialTypes t2, EMaterialTypes t3, EMaterialTypes t4,
                            CStateManager& mgr) {
  x68_material.Remove(t1);
  RemoveMaterial(t2, t3, t4, mgr);
}

void CActor::RemoveMaterial(EMaterialTypes t1, EMaterialTypes t2, EMaterialTypes t3, CStateManager& mgr) {
  x68_material.Remove(t1);
  RemoveMaterial(t2, t3, mgr);
}

void CActor::RemoveMaterial(EMaterialTypes t1, EMaterialTypes t2, CStateManager& mgr) {
  x68_material.Remove(t1);
  RemoveMaterial(t2, mgr);
}

void CActor::RemoveMaterial(EMaterialTypes t, CStateManager& mgr) {
  x68_material.Remove(t);
  mgr.UpdateObjectInLists(*this);
}

void CActor::AddMaterial(EMaterialTypes t1, EMaterialTypes t2, EMaterialTypes t3, EMaterialTypes t4, EMaterialTypes t5,
                         CStateManager& mgr) {
  x68_material.Add(t1);
  AddMaterial(t2, t3, t4, t5, mgr);
}

void CActor::AddMaterial(EMaterialTypes t1, EMaterialTypes t2, EMaterialTypes t3, EMaterialTypes t4,
                         CStateManager& mgr) {
  x68_material.Add(t1);
  AddMaterial(t2, t3, t4, mgr);
}

void CActor::AddMaterial(EMaterialTypes t1, EMaterialTypes t2, EMaterialTypes t3, CStateManager& mgr) {
  x68_material.Add(t1);
  AddMaterial(t2, t3, mgr);
}

void CActor::AddMaterial(EMaterialTypes t1, EMaterialTypes t2, CStateManager& mgr) {
  x68_material.Add(t1);
  AddMaterial(t2, mgr);
}

void CActor::AddMaterial(EMaterialTypes type, CStateManager& mgr) {
  x68_material.Add(type);
  mgr.UpdateObjectInLists(*this);
}

void CActor::AddMaterial(const CMaterialList& l) { x68_material.Add(l); }

void CActor::CreateShadow(bool enabled) {
  if (enabled) {
    _CreateShadow();
    if (!xe5_24_shadowEnabled && x94_simpleShadow)
      xe5_25_shadowDirty = true;
  }
  xe5_24_shadowEnabled = enabled;
}

void CActor::_CreateShadow() {
  if (!x94_simpleShadow && x64_modelData && (x64_modelData->HasAnimData() || x64_modelData->HasNormalModel())) {
    x94_simpleShadow = std::make_unique<CSimpleShadow>(1.f, 1.f, 20.f, 0.05f);
  }
}

void CActor::_CreateReflectionCube() {
  if (hecl::com_cubemaps->toBoolean()) {
    CGraphics::CommitResources([this](boo::IGraphicsDataFactory::Context& ctx) {
      m_reflectionCube = ctx.newCubeRenderTexture(CUBEMAP_RES, CUBEMAP_MIPS);
      return true;
    } BooTrace);
  }
}

void CActor::SetCallTouch(bool callTouch) { xe5_28_callTouch = callTouch; }

bool CActor::GetCallTouch() const { return xe5_28_callTouch; }

void CActor::SetUseInSortedLists(bool use) { xe5_27_useInSortedLists = use; }

bool CActor::GetUseInSortedLists() const { return xe5_27_useInSortedLists; }

void CActor::SetInFluid(bool in, TUniqueId uid) {
  if (in) {
    xe6_24_fluidCounter += 1;
    xc4_fluidId = uid;
  } else {
    if (!xe6_24_fluidCounter)
      return;

    xe6_24_fluidCounter -= 1;
    if (xe6_24_fluidCounter == 0)
      xc4_fluidId = kInvalidUniqueId;
  }
}

bool CActor::HasModelData() const { return bool(x64_modelData); }

void CActor::SetSoundEventPitchBend(s32 val) {
  xe6_30_enablePitchBend = true;
  xc0_pitchBend = val / 8192.f - 1.f;
  if (!x8c_loopingSfxHandle)
    return;

  CSfxManager::PitchBend(x8c_loopingSfxHandle, xc0_pitchBend);
}

void CActor::SetRotation(const zeus::CQuaternion& q) {
  x34_transform = q.toTransform(x34_transform.origin);
  xe4_27_notInSortedLists = true;
  xe4_28_transformDirty = true;
  xe4_29_actorLightsDirty = true;
}

void CActor::SetTranslation(const zeus::CVector3f& tr) {
  x34_transform.origin = tr;
  xe4_27_notInSortedLists = true;
  xe4_28_transformDirty = true;
  xe4_29_actorLightsDirty = true;
}

void CActor::SetTransform(const zeus::CTransform& tr) {
  x34_transform = tr;
  xe4_27_notInSortedLists = true;
  xe4_28_transformDirty = true;
  xe4_29_actorLightsDirty = true;
}

void CActor::SetAddedToken(u32 tok) { xcc_addedToken = tok; }

float CActor::GetPitch() const { return zeus::CQuaternion(x34_transform.buildMatrix3f()).pitch(); }

float CActor::GetYaw() const { return zeus::CQuaternion(x34_transform.buildMatrix3f()).yaw(); }

void CActor::EnsureRendered(const CStateManager& mgr) {
  const zeus::CAABox aabb = GetSortingBounds(mgr);
  EnsureRendered(mgr, aabb.closestPointAlongVector(CGraphics::g_ViewMatrix.basis[1]), aabb);
}

void CActor::EnsureRendered(const CStateManager& stateMgr, const zeus::CVector3f& pos, const zeus::CAABox& aabb) {
  if (x64_modelData) {
    x64_modelData->RenderUnsortedParts(x64_modelData->GetRenderingModel(stateMgr), x34_transform, x90_actorLights.get(),
                                       xb4_drawFlags);
  }
  stateMgr.AddDrawableActor(*this, pos, aabb);
}

void CActor::UpdateSfxEmitters() {
  for (CSfxHandle& sfx : xd8_nonLoopingSfxHandles)
    CSfxManager::UpdateEmitter(sfx, x34_transform.origin, zeus::skZero3f, xd4_maxVol);
}

void CActor::ProcessSoundEvent(u32 sfxId, float weight, u32 flags, float falloff, float maxDist, float minVol,
                               float maxVol, const zeus::CVector3f& toListener, const zeus::CVector3f& position,
                               TAreaId aid, CStateManager& mgr, bool translateId) {
  if (toListener.magSquared() >= maxDist * maxDist)
    return;
  u16 id = translateId ? CSfxManager::TranslateSFXID(sfxId) : sfxId;

  u32 musyxFlags = 0x1; // Continuous parameter update
  if (flags & 0x8)
    musyxFlags |= 0x8; // Doppler FX

  CAudioSys::C3DEmitterParmData parms;
  parms.x0_pos = position;
  parms.xc_dir = zeus::skZero3f;
  parms.x18_maxDist = maxDist;
  parms.x1c_distComp = falloff;
  parms.x20_flags = musyxFlags;
  parms.x24_sfxId = id;
  parms.x26_maxVol = maxVol;
  parms.x27_minVol = minVol;
  parms.x28_important = false;
  parms.x29_prio = 0x7f;

  bool useAcoustics = (flags & 0x80) == 0;
  bool looping = (sfxId & 0x80000000) != 0;
  bool nonEmitter = (sfxId & 0x40000000) != 0;
  bool continuousUpdate = (sfxId & 0x20000000) != 0;

  if (mgr.GetActiveRandom()->Float() > weight)
    return;

  if (looping) {
    u16 curId = x88_sfxId;
    if (!x8c_loopingSfxHandle) {
      CSfxHandle handle;
      if (nonEmitter)
        handle = CSfxManager::SfxStart(id, 1.f, 0.f, true, 0x7f, true, aid);
      else
        handle = CSfxManager::AddEmitter(parms, useAcoustics, 0x7f, true, aid);
      if (handle) {
        x88_sfxId = id;
        x8c_loopingSfxHandle = handle;
        if (xe6_30_enablePitchBend)
          CSfxManager::PitchBend(handle, xc0_pitchBend);
      }
    } else if (curId == id) {
      CSfxManager::UpdateEmitter(x8c_loopingSfxHandle, position, zeus::skZero3f, maxVol);
    } else if (flags & 0x4) {
      CSfxManager::RemoveEmitter(x8c_loopingSfxHandle);
      CSfxHandle handle = CSfxManager::AddEmitter(parms, useAcoustics, 0x7f, true, aid);
      if (handle) {
        x88_sfxId = id;
        x8c_loopingSfxHandle = handle;
        if (xe6_30_enablePitchBend)
          CSfxManager::PitchBend(handle, xc0_pitchBend);
      }
    }
  } else {
    CSfxHandle handle;
    if (nonEmitter)
      handle = CSfxManager::SfxStart(id, 1.f, 0.f, useAcoustics, 0x7f, false, aid);
    else
      handle = CSfxManager::AddEmitter(parms, useAcoustics, 0x7f, false, aid);

    if (continuousUpdate) {
      xd8_nonLoopingSfxHandles[xe4_24_nextNonLoopingSfxHandle] = handle;
      xe4_24_nextNonLoopingSfxHandle = (xe4_24_nextNonLoopingSfxHandle + 1) % xd8_nonLoopingSfxHandles.size();
    }

    if (xe6_30_enablePitchBend)
      CSfxManager::PitchBend(handle, xc0_pitchBend);
  }
}

SAdvancementDeltas CActor::UpdateAnimation(float dt, CStateManager& mgr, bool advTree) {
  SAdvancementDeltas deltas = x64_modelData->AdvanceAnimation(dt, mgr, GetAreaId(), advTree);
  x64_modelData->AdvanceParticles(x34_transform, dt, mgr);
  UpdateSfxEmitters();
  if (x64_modelData && x64_modelData->HasAnimData()) {
    zeus::CVector3f toCamera = mgr.GetCameraManager()->GetCurrentCamera(mgr)->GetTranslation() - x34_transform.origin;

    for (int i = 0; i < x64_modelData->GetAnimationData()->GetPassedSoundPOICount(); ++i) {
      CSoundPOINode& poi = CAnimData::g_SoundPOINodes[i];
      if (poi.GetPoiType() != EPOIType::Sound)
        continue;
      if (xe5_26_muted)
        continue;
      if (poi.GetCharacterIndex() != -1 &&
          x64_modelData->GetAnimationData()->GetCharacterIndex() != poi.GetCharacterIndex())
        continue;
      ProcessSoundEvent(poi.GetSfxId(), poi.GetWeight(), poi.GetFlags(), poi.GetFalloff(), poi.GetMaxDist(), 0.16f,
                        xd4_maxVol, toCamera, x34_transform.origin, x4_areaId, mgr, true);
    }

    for (int i = 0; i < x64_modelData->GetAnimationData()->GetPassedIntPOICount(); ++i) {
      CInt32POINode& poi = CAnimData::g_Int32POINodes[i];
      if (poi.GetPoiType() == EPOIType::SoundInt32) {
        if (xe5_26_muted)
          continue;
        if (poi.GetCharacterIndex() != -1 &&
            x64_modelData->GetAnimationData()->GetCharacterIndex() != poi.GetCharacterIndex())
          continue;
        ProcessSoundEvent(poi.GetValue(), poi.GetWeight(), poi.GetFlags(), 0.1f, 150.f, 0.16f, xd4_maxVol, toCamera,
                          x34_transform.origin, x4_areaId, mgr, true);
      } else if (poi.GetPoiType() == EPOIType::UserEvent) {
        DoUserAnimEvent(mgr, poi, EUserEventType(poi.GetValue()), dt);
      }
    }

    for (int i = 0; i < x64_modelData->GetAnimationData()->GetPassedParticlePOICount(); ++i) {
      CParticlePOINode& poi = CAnimData::g_ParticlePOINodes[i];
      if (poi.GetCharacterIndex() != -1 &&
          x64_modelData->GetAnimationData()->GetCharacterIndex() != poi.GetCharacterIndex())
        continue;
      x64_modelData->GetAnimationData()->GetParticleDB().SetParticleEffectState(poi.GetString(), true, mgr);
    }
  }
  return deltas;
}

void CActor::SetActorLights(std::unique_ptr<CActorLights>&& lights) {
  x90_actorLights = std::move(lights);
  xe4_31_calculateLighting = true;
}

bool CActor::CanDrawStatic() const {
  if (!x30_24_active)
    return false;

  if (x64_modelData && x64_modelData->HasNormalModel())
    return xb4_drawFlags.x0_blendMode <= 4;

  return false;
}

const CScannableObjectInfo* CActor::GetScannableObjectInfo() const {
  if (!x98_scanObjectInfo || !x98_scanObjectInfo.IsLoaded())
    return nullptr;
  return x98_scanObjectInfo.GetObj();
}

void CActor::SetCalculateLighting(bool c) {
  if (!x90_actorLights)
    x90_actorLights = std::make_unique<CActorLights>(8, zeus::skZero3f, 4, 4, false, false, false, 0.1f);
  xe4_31_calculateLighting = c;
}

float CActor::GetAverageAnimVelocity(int anim) const {
  if (HasModelData() && GetModelData()->HasAnimData())
    return GetModelData()->GetAnimationData()->GetAverageVelocity(anim);
  return 0.f;
}

void CActor::SetModelData(std::unique_ptr<CModelData>&& mData) {
  if (mData->IsNull())
    x64_modelData.reset();
  else
    x64_modelData = std::move(mData);
}

void CActor::SetMuted(bool muted) {
  xe5_26_muted = muted;
  RemoveEmitter();
}

void CActor::MoveScannableObjectInfoToActor(CActor* act, CStateManager& mgr) {
  if (!act)
    return;

  act->x98_scanObjectInfo = x98_scanObjectInfo;
  x98_scanObjectInfo = TLockedToken<CScannableObjectInfo>();
  act->AddMaterial(EMaterialTypes::Scannable, mgr);
  RemoveMaterial(EMaterialTypes::Scannable, mgr);
}
} // namespace urde
