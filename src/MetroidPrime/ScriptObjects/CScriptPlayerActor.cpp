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
, mSuitRes(animRes)
, mBeam(beam)
, mSuit(CPlayerState::kPS_Invalid)
, mSetBeamId(CPlayerState::kBI_Invalid)
, mLoadedCharIdx(-1)
, mBeamModelData(nullptr)
, mSuitModelData(nullptr)
, mBeamModel(nullptr)
, mSuitModel(nullptr)
, mSuitSkin(nullptr)
, mBackupModelData(rstl::optional_object_null())
, mPhazonIndirectTexture(rstl::optional_object_null())
, mDeallocateBackupCountdown(0)
, mPhazonOffsetAngle(0.f)
, mFlags(flags)
, mSetBoundingBox(setBoundingBox)
, mDeferOnlineModelData(false)
, mDeferOfflineModelData(false)
, mBeamModelLoading(false)
, mSuitModelLoading(false)
, mLoading(true)
, mEnableLoading(true)
, mDeferOnlineLoad(false)
, mAreaTrackingLoad(false)
, mNextPlayerActor(kInvalidUniqueId) {
  CMaterialList exclude = GetMaterialFilter().GetExcludeList();
  CMaterialList include = GetMaterialFilter().GetIncludeList();
  exclude.Add(kMT_Player);
  SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(include, exclude));
  SetActorLights(aParams.GetLighting().MakeActorLights());
  SetDrawEnabled(true);
  mIsPlayerActor = true;
}

void CScriptPlayerActor::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid,
                                         CStateManager& mgr) {
  switch (msg) {
  case kSM_InitializedInArea:
    mDeferOnlineLoad = true;
    if (mFlags & 8) {
      const TAreaId areaId = GetCurrentAreaId();
      CGameArea* area = mgr.World()->Area(areaId);
      ++area->GetPostConstructed()->mPlayerActorsLoading;
      mAreaTrackingLoad = true;
    }
    if (GetActive()) {
      SetupEnvFx(mgr, true);
      SetIntoStateManager(mgr, true);
    }
    break;
  case kSM_Activate:
    if (!GetActive()) {
      if (mFlags & 1) {
        LoadSuit(GetNextSuitCharIdx(mgr));
      }
      SetIntoStateManager(mgr, true);
      SetupEnvFx(mgr, true);
      mEnableLoading = true;
    }
    break;
  case kSM_Increment:
    if (mFlags & 1) {
      mDeferOnlineModelData = false;
      mDeferOfflineModelData = true;
      mgr.Player()->AsyncLoadSuit(mgr);
    }
    break;
  case kSM_Deactivate:
    if (GetActive()) {
      if (!(mFlags & 0x10)) {
        SetIntoStateManager(mgr, false);
      }
      SetupEnvFx(mgr, false);
    }
    if (!(mFlags & 4)) {
      break;
    }
  case kSM_Reset:
    if (GetActive() || msg == kSM_Reset) {
      mSetBeamId = CPlayerState::kBI_Invalid;
      mLoadedCharIdx = -1;
      mBeamModelData = nullptr;
      mSuitModelData = nullptr;
      mBeamModel = nullptr;
      mSuitModel = nullptr;
      mSuitSkin = nullptr;
      mBackupModelData = rstl::optional_object_null();
      mPhazonIndirectTexture = rstl::optional_object_null();
      mDeallocateBackupCountdown = 0;
      mFlags &= ~1;
      mDeferOnlineModelData = false;
      mDeferOfflineModelData = false;
      mBeamModelLoading = false;
      mSuitModelLoading = false;
      mEnableLoading = false;
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
  if (mDeferOnlineLoad) {
    mDeferOnlineModelData = true;
    mDeferOnlineLoad = false;
    mSuit = playerState.GetCurrentSuitRaw();
    LoadSuit(GetSuitCharIdx(mgr, mSuit));
  }
  if (mEnableLoading) {
    if (!(mFlags & 1)) {
      int charIdx = GetSuitCharIdx(mgr, playerState.GetCurrentSuitRaw());
      if (charIdx != mLoadedCharIdx) {
        SetModelData(CModelData::CModelDataNull());
        LoadSuit(charIdx);
        mDeferOnlineModelData = true;
      }
    }
    LoadBeam(mBeam != CPlayerState::kBI_Invalid ? mBeam : playerState.GetCurrentBeam());
    if (mBeamModelLoading) {
      PumpBeamModel(mgr);
    }
    if (mSuitModelLoading) {
      PumpSuitModel(mgr);
    }
    if (!mLoading) {
      if (mSuitModelLoading || mBeamModelLoading || !HasModelData() ||
          !GetModelData()->IsLoaded(0)) {
        mLoading = true;
      }
    }
    if (mLoading && !mSuitModelLoading && !mBeamModelLoading &&
        HasModelData() && GetModelData()->IsLoaded(0)) {
      if (mAreaTrackingLoad) {
        const TAreaId areaId = GetCurrentAreaId();
        --mgr.World()->Area(areaId)->GetPostConstructed()->mPlayerActorsLoading;
        mAreaTrackingLoad = false;
      }
      mLoading = false;
      SendScriptMsgs(kSS_Arrived, mgr, kSM_None);
    }
  }
  if (mSuitRes.GetCharacterNodeId() == 3) {
    if (!mPhazonIndirectTexture) {
      mPhazonIndirectTexture =
          TCachedToken< CTexture >(gpSimplePool->GetObj("PhazonIndirectTexture"));
      mPhazonIndirectTexture->Lock();
    }
  } else if (mPhazonIndirectTexture) {
    mPhazonIndirectTexture = rstl::optional_object_null();
  }
  if (mPhazonIndirectTexture) {
    mPhazonIndirectTexture->TryCache();
    mPhazonOffsetAngle += 0.03f;
    mPhazonOffsetAngle = CMath::ClampRadians(mPhazonOffsetAngle);
  }
  CScriptActor::Think(dt, mgr);
}

void CScriptPlayerActor::Render(const CStateManager& mgr) const {
  const bool phazonSuit = mSuitRes.GetCharacterNodeId() == 3;
  if (phazonSuit) {
    GXSetDstAlpha(GX_TRUE, 255);
  }
  CPhysicsActor::Render(mgr);
  if (HasGunModelData() && HasModelData()) {
    const CTransform4f locator =
        GetModelData()->GetScaledLocatorTransform(rstl::string_l(kGunLocator));
    const CTransform4f modelXf = GetTransform() * locator;
    mBeamModelData->Render(mgr, modelXf, GetActorLights(),
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
        CColor(0.1f, 0.1f, 0.1f, 1.f), mPhazonIndirectTexture, radius, 0.05f,
        CMath::FastSinR(mPhazonOffsetAngle), 0.5f * CMath::FastSinR(mPhazonOffsetAngle));
  }
}

void CScriptPlayerActor::TouchModels(const CStateManager& mgr) const {
  TouchModels_Internal(mgr);
  TUniqueId id = mNextPlayerActor;
  while (id != kInvalidUniqueId) {
    const CScriptActor* actor = TCastToConstPtr< CScriptActor >(mgr.GetObjectById(id));
    if (actor && actor->IsPlayerActor()) {
      const CScriptPlayerActor* playerActor = static_cast< const CScriptPlayerActor* >(actor);
      playerActor->TouchModels_Internal(mgr);
      id = playerActor->mNextPlayerActor;
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
    mSuitModelData->Touch(mgr, 0);
  }
  if (!mBeamModelLoading && HasGunModelData()) {
    mBeamModelData->Touch(mgr, 0);
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
  const CStaticRes res(gpTweakPlayerRes->GetCinematicBeamResId(mSetBeamId),
                       mSuitRes.GetScale());
  mBeamModelData = rs_new CModelData(res);
}

void CScriptPlayerActor::SetupOnlineModelData() {
  if (mLoadedCharIdx == mSuitRes.GetCharacterNodeId() && HasModelData() &&
      GetModelData()->HasAnimation()) {
    return;
  }
  mSuitRes = CAnimRes(mSuitRes.GetId(), mLoadedCharIdx, mSuitRes.GetScale(),
                          mSuitRes.GetDefaultAnim(), mSuitRes.CanLoop());
  CModelData modelData(mSuitRes);
  SetModelData(modelData);
  const CAnimPlaybackParms parms(mSuitRes.GetDefaultAnim(), -1, 1.f, true);
  AnimationData()->SetAnimation(parms, false);
  if (mSetBoundingBox) {
    SetBoundingBox(GetModelData()->GetBounds(GetTransform().GetRotation()));
  }
}

void CScriptPlayerActor::SetupOfflineModelData() {
  mSuitRes = CAnimRes(mSuitRes.GetId(), mLoadedCharIdx, mSuitRes.GetScale(),
                          mSuitRes.GetDefaultAnim(), mSuitRes.CanLoop());
  mSuitModelData = rs_new CModelData(mSuitRes);
  if (!gpMain->GetScreenFading()) {
    mBackupModelData = GetAnimationData()->GetModelData();
    mDeallocateBackupCountdown = 2;
  }
  AnimationData()->SubstituteModelData(mSuitModelData->GetAnimationData()->GetModelData());
}

void CScriptPlayerActor::LoadSuit(int charIdx) {
  if (charIdx == mLoadedCharIdx) {
    return;
  }
  TLockedToken< CCharacterFactory > factory = gpCharacterFactoryBuilder->GetFactory(mSuitRes);
  const CCharacterInfo& charInfo = factory->GetCharInfo(charIdx);
  mSuitModel = rs_new TCachedToken< CModel >(
      gpSimplePool->GetObj(SObjectTag('CMDL', charInfo.GetModelId())));
  mSuitModel->Lock();
  mSuitSkin = rs_new TToken< CSkinRules >(
      gpSimplePool->GetObj(SObjectTag('CSKR', charInfo.GetSkinRulesId())));
  mSuitSkin->Lock();
  mSuitModelLoading = true;
  mLoadedCharIdx = charIdx;
}

void CScriptPlayerActor::LoadBeam(CPlayerState::EBeamId beam) {
  if (beam == mSetBeamId) {
    return;
  }
  CAssetId id = gpTweakPlayerRes->GetCinematicBeamResId(beam);
  mBeamModel = rs_new TToken< CModel >(gpSimplePool->GetObj(SObjectTag('CMDL', id)));
  mBeamModel->Lock();
  mBeamModelLoading = true;
  mSetBeamId = beam;
}

void CScriptPlayerActor::PumpSuitModel(CStateManager& mgr) {
  if (!mSuitModel.null() && mSuitModel->TryCache() && mSuitSkin->IsLoaded()) {
    if (mSuitModel->IsLoaded()) {
      mSuitModel->GetObject()->Touch(0);
      mgr.World()->CyclePauseState();
      bool didSetup = false;
      if (mDeferOfflineModelData) {
        didSetup = true;
        mDeferOfflineModelData = false;
        SetupOfflineModelData();
      } else if (mDeferOnlineModelData) {
        didSetup = true;
        mDeferOnlineModelData = false;
        SetupOnlineModelData();
      }
      if (didSetup) {
        mSuitModelLoading = false;
        mSuitModel = nullptr;
        mSuitSkin = nullptr;
      }
    }
  }
}

void CScriptPlayerActor::PumpBeamModel(CStateManager& mgr) {
  if (!mBeamModel.null() && mBeamModel->IsLoaded()) {
    BuildBeamModelData();
    mBeamModelData->Touch(mgr, 0);
    mgr.World()->CyclePauseState();
    mBeamModel = nullptr;
    mBeamModelLoading = false;
  }
}

int CScriptPlayerActor::GetNextSuitCharIdx(const CStateManager& mgr) const {
  CPlayerState::EPlayerSuit nextSuit = CPlayerState::kPS_Phazon;
  if (mFlags & 2) {
    switch (mSuit) {
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
    switch (mSuit) {
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
  if (mBackupModelData) {
    if (mDeallocateBackupCountdown == 0) {
      mBackupModelData = rstl::optional_object_null();
    } else {
      --mDeallocateBackupCountdown;
    }
  }
  if (mSuitRes.GetCharacterNodeId() == 3) {
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
        id = previous->mNextPlayerActor;
      } else {
        id = kInvalidUniqueId;
        SetNextPlayerActor(kInvalidUniqueId);
      }
    }
    if (set) {
      mNextPlayerActor = mgr.GetPlayerActorHead();
      mgr.SetPlayerActorHead(selfId);
    }
  }
}

CScriptPlayerActor::~CScriptPlayerActor() {}
