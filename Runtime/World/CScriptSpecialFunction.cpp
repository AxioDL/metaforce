#include "CScriptSpecialFunction.hpp"
#include "Audio/CSfxManager.hpp"
#include "Camera/CCameraManager.hpp"
#include "Character/CModelData.hpp"
#include "Graphics/CTexture.hpp"
#include "World/CActorParameters.hpp"
#include "World/CPlayer.hpp"
#include "GameGlobalObjects.hpp"
#include "CMemoryCardSys.hpp"
#include "CGameState.hpp"
#include "CSimplePool.hpp"
#include "CStateManager.hpp"
#include "IMain.hpp"
#include "TCastTo.hpp"

#include "hecl/CVarManager.hpp"

namespace urde {

CScriptSpecialFunction::CScriptSpecialFunction(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                                               const zeus::CTransform& xf, ESpecialFunction func,
                                               std::string_view lcName, float f1, float f2, float f3, float f4,
                                               const zeus::CVector3f& vec, const zeus::CColor& col, bool active,
                                               const CDamageInfo& dInfo, s32 aId1, s32 aId2,
                                               CPlayerState::EItemType itemType, s16 sId1, s16 sId2, s16 sId3)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), CMaterialList(), CActorParameters::None(),
         kInvalidUniqueId)
, xe8_function(func)
, xec_locatorName(lcName)
, xfc_(f1)
, x100_(f2)
, x104_(f3)
, x108_(f4)
, x10c_(vec)
, x118_(col)
, x11c_damageInfo(dInfo)
, x170_(CSfxManager::TranslateSFXID(sId1))
, x172_(CSfxManager::TranslateSFXID(sId2))
, x174_(CSfxManager::TranslateSFXID(sId3))
, x1bc_areaSaveId(aId1)
, x1c0_layerIdx(aId2)
, x1c4_item(itemType) {
  x1e4_26_ = true;
  if (xe8_function == ESpecialFunction::HUDTarget)
    x1c8_ = {{zeus::CVector3f(-1.f), zeus::CVector3f(1.f)}};
}

void CScriptSpecialFunction::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CScriptSpecialFunction::Think(float dt, CStateManager& mgr) {
  switch (xe8_function) {
  case ESpecialFunction::PlayerFollowLocator:
    ThinkPlayerFollowLocator(dt, mgr);
    break;
  case ESpecialFunction::SpinnerController:
    ThinkSpinnerController(dt, mgr, ESpinnerControllerMode::Zero);
    break;
  case ESpecialFunction::ShotSpinnerController:
    ThinkSpinnerController(dt, mgr, ESpinnerControllerMode::One);
    break;
  case ESpecialFunction::ObjectFollowLocator:
    ThinkObjectFollowLocator(dt, mgr);
    break;
  case ESpecialFunction::ObjectFollowObject:
    ThinkObjectFollowObject(dt, mgr);
    break;
  case ESpecialFunction::ChaffTarget:
    ThinkChaffTarget(dt, mgr);
    break;
  case ESpecialFunction::ViewFrustumTester: {
    if (x1e4_28_frustumEntered) {
      x1e4_28_frustumEntered = false;
      SendScriptMsgs(EScriptObjectState::Entered, mgr, EScriptObjectMessage::None);
    }
    if (x1e4_29_frustumExited) {
      x1e4_29_frustumExited = false;
      SendScriptMsgs(EScriptObjectState::Exited, mgr, EScriptObjectMessage::None);
    }
    break;
  }
  case ESpecialFunction::SaveStation:
    ThinkSaveStation(dt, mgr);
    break;
  case ESpecialFunction::IntroBossRingController:
    ThinkIntroBossRingController(dt, mgr);
    break;
  case ESpecialFunction::RainSimulator:
    ThinkRainSimulator(dt, mgr);
    break;
  case ESpecialFunction::AreaDamage:
    ThinkAreaDamage(dt, mgr);
    break;
  case ESpecialFunction::ScaleActor:
    ThinkActorScale(dt, mgr);
    break;
  case ESpecialFunction::PlayerInAreaRelay:
    ThinkPlayerInArea(dt, mgr);
    break;
  case ESpecialFunction::Billboard: {
    if (x1e8_ && x1e5_26_displayBillboard) {
      SendScriptMsgs(EScriptObjectState::MaxReached, mgr, EScriptObjectMessage::None);
      x1e5_26_displayBillboard = false;
    }
    break;
  }
  default:
    break;
  }
}
static const ERumbleFxId fxTranslation[6] = {ERumbleFxId::Twenty,    ERumbleFxId::One,         ERumbleFxId::TwentyOne,
                                             ERumbleFxId::TwentyTwo, ERumbleFxId::TwentyThree, ERumbleFxId::Zero};

void CScriptSpecialFunction::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  if (GetActive() && msg == EScriptObjectMessage::Deactivate && xe8_function == ESpecialFunction::Billboard) {
    mgr.SetPendingOnScreenTex(CAssetId(), zeus::CVector2i(), zeus::CVector2i());
    if (x1e8_)
      x1e8_ = TLockedToken<CTexture>();
    x1e5_26_displayBillboard = false;
  }
  CActor::AcceptScriptMsg(msg, uid, mgr);

  if (xe8_function == ESpecialFunction::ChaffTarget && msg == EScriptObjectMessage::InitializedInArea)
    AddMaterial(EMaterialTypes::Target, mgr);

  if (GetActive()) {
    switch (xe8_function) {
    case ESpecialFunction::HUDFadeIn: {
      if (msg == EScriptObjectMessage::Action)
        mgr.Player()->SetHudDisable(xfc_, 0.f, 0.5f);
      break;
    }
    case ESpecialFunction::EscapeSequence: {
      if (msg == EScriptObjectMessage::Action && xfc_ >= 0.f)
        mgr.ResetEscapeSequenceTimer(xfc_);
      break;
    }
    case ESpecialFunction::SpinnerController: {
      switch (msg) {
      case EScriptObjectMessage::Stop: {
        x1e4_25_spinnerCanMove = false;
        break;
      }
      case EScriptObjectMessage::Play: {
        x1e4_25_spinnerCanMove = true;
        mgr.Player()->SetAngularVelocityWR(zeus::CAxisAngle::sIdentity);
        break;
      }
      case EScriptObjectMessage::Deactivate:
        DeleteEmitter(x178_sfxHandle);
        break;
      default:
        break;
      }
      break;
    }
    case ESpecialFunction::ShotSpinnerController: {
      switch (msg) {
      case EScriptObjectMessage::Increment: {
        x16c_ = zeus::clamp(0.f, x16c_ + 1.f, 1.f);
        SendScriptMsgs(EScriptObjectState::Play, mgr, EScriptObjectMessage::None);
        break;
      }
      case EScriptObjectMessage::SetToMax: {
        SendScriptMsgs(EScriptObjectState::Play, mgr, EScriptObjectMessage::None);
        break;
      }
      case EScriptObjectMessage::SetToZero: {
        x16c_ = -0.5f * x104_;
        break;
      }
      default:
        break;
      }
      break;
    }
    case ESpecialFunction::MapStation: {
      if (msg == EScriptObjectMessage::Action) {
        mgr.MapWorldInfo()->SetMapStationUsed(true);
        const_cast<CMapWorld&>(*mgr.WorldNC()->GetMapWorld())
            .RecalculateWorldSphere(*mgr.MapWorldInfo(), *mgr.GetWorld());
      }
      break;
    }
    case ESpecialFunction::MissileStation: {
      if (msg == EScriptObjectMessage::Action) {
        CPlayerState& pState = *mgr.GetPlayerState().get();
        pState.ResetAndIncrPickUp(CPlayerState::EItemType::Missiles,
                                  pState.GetItemCapacity(CPlayerState::EItemType::Missiles));
      }
      break;
    }
    case ESpecialFunction::PowerBombStation: {
      if (msg == EScriptObjectMessage::Action) {
        CPlayerState& pState = *mgr.GetPlayerState().get();
        pState.ResetAndIncrPickUp(CPlayerState::EItemType::PowerBombs,
                                  pState.GetItemCapacity(CPlayerState::EItemType::PowerBombs));
      }
      break;
    }
    case ESpecialFunction::SaveStation: {
      if (msg == EScriptObjectMessage::Action) {
        g_GameState->GetPlayerState()->IncrPickup(CPlayerState::EItemType::EnergyTanks, 1);
        if (g_GameState->GetCardSerial() == 0)
          SendScriptMsgs(EScriptObjectState::Closed, mgr, EScriptObjectMessage::None);
        else {
          mgr.DeferStateTransition(EStateManagerTransition::SaveGame);
          x1e5_24_doSave = true;
        }
      }
      break;
    }
    case ESpecialFunction::IntroBossRingController: {
      if (x1a8_ != 3) {
        switch (msg) {
        case EScriptObjectMessage::Play: {
          if (x1a8_ != 0)
            RingScramble(mgr);

          for (SRingController& cont : x198_ringControllers) {
            if (TCastToPtr<CActor> act = mgr.ObjectById(cont.x0_id))
              cont.xc_ = act->GetTransform().frontVector();
            else
              cont.xc_ = zeus::CVector3f::skForward;
          }

          x1a8_ = 3;
          break;
        }
        case EScriptObjectMessage::SetToZero: {
          x1a8_ = 1;
          x1ac_ = GetTranslation() - mgr.GetPlayer().GetTranslation();
          x1ac_.z() = 0.f;
          x1ac_.normalize();
          break;
        }
        case EScriptObjectMessage::Action: {
          RingScramble(mgr);
          break;
        }
        case EScriptObjectMessage::InitializedInArea: {
          x198_ringControllers.reserve(3);
          for (const SConnection& conn : x20_conns) {
            if (conn.x0_state != EScriptObjectState::Play || conn.x4_msg != EScriptObjectMessage::Activate)
              continue;

            auto search = mgr.GetIdListForScript(conn.x8_objId);
            for (auto it = search.first; it != search.second; ++it) {
              if (TCastToPtr<CActor> act = mgr.ObjectById(it->second)) {
                x198_ringControllers.push_back(SRingController(it->second, 0.f, false));
                act->RemoveMaterial(EMaterialTypes::Occluder, mgr);
              }
            }

            // std::sort(x198_ringControllers.begin(), x198_ringControllers.end());
          }
          break;
        }
        default:
          break;
        }
      }
      break;
    }
    case ESpecialFunction::RadialDamage: {
      if (msg == EScriptObjectMessage::Action) {
        CDamageInfo dInfo = x11c_damageInfo;
        dInfo.SetRadius(xfc_);
        mgr.ApplyDamageToWorld(GetUniqueId(), *this, GetTranslation(), dInfo,
                               CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {0ull}));
      }
      break;
    }
    case ESpecialFunction::BossEnergyBar: {
      if (msg == EScriptObjectMessage::Increment)
        mgr.SetBossParams(uid, xfc_, u32(x100_) + 86);
      else if (msg == EScriptObjectMessage::Decrement)
        mgr.SetBossParams(kInvalidUniqueId, 0.f, 0);
      break;
    }
    case ESpecialFunction::EndGame: {
      if (msg == EScriptObjectMessage::Action) {
        switch (GetSpecialEnding(mgr)) {
        case 0:
          g_Main->SetFlowState(EFlowState::WinBad);
          break;
        case 1:
          g_Main->SetFlowState(EFlowState::WinGood);
          break;
        case 2:
          g_Main->SetFlowState(EFlowState::WinBest);
          break;
        }
        mgr.SetShouldQuitGame(true);
      }
      break;
    }
    case ESpecialFunction::CinematicSkip: {
      if (msg == EScriptObjectMessage::Increment) {
        if (ShouldSkipCinematic(mgr))
          mgr.SetSkipCinematicSpecialFunction(GetUniqueId());
      } else if (msg == EScriptObjectMessage::Decrement) {
        mgr.SetSkipCinematicSpecialFunction(kInvalidUniqueId);
        g_GameState->SystemOptions().SetCinematicState(mgr.GetWorld()->GetWorldAssetId(), GetEditorId(), true);
      }
      break;
    }
    case ESpecialFunction::ScriptLayerController: {
      if (msg == EScriptObjectMessage::Decrement || msg == EScriptObjectMessage::Increment) {
        if (x1bc_areaSaveId != -1 && x1c0_layerIdx != -1) {
          TAreaId aId = mgr.GetWorld()->GetAreaIdForSaveId(x1bc_areaSaveId);
          std::shared_ptr<CWorldLayerState> worldLayerState;
          if (aId != kInvalidAreaId)
            worldLayerState = mgr.WorldLayerStateNC();
          else {
            std::pair<CAssetId, TAreaId> worldAreaPair = g_MemoryCardSys->GetAreaAndWorldIdForSaveId(x1bc_areaSaveId);
            if (worldAreaPair.first.IsValid()) {
              worldLayerState = g_GameState->StateForWorld(worldAreaPair.first).GetLayerState();
              aId = worldAreaPair.second;
            }
          }

          if (aId != kInvalidAreaId)
            worldLayerState->SetLayerActive(aId, x1c0_layerIdx, msg == EScriptObjectMessage::Increment);
        }
      }
      break;
    }
    /*
    For some bizarre reason ScriptLayerController drops into EnvFxDensityController
    [[fallthrough]];
    We won't do that though
    */
    case ESpecialFunction::EnvFxDensityController: {
      if (msg == EScriptObjectMessage::Action)
        mgr.GetEnvFxManager()->SetFxDensity(s32(x100_), xfc_);
      break;
    }
    case ESpecialFunction::RumbleEffect:
      if (msg == EScriptObjectMessage::Action) {
        s32 rumbFx = s32(x100_);
        /* Retro originally did not check the upper bounds, this could potentially cause a crash
         * with some runtimes, so let's make sure we're not out of bounds in either direction
         */
        if (rumbFx < 0 || rumbFx >= 6)
          break;

        mgr.GetRumbleManager().Rumble(mgr, fxTranslation[rumbFx], 1.f, ERumblePriority::One);
      }
      break;
    case ESpecialFunction::InventoryActivator: {
      if (msg == EScriptObjectMessage::Action && mgr.GetPlayerState()->HasPowerUp(x1c4_item))
        SendScriptMsgs(EScriptObjectState::Zero, mgr, EScriptObjectMessage::None);
      break;
    }
    case ESpecialFunction::FusionRelay: {
      if (msg == EScriptObjectMessage::Action && mgr.GetPlayerState()->IsFusionEnabled())
        SendScriptMsgs(EScriptObjectState::Zero, mgr, EScriptObjectMessage::None);
      break;
    }
    case ESpecialFunction::AreaDamage: {
      if ((msg == EScriptObjectMessage::Deleted || msg == EScriptObjectMessage::Deactivate) && x1e4_31_) {
        x1e4_31_ = false;
        mgr.GetPlayer().DecrementPhazon();
        mgr.SetIsFullThreat(false);
      }
      break;
    }
    case ESpecialFunction::DropBomb: {
      if (msg == EScriptObjectMessage::Action) {
        if (xfc_ >= 1.f)
          mgr.GetPlayer().GetPlayerGun()->DropBomb(CPlayerGun::EBWeapon::PowerBomb, mgr);
        else
          mgr.GetPlayer().GetPlayerGun()->DropBomb(CPlayerGun::EBWeapon::Bomb, mgr);
      }
      break;
    }
    case ESpecialFunction::RedundantHintSystem: {
      CHintOptions& hintOptions = g_GameState->HintOptions();
      if (msg == EScriptObjectMessage::Action)
        hintOptions.ActivateContinueDelayHintTimer(xec_locatorName.c_str());
      else if (msg == EScriptObjectMessage::Increment)
        hintOptions.ActivateImmediateHintTimer(xec_locatorName.c_str());
      else if (msg == EScriptObjectMessage::Decrement)
        hintOptions.DelayHint(xec_locatorName.c_str());
      break;
    }
    case ESpecialFunction::Billboard: {
      if (msg == EScriptObjectMessage::Increment) {
        const SObjectTag* objectTag = g_ResFactory->GetResourceIdByName(xec_locatorName);
        CAssetId assetId = objectTag ? objectTag->id : CAssetId();

        mgr.SetPendingOnScreenTex(assetId, {int(x104_), int(x108_)}, {int(xfc_), int(x100_)});
        if (objectTag) {
          x1e8_ = g_SimplePool->GetObj(*objectTag);
          x1e5_26_displayBillboard = true;
        }
      } else if (msg == EScriptObjectMessage::Decrement) {
        mgr.SetPendingOnScreenTex({}, {int(x104_), int(x108_)}, {int(xfc_), int(x100_)});
        if (x1e8_)
          x1e8_ = TLockedToken<CTexture>();
        x1e5_26_displayBillboard = false;
      }
      break;
    }
    case ESpecialFunction::PlayerInAreaRelay: {
      if ((msg == EScriptObjectMessage::Action || msg == EScriptObjectMessage::SetToZero) &&
          GetAreaIdAlways() == mgr.GetPlayer().GetAreaIdAlways()) {
        SendScriptMsgs(EScriptObjectState::Zero, mgr, EScriptObjectMessage::None);
      }
      break;
    }
    case ESpecialFunction::HUDTarget: {
      if (msg == EScriptObjectMessage::Increment)
        AddMaterial(EMaterialTypes::Target, EMaterialTypes::RadarObject, mgr);
      else if (msg == EScriptObjectMessage::Decrement)
        RemoveMaterial(EMaterialTypes::Target, EMaterialTypes::RadarObject, mgr);
      break;
    }
    case ESpecialFunction::FogFader: {
      if (msg == EScriptObjectMessage::Increment)
        mgr.GetCameraManager()->SetFogDensity(x100_, xfc_);
      else if (msg == EScriptObjectMessage::Decrement)
        mgr.GetCameraManager()->SetFogDensity(x100_, 1.f);
      break;
    }
    case ESpecialFunction::EnterLogbook: {
      if (msg == EScriptObjectMessage::Action)
        mgr.DeferStateTransition(EStateManagerTransition::LogBook);
      break;
    }
    case ESpecialFunction::Ending: {
      if (msg == EScriptObjectMessage::Action && GetSpecialEnding(mgr) == u32(xfc_))
        SendScriptMsgs(EScriptObjectState::Zero, mgr, EScriptObjectMessage::None);
    }
    default:
      break;
    }
  }
}

void CScriptSpecialFunction::PreRender(CStateManager&, const zeus::CFrustum&) {}

void CScriptSpecialFunction::AddToRenderer(const zeus::CFrustum&, const CStateManager&) const {}

void CScriptSpecialFunction::Render(const CStateManager&) const {}

void CScriptSpecialFunction::SkipCinematic(CStateManager& stateMgr) {
  SendScriptMsgs(EScriptObjectState::Zero, stateMgr, EScriptObjectMessage::None);
  stateMgr.SetSkipCinematicSpecialFunction(kInvalidUniqueId);
}

void CScriptSpecialFunction::RingMoveCloser(CStateManager&, float) {}

void CScriptSpecialFunction::RingMoveAway(CStateManager&, float) {}

void CScriptSpecialFunction::ThinkRingPuller(float, CStateManager&) {}

void CScriptSpecialFunction::RingScramble(CStateManager&) {}

void CScriptSpecialFunction::ThinkIntroBossRingController(float, CStateManager&) {}

void CScriptSpecialFunction::ThinkPlayerFollowLocator(float, CStateManager&) {}

void CScriptSpecialFunction::ThinkSpinnerController(float, CStateManager&,
                                                    CScriptSpecialFunction::ESpinnerControllerMode) {}

void CScriptSpecialFunction::ThinkObjectFollowLocator(float, CStateManager&) {}

void CScriptSpecialFunction::ThinkObjectFollowObject(float, CStateManager&) {}

void CScriptSpecialFunction::ThinkChaffTarget(float, CStateManager&) {}

void CScriptSpecialFunction::ThinkActorScale(float dt, CStateManager& mgr) {
  float deltaScale = dt * xfc_;

  for (const SConnection& conn : x20_conns) {
    if (conn.x0_state != EScriptObjectState::Play || conn.x4_msg != EScriptObjectMessage::Activate)
      continue;
    if (TCastToPtr<CActor> act = mgr.ObjectById(mgr.GetIdForScript(conn.x8_objId))) {
      CModelData* mData = act->ModelData();
      if (mData && (mData->HasAnimData() || mData->HasNormalModel())) {
        zeus::CVector3f scale = mData->GetScale();

        if (deltaScale > 0.f)
          scale = zeus::min(zeus::CVector3f(deltaScale) + scale, zeus::CVector3f(x100_));
        else
          scale = zeus::max(zeus::CVector3f(deltaScale) + scale, zeus::CVector3f(x100_));

        mData->SetScale(scale);
      }
    }
  }
}

void CScriptSpecialFunction::ThinkSaveStation(float, CStateManager& mgr) {
  if (x1e5_24_doSave && mgr.GetDeferredStateTransition() != EStateManagerTransition::SaveGame) {
    x1e5_24_doSave = false;
    if (mgr.GetInSaveUI())
      SendScriptMsgs(EScriptObjectState::MaxReached, mgr, EScriptObjectMessage::None);
    else
      SendScriptMsgs(EScriptObjectState::Zero, mgr, EScriptObjectMessage::None);
  }
}

void CScriptSpecialFunction::ThinkRainSimulator(float, CStateManager& mgr) {
  if ((float(mgr.GetInputFrameIdx()) / 3600.f) < 0.5f)
    SendScriptMsgs(EScriptObjectState::MaxReached, mgr, EScriptObjectMessage::None);
  else
    SendScriptMsgs(EScriptObjectState::Zero, mgr, EScriptObjectMessage::None);
}

void CScriptSpecialFunction::ThinkAreaDamage(float, CStateManager&) {}

void CScriptSpecialFunction::ThinkPlayerInArea(float dt, CStateManager& mgr) {
  if (mgr.GetPlayer().GetAreaIdAlways() == GetAreaIdAlways()) {
    if (x1e5_25_playerInArea)
      return;

    x1e5_25_playerInArea = true;
    SendScriptMsgs(EScriptObjectState::Entered, mgr, EScriptObjectMessage::None);
  } else if (x1e5_25_playerInArea) {
    x1e5_25_playerInArea = false;
    SendScriptMsgs(EScriptObjectState::Exited, mgr, EScriptObjectMessage::None);
  }
}

bool CScriptSpecialFunction::ShouldSkipCinematic(CStateManager& stateMgr) const {
  if (hecl::com_developer->toBoolean())
    return true;
  return g_GameState->SystemOptions().GetCinematicState(stateMgr.GetWorld()->IGetWorldAssetId(), GetEditorId());
}

void CScriptSpecialFunction::DeleteEmitter(const CSfxHandle& handle) {
  if (handle)
    CSfxManager::RemoveEmitter(handle);
}

u32 CScriptSpecialFunction::GetSpecialEnding(const CStateManager& mgr) const {
  const u32 rate = (mgr.GetPlayerState()->CalculateItemCollectionRate() * 100) / mgr.GetPlayerState()->GetPickupTotal();
  if (rate < 75)
    return 0;
  else if (rate < 100)
    return 1;
  return 2;
}

CScriptSpecialFunction::SRingController::SRingController(TUniqueId uid, float f, bool b) : x0_id(uid), x4_(f), x8_(b) {}

} // namespace urde
