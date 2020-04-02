#include "Runtime/MP1/World/COmegaPirate.hpp"

#include "Runtime/Collision/CCollisionActor.hpp"
#include "Runtime/Collision/CCollisionActorManager.hpp"
#include "Runtime/CSimplePool.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "Runtime/World/CWorld.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde::MP1 {
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
  x9a4_.reserve(3);
  x9b8_.reserve(24);
  x9dc_.reserve(4);
  xaa0_.reserve(4);
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
  CElitePirate::AcceptScriptMsg(msg, uid, mgr);
  // TODO
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
  CPatterned::Render(mgr);
  // TODO
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
  CElitePirate::Think(dt, mgr);
  // TODO
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
} // namespace urde::MP1
