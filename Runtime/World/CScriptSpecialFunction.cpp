#include "Runtime/World/CScriptSpecialFunction.hpp"

#include <array>

#include "Runtime/CGameState.hpp"
#include "Runtime/CMemoryCardSys.hpp"
#include "Runtime/CSimplePool.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/IMain.hpp"
#include "Runtime/Audio/CSfxManager.hpp"
#include "Runtime/Camera/CCameraManager.hpp"
#include "Runtime/Character/CModelData.hpp"
#include "Runtime/Graphics/CBooRenderer.hpp"
#include "Runtime/Graphics/CTexture.hpp"
#include "Runtime/World/CActorParameters.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "Runtime/World/CScriptPlatform.hpp"
#include "Runtime/Weapon/CEnergyProjectile.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

#include <hecl/CVarManager.hpp>

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
, xfc_float1(f1)
, x100_float2(f2)
, x104_float3(f3)
, x108_float4(f4)
, x10c_vector3f(vec)
, x118_color(col)
, x11c_damageInfo(dInfo)
, x170_sfx1(CSfxManager::TranslateSFXID(sId1))
, x172_sfx2(CSfxManager::TranslateSFXID(sId2))
, x174_sfx3(CSfxManager::TranslateSFXID(sId3))
, x184_(0.f)
, x1bc_areaSaveId(aId1)
, x1c0_layerIdx(aId2)
, x1c4_item(itemType) {
  if (xe8_function == ESpecialFunction::HUDTarget)
    x1c8_touchBounds = {-1.f, 1.f};
}

void CScriptSpecialFunction::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CScriptSpecialFunction::Think(float dt, CStateManager& mgr) {
  if (!GetActive())
    return;
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

constexpr std::array fxTranslation{
    ERumbleFxId::Twenty,    ERumbleFxId::One,         ERumbleFxId::TwentyOne,
    ERumbleFxId::TwentyTwo, ERumbleFxId::TwentyThree, ERumbleFxId::Zero,
};

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
        mgr.Player()->SetHudDisable(xfc_float1, 0.f, 0.5f);
      break;
    }
    case ESpecialFunction::EscapeSequence: {
      if (msg == EScriptObjectMessage::Action && xfc_float1 >= 0.f)
        mgr.ResetEscapeSequenceTimer(xfc_float1);
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
        mgr.Player()->SetAngularVelocityWR(zeus::CAxisAngle());
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
        x16c_ = -0.5f * x104_float3;
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
        mgr.GetWorld()->GetMapWorld()->RecalculateWorldSphere(*mgr.MapWorldInfo(), *mgr.GetWorld());
      }
      break;
    }
    case ESpecialFunction::MissileStation: {
      if (msg == EScriptObjectMessage::Action) {
        CPlayerState& pState = *mgr.GetPlayerState();
        pState.ResetAndIncrPickUp(CPlayerState::EItemType::Missiles,
                                  pState.GetItemCapacity(CPlayerState::EItemType::Missiles));
      }
      break;
    }
    case ESpecialFunction::PowerBombStation: {
      if (msg == EScriptObjectMessage::Action) {
        CPlayerState& pState = *mgr.GetPlayerState();
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
      if (x1a8_ringState != ERingState::Breakup) {
        switch (msg) {
        case EScriptObjectMessage::Play: {
          if (x1a8_ringState != ERingState::Scramble)
            RingScramble(mgr);

          for (SRingController& cont : x198_ringControllers) {
            if (TCastToPtr<CActor> act = mgr.ObjectById(cont.x0_id))
              cont.xc_ = act->GetTransform().frontVector();
            else
              cont.xc_ = zeus::skForward;
          }

          x1a8_ringState = ERingState::Breakup;
          break;
        }
        case EScriptObjectMessage::SetToZero: {
          x1a8_ringState = ERingState::Rotate;
          x1ac_ringRotateTarget = GetTranslation() - mgr.GetPlayer().GetTranslation();
          x1ac_ringRotateTarget.z() = 0.f;
          x1ac_ringRotateTarget.normalize();
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
                x198_ringControllers.emplace_back(it->second, 0.f, false);
                act->RemoveMaterial(EMaterialTypes::Occluder, mgr);
              }
            }
          }

          std::sort(x198_ringControllers.begin(), x198_ringControllers.end(),
                    [&mgr](const SRingController& a, const SRingController& b) {
                      TCastToConstPtr<CActor> actA(mgr.GetObjectById(a.x0_id));
                      TCastToConstPtr<CActor> actB(mgr.GetObjectById(b.x0_id));
                      if (actA && actB)
                        return actA->GetTranslation().z() < actB->GetTranslation().z();
                      return false;
                    });

          for (auto& rc : x198_ringControllers) {
            rc.x4_rotateSpeed = (x1b8_ringReverse ? 1.f : -1.f) * xfc_float1;
            rc.x8_reachedTarget = false;
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
        dInfo.SetRadius(xfc_float1);
        mgr.ApplyDamageToWorld(GetUniqueId(), *this, GetTranslation(), dInfo,
                               CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {0ull}));
      }
      break;
    }
    case ESpecialFunction::BossEnergyBar: {
      if (msg == EScriptObjectMessage::Increment)
        mgr.SetBossParams(uid, xfc_float1, u32(x100_float2) + 86);
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
            worldLayerState = mgr.WorldLayerState();
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
        mgr.GetEnvFxManager()->SetFxDensity(s32(x100_float2), xfc_float1);
      break;
    }
    case ESpecialFunction::RumbleEffect:
      if (msg == EScriptObjectMessage::Action) {
        s32 rumbFx = s32(x100_float2);
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
      if ((msg == EScriptObjectMessage::Deleted || msg == EScriptObjectMessage::Deactivate) && x1e4_31_inAreaDamage) {
        x1e4_31_inAreaDamage = false;
        mgr.GetPlayer().DecrementEnvironmentDamage();
        mgr.SetIsFullThreat(false);
      }
      break;
    }
    case ESpecialFunction::DropBomb: {
      if (msg == EScriptObjectMessage::Action) {
        if (xfc_float1 >= 1.f)
          mgr.GetPlayer().GetPlayerGun()->DropBomb(CPlayerGun::EBWeapon::PowerBomb, mgr);
        else
          mgr.GetPlayer().GetPlayerGun()->DropBomb(CPlayerGun::EBWeapon::Bomb, mgr);
      }
      break;
    }
    case ESpecialFunction::RedundantHintSystem: {
      CHintOptions& hintOptions = g_GameState->HintOptions();
      if (msg == EScriptObjectMessage::Action) {
        hintOptions.ActivateContinueDelayHintTimer(xec_locatorName);
      } else if (msg == EScriptObjectMessage::Increment) {
        hintOptions.ActivateImmediateHintTimer(xec_locatorName);
      } else if (msg == EScriptObjectMessage::Decrement) {
        hintOptions.DelayHint(xec_locatorName);
      }
      break;
    }
    case ESpecialFunction::Billboard: {
      if (msg == EScriptObjectMessage::Increment) {
        const SObjectTag* objectTag = g_ResFactory->GetResourceIdByName(xec_locatorName);
        CAssetId assetId = objectTag ? objectTag->id : CAssetId();

        mgr.SetPendingOnScreenTex(assetId, {int(x104_float3), int(x108_float4)}, {int(xfc_float1), int(x100_float2)});
        if (objectTag) {
          x1e8_ = g_SimplePool->GetObj(*objectTag);
          x1e5_26_displayBillboard = true;
        }
      } else if (msg == EScriptObjectMessage::Decrement) {
        mgr.SetPendingOnScreenTex({}, {int(x104_float3), int(x108_float4)}, {int(xfc_float1), int(x100_float2)});
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
        mgr.GetCameraManager()->SetFogDensity(x100_float2, xfc_float1);
      else if (msg == EScriptObjectMessage::Decrement)
        mgr.GetCameraManager()->SetFogDensity(x100_float2, 1.f);
      break;
    }
    case ESpecialFunction::EnterLogbook: {
      if (msg == EScriptObjectMessage::Action)
        mgr.DeferStateTransition(EStateManagerTransition::LogBook);
      break;
    }
    case ESpecialFunction::Ending: {
      if (msg == EScriptObjectMessage::Action && GetSpecialEnding(mgr) == u32(xfc_float1))
        SendScriptMsgs(EScriptObjectState::Zero, mgr, EScriptObjectMessage::None);
      break;
    }
    default:
      break;
    }
  }
}

void CScriptSpecialFunction::PreRender(CStateManager&, const zeus::CFrustum& frustum) {
  if (xe8_function != ESpecialFunction::FogVolume && xe8_function != ESpecialFunction::ViewFrustumTester)
    return;
  if (!GetActive())
    return;

  bool val;
  if (xe8_function == ESpecialFunction::FogVolume) {
    val = frustum.aabbFrustumTest(zeus::CAABox(GetTranslation() - x10c_vector3f, GetTranslation() + x10c_vector3f));
  } else {
    val = frustum.pointFrustumTest(GetTranslation());
  }

  if (x1e4_30_ == val)
    return;
  if (!val)
    x1e4_29_frustumExited = true;
  else
    x1e4_28_frustumEntered = true;
}

void CScriptSpecialFunction::AddToRenderer(const zeus::CFrustum&, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  if (xe8_function == ESpecialFunction::FogVolume && x1e4_30_) {
    EnsureRendered(mgr);
  }
}

void CScriptSpecialFunction::Render(CStateManager& mgr) {
  if (xe8_function == ESpecialFunction::FogVolume) {
    float z = mgr.IntegrateVisorFog(xfc_float1 * std::sin(CGraphics::GetSecondsMod900()));
    if (z > 0.f) {
      zeus::CVector3f max = GetTranslation() + x10c_vector3f;
      max.z() += z;
      zeus::CAABox box(GetTranslation() - x10c_vector3f, max);
      zeus::CTransform modelMtx = zeus::CTransform::Translate(box.center()) * zeus::CTransform::Scale(box.extents());
      g_Renderer->SetModelMatrix(modelMtx);
      g_Renderer->RenderFogVolume(x118_color, zeus::CAABox(-1.f, 1.f), nullptr, nullptr);
    }

  } else
    CActor::Render(mgr);
}

void CScriptSpecialFunction::SkipCinematic(CStateManager& stateMgr) {
  SendScriptMsgs(EScriptObjectState::Zero, stateMgr, EScriptObjectMessage::None);
  stateMgr.SetSkipCinematicSpecialFunction(kInvalidUniqueId);
}

void CScriptSpecialFunction::RingScramble(CStateManager& mgr) {
  SendScriptMsgs(EScriptObjectState::Zero, mgr, EScriptObjectMessage::None);
  x1a8_ringState = ERingState::Scramble;
  x1b8_ringReverse = !x1b8_ringReverse;
  float dir = (x1b8_ringReverse ? 1.f : -1.f);
  for (auto& rc : x198_ringControllers) {
    rc.x4_rotateSpeed = dir * mgr.GetActiveRandom()->Range(x100_float2, x104_float3);
    dir = -dir;
    rc.x8_reachedTarget = false;
  }
}

void CScriptSpecialFunction::ThinkIntroBossRingController(float dt, CStateManager& mgr) {
  if (x1a8_ringState != ERingState::Breakup) {
    for (const auto& rc : x198_ringControllers) {
      if (TCastToPtr<CActor> act = mgr.ObjectById(rc.x0_id)) {
        zeus::CTransform newXf = act->GetTransform();
        newXf.rotateLocalZ(zeus::degToRad(rc.x4_rotateSpeed * dt));
        act->SetTransform(newXf);
      }
    }
  }
  switch (x1a8_ringState) {
  case ERingState::Breakup: {
    float minMag = 0.f;
    for (const auto& rc : x198_ringControllers) {
      if (TCastToPtr<CActor> act = mgr.ObjectById(rc.x0_id)) {
        act->SetTranslation(act->GetTransform().basis[1] * 50.f * dt + act->GetTranslation());
        minMag = std::min(act->GetTranslation().magnitude(), minMag);
      }
    }
    CalculateRenderBounds();
    if (minMag != 0.f) {
      /* Never actually happens */
      for (const auto& rc : x198_ringControllers) {
        if (CEntity* ent = mgr.ObjectById(rc.x0_id))
          ent->SetActive(false);
      }
      SetActive(false);
    }
    break;
  }
  case ERingState::Rotate: {
    x1ac_ringRotateTarget =
        zeus::CQuaternion::fromAxisAngle(zeus::skUp, zeus::degToRad(xfc_float1 * (x1b8_ringReverse ? 1.f : -1.f) * dt))
            .transform(x1ac_ringRotateTarget);
    bool allReachedTarget = true;
    for (auto& rc : x198_ringControllers) {
      if (TCastToPtr<CActor> act = mgr.ObjectById(rc.x0_id)) {
        zeus::CVector3f lookDirFlat = act->GetTransform().basis[1];
        lookDirFlat.z() = 0.f;
        lookDirFlat.normalize();
        if (std::acos(zeus::clamp(-1.f, lookDirFlat.dot(x1ac_ringRotateTarget), 1.f)) <=
            zeus::degToRad((xfc_float1 + std::fabs(rc.x4_rotateSpeed)) / 30.f)) {
          zeus::CTransform newXf = zeus::lookAt(zeus::skZero3f, x1ac_ringRotateTarget);
          newXf.origin = act->GetTranslation();
          act->SetTransform(newXf);
          rc.x4_rotateSpeed = (x1b8_ringReverse ? 1.f : -1.f) * xfc_float1;
          rc.x8_reachedTarget = true;
        } else {
          allReachedTarget = false;
          break;
        }
      }
    }
    if (allReachedTarget) {
      SendScriptMsgs(EScriptObjectState::MaxReached, mgr, EScriptObjectMessage::None);
      x1a8_ringState = ERingState::Stopped;
      for (auto& rc : x198_ringControllers)
        rc.x8_reachedTarget = false;
    }
    break;
  }
  default:
    break;
  }
}

void CScriptSpecialFunction::ThinkPlayerFollowLocator(float, CStateManager& mgr) {
  for (const SConnection& conn : GetConnectionList()) {
    if (conn.x0_state == EScriptObjectState::Play && conn.x4_msg == EScriptObjectMessage::Activate) {
      auto search = mgr.GetIdListForScript(conn.x8_objId);
      for (auto it = search.first; it != search.second; ++it) {
        if (TCastToConstPtr<CActor> act = mgr.GetObjectById(it->second)) {
          zeus::CTransform xf = act->GetTransform() * act->GetLocatorTransform(xec_locatorName);
          CPlayer& pl = mgr.GetPlayer();
          pl.SetTransform(xf);
          pl.SetVelocityWR({});
          pl.SetAngularVelocityWR({});
          pl.ClearForcesAndTorques();
          return;
        }
      }
    }
  }
}

void CScriptSpecialFunction::ThinkSpinnerController(float dt, CStateManager& mgr, ESpinnerControllerMode mode) {
  bool allowWrap = xec_locatorName.find("AllowWrap") != std::string::npos;
  bool noBackward = xec_locatorName.find("NoBackward") != std::string::npos;
  float pointOneByDt = 0.1f * dt;
  float twoByDt = 2.f * dt;

  for (const SConnection& conn : x20_conns) {
    if (conn.x0_state != EScriptObjectState::Play || conn.x4_msg != EScriptObjectMessage::Activate)
      continue;

    auto search = mgr.GetIdListForScript(conn.x8_objId);
    for (auto it = search.first; it != search.second; ++it) {
      if (TCastToPtr<CScriptPlatform> plat = mgr.ObjectById((*it).second)) {
        if (plat->HasModelData() && plat->GetModelData()->HasAnimData()) {
          plat->SetControlledAnimation(true);
          if (!x1e4_24_) {
            x13c_ = plat->GetTransform();
            x1e4_24_ = true;
          }

          float f28 = x138_;
          float f29 = pointOneByDt * x100_float2;

          if (mode == ESpinnerControllerMode::Zero) {
            if (x1e4_25_spinnerCanMove) {
              CPlayer& pl = mgr.GetPlayer();
              zeus::CVector3f angVel = pl.GetAngularVelocityOR().getVector();
              float mag = 0.f;
              if (angVel.canBeNormalized())
                mag = angVel.magnitude();

              float spinImpulse =
                  (pl.GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Morphed ? 0.025f * mag : 0.f);
              if (spinImpulse >= x180_)
                SendScriptMsgs(EScriptObjectState::Play, mgr, EScriptObjectMessage::None);

              x180_ = spinImpulse;
              x138_ += 0.01f * spinImpulse * xfc_float1;

              if (!noBackward)
                x138_ -= f29;
            } else if (!noBackward) {
              x138_ = f28 - twoByDt;
            }
          } else if (mode == ESpinnerControllerMode::One) {
            x138_ = (0.1f * x16c_) * xfc_float1 + f28;

            if (!noBackward) {
              x138_ -= f29;

              if (std::fabs(x16c_) < dt)
                x16c_ = 0.f;
              else
                x16c_ -= (dt * (x16c_ <= 0.f ? -1.f : 1.f));
            }
          }

          if (allowWrap) {
            x138_ = std::fmod(x138_, 1.f);
            if (x138_ < 0.f)
              x138_ += 1.f;
          } else {
            x138_ = zeus::clamp(0.f, x138_, 1.f);
          }

          bool r23 = true;
          f28 = x138_ - f28;
          if (zeus::close_enough(x138_, 1.f, FLT_EPSILON)) {
            if (!x1e4_27_sfx3Played) {
              if (x174_sfx3 != 0xFFFF)
                CSfxManager::AddEmitter(x174_sfx3, GetTranslation(), {}, true, false, 0x7F, kInvalidAreaId);

              x1e4_27_sfx3Played = true;
            }

            SendScriptMsgs(EScriptObjectState::MaxReached, mgr, EScriptObjectMessage::None);
            r23 = false;
          } else
            x1e4_27_sfx3Played = false;

          if (zeus::close_enough(x138_, 0.f, FLT_EPSILON)) {
            if (!x1e4_26_sfx2Played) {
              if (x172_sfx2 != 0xFFFF)
                CSfxManager::AddEmitter(x172_sfx2, GetTranslation(), {}, true, false, 0x7F, kInvalidAreaId);

              x1e4_26_sfx2Played = true;
            }

            SendScriptMsgs(EScriptObjectState::Zero, mgr, EScriptObjectMessage::None);
            r23 = false;
          } else
            x1e4_26_sfx2Played = false;

          // local_1ac = x184_.GetAverage();
          if (r23) {
            if (x170_sfx1 != 0xFFFF) {
              if (r23) { // ?
                x184_.AddValue(0.f <= f28 ? 100 : 0x7f);
              } else {
                x184_.AddValue(0.f);
              }
              const std::optional<float>& avg = x184_.GetAverage();
              AddOrUpdateEmitter(0.f <= f28 ? x108_float4 : 1.f, x178_sfxHandle, x170_sfx1, GetTranslation(),
                                 avg.value());
            }
          } else {
            DeleteEmitter(x178_sfxHandle);
          }

          CAnimData* animData = plat->GetModelData()->GetAnimationData();
          float dur = animData->GetAnimationDuration(animData->GetDefaultAnimation()) * x138_;
          animData->SetPhase(0.f);
          animData->SetPlaybackRate(1.f);
          const SAdvancementDeltas& deltas = plat->UpdateAnimation(dur, mgr, true);
          plat->SetTransform(x13c_ * deltas.xc_rotDelta.toTransform(deltas.x0_posDelta));
        }
      }
    }
  }
}

void CScriptSpecialFunction::ThinkObjectFollowLocator(float, CStateManager& mgr) {
  TUniqueId followerAct = kInvalidUniqueId;
  TUniqueId followedAct = kInvalidUniqueId;
  for (const SConnection& conn : x20_conns) {
    if (conn.x0_state != EScriptObjectState::Play ||
        (conn.x4_msg != EScriptObjectMessage::Activate && conn.x4_msg != EScriptObjectMessage::Deactivate))
      continue;

    auto search = mgr.GetIdListForScript(conn.x8_objId);
    for (auto it = search.first; it != search.second; ++it) {
      if (TCastToConstPtr<CActor> act = mgr.GetObjectById(it->second)) {
        if (conn.x4_msg == EScriptObjectMessage::Activate &&
            (act->HasModelData() && act->GetModelData()->HasAnimData()) && act->GetActive()) {
          followedAct = it->second;
        } else if (conn.x4_msg == EScriptObjectMessage::Deactivate) {
          followerAct = it->second;
        }
      }
    }
  }

  if (followerAct == kInvalidUniqueId || followedAct == kInvalidUniqueId)
    return;

  TCastToConstPtr<CActor> fromAct = mgr.GetObjectById(followedAct);
  TCastToPtr<CActor> toAct = mgr.ObjectById(followerAct);
  toAct->SetTransform(fromAct->GetTransform() * fromAct->GetScaledLocatorTransform(xec_locatorName));
}

void CScriptSpecialFunction::ThinkObjectFollowObject(float, CStateManager& mgr) {
  TUniqueId followerAct = kInvalidUniqueId;
  TUniqueId followedAct = kInvalidUniqueId;
  for (const SConnection& conn : x20_conns) {
    if (conn.x0_state != EScriptObjectState::Play ||
        (conn.x4_msg != EScriptObjectMessage::Activate && conn.x4_msg != EScriptObjectMessage::Deactivate))
      continue;

    auto search = mgr.GetIdListForScript(conn.x8_objId);
    for (auto it = search.first; it != search.second; ++it) {
      if (TCastToConstPtr<CActor> act = mgr.GetObjectById(it->second)) {
        if (conn.x4_msg == EScriptObjectMessage::Activate && act->GetActive()) {
          followedAct = it->second;
        } else if (conn.x4_msg == EScriptObjectMessage::Deactivate) {
          followerAct = it->second;
        }
      }
    }
  }

  TCastToConstPtr<CActor> followed = mgr.GetObjectById(followedAct);
  TCastToPtr<CActor> follower = mgr.ObjectById(followerAct);
  if (followed && follower)
    follower->SetTransform(followed->GetTransform());
}

void CScriptSpecialFunction::ThinkChaffTarget(float dt, CStateManager& mgr) {
  zeus::CAABox box(5.f - GetTranslation(), 5.f + GetTranslation());
  rstl::reserved_vector<TUniqueId, 1024> nearList;
  mgr.BuildNearList(nearList, box, CMaterialFilter::MakeInclude({EMaterialTypes::Projectile}), nullptr);
  CCameraFilterPassPoly& filter = mgr.GetCameraFilterPass(7);

  for (TUniqueId uid : nearList) {
    if (TCastToPtr<CEnergyProjectile> proj = mgr.ObjectById(uid)) {
      if (proj->GetHomingTargetId() == GetUniqueId()) {
        proj->Set3d0_26(true);
        if (mgr.GetPlayer().GetAreaIdAlways() == GetAreaIdAlways()) {
          mgr.GetPlayer().SetHudDisable(x100_float2, 0.5f, 2.5f);
          filter.SetFilter(EFilterType::Blend, EFilterShape::Fullscreen, 0.f, zeus::skWhite, CAssetId());
          filter.DisableFilter(0.1f);
        }
      }
    }
  }

  x194_ = zeus::max(0.f, x194_ - dt);
  if (x194_ != 0.f && mgr.GetPlayer().GetAreaIdAlways() == GetAreaIdAlways()) {
    float intfMag = x104_float3 * (0.5f + ((0.5f + x194_) / xfc_float1));
    if (x194_ < 1.f)
      intfMag *= x194_;

    mgr.GetPlayerState()->GetStaticInterference().AddSource(GetUniqueId(), intfMag, .5f);

    if (mgr.GetPlayerState()->GetCurrentVisor() != CPlayerState::EPlayerVisor::Scan)
      mgr.GetPlayer().AddOrbitDisableSource(mgr, GetUniqueId());
    else
      mgr.GetPlayer().RemoveOrbitDisableSource(GetUniqueId());
  }
}

void CScriptSpecialFunction::ThinkActorScale(float dt, CStateManager& mgr) {
  float deltaScale = dt * xfc_float1;

  for (const SConnection& conn : x20_conns) {
    if (conn.x0_state != EScriptObjectState::Play || conn.x4_msg != EScriptObjectMessage::Activate)
      continue;
    if (TCastToPtr<CActor> act = mgr.ObjectById(mgr.GetIdForScript(conn.x8_objId))) {
      CModelData* mData = act->GetModelData();
      if (mData && (mData->HasAnimData() || mData->HasNormalModel())) {
        zeus::CVector3f scale = mData->GetScale();

        if (deltaScale > 0.f)
          scale = zeus::min(zeus::CVector3f(deltaScale) + scale, zeus::CVector3f(x100_float2));
        else
          scale = zeus::max(zeus::CVector3f(deltaScale) + scale, zeus::CVector3f(x100_float2));

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

void CScriptSpecialFunction::ThinkAreaDamage(float dt, CStateManager& mgr) {
  const auto& playerState = mgr.GetPlayerState();
  CPlayer& player = mgr.GetPlayer();
  /* The following check is a URDE addition */
  if (!playerState->CanTakeDamage()) {
    /* Make sure we're not currently set to take damage, if so reset our state to be as if we're not */
    if (x1e4_31_inAreaDamage) {
      x1e4_31_inAreaDamage = false;
      player.DecrementEnvironmentDamage();
      SendScriptMsgs(EScriptObjectState::Exited, mgr, EScriptObjectMessage::None);
      mgr.SetIsFullThreat(false);
    }
    return;
  }
  /* End URDE Addition */

  if (!x1e4_31_inAreaDamage) {
    if (mgr.GetPlayer().GetAreaIdAlways() != GetAreaIdAlways() ||
        playerState->GetCurrentSuitRaw() != CPlayerState::EPlayerSuit::Power)
      return;
    x1e4_31_inAreaDamage = true;
    player.IncrementEnvironmentDamage();
    SendScriptMsgs(EScriptObjectState::Entered, mgr, EScriptObjectMessage::None);
    mgr.SetIsFullThreat(true);
  } else if (mgr.GetPlayer().GetAreaIdAlways() != GetAreaIdAlways() ||
             playerState->GetCurrentSuitRaw() != CPlayerState::EPlayerSuit::Power) {
    x1e4_31_inAreaDamage = false;
    player.DecrementEnvironmentDamage();
    SendScriptMsgs(EScriptObjectState::Exited, mgr, EScriptObjectMessage::None);
    mgr.SetIsFullThreat(false);
    return;
  }

  CDamageInfo dInfo(CWeaponMode(EWeaponType::Heat), xfc_float1 * dt, 0.f, 0.f);
  dInfo.SetNoImmunity(true);
  mgr.ApplyDamage(GetUniqueId(), player.GetUniqueId(), GetUniqueId(), dInfo,
                  CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {}), {});
}

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

void CScriptSpecialFunction::AddOrUpdateEmitter(float pitch, CSfxHandle& handle, u16 id, const zeus::CVector3f& pos,
                                                float vol) {
  if (!handle) {
    handle = CSfxManager::AddEmitter(id, pos, zeus::skZero3f, vol, true, true, 0x7f, kInvalidAreaId);
  } else {
    CSfxManager::UpdateEmitter(handle, pos, zeus::skZero3f, vol);
    CSfxManager::PitchBend(handle, 8192.f * pitch + 8192.f);
  }
}

CScriptSpecialFunction::SRingController::SRingController(TUniqueId uid, float rotateSpeed, bool reachedTarget)
: x0_id(uid), x4_rotateSpeed(rotateSpeed), x8_reachedTarget(reachedTarget) {}

} // namespace urde
