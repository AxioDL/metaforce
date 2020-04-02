#include "Runtime/MP1/World/COmegaPirate.hpp"

#include "Runtime/Collision/CCollisionActor.hpp"
#include "Runtime/Collision/CCollisionActorManager.hpp"
#include "Runtime/CSimplePool.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Graphics/CBooRenderer.hpp"
#include "Runtime/Weapon/CGameProjectile.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "Runtime/World/CScriptEffect.hpp"
#include "Runtime/World/CScriptPlatform.hpp"
#include "Runtime/World/CScriptSound.hpp"
#include "Runtime/World/CScriptWaypoint.hpp"
#include "Runtime/World/CWorld.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde::MP1 {
namespace {
constexpr std::array<SSphereJointInfo, 1> skSphereJoints{{
    {"lockon_target_LCTR", 1.f},
}};

constexpr std::array<SOBBJointInfo, 11> skObbJoints{{
    {"Spine_2", "Collar", zeus::skOne3f},
    {"R_toe_1", "R_ankle", zeus::skOne3f},
    {"L_toe_1", "L_ankle", zeus::skOne3f},
    {"R_knee", "R_ankle", zeus::skOne3f},
    {"L_knee", "L_ankle", zeus::skOne3f},
    {"R_elbow", "R_wrist", zeus::skOne3f},
    {"L_elbow", "L_wrist", zeus::skOne3f},
    {"R_wrist", "R_index_1", zeus::skOne3f},
    {"L_wrist", "L_index_1", zeus::skOne3f},
    {"R_index_1", "R_index_3_SDK", zeus::CVector3f{2.f}},
    {"L_index_1", "L_index_3_SDK", zeus::CVector3f{2.f}},
}};
} // namespace

COmegaPirate::CFlash::CFlash(TUniqueId uid, const CEntityInfo& info, const zeus::CVector3f& pos,
                             TToken<CTexture>& thermalSpot, float p5)
: CActor(uid, true, "Omega Pirate Flash", info, zeus::CTransform::Translate(pos), CModelData::CModelDataNull(), {},
         CActorParameters::None(), kInvalidUniqueId)
, xe8_thermalSpot(thermalSpot)
, xf4_(p5) {}

void COmegaPirate::CFlash::Accept(IVisitor& visitor) { visitor.Visit(this); }

COmegaPirate::COmegaPirate(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                           CModelData&& mData, const CPatternedInfo& pInfo, const CActorParameters& actParms,
                           CElitePirateData data, CAssetId w1, CAssetId w2, CAssetId w3)
: CElitePirate(uid, name, info, xf, std::move(mData), pInfo, actParms, data)
, x9d0_(GetModelData()->GetScale())
, x9f0_(*g_SimplePool, w1, w2, w3, 0, 0)
, xb70_thermalSpot(g_SimplePool->GetObj("Thermal_Spot_2"sv)) {
  x9a4_scriptWaypointPlatforms.reserve(3);
  x9b8_scriptEffects.reserve(24);
  x9dc_scriptPlatforms.reserve(4);
  xaa0_scriptSounds.reserve(4);
  xab4_.reserve(3);
  xb7c_.resize(4, 0);

  SetMass(100000.f);

  CMaterialFilter filter = GetMaterialFilter();
  filter.ExcludeList().Add(
      CMaterialList{EMaterialTypes::Character, EMaterialTypes::CollisionActor, EMaterialTypes::Platform});
  SetMaterialFilter(filter);

  GetSearchPath()->SetPadding(20.f);
}

void COmegaPirate::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  switch (msg) {
  default:
    CElitePirate::AcceptScriptMsg(msg, uid, mgr);
    break;
  case EScriptObjectMessage::Activate:
    CElitePirate::AcceptScriptMsg(msg, uid, mgr);
    xa38_collisionActorMgr1->SetActive(mgr, true);
    xa9c_collisionActorMgr2->SetActive(mgr, true);
    GetKnockBackController().SetAutoResetImpulse(false);
    if (auto* entity = mgr.ObjectById(x990_launcherId2)) {
      entity->SetActive(true);
    }
    break;
  case EScriptObjectMessage::Deactivate:
    CElitePirate::AcceptScriptMsg(msg, uid, mgr);
    xa38_collisionActorMgr1->SetActive(mgr, false);
    xa9c_collisionActorMgr2->SetActive(mgr, false);
    if (auto* entity = mgr.ObjectById(x990_launcherId2)) {
      entity->SetActive(false);
    }
    break;
  case EScriptObjectMessage::Decrement:
    x9ec_ = true;
    break;
  case EScriptObjectMessage::Increment:
    SetShotAt(true, mgr);
    break;
  case EScriptObjectMessage::Open:
    xb7c_[3] -= xb7c_[3] == 0 ? 0 : 1;
    break;
  case EScriptObjectMessage::Reset:
    xb78_ = true;
    break;
  case EScriptObjectMessage::SetToMax:
    xa3c_hearPlayer = true;
    break;
  case EScriptObjectMessage::SetToZero:
    xb7c_[2] -= xb7c_[2] == 0 ? 0 : 1;
    break;
  case EScriptObjectMessage::Start:
    x3b4_speed = 1.f;
    ++xade_;
    if (xade_ < 4) {
      GetBodyController()->GetCommandMgr().DeliverCmd(CBCKnockBackCmd(zeus::skLeft, pas::ESeverity::One));
    }
    break;
  case EScriptObjectMessage::Stop:
    SetupCollisionManager(mgr);
    break;
  case EScriptObjectMessage::StopAndReset:
    xb7c_[1] -= xb7c_[1] == 0 ? 0 : 1;
    break;
  case EScriptObjectMessage::UNKM18:
    xb7c_[0] -= xb7c_[0] == 0 ? 0 : 1;
    break;
  case EScriptObjectMessage::Action:
    x3b4_speed = 1.f;
    ++xade_;
    if (xade_ < 4) {
      GetBodyController()->GetCommandMgr().DeliverCmd(CBCKnockBackCmd(zeus::skRight, pas::ESeverity::One));
    }
    break;
  case EScriptObjectMessage::Alert:
    CElitePirate::AcceptScriptMsg(msg, uid, mgr);
    break;
  case EScriptObjectMessage::Touched:
    CElitePirate::AcceptScriptMsg(msg, uid, mgr);
    if (uid == x990_launcherId2 && x990_launcherId2 != kInvalidUniqueId) {
      SetShotAt(true, mgr);
    }
    if (TCastToPtr<CCollisionActor> actor = mgr.ObjectById(uid)) {
      if (TCastToPtr<CPlayer> player = mgr.ObjectById(actor->GetLastTouchedObject())) {
        if (x420_curDamageRemTime <= 0.f) {
          mgr.ApplyDamage(GetUniqueId(), player->GetUniqueId(), GetUniqueId(), GetContactDamage(),
                          CMaterialFilter::MakeInclude({EMaterialTypes::Solid}), zeus::skZero3f);
          x420_curDamageRemTime = x424_damageWaitTime;
        }
      }
    }
    break;
  case EScriptObjectMessage::Registered:
    CElitePirate::AcceptScriptMsg(msg, uid, mgr);
    x990_launcherId2 = mgr.AllocateUniqueId();
    CreateGrenadeLauncher(mgr, x990_launcherId2);
    SetupCollisionManager(mgr);
    GetBodyController()->SetLocomotionType(pas::ELocomotionType::Internal8);
    x402_27_noXrayModel = false;
    xa4c_initialXf = x34_transform;
    xa98_maxEnergy = HealthInfo(mgr)->GetHP();
    if (auto* actor = static_cast<CActor*>(mgr.ObjectById(GetLauncherId()))) {
      actor->RemoveMaterial(EMaterialTypes::Scannable, mgr);
    }
    if (auto* actor = static_cast<CActor*>(mgr.ObjectById(x990_launcherId2))) {
      actor->RemoveMaterial(EMaterialTypes::Scannable, mgr);
    }
    GetKnockBackController().SetAutoResetImpulse(false);
    SetupPathFindSearch();
    break;
  case EScriptObjectMessage::Deleted:
    CElitePirate::AcceptScriptMsg(msg, uid, mgr);
    xa38_collisionActorMgr1->Destroy(mgr);
    xa9c_collisionActorMgr2->Destroy(mgr);
    mgr.FreeScriptObject(x990_launcherId2);
    break;
  case EScriptObjectMessage::InitializedInArea:
    CElitePirate::AcceptScriptMsg(msg, uid, mgr);

    for (const SConnection& conn : GetConnectionList()) {
      TUniqueId connId = mgr.GetIdForScript(conn.x8_objId);
      if (connId == kInvalidUniqueId || conn.x0_state != EScriptObjectState::Attack) {
        continue;
      }

      if (conn.x4_msg == EScriptObjectMessage::Activate) {
        if (TCastToPtr<CScriptEffect> effect = mgr.ObjectById(connId)) {
          x9b8_scriptEffects.emplace_back(connId, effect->GetName());
        } else if (TCastToPtr<CScriptPlatform> platform = mgr.ObjectById(connId)) {
          x9dc_scriptPlatforms.emplace_back(connId, platform->GetName());
          platform->AddMaterial(EMaterialTypes::Target, EMaterialTypes::Orbit, EMaterialTypes::Character, mgr);
          platform->RemoveMaterial(EMaterialTypes::Scannable, mgr);
          CMaterialList excludes = platform->GetMaterialFilter().GetExcludeList();
          excludes.Add({EMaterialTypes::Player, EMaterialTypes::Character, EMaterialTypes::CollisionActor});
          CMaterialList includes = GetMaterialFilter().GetIncludeList();
          platform->SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(includes, excludes));
          xae4_platformVuln = *platform->GetDamageVulnerability();
          xb54_platformColor = platform->GetDrawFlags().x4_color; // TODO does this work?
        } else if (TCastToPtr<CScriptSound> sound = mgr.ObjectById(connId)) {
          xaa0_scriptSounds.emplace_back(connId, sound->GetName());
        }
      } else if (conn.x4_msg == EScriptObjectMessage::Follow) {
        if (TCastToPtr<CScriptWaypoint> waypoint = mgr.ObjectById(connId)) {
          std::vector<TUniqueId> waypointPlatformIds;
          waypointPlatformIds.reserve(3);
          for (const SConnection& waypointConn : waypoint->GetConnectionList()) {
            auto waypointConnId = mgr.GetIdForScript(waypointConn.x8_objId);
            if (TCastToPtr<CScriptPlatform> platform = mgr.ObjectById(waypointConnId)) {
              platform->AddMaterial(EMaterialTypes::Target, EMaterialTypes::Orbit, mgr);
              waypointPlatformIds.push_back(waypointConnId);
            }
          }
          x9a4_scriptWaypointPlatforms.emplace_back(connId, waypointPlatformIds);
        }
      }
    }
    break;
  case EScriptObjectMessage::Damage:
    if (uid == x990_launcherId2 && x990_launcherId2 != kInvalidUniqueId) {
      GetBodyController()->GetCommandMgr().DeliverCmd(
          CBCKnockBackCmd(GetTransform().frontVector(), pas::ESeverity::Eight));
    }
    CElitePirate::AcceptScriptMsg(msg, uid, mgr);
    if (uid == xa46_ && xa7c_ == 2) {
      xa7c_ = 3;
      xa84_ = 0.f;
    }
    break;
  case EScriptObjectMessage::InvulnDamage:
    if (const TCastToConstPtr<CGameProjectile> projectile = mgr.GetObjectById(uid)) {
      if (xa4a_) {
        mgr.ApplyDamage(uid, xa46_, projectile->GetOwnerId(), projectile->GetDamageInfo(),
                        CMaterialFilter::MakeInclude({EMaterialTypes::Solid}), zeus::skZero3f);
      }
    }
    SetShotAt(true, mgr);
  }
}

bool COmegaPirate::AggressionCheck(CStateManager& mgr, float arg) {
  return x990_launcherId2 == kInvalidUniqueId && CElitePirate::AggressionCheck(mgr, arg);
}

void COmegaPirate::Attack(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    x402_28_isMakingBigStrike = true;
    x504_damageDur = 1.f;
  } else if (msg == EStateMsg::Deactivate) {
    x402_28_isMakingBigStrike = false;
    x504_damageDur = 0.f;
  }
  CElitePirate::Attack(mgr, msg, dt);
}

bool COmegaPirate::CodeTrigger(CStateManager&, float) { return xb78_; }

void COmegaPirate::Cover(CStateManager& mgr, EStateMsg msg, float dt) {
  CElitePirate::Cover(mgr, msg, dt);
  if (msg == EStateMsg::Activate) {
    xad4_cachedSpeed = x3b4_speed;
    xad8_cover = true;
  } else if (msg == EStateMsg::Deactivate) {
    xad8_cover = false;
  }
}

bool COmegaPirate::CoverBlown(CStateManager&, float) {
  if (x9b4_) {
    x9b4_ = false;
    xb5c_ = 0.f;
    return true;
  }
  return false;
}

void COmegaPirate::Dizzy(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    xa44_ = true;
  } else if (msg == EStateMsg::Update) {
    GetBodyController()->GetCommandMgr().DeliverCmd(CBCLoopReactionCmd(pas::EReactionType::Two));
  } else if (msg == EStateMsg::Deactivate) {
    GetBodyController()->GetCommandMgr().DeliverCmd(CBodyStateCmd(EBodyStateCmd::ExitState));
  }
}

void COmegaPirate::DoubleSnap(CStateManager& mgr, EStateMsg msg, float dt) {
  // TODO
}

void COmegaPirate::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type, float dt) {
  switch (type) {
  case EUserEventType::EggLay:
    if (x990_launcherId2 != kInvalidUniqueId) {
      if (auto entity = mgr.ObjectById(x990_launcherId2)) {
        mgr.SendScriptMsg(entity, GetUniqueId(), EScriptObjectMessage::Action);
      }
    }
    break;
  case EUserEventType::FadeIn:
    x9a1_ = true;
    break;
  case EUserEventType::FadeOut:
    if (x994_ != 2 && x9a1_) {
      x994_ = 1;
      xa30_ = 3;
    }
    break;
  case EUserEventType::ObjectPickUp:
    // TODO
    break;
  case EUserEventType::Projectile:
  case EUserEventType::DamageOn:
  case EUserEventType::DamageOff:
  case EUserEventType::ScreenShake:
  case EUserEventType::BeginAction:
  case EUserEventType::BecomeShootThrough:
  default:
    CElitePirate::DoUserAnimEvent(mgr, node, type, dt);
  }
}

void COmegaPirate::Enraged(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    SetState(EState::Zero);
  } else if (msg == EStateMsg::Update) {
    if (GetState() == CElitePirate::EState::Zero) {
      if (GetBodyController()->GetCurrentStateId() == pas::EAnimationState::Taunt) {
        SetState(CElitePirate::EState::Two);
      } else {
        GetBodyController()->GetCommandMgr().DeliverCmd(CBCTauntCmd(pas::ETauntType::Zero));
      }
    } else if (GetState() == CElitePirate::EState::Two &&
               GetBodyController()->GetCurrentStateId() != pas::EAnimationState::Taunt) {
      SetState(CElitePirate::EState::Over);
    }
  } else if (msg == EStateMsg::Deactivate) {
    xadf_ = true;
    xae0_ = true;
  }
}

void COmegaPirate::Explode(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    SetState(EState::Zero);
    xad0_ = false;
  } else if (msg == EStateMsg::Update) {
    if (GetState() == CElitePirate::EState::Zero) {
      if (GetBodyController()->GetCurrentStateId() == pas::EAnimationState::Step) {
        SetState(CElitePirate::EState::Two);
      } else {
        GetBodyController()->GetCommandMgr().DeliverCmd(
            CBCStepCmd(pas::EStepDirection::Forward, pas::EStepType::Dodge));
      }
    } else if (GetState() == CElitePirate::EState::Two &&
               GetBodyController()->GetCurrentStateId() != pas::EAnimationState::Step) {
      SetState(CElitePirate::EState::Over);
    }
  } else if (msg == EStateMsg::Deactivate) {
    GetBodyController()->SetLocomotionType(xa40_);
  }
}

void COmegaPirate::Faint(CStateManager& mgr, EStateMsg msg, float dt) {
  // TODO
}

void COmegaPirate::Growth(CStateManager& mgr, EStateMsg msg, float dt) {
  // TODO
}

void COmegaPirate::JumpBack(CStateManager& mgr, EStateMsg msg, float dt) {
  // TODO
}

bool COmegaPirate::Landed(CStateManager& mgr, float arg) { return (xb4c_ & 0xe7) == 0; }

zeus::CVector3f COmegaPirate::GetOrbitPosition(const CStateManager& mgr) const {
  if (x990_launcherId2 != kInvalidUniqueId &&
      mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::EPlayerVisor::Thermal) {
    if (const auto actor = static_cast<const CActor*>(mgr.GetObjectById(x990_launcherId2))) {
      return GetLockOnPosition(actor);
    }
  }
  return CElitePirate::GetOrbitPosition(mgr);
}

bool COmegaPirate::HearPlayer(CStateManager& mgr, float arg) { return xa3c_hearPlayer; }

void COmegaPirate::PathFind(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    xad4_cachedSpeed = x3b4_speed;
    x3b4_speed = 1.4f * xad4_cachedSpeed;
  } else if (msg == EStateMsg::Update) {
    if (GetBodyController()->GetCurrentStateId() == pas::EAnimationState::KnockBack) {
      x3b4_speed = xad4_cachedSpeed;
    } else if (xad4_cachedSpeed == x3b4_speed) {
      x3b4_speed = 1.4f * xad4_cachedSpeed;
    }
  } else if (msg == EStateMsg::Deactivate) {
    x3b4_speed = xad4_cachedSpeed;
  }
  CElitePirate::PathFind(mgr, msg, dt);
}

void COmegaPirate::PreRender(CStateManager& mgr, const zeus::CFrustum& frustum) {
  CElitePirate::PreRender(mgr, frustum);
  if (mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::EPlayerVisor::XRay) {
    xb4_drawFlags = CModelFlags{1, 0, 3, zeus::CColor{xa80_}};
  }
}

void COmegaPirate::Render(const CStateManager& mgr) const {
  const auto* mData = GetModelData();
  auto* animData = const_cast<CAnimData*>(mData->GetAnimationData());

  CGraphics::SetModelMatrix(GetTransform() * zeus::CTransform::Scale(mData->GetScale()));

  if (mgr.GetPlayerState()->GetCurrentVisor() != CPlayerState::EPlayerVisor::XRay && xa2c_ > 0.f) {
    auto& model = const_cast<CSkinnedModel&>(x9f0_);
    const CModelFlags flags{5, 0, 3, zeus::CColor{1.f, xa2c_}};
    animData->Render(model, flags, std::nullopt, nullptr);
  }
  if (x9a0_) {
    bool isXRay = mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::EPlayerVisor::XRay;
    if (isXRay) {
      g_Renderer->SetWorldFog(ERglFogMode::None, 0.f, 1.f, zeus::skBlack);
      const CModelFlags flags{5, 0, 1, zeus::CColor{1.f, 0.2f}};
      auto& model = const_cast<CSkinnedModel&>(*animData->GetModelData().GetObj());
      animData->Render(model, flags, std::nullopt, nullptr);
    }
    CPatterned::Render(mgr);
    if (isXRay) {
      mgr.SetupFogForArea(GetAreaIdAlways());
    }
  }
}

void COmegaPirate::Retreat(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    SetShotAt(false, mgr);
    SetState(CElitePirate::EState::Zero);
    SendScriptMsgs(EScriptObjectState::Inside, mgr, EScriptObjectMessage::None);
    xad0_ = false;
    xa44_ = false;
    xa4a_ = false;
    xb5c_ = 0.f;
    xb60_ = 0.f;
    xb64_ = 5.f;
    ++xb68_;
  } else if (msg == EStateMsg::Update) {
    if (GetState() == CElitePirate::EState::Zero) {
      if (GetBodyController()->GetCurrentStateId() == pas::EAnimationState::Step) {
        SetState(CElitePirate::EState::Two);
      } else {
        GetBodyController()->GetCommandMgr().DeliverCmd(
            CBCStepCmd(pas::EStepDirection::Forward, pas::EStepType::BreakDodge));
      }
    } else if (GetState() == CElitePirate::EState::Two &&
               GetBodyController()->GetCurrentStateId() != pas::EAnimationState::Step) {
      SetState(CElitePirate::EState::Over);
    }
  }
}

void COmegaPirate::Run(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    xad4_cachedSpeed = x3b4_speed;
    x3b4_speed = 1.4f * xad4_cachedSpeed;
  } else if (msg == EStateMsg::Update) {
    if (GetBodyController()->GetCurrentStateId() == pas::EAnimationState::KnockBack) {
      x3b4_speed = xad4_cachedSpeed;
    } else if (xad4_cachedSpeed == x3b4_speed) {
      x3b4_speed = 1.4f * xad4_cachedSpeed;
    }
  } else if (msg == EStateMsg::Deactivate) {
    x3b4_speed = xad4_cachedSpeed;
  }
  CElitePirate::Run(mgr, msg, dt);
}

bool COmegaPirate::ShotAt(CStateManager& mgr, float arg) { return CElitePirate::ShotAt(mgr, arg); }

bool COmegaPirate::ShouldCallForBackup(CStateManager& mgr, float arg) {
  if (CElitePirate::ShouldCallForBackup(mgr, arg)) {
    return ShouldCallForBackupFromLauncher(mgr, x990_launcherId2);
  }
  return false;
}

bool COmegaPirate::ShouldFire(CStateManager& mgr, float arg) {
  if (CElitePirate::ShouldFire(mgr, arg)) {
    return ShouldFireFromLauncher(mgr, x990_launcherId2);
  }
  return false;
}

bool COmegaPirate::ShouldMove(CStateManager& mgr, float arg) { return xb64_ < x330_stateMachineState.GetTime(); }

void COmegaPirate::Shuffle(CStateManager& mgr, EStateMsg msg, float dt) {}

void COmegaPirate::Skid(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    SetState(EState::Zero);
  } else if (msg == EStateMsg::Update) {
    if (GetState() == CElitePirate::EState::Zero) {
      if (GetBodyController()->GetCurrentStateId() == pas::EAnimationState::Step) {
        SetState(CElitePirate::EState::Two);
      } else {
        GetBodyController()->GetCommandMgr().DeliverCmd(
            CBCStepCmd(pas::EStepDirection::Forward, pas::EStepType::Normal));
      }
    } else if (GetState() == CElitePirate::EState::Two &&
               GetBodyController()->GetCurrentStateId() != pas::EAnimationState::Step) {
      SetState(CElitePirate::EState::Over);
    }
  }
}

void COmegaPirate::Suck(CStateManager& mgr, EStateMsg msg, float dt) {
  // TODO
}

void COmegaPirate::TargetPatrol(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    xad4_cachedSpeed = x3b4_speed;
    x3b4_speed = 1.4f * xad4_cachedSpeed;
  } else if (msg == EStateMsg::Update) {
    if (GetBodyController()->GetCurrentStateId() == pas::EAnimationState::KnockBack) {
      x3b4_speed = xad4_cachedSpeed;
    } else if (xad4_cachedSpeed == x3b4_speed) {
      x3b4_speed = 1.4f * xad4_cachedSpeed;
    }
  } else if (msg == EStateMsg::Deactivate) {
    x3b4_speed = xad4_cachedSpeed;
  }
  CElitePirate::TargetPatrol(mgr, msg, dt);
}

void COmegaPirate::Think(float dt, CStateManager& mgr) {
  if (GetActive()) {
    SetAlert(true);
    CElitePirate::Think(dt, mgr);

    float maxHealth = xa98_maxEnergy;
    CHealthInfo* healthInfo = HealthInfo(mgr);
    if (healthInfo->GetHP() > 0.2f * maxHealth) {
      if (healthInfo->GetHP() > 0.7f * maxHealth) { // ??
        if (xacc_ > 4) {
          xac4_ = 1;
        }
      } else {
        xac4_ = 2;
      }
    } else {
      xac4_ = 3;
    }

    UpdateActorTransform(mgr, x990_launcherId2, "grenadeLauncher2_LCTR"sv);

    // TODO
  }
}

void COmegaPirate::WallDetach(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    SetState(CElitePirate::EState::Zero);
  } else if (msg == EStateMsg::Update) {
    if (GetState() == CElitePirate::EState::Zero) {
      if (GetBodyController()->GetCurrentStateId() == pas::EAnimationState::Step) {
        SetState(CElitePirate::EState::Two);
      } else {
        GetBodyController()->GetCommandMgr().DeliverCmd(CBCStepCmd(pas::EStepDirection::Left, pas::EStepType::Dodge));
      }
    } else if (GetState() == CElitePirate::EState::Two &&
               GetBodyController()->GetCurrentStateId() != pas::EAnimationState::Step) {
      SetState(CElitePirate::EState::Over);
    }
  } else if (msg == EStateMsg::Deactivate) {
    mgr.SetBossParams(GetUniqueId(), xa98_maxEnergy, 89);
    xb79_ = true;
  }
}

void COmegaPirate::WallHang(CStateManager& mgr, EStateMsg msg, float dt) {}

void COmegaPirate::SetupHealthInfo(CStateManager& mgr) {
  CElitePirate::SetupHealthInfo(mgr);
  SetupLauncherHealthInfo(mgr, x990_launcherId2);
}

void COmegaPirate::SetLaunchersActive(CStateManager& mgr, bool val) {
  CElitePirate::SetLaunchersActive(mgr, val);
  SetLauncherActive(mgr, val, x990_launcherId2);
}

void COmegaPirate::CreateFlash(CStateManager& mgr, float arg) {
  mgr.AddObject(new CFlash(mgr.AllocateUniqueId(), CEntityInfo{GetAreaIdAlways(), CEntity::NullConnectionList},
                           GetRenderBounds().center(), xb70_thermalSpot, arg));
}

void COmegaPirate::SetupCollisionManager(CStateManager& mgr) {
  std::vector<CJointCollisionDescription> sphereJoints;
  sphereJoints.reserve(skSphereJoints.size());
  AddSphereCollisionList(skSphereJoints.data(), skSphereJoints.size(), sphereJoints);
  xa38_collisionActorMgr1 =
      std::make_unique<CCollisionActorManager>(mgr, GetUniqueId(), GetAreaIdAlways(), sphereJoints, true);
  SetupCollisionActorInfo1(xa38_collisionActorMgr1, mgr);
  xa46_ = xa38_collisionActorMgr1->GetCollisionDescFromIndex(0).GetCollisionActorId();
  if (auto* actor = static_cast<CActor*>(mgr.ObjectById(xa46_))) {
    *actor->HealthInfo(mgr) = *HealthInfo(mgr);
  }

  std::vector<CJointCollisionDescription> obbJoints;
  obbJoints.reserve(skObbJoints.size());
  AddOBBAutoSizeCollisionList(skObbJoints.data(), skObbJoints.size(), obbJoints);
  xa9c_collisionActorMgr2 =
      std::make_unique<CCollisionActorManager>(mgr, GetUniqueId(), GetAreaIdAlways(), obbJoints, true);
  SetupCollisionActorInfo2(xa9c_collisionActorMgr2, mgr);
  xa48_ = xa9c_collisionActorMgr2->GetCollisionDescFromIndex(0).GetCollisionActorId();
}

void COmegaPirate::AddSphereCollisionList(const SSphereJointInfo* joints, size_t count,
                                          std::vector<CJointCollisionDescription>& outJoints) const {
  const CAnimData* animData = GetModelData()->GetAnimationData();
  for (size_t i = 0; i < count; ++i) {
    const auto& joint = joints[i];
    const CSegId seg = animData->GetLocatorSegId(joint.name);
    if (seg.IsInvalid()) {
      continue;
    }
    outJoints.emplace_back(CJointCollisionDescription::SphereCollision(seg, joint.radius, joint.name, 0.001f));
  }
}

void COmegaPirate::AddOBBAutoSizeCollisionList(const SOBBJointInfo* joints, size_t count,
                                               std::vector<CJointCollisionDescription>& outJoints) const {
  const CAnimData* animData = GetModelData()->GetAnimationData();
  for (size_t i = 0; i < count; ++i) {
    const auto& joint = joints[i];
    const CSegId from = animData->GetLocatorSegId(joint.from);
    const CSegId to = animData->GetLocatorSegId(joint.to);
    if (to.IsInvalid() || from.IsInvalid()) {
      continue;
    }
    outJoints.emplace_back(CJointCollisionDescription::OBBAutoSizeCollision(
        from, to, joint.bounds, CJointCollisionDescription::EOrientationType::One,
        "Omega_Pirate_OBB_"s + std::to_string(i), 0.001f));
  }
}

void COmegaPirate::SetupCollisionActorInfo1(const std::unique_ptr<CCollisionActorManager>& actMgr, CStateManager& mgr) {
  for (size_t i = 0; i < actMgr->GetNumCollisionActors(); ++i) {
    const auto& colDesc = actMgr->GetCollisionDescFromIndex(i);
    const TUniqueId uid = colDesc.GetCollisionActorId();
    if (TCastToPtr<CCollisionActor> act = mgr.ObjectById(uid)) {
      act->AddMaterial(EMaterialTypes::ScanPassthrough, EMaterialTypes::CameraPassthrough, EMaterialTypes::AIJoint,
                       EMaterialTypes::Immovable, mgr);
      const CMaterialFilter& selfFilter = GetMaterialFilter();
      const CMaterialFilter& actFilter = act->GetMaterialFilter();
      CMaterialFilter filter =
          CMaterialFilter::MakeIncludeExclude(selfFilter.GetIncludeList(), selfFilter.GetExcludeList());
      filter.IncludeList().Add(actFilter.GetIncludeList());
      filter.ExcludeList().Add(actFilter.GetExcludeList());
      filter.ExcludeList().Add(EMaterialTypes::Platform); // ?
      act->SetMaterialFilter(filter);
      act->RemoveMaterial(EMaterialTypes::ProjectilePassthrough, mgr);
    }
  }
}

void COmegaPirate::SetupCollisionActorInfo2(const std::unique_ptr<CCollisionActorManager>& actMgr, CStateManager& mgr) {
  for (size_t i = 0; i < actMgr->GetNumCollisionActors(); ++i) {
    const auto& colDesc = actMgr->GetCollisionDescFromIndex(i);
    const TUniqueId uid = colDesc.GetCollisionActorId();
    if (TCastToPtr<CCollisionActor> act = mgr.ObjectById(uid)) {
      act->AddMaterial(EMaterialTypes::ScanPassthrough, EMaterialTypes::CameraPassthrough, EMaterialTypes::AIJoint,
                       EMaterialTypes::Immovable, mgr);
      const CMaterialFilter& selfFilter = GetMaterialFilter();
      const CMaterialFilter& actFilter = act->GetMaterialFilter();
      CMaterialFilter filter =
          CMaterialFilter::MakeIncludeExclude(selfFilter.GetIncludeList(), selfFilter.GetExcludeList());
      filter.IncludeList().Add(actFilter.GetIncludeList());
      filter.IncludeList().Add(EMaterialTypes::Player); // ?
      filter.ExcludeList().Add(actFilter.GetExcludeList());
      filter.ExcludeList().Remove(EMaterialTypes::Player); // ?
      filter.ExcludeList().Add(EMaterialTypes::Platform);  // ?
      act->SetMaterialFilter(filter);
      act->RemoveMaterial(EMaterialTypes::ProjectilePassthrough, mgr);
      act->SetDamageVulnerability(CDamageVulnerability::ReflectVulnerabilty());
    }
  }
}

} // namespace urde::MP1
