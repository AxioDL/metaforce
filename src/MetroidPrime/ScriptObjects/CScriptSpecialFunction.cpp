#include "MetroidPrime/ScriptObjects/CScriptSpecialFunction.hpp"

#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CEnvFxManager.hpp"
#include "MetroidPrime/CMain.hpp"
#include "MetroidPrime/CMapWorld.hpp"
#include "MetroidPrime/CMapWorldInfo.hpp"
#include "MetroidPrime/CRumbleManager.hpp"
#include "MetroidPrime/CScriptLayerManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerGun.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/Player/CWorldState.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPlatform.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CEnergyProjectile.hpp"

#include "MetaRender/CCubeRenderer.hpp"

#include "Collision/CMaterialFilter.hpp"

#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Math/CFrustumPlanes.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"
#include "Kyoto/Math/CVector2i.hpp"
#include "Kyoto/Math/CloseEnough.hpp"

#include "rstl/algorithm.hpp"
#include "rstl/iterator.hpp"
#include "rstl/math.hpp"

#include "math.h"
#include "rstl/optional_object.hpp"

CScriptSpecialFunction::CScriptSpecialFunction(
    TUniqueId uid, const rstl::string& name, const CEntityInfo& info, const CTransform4f& xf,
    ESpecialFunction func, const rstl::string& lcName, float f1, float f2, float f3, float f4,
    const CVector3f& vec, const CColor& col, const bool active, const CDamageInfo& dInfo, int aId1,
    int aId2, CPlayerState::EItemType itemType, ushort sId1, ushort sId2, ushort sId3)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), CMaterialList(),
         CActorParameters::None(), kInvalidUniqueId)
, mFunction(func)
, mLocatorName(lcName)
, mFloat1(f1)
, mFloat2(f2)
, mFloat3(f3)
, mFloat4(f4)
, mVector3f(vec)
, mColor(col)
, mDamageInfo(dInfo)
, mSpinnerInitialXf(CTransform4f::Identity())
, x138_(0.f)
, x16c_(0.f)
, mSfx1(CSfxManager::TranslateSFXID(sId1))
, mSfx2(CSfxManager::TranslateSFXID(sId2))
, mSfx3(CSfxManager::TranslateSFXID(sId3))
, x180_(0.f)
, x184_(6, 0.f)
, x194_(0.f)
, mRingState(kRS_Stopped)
, mRingRotateTarget(CVector3f::Zero())
, mRingReverse(true)
, mAreaSaveId(aId1)
, mLayerIdx(aId2)
, mItem(itemType)
, mSpinnerInitializedXf(false)
, mSpinnerCanMove(false)
, mSfx2Played(true)
, mSfx3Played(false)
// , x1e4_28_frustumEntered(false)
// , x1e4_29_frustumExited(false)
// , x1e4_30_(false)
, mInAreaDamage(false)
, mDoSave(false)
, mPlayerInArea(false)
, mDisplayBillboard(false) {
  if (mFunction == kSF_HUDTarget) {
    mTouchBounds = CAABox(CVector3f(-1.f, -1.f, -1.f), CVector3f(1.f, 1.f, 1.f));
  }
}

void CScriptSpecialFunction::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  switch (mFunction) {
  case kSF_PlayerFollowLocator:
    ThinkPlayerFollowLocator(dt, mgr);
    break;
  case kSF_SpinnerController:
    ThinkSpinnerController(dt, mgr, kSCM_Zero);
    break;
  case kSF_ShotSpinnerController:
    ThinkSpinnerController(dt, mgr, kSCM_One);
    break;
  case kSF_ObjectFollowLocator:
    ThinkObjectFollowLocator(dt, mgr);
    break;
  case kSF_ObjectFollowObject:
    ThinkObjectFollowObject(dt, mgr);
    break;
  case kSF_ChaffTarget:
    ThinkChaffTarget(dt, mgr);
    break;
  case kSF_ViewFrustumTester: {
    if (mFrustumEntered) {
      mFrustumEntered = false;
      SendScriptMsgs(kSS_Entered, mgr, kSM_None);
    }
    if (mFrustumExited) {
      mFrustumExited = false;
      SendScriptMsgs(kSS_Exited, mgr, kSM_None);
    }
    break;
  }
  case kSF_SaveStation:
    ThinkSaveStation(dt, mgr);
    break;
  case kSF_IntroBossRingController:
    ThinkIntroBossRingController(dt, mgr);
    break;
  case kSF_RainSimulator:
    ThinkRainSimulator(dt, mgr);
    break;
  case kSF_AreaDamage:
    ThinkAreaDamage(dt, mgr);
    break;
  case kSF_ScaleActor:
    ThinkActorScale(dt, mgr);
    break;
  case kSF_PlayerInAreaRelay:
    ThinkPlayerInArea(dt, mgr);
    break;
  case kSF_Billboard: {
    if (x1e8_.valid() && x1e8_.data().IsLoaded() && mDisplayBillboard) {
      SendScriptMsgs(kSS_MaxReached, mgr, kSM_None);
      mDisplayBillboard = false;
    }
    break;
  }
  default:
    break;
  }
}

void CScriptSpecialFunction::AddToRenderer(const CFrustumPlanes&, const CStateManager& mgr) const {
  if (!GetActive()) {
    return;
  }

  if (mFunction == kSF_FogVolume && x1e4_30_) {
    EnsureRendered(mgr);
  }
}

void CScriptSpecialFunction::PreRender(CStateManager&, const CFrustumPlanes& frustum) {
  switch (mFunction) {
  case kSF_FogVolume:
  case kSF_ViewFrustumTester: {
    if (!GetActive()) {
      break;
    }

    bool val;
    if (mFunction == kSF_FogVolume) {
      CVector3f pos = GetTranslation();
      CVector3f max = pos + mVector3f;
      max[kDZ] += mFloat1;
      CAABox aabb(pos - mVector3f, max);
      val = frustum.BoxInFrustumPlanes(aabb);
    } else {
      val = frustum.PointInFrustumPlanes(GetTranslation());
    }

    if (x1e4_30_ == val) {
      break;
    }
    if (val) {
      mFrustumEntered = true;
    } else {
      mFrustumExited = true;
    }
    x1e4_30_ = val;
    break;
  }
  default:
    break;
  }
}

static const ERumbleFxId skRumbleFxList[6] = {
    kRFX_Twenty, kRFX_One, kRFX_TwentyOne, kRFX_TwentyTwo, kRFX_TwentyThree, kRFX_Zero,
};

namespace {
class CRingSorter {
  const CStateManager& mgr;

public:
  CRingSorter(const CStateManager& mgr) : mgr(mgr) {}

  bool operator()(const CScriptSpecialFunction::SRingController& a,
                  const CScriptSpecialFunction::SRingController& b) const {
    const CActor* actA = TCastToConstPtr< CActor >(mgr.GetObjectById(a.mId));
    const CActor* actB = TCastToConstPtr< CActor >(mgr.GetObjectById(b.mId));
    if (actA && actB) {
      return actA->GetTranslation().GetZ() < actB->GetTranslation().GetZ();
    }
    return false;
  }
};
} // namespace

void CScriptSpecialFunction::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid,
                                             CStateManager& mgr) {
  if (GetActive() && msg == kSM_Deactivate && mFunction == kSF_Billboard) {
    mgr.SetPendingOnScreenTex(kInvalidAssetId, CVector2i(0, 0), CVector2i(0, 0));
    x1e8_ = rstl::optional_object_null();
    mDisplayBillboard = false;
  }
  CActor::AcceptScriptMsg(msg, uid, mgr);

  if (mFunction == kSF_ChaffTarget && msg == kSM_InitializedInArea) {
    AddMaterial(kMT_Target, mgr);
  }

  if (GetActive()) {
    switch (mFunction) {
    case kSF_HUDFadeIn: {
      if (msg == kSM_Action) {
        mgr.Player()->SetHudDisable(mFloat1, 0.f, 0.5f);
      }
      break;
    }
    case kSF_EscapeSequence: {
      if (msg == kSM_Action && mFloat1 >= 0.f) {
        mgr.ResetEscapeSequenceTimer(mFloat1);
      }
      break;
    }
    case kSF_SpinnerController: {
      switch (msg) {
      case kSM_Play: {
        mSpinnerCanMove = true;
        mgr.Player()->SetAngularVelocityWR(CAxisAngle::Identity());
        break;
      }
      case kSM_Stop: {
        mSpinnerCanMove = false;
        break;
      }
      case kSM_Deactivate:
        DeleteEmitter(mSfxHandle);
        break;
      default:
        break;
      }
      break;
    }
    case kSF_ShotSpinnerController: {
      if (msg == kSM_Increment) {
        x16c_ = rstl::max_val(0.f, rstl::min_val(x16c_ + 1.f, 1.f));
        SendScriptMsgs(kSS_Play, mgr, kSM_None);
      } else if (msg == kSM_SetToMax) {
        x16c_ = mFloat3;
        SendScriptMsgs(kSS_Play, mgr, kSM_None);
      } else if (msg == kSM_SetToZero) {
        x16c_ = -0.5f * mFloat3;
      }
      break;
    }
    case kSF_MapStation: {
      if (msg == kSM_Action) {
        mgr.MapWorldInfo()->SetIsMapped(true);
        CMapWorld* mapWorld = mgr.GetWorld()->GetMapWorld();
        mapWorld->RecalculateWorldSphere(*mgr.MapWorldInfo(), *mgr.GetWorld());
        mgr.EnterMapScreen();
      }
      break;
    }
    case kSF_MissileStation: {
      if (msg == kSM_Action) {
        CPlayerState& pState = *mgr.PlayerState();
        pState.SetPickup(CPlayerState::kIT_Missiles,
                         pState.GetItemCapacity(CPlayerState::kIT_Missiles));
      }
      break;
    }
    case kSF_PowerBombStation: {
      if (msg == kSM_Action) {
        CPlayerState& pState = *mgr.PlayerState();
        pState.SetPickup(CPlayerState::kIT_PowerBombs,
                         pState.GetItemCapacity(CPlayerState::kIT_PowerBombs));
      }
      break;
    }
    case kSF_SaveStation: {
      if (msg == kSM_Action) {
        const bool noCard = gpGameState->CardSerial() == 0;
        mgr.PlayerState()->IncrPickUp(CPlayerState::kIT_EnergyTanks, 1);
        if (noCard) {
          SendScriptMsgs(kSS_Closed, mgr, kSM_None);
        } else if (!noCard) {
          mgr.EnterSaveGameScreen();
          mDoSave = true;
        }
      }
      break;
    }
    case kSF_IntroBossRingController: {
      if (mRingState != kRS_Breakup) {
        switch (msg) {
        case kSM_InitializedInArea: {
          mRingControllers.reserve(3);

          for (rstl::vector< SConnection >::const_iterator conn = GetConnectionList().begin();
               conn != GetConnectionList().end(); ++conn) {
            if (conn->mState != kSS_Play || conn->mMsg != kSM_Activate) {
              continue;
            }

            const CStateManager::TIdListResult it = mgr.GetIdListForScript(conn->mObjId);
            AUTO(id, it.first);
            if (!(id == it.second)) {
              if (CActor* const act = TCastToPtr< CActor >(mgr.ObjectById(id->second))) {
                mRingControllers.reserve(mRingControllers.size() + 1);
                mRingControllers.push_back(SRingController(id->second, 0.f, false));
                act->RemoveMaterial(kMT_Occluder, mgr);
              }
            }
          }

          rstl::sort(mRingControllers.begin(), mRingControllers.end(), CRingSorter(mgr));

          for (int i = 0; i < mRingControllers.size(); ++i) {
            mRingControllers[i].mRotateSpeed = mFloat1 * (mRingReverse ? 1.f : -1.f);
            mRingControllers[i].mReachedTarget = false;
          }
          break;
        }
        case kSM_Play: {
          if (mRingState != kRS_Scramble) {
            RingScramble(mgr);
          }

          for (int i = 0; i < mRingControllers.size(); ++i) {
            if (CActor* const act =
                    TCastToPtr< CActor >(mgr.ObjectById(mRingControllers[i].mId))) {
              mRingControllers[i].xc_ = act->GetTransform().GetForward();
            } else {
              mRingControllers[i].xc_ = CVector3f::Forward();
            }
          }

          mRingState = kRS_Breakup;
          break;
        }
        case kSM_Action: {
          RingScramble(mgr);
          break;
        }
        case kSM_SetToZero: {
          mRingState = kRS_Rotate;
          mRingRotateTarget = GetTranslation() - mgr.GetPlayer()->GetTranslation();
          mRingRotateTarget.SetZ(0.f);
          mRingRotateTarget.Normalize();
          break;
        }
        default:
          break;
        }
      }
      break;
    }
    case kSF_RadialDamage: {
      if (msg == kSM_Action) {
        CDamageInfo dInfo = mDamageInfo;
        dInfo.SetRadius(mFloat1);
        mgr.ApplyDamageToWorld(
            GetUniqueId(), *this, GetTranslation(), dInfo,
            CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList(0)));
      }
      break;
    }
    case kSF_BossEnergyBar: {
      if (msg == kSM_Increment) {
        const int stringIdx = static_cast< int >(mFloat2) + 86;
        mgr.SetBossParams(uid, mFloat1, stringIdx);
      } else if (msg == kSM_Decrement) {
        mgr.SetBossParams(kInvalidUniqueId, 0.f, 0);
      }
      break;
    }
    case kSF_EndGame: {
      if (msg == kSM_Action) {
        switch (GetSpecialEnding(mgr)) {
        case 0:
          gpMain->SetRestartMode(CMain::kRM_WinBad);
          break;
        case 1:
          gpMain->SetRestartMode(CMain::kRM_WinGood);
          break;
        case 2:
          gpMain->SetRestartMode(CMain::kRM_WinBest);
          break;
        }
        mgr.QuitGame();
      }
      break;
    }
    case kSF_CinematicSkip: {
      if (msg == kSM_Increment) {
        if (ShouldSkipCinematic(mgr)) {
          mgr.SetCinematicSkipObject(GetUniqueId());
        }
      } else if (msg == kSM_Decrement) {
        mgr.SetCinematicSkipObject(kInvalidUniqueId);
        gpGameState->SystemState().SetCinematicState(
            rstl::pair< CAssetId, TEditorId >(mgr.GetWorld()->GetWorldAssetId(), GetEditorId()),
            true);
      }
      break;
    }
    case kSF_ScriptLayerController: {
      switch (msg) {
      case kSM_Decrement:
      case kSM_Increment: {
        if (mAreaSaveId != -1u && mLayerIdx != -1u) {
          bool active = msg == kSM_Increment;
          TAreaId aId = mgr.GetWorld()->GetAreaIdForSaveId(mAreaSaveId);
          CScriptLayerManager* worldLayerState = nullptr;

          if (aId.Value() != -1) {
            worldLayerState = mgr.WorldLayerState().GetPtr();
          } else {
            const rstl::pair< CAssetId, int > worldAreaPair =
                gpMemoryCard->GetAreaAndWorldIdForSaveId(mAreaSaveId);

            if (worldAreaPair.first != kInvalidAssetId) {
              worldLayerState =
                  gpGameState->StateForWorld(worldAreaPair.first).GetLayerState().GetPtr();
              aId = worldAreaPair.second;
            }
          }

          if (aId.Value() != -1) {
            worldLayerState->SetLayerActive(aId, TLayerId(mLayerIdx), active);
          }
        }
      } break;
      }
      // Fall through to the density controller.
    }
    case kSF_EnvFxDensityController: {
      if (msg == kSM_Action) {
        mgr.EnvFxManager()->SetFxDensity(int(mFloat2), mFloat1);
      }
      break;
    }
    case kSF_RumbleEffect: {
      if (msg != kSM_Action) {
        break;
      }
      int rumbFxIdx = int(mFloat2);
      if (rumbFxIdx < 0 || rumbFxIdx >= int(sizeof(skRumbleFxList) / sizeof(ERumbleFxId))) {
        break;
      }
      ERumbleFxId rumbFx = skRumbleFxList[rumbFxIdx];
      uint param3 = mFloat3;
      if ((param3 & 1) != 0) {
        mgr.GetRumbleManager()->Rumble(mgr, rumbFx, 1.f, kRP_One);
      } else {
        CVector3f pos = GetTranslation();
        if ((param3 & 2) != 0) {
          if (const CActor* const act = TCastToConstPtr< CActor >(mgr.GetObjectById(uid))) {
            pos = act->GetTranslation();
          }
        }
        mgr.GetRumbleManager()->Rumble(mgr, pos, rumbFx, mFloat1, kRP_One);
      }
      break;
    }
    case kSF_InventoryActivator: {
      if (msg == kSM_Action && mgr.GetPlayerState()->HasPowerUp(mItem)) {
        SendScriptMsgs(kSS_Zero, mgr, kSM_None);
      }
      break;
    }
    case kSF_FusionRelay: {
      if (msg == kSM_Action && mgr.GetPlayerState()->GetIsFusionEnabled()) {
        SendScriptMsgs(kSS_Zero, mgr, kSM_None);
      }
      break;
    }
    case kSF_AreaDamage: {
      if ((msg == kSM_Deleted || msg == kSM_Deactivate) && mInAreaDamage) {
        mInAreaDamage = false;
        mgr.Player()->DecrementEnvironmentDamage();
        mgr.SetIsFullThreat(false);
      }
      break;
    }
    case kSF_DropBomb: {
      if (msg == kSM_Action) {
        if (mFloat1 >= 1.f) {
          mgr.Player()->PlayerGun()->DropBomb(CPlayerGun::kBW_PowerBomb, mgr);
        } else {
          mgr.Player()->PlayerGun()->DropBomb(CPlayerGun::kBW_Bomb, mgr);
        }
      }
      break;
    }
    case kSF_RedundantHintSystem: {
      CHintOptions& hintOptions = gpGameState->HintOptions();
      if (msg == kSM_Action) {
        hintOptions.ActivateContinueDelayHintTimer(mLocatorName);
      } else if (msg == kSM_Increment) {
        hintOptions.ActivateImmediateHintTimer(mLocatorName);
      } else if (msg == kSM_Decrement) {
        hintOptions.DelayHint(mLocatorName);
      }
      break;
    }
    case kSF_Billboard: {
      if (msg == kSM_Increment) {
        const SObjectTag* objectTag =
            gpResourceFactory->GetResourceIdByName(mLocatorName.data());
        const CAssetId assetId = objectTag ? objectTag->GetId() : kInvalidAssetId;

        mgr.SetPendingOnScreenTex(assetId, CVector2i(int(mFloat1), int(mFloat2)),
                                  CVector2i(int(mFloat3), int(mFloat4)));
        if (objectTag) {
          x1e8_ = gpSimplePool->GetObj(mLocatorName.data());
          x1e8_->Lock();
          mDisplayBillboard = true;
        }
      } else if (msg == kSM_Decrement) {
        mgr.SetPendingOnScreenTex(kInvalidAssetId, CVector2i(int(mFloat1), int(mFloat2)),
                                  CVector2i(int(mFloat3), int(mFloat4)));
        x1e8_ = rstl::optional_object_null();
        mDisplayBillboard = false;
      }
      break;
    }
    case kSF_PlayerInAreaRelay: {
      if ((msg == kSM_Action || msg == kSM_SetToZero) &&
          mgr.GetPlayer()->GetCurrentAreaId() == GetCurrentAreaId()) {
        SendScriptMsgs(kSS_Zero, mgr, kSM_None);
      }
      break;
    }
    case kSF_HUDTarget: {
      if (msg == kSM_Increment) {
        AddMaterial(kMT_Target, kMT_RadarObject, mgr);
      } else if (msg == kSM_Decrement) {
        RemoveMaterial(kMT_Target, kMT_RadarObject, mgr);
      }
      break;
    }
    case kSF_FogFader: {
      float speed = mFloat2;
      if (msg == kSM_Increment) {
        mgr.CameraManager()->SetWaterFogScale(mFloat1, speed);
      } else if (msg == kSM_Decrement) {
        mgr.CameraManager()->SetWaterFogScale(1.f, speed);
      }
      break;
    }
    case kSF_EnterLogbook: {
      if (msg == kSM_Action) {
        mgr.EnterLogBookScreen();
      }
      break;
    }
    case kSF_Ending: {
      if (msg == kSM_Action) {
        const int ending = GetSpecialEnding(mgr);
        if (ending == int(mFloat1)) {
          SendScriptMsgs(kSS_Zero, mgr, kSM_None);
        }
      }
      break;
    }
    default:
      break;
    }
  }
}

bool CScriptSpecialFunction::ShouldSkipCinematic(CStateManager& mgr) const {
  CAssetId mlvlId = mgr.GetWorld()->GetWorldAssetId();
  TEditorId cineId = GetEditorId();
  return gpGameState->SystemState().GetCinematicState(
      rstl::pair< CAssetId, TEditorId >(mlvlId, cineId));
}

void CScriptSpecialFunction::SkipCinematic(CStateManager& mgr) {
  SendScriptMsgs(kSS_Zero, mgr, kSM_None);
  mgr.SetCinematicSkipObject(kInvalidUniqueId);
}

ENTITY_ACCEPT_IMPL(CScriptSpecialFunction)

void CScriptSpecialFunction::RingScramble(CStateManager& mgr) {
  SendScriptMsgs(kSS_Zero, mgr, kSM_None);
  mRingState = kRS_Scramble;
  mRingReverse = !mRingReverse;
  float dir = mRingReverse ? 1.f : -1.f;
  for (int i = 0; i < mRingControllers.size(); ++i) {
    mRingControllers[i].mRotateSpeed = dir * mgr.Random()->Range(mFloat2, mFloat3);
    dir = -dir;
    mRingControllers[i].mReachedTarget = false;
  }
}

void CScriptSpecialFunction::ThinkSaveStation(float, CStateManager& mgr) {
  if (!mDoSave) {
    return;
  }
  if (!mgr.GetWantsToEnterSaveGameScreen()) {
    mDoSave = false;
    if (mgr.GetInSaveUI()) {
      SendScriptMsgs(kSS_MaxReached, mgr, kSM_None);
    } else {
      SendScriptMsgs(kSS_Zero, mgr, kSM_None);
    }
  }
}

void CScriptSpecialFunction::ThinkIntroBossRingController(float dt, CStateManager& mgr) {
  if (mRingState != kRS_Breakup) {
    for (int i = 0; i < mRingControllers.size(); ++i) {
      if (CActor* const act = TCastToPtr< CActor >(mgr.ObjectById(mRingControllers[i].mId))) {
        CTransform4f newXf = act->GetTransform();
        newXf.RotateLocalZ(CRelAngle::FromDegrees(dt * mRingControllers[i].mRotateSpeed));
        act->SetTransform(newXf);
      }
    }
  }
  switch (mRingState) {
  case kRS_Scramble:
    break;
  case kRS_Breakup: {
    float minMag = 0.f;
    for (int i = 0; i < mRingControllers.size(); ++i) {
      if (CActor* const act = TCastToPtr< CActor >(mgr.ObjectById(mRingControllers[i].mId))) {
        act->SetTranslation(act->GetTranslation() + act->GetTransform().GetForward() * 50.f * dt);
        minMag = rstl::min_val(minMag, act->GetTranslation().Magnitude());
      }
    }
    CalculateRenderBounds();
    if (minMag) {
      // Never actually happens
      for (int i = 0; i < mRingControllers.size(); ++i) {
        if (CEntity* ent = mgr.ObjectById(mRingControllers[i].mId)) {
          ent->SetActive(false);
        }
      }
      SetActive(false);
    }
    break;
  }
  case kRS_Rotate: {
    mRingRotateTarget =
        CQuaternion::AxisAngle(
            CUnitVector3f(CVector3f(0.f, 0.f, 1.f), CUnitVector3f::kN_Yes),
            CRelAngle::FromDegrees(mFloat1 * (mRingReverse ? 1.f : -1.f) * dt))
            .Transform(mRingRotateTarget);
    bool allReachedTarget = true;
    for (int i = 0; i < mRingControllers.size(); ++i) {
      if (mRingControllers[i].mReachedTarget) {
        continue;
      }
      if (CActor* const act = TCastToPtr< CActor >(mgr.ObjectById(mRingControllers[i].mId))) {
        CVector3f forward = act->GetTransform().GetForward();
        forward.SetZ(0.f);
        forward.Normalize();
        float f1 = CMath::Limit(CVector3f::Dot(forward, mRingRotateTarget), 1.f);
        float f2 = (mFloat1 + CMath::AbsF(mRingControllers[i].mRotateSpeed)) / 30.f;
        if (CMath::ArcCosineR(f1) <= CMath::Deg2Rad(f2)) {
          CTransform4f newXf = CTransform4f::LookAt(CVector3f::Zero(), mRingRotateTarget);
          newXf.SetTranslation(act->GetTranslation());
          act->SetTransform(newXf);
          mRingControllers[i].mRotateSpeed = (mRingReverse ? 1.f : -1.f) * mFloat1;
          mRingControllers[i].mReachedTarget = true;
        } else {
          allReachedTarget = false;
          break;
        }
      }
    }
    if (allReachedTarget) {
      SendScriptMsgs(kSS_MaxReached, mgr, kSM_None);
      mRingState = kRS_Stopped;
      for (int i = 0; i < mRingControllers.size(); ++i) {
        mRingControllers[i].mReachedTarget = false;
      }
    }
    break;
  }
  default:
    break;
  }
}

void CScriptSpecialFunction::ThinkPlayerFollowLocator(float, CStateManager& mgr) {
  for (rstl::vector< SConnection >::const_iterator conn = GetConnectionList().begin();
       conn != GetConnectionList().end(); ++conn) {
    if (conn->mState != kSS_Play || conn->mMsg != kSM_Activate) {
      continue;
    }

    const CStateManager::TIdListResult it = mgr.GetIdListForScript(conn->mObjId);
    if (!(it.first == it.second)) {
      if (const CActor* const act =
              TCastToConstPtr< CActor >(mgr.GetObjectById(it.first->second))) {
        if (!act->HasAnimation()) {
          continue;
        }
        CTransform4f xf = act->GetTransform() * act->GetLocatorTransform(mLocatorName);
        CPlayer& player = *mgr.Player();
        player.SetTransform(xf);
        player.SetVelocityWR(CVector3f::Zero());
        player.SetAngularVelocityWR(CAxisAngle::Identity());
        player.ClearForcesAndTorques();
        return;
      }
    }
  }
}

void CScriptSpecialFunction::ThinkSpinnerController(float dt, CStateManager& mgr,
                                                    ESpinnerControllerMode mode) {
  const bool allowWrap = mLocatorName.find(rstl::string_l("AllowWrap"), 0) != -1;
  const bool noBackward = mLocatorName.find(rstl::string_l("NoBackward"), 0) != -1;
  const float pointOneByDt = 0.1f * dt;
  const float twoByDt = 2.f * dt;

  for (rstl::vector< SConnection >::const_iterator conn = GetConnectionList().begin();
       conn != GetConnectionList().end(); ++conn) {
    if (conn->mState != kSS_Play || conn->mMsg != kSM_Activate) {
      continue;
    }

    const CStateManager::TIdListResult it = mgr.GetIdListForScript(conn->mObjId);
    if (it.first == it.second) {
      continue;
    }

    if (CScriptPlatform* plat = TCastToPtr< CScriptPlatform >(mgr.ObjectById(it.first->second))) {
      if (plat->HasAnimation()) {
        plat->SetControlledAnimation(true);
        if (!mSpinnerInitializedXf) {
          mSpinnerInitialXf = plat->GetTransform();
          mSpinnerInitializedXf = true;
        }

        const float decay = pointOneByDt * mFloat2;
        const float previous = x138_;

        if (mode == kSCM_Zero) {
          if (mSpinnerCanMove) {
            bool isMorphed = mgr.GetPlayer()->GetMorphballTransitionState() == CPlayer::kMS_Morphed;
            const CVector3f angVel = mgr.GetPlayer()->GetAngularVelocityOR().GetVector();
            float mag = angVel.CanBeNormalized() ? angVel.Magnitude() : 0.f;
            const float spinImpulse = (isMorphed ? 0.025f * mag : 0.f);
            if (spinImpulse > x180_) {
              SendScriptMsgs(kSS_Play, mgr, kSM_None);
            }

            x180_ = spinImpulse;
            x138_ += 0.01f * spinImpulse * mFloat1;

            if (!noBackward) {
              x138_ -= decay;
            }
          } else if (!noBackward) {
            x138_ = previous - twoByDt;
          }
        } else if (mode == kSCM_One) {
          x138_ = (0.01f * x16c_) * mFloat1 + previous;

          if (!noBackward) {
            x138_ -= decay;

            if (CMath::AbsF(x16c_) < dt) {
              x16c_ = 0.f;
            } else {
              float multi = CMath::Sign(x16c_);
              x16c_ = -(dt * multi - x16c_);
            }
          }
        }

        if (allowWrap) {
          x138_ = fmod(x138_, 1.f);
          if (x138_ < 0.f) {
            x138_ += 1.f;
          }
        } else {
          x138_ = rstl::min_val(1.f, rstl::max_val(0.f, x138_));
        }

        bool noSfxPlayed = true;
        const float movementDelta = x138_ - previous;
        if (close_enough(x138_, 1.f)) {
          if (!mSfx3Played) {
            if (mSfx3 != CSfxManager::kInternalInvalidSfxId) {
              CSfxManager::AddEmitter(mSfx3, GetTranslation(), CVector3f::Zero(), true, false);
            }

            mSfx3Played = true;
          }

          SendScriptMsgs(kSS_MaxReached, mgr, kSM_None);
          noSfxPlayed = false;
        } else {
          mSfx3Played = false;
        }

        if (close_enough(x138_, 0.f)) {
          if (!mSfx2Played) {
            if (mSfx2 != CSfxManager::kInternalInvalidSfxId) {
              CSfxManager::AddEmitter(mSfx2, GetTranslation(), CVector3f::Zero(), true, false);
            }

            mSfx2Played = true;
          }

          SendScriptMsgs(kSS_Zero, mgr, kSM_None);
          noSfxPlayed = false;
        } else {
          mSfx2Played = false;
        }

        // Retail samples the average before updating it with the current movement.
        rstl::optional_object< float > previousAverage = x184_.GetAverage();

        if (noSfxPlayed) {
          if (mSfx1 != CSfxManager::kInternalInvalidSfxId) {
            bool movingForward = movementDelta >= 0.f;
            if (noSfxPlayed) {
              x184_.AddValue(movingForward ? uchar(100) : uchar(0x7F));
            } else {
              x184_.AddValue(0.f);
            }
            const rstl::optional_object< float >& volume = x184_.GetAverage();
            float pitch = movingForward ? mFloat4 : 1.f;
            AddOrUpdateEmitter(pitch, mSfxHandle, mSfx1, GetTranslation(), volume.data());
          }
        } else {
          DeleteEmitter(mSfxHandle);
        }

        const CAnimData* animData = plat->GetAnimationData();
        const float dur = animData->GetAnimationDuration(animData->GetCurrentAnimation()) * x138_;
        plat->AnimationData()->SetPhase(0.f);
        plat->AnimationData()->SetPlaybackRate(1.f);
        CAdvancementDeltas deltas = plat->UpdateAnimation(dur, mgr, true);
        plat->SetTransform(mSpinnerInitialXf *
                           deltas.GetOrientationDelta().BuildTransform4f(deltas.GetOffsetDelta()));
      }
    }
  }
}

void CScriptSpecialFunction::ThinkObjectFollowLocator(float, CStateManager& mgr) {
  TUniqueId followerAct = kInvalidUniqueId;
  TUniqueId followedAct = kInvalidUniqueId;
  for (rstl::vector< SConnection >::const_iterator conn = GetConnectionList().begin();
       conn != GetConnectionList().end(); ++conn) {
    if (conn->mState != kSS_Play ||
        (conn->mMsg != kSM_Activate && conn->mMsg != kSM_Deactivate)) {
      continue;
    }

    const CStateManager::TIdListResult it = mgr.GetIdListForScript(conn->mObjId);
    if (!(it.first == it.second)) {
      TUniqueId uid = it.first->second;
      if (const CActor* const act = TCastToConstPtr< CActor >(mgr.GetObjectById(uid))) {
        if (conn->mMsg == kSM_Activate && act->HasAnimation()) {
          if (!act->GetActive()) {
            return;
          }
          followedAct = uid;
        } else if (conn->mMsg == kSM_Deactivate) {
          followerAct = uid;
        }
      }
    }
  }

  if (followerAct != kInvalidUniqueId && followedAct != kInvalidUniqueId) {
    const CActor* fromAct = TCastToConstPtr< CActor >(mgr.GetObjectById(followedAct));
    CActor* toAct = TCastToPtr< CActor >(mgr.ObjectById(followerAct));
    if (fromAct && toAct) {
      CTransform4f xf =
          fromAct->GetTransform() * fromAct->GetScaledLocatorTransform(mLocatorName);
      toAct->SetTransform(xf);
    }
  }
}

void CScriptSpecialFunction::ThinkObjectFollowObject(float, CStateManager& mgr) {
  TUniqueId followerAct = kInvalidUniqueId;
  TUniqueId followedAct = kInvalidUniqueId;
  for (rstl::vector< SConnection >::const_iterator conn = GetConnectionList().begin();
       conn != GetConnectionList().end(); ++conn) {
    if (conn->mState != kSS_Play ||
        (conn->mMsg != kSM_Activate && conn->mMsg != kSM_Deactivate)) {
      continue;
    }

    const CStateManager::TIdListResult it = mgr.GetIdListForScript(conn->mObjId);
    if (!(it.first == it.second)) {
      TUniqueId uid = it.first->second;
      if (const CActor* const act = TCastToConstPtr< CActor >(mgr.GetObjectById(uid))) {
        if (conn->mMsg == kSM_Activate) {
          if (!act->GetActive()) {
            return;
          }
          followedAct = uid;
        } else if (conn->mMsg == kSM_Deactivate) {
          followerAct = uid;
        }
      }
    }
  }

  const CActor* followed = TCastToConstPtr< CActor >(mgr.GetObjectById(followedAct));
  CActor* follower = TCastToPtr< CActor >(mgr.ObjectById(followerAct));
  if (follower && followed) {
    follower->SetTransform(followed->GetTransform());
  }
}

void CScriptSpecialFunction::Render(const CStateManager& mgr) const {
  if (mFunction == kSF_FogVolume) {
    if (GetActive()) {
      const float z = mgr.IntegrateVisorFog(
          mFloat1 * CMath::FastSinR(CGraphics::GetSecondsMod900() * mFloat2));
      if (z > 0.f) {
        const CVector3f pos = GetTranslation();
        CVector3f min(pos - mVector3f);
        CVector3f max(pos + mVector3f);
        max[kDZ] += z;
        CAABox box(min, max);
        CTransform4f modelMtx = CTransform4f::Translate(box.GetCenterPoint()) *
                                CTransform4f::Scale((box.GetMaxPoint() - box.GetMinPoint()) * 0.5f);

        CAABox renderbox(CVector3f(-1.f, -1.f, -1.f), CVector3f(1.f, 1.f, 1.f));

        gpRender->SetModelMatrix(modelMtx);
        gpRender->RenderFogVolume(mColor, renderbox, nullptr, nullptr);
      }
    }
  } else {
    CActor::Render(mgr);
  }
}

void CScriptSpecialFunction::ThinkChaffTarget(float dt, CStateManager& mgr) {
  TEntityList nearList;
  const CVector3f offset(5.f, 5.f, 5.f);
  const CAABox box(GetTranslation() - offset, GetTranslation() + offset);
  mgr.BuildNearList(nearList, box, CMaterialFilter::MakeInclude(CMaterialList(kMT_Projectile)),
                    nullptr);

  for (int i = 0; i < nearList.size(); ++i) {
    if (CEnergyProjectile* proj = TCastToPtr< CEnergyProjectile >(mgr.ObjectById(nearList[i]))) {
      if (proj->GetHomingTargetId() == GetUniqueId()) {
        proj->Set3d0_26(true);
        if (mgr.GetPlayer()->GetCurrentAreaId() == GetCurrentAreaId()) {
          mgr.Player()->SetHudDisable(mFloat2);
          x194_ = mFloat1;

          CCameraFilterPass& filter = mgr.CameraFilterPass(CStateManager::kCFS_Seven);
          filter.SetFilter(CCameraFilterPass::kFT_Blend, CCameraFilterPass::kFS_Fullscreen, 0.f,
                           CColor(1.f, 1.f, 1.f), kInvalidAssetId);
          filter.DisableFilter(0.1f);
        }
      }
    }
  }

  bool addedInterference = false;
  x194_ = CMath::Max(0.f, x194_ - dt);
  if (x194_ && mgr.GetPlayer()->GetCurrentAreaId() == GetCurrentAreaId()) {
    addedInterference = true;
    float intfMag = mFloat3 * (0.5f + ((0.5f * x194_) / mFloat1));
    if (x194_ < 1.f) {
      intfMag *= x194_;
    }
    mgr.PlayerState()->StaticInterference().AddSource(GetUniqueId(), intfMag, 0.5f);
  }
  if (addedInterference && mgr.GetPlayerState()->GetCurrentVisor() != CPlayerState::kPV_Thermal) {
    mgr.Player()->AddOrbitDisableSource(mgr, GetUniqueId());
  } else {
    mgr.Player()->RemoveOrbitDisableSource(GetUniqueId());
  }
}

void CScriptSpecialFunction::ThinkRainSimulator(float, CStateManager& mgr) {
  if ((static_cast< float >(mgr.GetInputFrameIdx() % 3600)) / 3600.f < 0.5f) {
    SendScriptMsgs(kSS_MaxReached, mgr, kSM_None);
  } else {
    SendScriptMsgs(kSS_Zero, mgr, kSM_None);
  }
}

void CScriptSpecialFunction::ThinkAreaDamage(float dt, CStateManager& mgr) {
  const CPlayer* player = mgr.GetPlayer();
  bool inArea = player->GetCurrentAreaId() == GetCurrentAreaId();
  bool immune = mgr.GetPlayerState()->GetCurrentSuitRaw() > CPlayerState::kPS_Power;
  if (mInAreaDamage) {
    if (!inArea || immune) {
      mInAreaDamage = false;
      mgr.Player()->DecrementEnvironmentDamage();
      SendScriptMsgs(kSS_Exited, mgr, kSM_None);
      mgr.SetIsFullThreat(false);
      return;
    }
  } else if (!inArea || immune) {
    return;
  } else {
    mInAreaDamage = true;
    mgr.Player()->IncrementEnvironmentDamage();
    SendScriptMsgs(kSS_Entered, mgr, kSM_None);
    mgr.SetIsFullThreat(true);
  }

  CDamageInfo dInfo(CWeaponMode(kWT_Heat), mFloat1 * dt, 0.f, 0.f, true);
  mgr.ApplyDamage(GetUniqueId(), player->GetUniqueId(), GetUniqueId(), dInfo,
                  CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
                  CVector3f::Zero());
}

void CScriptSpecialFunction::ThinkActorScale(float dt, CStateManager& mgr) {
  const float deltaScale = dt * mFloat1;
  const float f2 = mFloat2;

  for (rstl::vector< SConnection >::const_iterator conn = GetConnectionList().begin();
       conn != GetConnectionList().end(); ++conn) {
    if (conn->mState != kSS_Play || conn->mMsg != kSM_Activate) {
      continue;
    }

    TUniqueId uid = mgr.GetIdForScript(conn->mObjId);
    if (CActor* act = TCastToPtr< CActor >(mgr.ObjectById(uid))) {
      if (act->HasModelData()) {
        CVector3f scale = act->GetModelData()->GetScale();
        float x, y, z;
        if (deltaScale > 0.f) {
          x = CMath::Min(deltaScale + scale.GetX(), f2);
          y = CMath::Min(deltaScale + scale.GetY(), f2);
          z = CMath::Min(deltaScale + scale.GetZ(), f2);
        } else {
          x = CMath::Max(deltaScale + scale.GetX(), f2);
          y = CMath::Max(deltaScale + scale.GetY(), f2);
          z = CMath::Max(deltaScale + scale.GetZ(), f2);
        }
        act->ModelData()->SetScale(CVector3f(x, y, z));
      }
    }
  }
}

void CScriptSpecialFunction::ThinkPlayerInArea(float dt, CStateManager& mgr) {
  if (mgr.GetPlayer()->GetCurrentAreaId() == GetCurrentAreaId()) {
    if (mPlayerInArea) {
      return;
    }

    mPlayerInArea = true;
    SendScriptMsgs(kSS_Entered, mgr, kSM_None);
  } else if (mPlayerInArea) {
    mPlayerInArea = false;
    SendScriptMsgs(kSS_Exited, mgr, kSM_None);
  }
}

void CScriptSpecialFunction::AddOrUpdateEmitter(const float pitch, CSfxHandle& handle,
                                                const ushort id, const CVector3f pos,
                                                const uchar vol) {
  if (!handle) {
    handle = CSfxManager::AddEmitter(id, pos, CVector3f::Zero(), vol, true, true);
  } else {
    CSfxManager::UpdateEmitter(handle, pos, CVector3f::Zero(), vol);
    CSfxManager::PitchBend(handle, static_cast< short >(8192.f * pitch + 8192.f));
  }
}

void CScriptSpecialFunction::DeleteEmitter(CSfxHandle& handle) {
  if (handle) {
    CSfxManager::RemoveEmitter(handle);
    handle.Clear();
  }
}

int CScriptSpecialFunction::GetSpecialEnding(const CStateManager& mgr) const {
  const int rate = (mgr.GetPlayerState()->CalculateItemCollectionRate() * 100) /
                   mgr.GetPlayerState()->GetTotalPickupCount();
  int result;
  if (rate < 75) {
    result = 0;
  } else {
    result = 2;
    if (rate < 100) {
      result = 1;
    }
  }
  return result;
}
