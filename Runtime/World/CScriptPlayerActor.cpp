#include "Runtime/World/CScriptPlayerActor.hpp"

#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Camera/CGameCamera.hpp"
#include "Runtime/Character/CAssetFactory.hpp"
#include "Runtime/Character/CCharacterFactory.hpp"
#include "Runtime/Graphics/CBooRenderer.hpp"
#include "Runtime/MP1/MP1.hpp"
#include "Runtime/World/CActorParameters.hpp"
#include "Runtime/World/CLightParameters.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "Runtime/World/CWorld.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde {
CScriptPlayerActor::CScriptPlayerActor(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                                       const zeus::CTransform& xf, const CAnimRes& animRes, CModelData&& mData,
                                       const zeus::CAABox& aabox, bool setBoundingBox, const CMaterialList& list,
                                       float mass, float zMomentum, const CHealthInfo& hInfo,
                                       const CDamageVulnerability& dVuln, const CActorParameters& aParams, bool loop,
                                       bool active, u32 flags, CPlayerState::EBeamId beam)
: CScriptActor(uid, name, info, xf, std::move(mData), aabox, mass, zMomentum, list, hInfo, dVuln, aParams, loop, active,
               0, 1.f, false, false, false, false)
, x2e8_suitRes(animRes)
, x304_beam(beam)
, x350_flags(flags)
, x354_24_setBoundingBox(setBoundingBox) {
  CMaterialList exclude = GetMaterialFilter().GetExcludeList();
  CMaterialList include = GetMaterialFilter().GetIncludeList();
  include.Add(EMaterialTypes::Player);
  SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(include, exclude));

  SetActorLights(aParams.GetLightParameters().MakeActorLights());
  xe7_29_drawEnabled = true;
  x2e3_24_isPlayerActor = true;

  _CreateReflectionCube();
}

u32 CScriptPlayerActor::GetSuitCharIdx(const CStateManager& mgr, CPlayerState::EPlayerSuit suit) const {
  if (mgr.GetPlayerState()->IsFusionEnabled()) {
    switch (suit) {
    case CPlayerState::EPlayerSuit::Power:
      return 4;
    case CPlayerState::EPlayerSuit::Varia:
      return 7;
    case CPlayerState::EPlayerSuit::Gravity:
      return 6;
    case CPlayerState::EPlayerSuit::Phazon:
      return 8;
    default:
      break;
    }
  }
  return u32(suit);
}

u32 CScriptPlayerActor::GetNextSuitCharIdx(const CStateManager& mgr) const {
  CPlayerState::EPlayerSuit nextSuit = CPlayerState::EPlayerSuit::Phazon;

  if (x350_flags & 0x2) {
    switch (x308_suit) {
    case CPlayerState::EPlayerSuit::Gravity:
      nextSuit = CPlayerState::EPlayerSuit::Varia;
      break;
    case CPlayerState::EPlayerSuit::Phazon:
      nextSuit = CPlayerState::EPlayerSuit::Gravity;
      break;
    case CPlayerState::EPlayerSuit::Varia:
    default:
      nextSuit = CPlayerState::EPlayerSuit::Power;
      break;
    }
  } else {
    switch (x308_suit) {
    case CPlayerState::EPlayerSuit::Power:
      nextSuit = CPlayerState::EPlayerSuit::Varia;
      break;
    case CPlayerState::EPlayerSuit::Varia:
      nextSuit = CPlayerState::EPlayerSuit::Gravity;
      break;
    case CPlayerState::EPlayerSuit::Gravity:
      nextSuit = CPlayerState::EPlayerSuit::Phazon;
      break;
    default:
      break;
    }
  }

  return GetSuitCharIdx(mgr, nextSuit);
}

void CScriptPlayerActor::LoadSuit(u32 charIdx) {
  if (charIdx != x310_loadedCharIdx) {
    TToken<CCharacterFactory> fac = g_CharFactoryBuilder->GetFactory(x2e8_suitRes);
    const CCharacterInfo& chInfo = fac->GetCharInfo(charIdx);
    x320_suitModel = g_SimplePool->GetObj({FOURCC('CMDL'), chInfo.GetModelId()});
    x324_suitSkin = g_SimplePool->GetObj({FOURCC('CSKR'), chInfo.GetSkinRulesId()});
    x354_28_suitModelLoading = true;
    x310_loadedCharIdx = charIdx;
  }
}

void CScriptPlayerActor::LoadBeam(CPlayerState::EBeamId beam) {
  if (beam != x30c_setBeamId) {
    x31c_beamModel = g_SimplePool->GetObj({FOURCC('CMDL'), g_tweakPlayerRes->GetBeamCineModel(beam)});
    x354_27_beamModelLoading = true;
    x30c_setBeamId = beam;
  }
}

void CScriptPlayerActor::PumpBeamModel(CStateManager& mgr) {
  if (!x31c_beamModel || !x31c_beamModel.IsLoaded())
    return;
  BuildBeamModelData();
  x314_beamModelData->Touch(mgr, 0);
  mgr.GetWorld()->CycleLoadPauseState();
  x31c_beamModel = TLockedToken<CModel>();
  x354_27_beamModelLoading = false;
}

void CScriptPlayerActor::BuildBeamModelData() {
  x314_beamModelData = std::make_unique<CModelData>(
      CStaticRes(g_tweakPlayerRes->GetBeamCineModel(x30c_setBeamId), x2e8_suitRes.GetScale()));
}

void CScriptPlayerActor::PumpSuitModel(CStateManager& mgr) {
  if (!x320_suitModel || !x320_suitModel.IsLoaded() || !x324_suitSkin || !x324_suitSkin.IsLoaded())
    return;

  x320_suitModel->Touch(0);
  mgr.GetWorld()->CycleLoadPauseState();

  bool didSetup = false;
  if (x354_26_deferOfflineModelData) {
    didSetup = true;
    x354_26_deferOfflineModelData = false;
    SetupOfflineModelData();
  } else if (x354_25_deferOnlineModelData) {
    didSetup = true;
    x354_25_deferOnlineModelData = false;
    SetupOnlineModelData();
  }

  if (didSetup) {
    x354_28_suitModelLoading = false;
    x320_suitModel = TLockedToken<CModel>();
    x324_suitSkin = TLockedToken<CSkinRules>();
  }
}

void CScriptPlayerActor::SetupOfflineModelData() {
  x2e8_suitRes.SetCharacterNodeId(x310_loadedCharIdx);
  x318_suitModelData = std::make_unique<CModelData>(x2e8_suitRes);
  if (!static_cast<MP1::CMain&>(*g_Main).GetScreenFading()) {
    x328_backupModelData = x64_modelData->GetAnimationData()->GetModelData();
    x348_deallocateBackupCountdown = 2;
  }
  x64_modelData->GetAnimationData()->SubstituteModelData(x318_suitModelData->GetAnimationData()->GetModelData());
}

void CScriptPlayerActor::SetupOnlineModelData() {
  if (x310_loadedCharIdx != x2e8_suitRes.GetCharacterNodeId() || !x64_modelData || !x64_modelData->HasAnimData()) {
    x2e8_suitRes.SetCharacterNodeId(x310_loadedCharIdx);
    SetModelData(std::make_unique<CModelData>(x2e8_suitRes));
    CAnimPlaybackParms parms(x2e8_suitRes.GetDefaultAnim(), -1, 1.f, true);
    x64_modelData->GetAnimationData()->SetAnimation(parms, false);
    if (x354_24_setBoundingBox)
      SetBoundingBox(x64_modelData->GetBounds(GetTransform().getRotation()));
  }
}

void CScriptPlayerActor::Think(float dt, CStateManager& mgr) {
  auto& pState = *mgr.GetPlayerState();

  if (x354_31_deferOnlineLoad) {
    x354_25_deferOnlineModelData = true;
    x354_31_deferOnlineLoad = false;
    x308_suit = pState.GetCurrentSuitRaw();
    LoadSuit(GetSuitCharIdx(mgr, x308_suit));
  }

  if (x354_30_enableLoading) {
    if (!(x350_flags & 0x1)) {
      u32 tmpIdx = GetSuitCharIdx(mgr, pState.GetCurrentSuitRaw());
      if (tmpIdx != x310_loadedCharIdx) {
        SetModelData(std::make_unique<CModelData>(CModelData::CModelDataNull()));
        LoadSuit(tmpIdx);
        x354_25_deferOnlineModelData = true;
      }
    }

    LoadBeam(x304_beam != CPlayerState::EBeamId::Invalid ? x304_beam : pState.GetCurrentBeam());

    if (x354_27_beamModelLoading)
      PumpBeamModel(mgr);

    if (x354_28_suitModelLoading)
      PumpSuitModel(mgr);

    if (!x354_29_loading) {
      if (x354_28_suitModelLoading || x354_27_beamModelLoading || !x64_modelData || x64_modelData->IsNull() ||
          !x64_modelData->IsLoaded(0))
        x354_29_loading = true;
    }

    if (x354_29_loading && !x354_28_suitModelLoading && !x354_27_beamModelLoading && x64_modelData &&
        !x64_modelData->IsNull() && x64_modelData->IsLoaded(0)) {
      if (x355_24_areaTrackingLoad) {
        CGameArea* area = mgr.GetWorld()->GetArea(x4_areaId);
        --area->GetPostConstructed()->x113c_playerActorsLoading;
        x355_24_areaTrackingLoad = false;
      }
      x354_29_loading = false;
      SendScriptMsgs(EScriptObjectState::Arrived, mgr, EScriptObjectMessage::None);
    }
  }

  if (x2e8_suitRes.GetCharacterNodeId() == 3) {
    if (!x338_phazonIndirectTexture)
      x338_phazonIndirectTexture = g_SimplePool->GetObj("PhazonIndirectTexture");
  } else {
    if (x338_phazonIndirectTexture)
      x338_phazonIndirectTexture = TLockedToken<CTexture>();
  }

  if (x338_phazonIndirectTexture) {
    x34c_phazonOffsetAngle += 0.03f;
    x34c_phazonOffsetAngle = zeus::CRelAngle(x34c_phazonOffsetAngle).asRel();
  }

  CScriptActor::Think(dt, mgr);
}

void CScriptPlayerActor::SetupEnvFx(CStateManager& mgr, bool set) {
  if (set) {
    if (mgr.GetWorld()->GetNeededEnvFx() == EEnvFxType::Rain)
      if (x64_modelData && !x64_modelData->IsNull())
        if (mgr.GetEnvFxManager()->GetRainMagnitude() != 0.f)
          mgr.GetActorModelParticles()->AddRainSplashGenerator(*this, mgr, 250, 10, 1.2f);
  } else {
    mgr.GetActorModelParticles()->RemoveRainSplashGenerator(*this);
  }
}

void CScriptPlayerActor::SetIntoStateManager(CStateManager& mgr, bool set) {
  if (!set && mgr.GetPlayerActorHead() == x8_uid) {
    mgr.SetPlayerActorHead(x356_nextPlayerActor);
    x356_nextPlayerActor = kInvalidUniqueId;
  } else {
    TUniqueId paId = mgr.GetPlayerActorHead();
    CScriptPlayerActor* other = nullptr;
    while (paId != kInvalidUniqueId) {
      if (paId == x8_uid) {
        if (!set && other) {
          other->x356_nextPlayerActor = x356_nextPlayerActor;
          x356_nextPlayerActor = kInvalidUniqueId;
        }
        return;
      }

      TCastToPtr<CScriptActor> act = mgr.ObjectById(paId);
      if (act && act->IsPlayerActor()) {
        other = static_cast<CScriptPlayerActor*>(act.GetPtr());
        paId = other->x356_nextPlayerActor;
      } else {
        paId = kInvalidUniqueId;
        x356_nextPlayerActor = kInvalidUniqueId;
      }
    }

    if (set) {
      x356_nextPlayerActor = mgr.GetPlayerActorHead();
      mgr.SetPlayerActorHead(x8_uid);
    }
  }
}

void CScriptPlayerActor::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  switch (msg) {
  case EScriptObjectMessage::InitializedInArea:
    x354_31_deferOnlineLoad = true;
    if (x350_flags & 0x8) {
      CGameArea* area = mgr.GetWorld()->GetArea(x4_areaId);
      ++area->GetPostConstructed()->x113c_playerActorsLoading;
      x355_24_areaTrackingLoad = true;
    }
    if (GetActive()) {
      SetupEnvFx(mgr, true);
      SetIntoStateManager(mgr, true);
    }
    break;
  case EScriptObjectMessage::Activate:
    if (!GetActive()) {
      if (x350_flags & 0x1)
        LoadSuit(GetNextSuitCharIdx(mgr));
      SetIntoStateManager(mgr, true);
      SetupEnvFx(mgr, true);
      x354_30_enableLoading = true;
    }
    break;
  case EScriptObjectMessage::Increment:
    if (x350_flags & 0x1) {
      x354_25_deferOnlineModelData = false;
      x354_26_deferOfflineModelData = true;
      mgr.GetPlayer().AsyncLoadSuit(mgr);
    }
    break;
  case EScriptObjectMessage::Deactivate:
    if (GetActive()) {
      if (!(x350_flags & 0x10))
        SetIntoStateManager(mgr, false);
      SetupEnvFx(mgr, false);
    }
    if (!(x350_flags & 0x4))
      break;
    [[fallthrough]];
  case EScriptObjectMessage::Reset:
    if (GetActive() || msg == EScriptObjectMessage::Reset) {
      x30c_setBeamId = CPlayerState::EBeamId::Invalid;
      x310_loadedCharIdx = -1;
      x314_beamModelData.reset();
      x318_suitModelData.reset();
      x31c_beamModel = TLockedToken<CModel>();
      x320_suitModel = TLockedToken<CModel>();
      x324_suitSkin = TLockedToken<CSkinRules>();
      x328_backupModelData = TLockedToken<CSkinnedModel>();
      x338_phazonIndirectTexture = TLockedToken<CTexture>();
      x348_deallocateBackupCountdown = 0;
      x350_flags &= ~0x1;
      x354_25_deferOnlineModelData = false;
      x354_26_deferOfflineModelData = false;
      x354_27_beamModelLoading = false;
      x354_28_suitModelLoading = false;
      x354_30_enableLoading = false;
      SetModelData(std::make_unique<CModelData>(CModelData::CModelDataNull()));
    }
    break;
  case EScriptObjectMessage::Deleted:
    SetIntoStateManager(mgr, false);
    break;
  default:
    break;
  }

  CScriptActor::AcceptScriptMsg(msg, uid, mgr);
}

void CScriptPlayerActor::SetActive(bool active) {
  CActor::SetActive(active);
  xe7_29_drawEnabled = true;
}

void CScriptPlayerActor::PreRender(CStateManager& mgr, const zeus::CFrustum& frustum) {
  if (x328_backupModelData) {
    if (x348_deallocateBackupCountdown == 0)
      x328_backupModelData = TLockedToken<CSkinnedModel>();
    else
      --x348_deallocateBackupCountdown;
  }
  if (x2e8_suitRes.GetCharacterNodeId() == 3)
    g_Renderer->AllocatePhazonSuitMaskTexture();
  xb4_drawFlags.m_extendedShader = EExtendedShader::LightingCubeReflection;
  CScriptActor::PreRender(mgr, frustum);
}

void CScriptPlayerActor::AddToRenderer(const zeus::CFrustum& frustum, CStateManager& mgr) {
  TouchModels_Internal(mgr);
  if (GetActive()) {
    CActor::AddToRenderer(frustum, mgr);
  }
}

void CScriptPlayerActor::Render(CStateManager& mgr) {
  CBooModel::SetReflectionCube(m_reflectionCube);

  bool phazonSuit = x2e8_suitRes.GetCharacterNodeId() == 3;
  if (phazonSuit) {
    // Draw into alpha buffer
    CModelFlags flags = xb4_drawFlags;
    flags.x4_color = zeus::skWhite;
    flags.m_extendedShader = EExtendedShader::SolidColorBackfaceCullLEqualAlphaOnly;
    CModelData::EWhichModel which = CModelData::GetRenderingModel(mgr);
    x64_modelData->Render(which, x34_transform, x90_actorLights.get(), flags);
  }

  CPhysicsActor::Render(mgr);

  if (x314_beamModelData && !x314_beamModelData->IsNull() && x64_modelData && !x64_modelData->IsNull()) {
    zeus::CTransform modelXf = GetTransform() * x64_modelData->GetScaledLocatorTransform("GUN_LCTR");
    CModelFlags flags(5, 0, 3, zeus::skWhite);
    flags.m_extendedShader = EExtendedShader::SolidColorBackfaceCullLEqualAlphaOnly;
    x314_beamModelData->Render(mgr, modelXf, x90_actorLights.get(), flags);
    flags.m_extendedShader = EExtendedShader::LightingCubeReflection;
    flags.x4_color = zeus::CColor{1.f, xb4_drawFlags.x4_color.a()};
    x314_beamModelData->Render(mgr, modelXf, x90_actorLights.get(), flags);
  }

  if (phazonSuit) {
    zeus::CVector3f vecFromCam =
        GetBoundingBox().center() - mgr.GetCameraManager()->GetCurrentCamera(mgr)->GetTranslation();
    float radius = zeus::clamp(0.25f, (6.f - vecFromCam.magnitude()) / 6.f, 2.f);
    float offsetX = std::sin(x34c_phazonOffsetAngle);
    float offsetY = std::sin(x34c_phazonOffsetAngle) * 0.5f;
    g_Renderer->DrawPhazonSuitIndirectEffect(zeus::CColor(0.1f, 1.f), x338_phazonIndirectTexture, zeus::skWhite,
                                             radius, 0.05f, offsetX, offsetY);
  }
}

void CScriptPlayerActor::TouchModels_Internal(const CStateManager& mgr) const {
  if (x64_modelData && !x64_modelData->IsNull())
    x64_modelData->Touch(mgr, 0);

  if (x318_suitModelData && !x318_suitModelData->IsNull())
    x318_suitModelData->Touch(mgr, 0);

  if (!x354_27_beamModelLoading)
    if (x314_beamModelData && !x314_beamModelData->IsNull())
      x314_beamModelData->Touch(mgr, 0);
}

void CScriptPlayerActor::TouchModels(const CStateManager& mgr) const {
  TouchModels_Internal(mgr);
  TUniqueId paId = x356_nextPlayerActor;
  while (paId != kInvalidUniqueId) {
    TCastToConstPtr<CScriptActor> act = mgr.GetObjectById(paId);
    if (act && act->IsPlayerActor()) {
      const auto* pa = static_cast<const CScriptPlayerActor*>(act.GetPtr());
      pa->TouchModels_Internal(mgr);
      paId = pa->x356_nextPlayerActor;
    } else {
      paId = kInvalidUniqueId;
    }
  }
}
} // namespace urde
