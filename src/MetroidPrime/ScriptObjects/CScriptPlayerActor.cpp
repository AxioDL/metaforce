#include "MetroidPrime/ScriptObjects/CScriptPlayerActor.hpp"

#include "MetroidPrime/CActorLights.hpp"
#include "MetroidPrime/CActorModelParticles.hpp"
#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CAnimPlaybackParms.hpp"
#include "MetroidPrime/CEnvFxManager.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CMain.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/Factories/CCharacterFactory.hpp"
#include "MetroidPrime/Factories/CCharacterFactoryBuilder.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayerRes.hpp"

#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CModel.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "MetaRender/CCubeRenderer.hpp"

#include "dolphin/gx.h"

static const char* const kGunLocator = "GUN_LCTR";

CScriptPlayerActor::CScriptPlayerActor(TUniqueId uid, const rstl::string& name,
                                       const CEntityInfo& info, const CTransform4f& xf,
                                       const CAnimRes& animRes, const CModelData& mData,
                                       const CAABox& aabb, bool setBoundingBox,
                                       const CMaterialList& matList, float mass, float zMomentum,
                                       const CHealthInfo& hInfo, const CDamageVulnerability& dVuln,
                                       const CActorParameters& aParams, bool loop, bool active,
                                       uint flags, CPlayerState::EBeamId beam)
: CScriptActor(uid, name, info, xf, mData, aabb, matList, mass, zMomentum, hInfo, dVuln, aParams,
               loop, active, 0, 1.f, false, false, false, false)
, x2e8_suitRes(animRes)
, x304_beam(beam)
, x308_suit(CPlayerState::kPS_Invalid)
, x30c_setBeamId(CPlayerState::kBI_Invalid)
, x310_loadedCharIdx(-1)
, x314_beamModelData(nullptr)
, x318_suitModelData(nullptr)
, x31c_beamModel(nullptr)
, x320_suitModel(nullptr)
, x324_suitSkin(nullptr)
, x328_backupModelData(rstl::optional_object_null())
, x338_phazonIndirectTexture(rstl::optional_object_null())
, x348_deallocateBackupCountdown(0)
, x34c_phazonOffsetAngle(0.f)
, x350_flags(flags)
, x354_24_setBoundingBox(setBoundingBox)
, x354_25_deferOnlineModelData(false)
, x354_26_deferOfflineModelData(false)
, x354_27_beamModelLoading(false)
, x354_28_suitModelLoading(false)
, x354_29_loading(true)
, x354_30_enableLoading(true)
, x354_31_deferOnlineLoad(false)
, x355_24_areaTrackingLoad(false)
, x356_nextPlayerActor(kInvalidUniqueId) {
  CMaterialList exclude = GetMaterialFilter().GetExcludeList();
  CMaterialList include = GetMaterialFilter().GetIncludeList();
  exclude.Add(kMT_Player);
  SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(include, exclude));
  SetActorLights(aParams.GetLighting().MakeActorLights());
  SetDrawEnabled(true);
  x2e3_24_isPlayerActor = true;
}

void CScriptPlayerActor::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid,
                                         CStateManager& mgr) {
  switch (msg) {
  case kSM_InitializedInArea:
    x354_31_deferOnlineLoad = true;
    if (x350_flags & 8) {
      const TAreaId areaId = GetCurrentAreaId();
      CGameArea* area = mgr.World()->Area(areaId);
      ++area->GetPostConstructed()->x113c_playerActorsLoading;
      x355_24_areaTrackingLoad = true;
    }
    if (GetActive()) {
      SetupEnvFx(mgr, true);
      SetIntoStateManager(mgr, true);
    }
    break;
  case kSM_Activate:
    if (!GetActive()) {
      if (x350_flags & 1) {
        LoadSuit(GetNextSuitCharIdx(mgr));
      }
      SetIntoStateManager(mgr, true);
      SetupEnvFx(mgr, true);
      x354_30_enableLoading = true;
    }
    break;
  case kSM_Increment:
    if (x350_flags & 1) {
      x354_25_deferOnlineModelData = false;
      x354_26_deferOfflineModelData = true;
      mgr.Player()->AsyncLoadSuit(mgr);
    }
    break;
  case kSM_Deactivate:
    if (GetActive()) {
      if (!(x350_flags & 0x10)) {
        SetIntoStateManager(mgr, false);
      }
      SetupEnvFx(mgr, false);
    }
    if (!(x350_flags & 4)) {
      break;
    }
  case kSM_Reset:
    if (GetActive() || msg == kSM_Reset) {
      x30c_setBeamId = CPlayerState::kBI_Invalid;
      x310_loadedCharIdx = -1;
      x314_beamModelData = nullptr;
      x318_suitModelData = nullptr;
      x31c_beamModel = nullptr;
      x320_suitModel = nullptr;
      x324_suitSkin = nullptr;
      x328_backupModelData = rstl::optional_object_null();
      x338_phazonIndirectTexture = rstl::optional_object_null();
      x348_deallocateBackupCountdown = 0;
      x350_flags &= ~1;
      x354_25_deferOnlineModelData = false;
      x354_26_deferOfflineModelData = false;
      x354_27_beamModelLoading = false;
      x354_28_suitModelLoading = false;
      x354_30_enableLoading = false;
      SetModelData(CModelData::CModelDataNull());
      SetActive(false);
    }
    break;
  case kSM_Deleted:
    SetIntoStateManager(mgr, false);
    break;
  default:
    break;
  }
  CScriptActor::AcceptScriptMsg(msg, uid, mgr);
}

void CScriptPlayerActor::Think(float dt, CStateManager& mgr) {
  const CPlayerState& playerState = *mgr.GetPlayerState();
  if (x354_31_deferOnlineLoad) {
    x354_25_deferOnlineModelData = true;
    x354_31_deferOnlineLoad = false;
    x308_suit = playerState.GetCurrentSuitRaw();
    LoadSuit(GetSuitCharIdx(mgr, x308_suit));
  }
  if (x354_30_enableLoading) {
    if (!(x350_flags & 1)) {
      int charIdx = GetSuitCharIdx(mgr, playerState.GetCurrentSuitRaw());
      if (charIdx != x310_loadedCharIdx) {
        SetModelData(CModelData::CModelDataNull());
        LoadSuit(charIdx);
        x354_25_deferOnlineModelData = true;
      }
    }
    LoadBeam(x304_beam != CPlayerState::kBI_Invalid ? x304_beam : playerState.GetCurrentBeam());
    if (x354_27_beamModelLoading) {
      PumpBeamModel(mgr);
    }
    if (x354_28_suitModelLoading) {
      PumpSuitModel(mgr);
    }
    if (!x354_29_loading) {
      if (x354_28_suitModelLoading || x354_27_beamModelLoading || !HasModelData() ||
          !GetModelData()->IsLoaded(0)) {
        x354_29_loading = true;
      }
    }
    if (x354_29_loading && !x354_28_suitModelLoading && !x354_27_beamModelLoading &&
        HasModelData() && GetModelData()->IsLoaded(0)) {
      if (x355_24_areaTrackingLoad) {
        const TAreaId areaId = GetCurrentAreaId();
        --mgr.World()->Area(areaId)->GetPostConstructed()->x113c_playerActorsLoading;
        x355_24_areaTrackingLoad = false;
      }
      x354_29_loading = false;
      SendScriptMsgs(kSS_Arrived, mgr, kSM_None);
    }
  }
  if (x2e8_suitRes.GetCharacterNodeId() == 3) {
    if (!x338_phazonIndirectTexture) {
      x338_phazonIndirectTexture =
          TCachedToken< CTexture >(gpSimplePool->GetObj("PhazonIndirectTexture"));
      x338_phazonIndirectTexture->Lock();
    }
  } else if (x338_phazonIndirectTexture) {
    x338_phazonIndirectTexture = rstl::optional_object_null();
  }
  if (x338_phazonIndirectTexture) {
    x338_phazonIndirectTexture->TryCache();
    x34c_phazonOffsetAngle += 0.03f;
    x34c_phazonOffsetAngle = CMath::ClampRadians(x34c_phazonOffsetAngle);
  }
  CScriptActor::Think(dt, mgr);
}

void CScriptPlayerActor::Render(const CStateManager& mgr) const {
  const bool phazonSuit = x2e8_suitRes.GetCharacterNodeId() == 3;
  if (phazonSuit) {
    GXSetDstAlpha(GX_TRUE, 255);
  }
  CPhysicsActor::Render(mgr);
  if (HasGunModelData() && HasModelData()) {
    const CTransform4f locator =
        GetModelData()->GetScaledLocatorTransform(rstl::string_l(kGunLocator));
    const CTransform4f modelXf = GetTransform() * locator;
    x314_beamModelData->Render(mgr, modelXf, GetActorLights(),
                               CModelFlags::AlphaBlended(GetModelFlags().GetColorRef().GetAlpha())
                                   .DepthCompareUpdate(true, true));
  }
  if (phazonSuit) {
    CCubeRenderer* const renderer = gpRender;
    renderer->CopyTex(1, true, CGraphics::GetDolphinSpareBuffer(), GX_TF_RGB565, false);
    const CVector3f& delta = GetBoundingBox().GetCenterPoint() -
                             mgr.GetCameraManager()->GetCurrentCamera(mgr).GetTranslation();
    const float radius = CMath::Clamp(0.25f, (6.f - delta.Magnitude()) / 6.f, 2.f);
    renderer->DrawPhazonSuitIndirectEffect(
        CColor(0.1f, 0.1f, 0.1f, 1.f), x338_phazonIndirectTexture, radius, 0.05f,
        CMath::FastSinR(x34c_phazonOffsetAngle), 0.5f * CMath::FastSinR(x34c_phazonOffsetAngle));
  }
}

void CScriptPlayerActor::TouchModels(const CStateManager& mgr) const {
  TouchModels_Internal(mgr);
  TUniqueId id = x356_nextPlayerActor;
  while (id != kInvalidUniqueId) {
    const CScriptActor* actor = TCastToConstPtr< CScriptActor >(mgr.GetObjectById(id));
    if (actor && actor->IsPlayerActor()) {
      const CScriptPlayerActor* playerActor = static_cast< const CScriptPlayerActor* >(actor);
      playerActor->TouchModels_Internal(mgr);
      id = playerActor->x356_nextPlayerActor;
    } else {
      id = kInvalidUniqueId;
    }
  }
}

void CScriptPlayerActor::TouchModels_Internal(const CStateManager& mgr) const {
  if (HasModelData()) {
    GetModelData()->Touch(mgr, 0);
  }
  if (HasSuitModelData()) {
    x318_suitModelData->Touch(mgr, 0);
  }
  if (!x354_27_beamModelLoading && HasGunModelData()) {
    x314_beamModelData->Touch(mgr, 0);
  }
}

void CScriptPlayerActor::AddToRenderer(const CFrustumPlanes& frustum,
                                       const CStateManager& mgr) const {
  TouchModels_Internal(mgr);
  if (GetActive()) {
    CActor::AddToRenderer(frustum, mgr);
  }
}

void CScriptPlayerActor::BuildBeamModelData() {
  const CStaticRes res(gpTweakPlayerRes->GetCinematicBeamResId(x30c_setBeamId),
                       x2e8_suitRes.GetScale());
  x314_beamModelData = rs_new CModelData(res);
}

void CScriptPlayerActor::SetupOnlineModelData() {
  if (x310_loadedCharIdx == x2e8_suitRes.GetCharacterNodeId() && HasModelData() &&
      GetModelData()->HasAnimation()) {
    return;
  }
  x2e8_suitRes = CAnimRes(x2e8_suitRes.GetId(), x310_loadedCharIdx, x2e8_suitRes.GetScale(),
                          x2e8_suitRes.GetDefaultAnim(), x2e8_suitRes.CanLoop());
  CModelData modelData(x2e8_suitRes);
  SetModelData(modelData);
  const CAnimPlaybackParms parms(x2e8_suitRes.GetDefaultAnim(), -1, 1.f, true);
  AnimationData()->SetAnimation(parms, false);
  if (x354_24_setBoundingBox) {
    SetBoundingBox(GetModelData()->GetBounds(GetTransform().GetRotation()));
  }
}

void CScriptPlayerActor::SetupOfflineModelData() {
  x2e8_suitRes = CAnimRes(x2e8_suitRes.GetId(), x310_loadedCharIdx, x2e8_suitRes.GetScale(),
                          x2e8_suitRes.GetDefaultAnim(), x2e8_suitRes.CanLoop());
  x318_suitModelData = rs_new CModelData(x2e8_suitRes);
  if (!gpMain->GetScreenFading()) {
    x328_backupModelData = GetAnimationData()->GetModelData();
    x348_deallocateBackupCountdown = 2;
  }
  AnimationData()->SubstituteModelData(x318_suitModelData->GetAnimationData()->GetModelData());
}

void CScriptPlayerActor::LoadSuit(int charIdx) {
  if (charIdx == x310_loadedCharIdx) {
    return;
  }
  TLockedToken< CCharacterFactory > factory = gpCharacterFactoryBuilder->GetFactory(x2e8_suitRes);
  const CCharacterInfo& charInfo = factory->GetCharInfo(charIdx);
  x320_suitModel = rs_new TCachedToken< CModel >(
      gpSimplePool->GetObj(SObjectTag('CMDL', charInfo.GetModelId())));
  x320_suitModel->Lock();
  x324_suitSkin = rs_new TToken< CSkinRules >(
      gpSimplePool->GetObj(SObjectTag('CSKR', charInfo.GetSkinRulesId())));
  x324_suitSkin->Lock();
  x354_28_suitModelLoading = true;
  x310_loadedCharIdx = charIdx;
}

void CScriptPlayerActor::LoadBeam(CPlayerState::EBeamId beam) {
  if (beam == x30c_setBeamId) {
    return;
  }
  CAssetId id = gpTweakPlayerRes->GetCinematicBeamResId(beam);
  x31c_beamModel = rs_new TToken< CModel >(gpSimplePool->GetObj(SObjectTag('CMDL', id)));
  x31c_beamModel->Lock();
  x354_27_beamModelLoading = true;
  x30c_setBeamId = beam;
}

void CScriptPlayerActor::PumpSuitModel(CStateManager& mgr) {
  if (!x320_suitModel.null() && x320_suitModel->TryCache() && x324_suitSkin->IsLoaded()) {
    if (x320_suitModel->IsLoaded()) {
      x320_suitModel->GetObject()->Touch(0);
      mgr.World()->CyclePauseState();
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
        x320_suitModel = nullptr;
        x324_suitSkin = nullptr;
      }
    }
  }
}

void CScriptPlayerActor::PumpBeamModel(CStateManager& mgr) {
  if (!x31c_beamModel.null() && x31c_beamModel->IsLoaded()) {
    BuildBeamModelData();
    x314_beamModelData->Touch(mgr, 0);
    mgr.World()->CyclePauseState();
    x31c_beamModel = nullptr;
    x354_27_beamModelLoading = false;
  }
}

int CScriptPlayerActor::GetNextSuitCharIdx(const CStateManager& mgr) const {
  CPlayerState::EPlayerSuit nextSuit = CPlayerState::kPS_Phazon;
  if (x350_flags & 2) {
    switch (x308_suit) {
    case CPlayerState::kPS_Gravity:
      nextSuit = CPlayerState::kPS_Varia;
      break;
    case CPlayerState::kPS_Phazon:
      nextSuit = CPlayerState::kPS_Gravity;
      break;
    default:
      nextSuit = CPlayerState::kPS_Power;
      break;
    }
  } else {
    switch (x308_suit) {
    case CPlayerState::kPS_Power:
      nextSuit = CPlayerState::kPS_Varia;
      break;
    case CPlayerState::kPS_Varia:
      nextSuit = CPlayerState::kPS_Gravity;
      break;
    case CPlayerState::kPS_Gravity:
      nextSuit = CPlayerState::kPS_Phazon;
      break;
    default:
      break;
    }
  }
  return GetSuitCharIdx(mgr, nextSuit);
}

int CScriptPlayerActor::GetSuitCharIdx(const CStateManager& mgr,
                                       CPlayerState::EPlayerSuit suit) const {
  int charIdx = suit;
  if (mgr.GetPlayerState()->GetIsFusionEnabled()) {
    switch (suit) {
    case CPlayerState::kPS_Power:
      charIdx = 4;
      break;
    case CPlayerState::kPS_Varia:
      charIdx = 7;
      break;
    case CPlayerState::kPS_Gravity:
      charIdx = 6;
      break;
    case CPlayerState::kPS_Phazon:
      charIdx = 8;
      break;
    default:
      break;
    }
  }
  return charIdx;
}

void CScriptPlayerActor::PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) {
  if (x328_backupModelData) {
    if (x348_deallocateBackupCountdown == 0) {
      x328_backupModelData = rstl::optional_object_null();
    } else {
      --x348_deallocateBackupCountdown;
    }
  }
  if (x2e8_suitRes.GetCharacterNodeId() == 3) {
    gpRender->AllocatePhazonSuitMaskTexture();
  }
  CScriptActor::PreRender(mgr, frustum);
}

void CScriptPlayerActor::SetActive(const bool active) {
  CActor::SetActive(active);
  SetDrawEnabled(true);
}

void CScriptPlayerActor::SetupEnvFx(CStateManager& mgr, bool set) {
  if (set) {
    const CEnvFxManager* envFx = mgr.GetEnvFxManager();
    if (mgr.GetWorld()->GetNeededEnvFx() == kEFX_Rain && HasModelData() &&
        envFx->GetRainMagnitude()) {
      mgr.ActorModelParticles()->AddRainSplashGenerator(*this, mgr, 250, 10, 1.2f);
    }
  } else {
    mgr.ActorModelParticles()->RemoveRainSplashGenerator(*this);
  }
}

void CScriptPlayerActor::SetIntoStateManager(CStateManager& mgr, bool set) {
  const TUniqueId selfId = GetUniqueId();
  if (!set && mgr.GetPlayerActorHead() == selfId) {
    mgr.SetPlayerActorHead(GetNextPlayerActor());
    SetNextPlayerActor(kInvalidUniqueId);
  } else {
    TUniqueId id = mgr.GetPlayerActorHead();
    CScriptPlayerActor* previous = nullptr;
    while (id != kInvalidUniqueId) {
      if (id == selfId) {
        if (!set && previous) {
          previous->SetNextPlayerActor(GetNextPlayerActor());
          SetNextPlayerActor(kInvalidUniqueId);
        }
        return;
      }
      CScriptActor* const actor = TCastToPtr< CScriptActor >(mgr.ObjectById(id));
      if (actor && actor->IsPlayerActor()) {
        previous = static_cast< CScriptPlayerActor* >(actor);
        id = previous->x356_nextPlayerActor;
      } else {
        id = kInvalidUniqueId;
        SetNextPlayerActor(kInvalidUniqueId);
      }
    }
    if (set) {
      x356_nextPlayerActor = mgr.GetPlayerActorHead();
      mgr.SetPlayerActorHead(selfId);
    }
  }
}

CScriptPlayerActor::~CScriptPlayerActor() {}
