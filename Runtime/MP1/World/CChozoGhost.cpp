#include "Runtime/MP1/World/CChozoGhost.hpp"

#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/CSimplePool.hpp"
#include "Runtime/CRandom16.hpp"
#include "Runtime/Character/CPASAnimParmData.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "Runtime/World/CTeamAiMgr.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path
namespace urde::MP1 {
CChozoGhost::CBehaveChance::CBehaveChance(CInputStream& in)
: x0_propertyCount(in.readUint32Big())
, x4_lurk(in.readFloatBig())
, x8_(in.readFloatBig())
, xc_attack(in.readFloatBig())
, x10_move(in.readFloatBig())
, x14_lurkTime(in.readFloatBig())
, x18_chargeAttack(x0_propertyCount <= 5 ? 0.5f : in.readFloatBig() * .01f)
, x1c_numBolts(x0_propertyCount <= 6 ? 2 : in.readUint32Big()) {
  float f2 = 1.f / (x10_move + xc_attack + x4_lurk + x8_);
  x4_lurk *= f2;
  x8_ *= f2;
  xc_attack *= f2;
  x10_move *= f2;
}

EBehaveType CChozoGhost::CBehaveChance::GetBehave(EBehaveType type, CStateManager& mgr) const {
  float dVar5 = x4_lurk;
  float dVar4 = x8_;
  float dVar3 = xc_attack;
  if (type == EBehaveType::Lurk) {
    float dVar2 = dVar5 / 3.f;
    dVar5 = 0.f;
    dVar4 += dVar2;
    dVar3 += dVar2;
  } else if (type == EBehaveType::One) {
    float dVar2 = dVar4 / 3.f;
    dVar4 = 0.f;
    dVar5 += dVar2;
    dVar3 += dVar2;
  } else if (type == EBehaveType::Attack) {
    float dVar2 = dVar3 / 3.f;
    dVar3 = 0.f;
    dVar5 += dVar2;
    dVar4 += dVar2;
  } else if (type == EBehaveType::Move) {
    float dVar2 = x10_move / 3.f;
    dVar5 += dVar2;
    dVar4 += dVar2;
    dVar3 += dVar2;
  }

  float rnd = mgr.GetActiveRandom()->Float();
  if (dVar5 > rnd)
    return EBehaveType::Lurk;
  else if (dVar4 > (rnd - dVar5))
    return EBehaveType::One;
  else if (dVar3 > (rnd - dVar5) - dVar4)
    return EBehaveType::Attack;
  return EBehaveType::Move;
}

CChozoGhost::CChozoGhost(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                         CModelData&& mData, const CActorParameters& actParms, const CPatternedInfo& pInfo,
                         float hearingRadius, float fadeOutDelay, float attackDelay, float freezeTime, CAssetId wpsc1,
                         const CDamageInfo& dInfo1, CAssetId wpsc2, const CDamageInfo& dInfo2,
                         const CBehaveChance& chance1, const CBehaveChance& chance2, const CBehaveChance& chance3,
                         u16 soundImpact, float f5, u16 sId2, u16 sId3, u32 w1, float f6, u32 w2, float hurlRecoverTime,
                         CAssetId projectileVisorEffect, s16 soundProjectileVisor, float f8, float f9, u32 nearChance,
                         u32 midChance)
: CPatterned(ECharacter::ChozoGhost, uid, name, EFlavorType::Zero, info, xf, std::move(mData), pInfo,
             EMovementType::Flyer, EColliderType::Zero, EBodyType::BiPedal, actParms, EKnockBackVariant::Medium)
, x568_hearingRadius(hearingRadius)
, x56c_fadeOutDelay(fadeOutDelay)
, x570_attackDelay(attackDelay)
, x574_freezeTime(freezeTime)
, x578_(wpsc1, dInfo1)
, x5a0_(wpsc2, dInfo2)
, x5c8_(chance1)
, x5e8_(chance2)
, x608_(chance3)
, x628_soundImpact(soundImpact)
, x62c_(f5)
, x630_(sId2)
, x632_(sId3)
, x634_(f6)
, x638_hurlRecoverTime(hurlRecoverTime)
, x63c_(w2)
, x650_sound_ProjectileVisor(soundProjectileVisor)
, x654_(f8)
, x658_(f9)
, x65c_nearChance(nearChance)
, x660_midChance(midChance)
, x664_24_onGround(w1)
, x664_25_flinch(w1)
, x664_26_(false)
, x664_27_(false)
, x664_28_(false)
, x664_29_(false)
, x664_30_(false)
, x664_31_(false)
, x665_24_(true)
, x665_25_(false)
, x665_26_(false)
, x665_27_playerInLeashRange(false)
, x665_28_inRange(false)
, x665_29_aggressive(false)
, x680_behaveType(x664_24_onGround ? EBehaveType::Attack : EBehaveType::Four)
, x68c_boneTracking(*GetModelData()->GetAnimationData(), "Head_1"sv, zeus::degToRad(80.f), zeus::degToRad(180.f),
                    EBoneTrackingFlags::None) {
  x578_.Token().Lock();
  x5a0_.Token().Lock();
  x668_ = GetModelData()->GetScale().z() *
          GetAnimationDistance(CPASAnimParmData(13, CPASAnimParm::FromEnum(3), CPASAnimParm::FromEnum(0)));
  x66c_ = GetModelData()->GetScale().z() *
          GetAnimationDistance(CPASAnimParmData(15, CPASAnimParm::FromEnum(1), CPASAnimParm::FromReal32(90.f)));
  x670_ = GetModelData()->GetScale().z() *
          GetAnimationDistance(CPASAnimParmData(7, CPASAnimParm::FromEnum(1), CPASAnimParm::FromEnum(2)));

  if (projectileVisorEffect.IsValid())
    x640_ = g_SimplePool->GetObj({SBIG('PART'), projectileVisorEffect});
  x460_knockBackController.SetEnableBurn(false);
  x460_knockBackController.SetEnableLaggedBurnDeath(false);
  x460_knockBackController.SetEnableShock(false);
  x460_knockBackController.SetEnableFreeze(false);
  CreateShadow(false);
  MakeThermalColdAndHot();
}
void CChozoGhost::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  CPatterned::AcceptScriptMsg(msg, uid, mgr);

  switch (msg) {
  case EScriptObjectMessage::Activate:
    AddToTeam(mgr);
    break;
  case EScriptObjectMessage::Deactivate:
  case EScriptObjectMessage::Deleted:
    RemoveFromTeam(mgr);
    break;
  case EScriptObjectMessage::Action:
    if (x664_26_)
      break;
    x664_26_ = true;
    x400_24_hitByPlayerProjectile = true;
    break;
  case EScriptObjectMessage::Falling:
  case EScriptObjectMessage::Jumped: {
    if (!x328_25_verticalMovement)
      x150_momentum = {0.f, 0.f, -(GetGravityConstant() * GetMass())};
    break;
  }
  case EScriptObjectMessage::InitializedInArea:
    if (GetActive())
      AddToTeam(mgr);
    break;
  default:
    break;
  }
}

void CChozoGhost::Think(float dt, CStateManager& mgr) {
  if (!GetActive())
    return;

  CPatterned::Think(dt, mgr);
  UpdateThermalFrozenState(false);
  x68c_boneTracking.Update(dt);
  x6c8_ = std::max(0.f, x6c8_ - dt);
  xe7_31_targetable = IsVisibleEnough(mgr);
}

void CChozoGhost::PreRender(CStateManager& mgr, const zeus::CFrustum& frustum) { CPatterned::PreRender(mgr, frustum); }

void CChozoGhost::Render(const CStateManager& mgr) const {
  if (x6c8_ > 0.f)
    mgr.DrawSpaceWarp(x6cc_, std::sin((M_PIF * x6c8_) / x56c_fadeOutDelay));

  if (mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::EPlayerVisor::XRay) {
    CElementGen::SetSubtractBlend(true);
    CElementGen::g_ParticleSystemInitialized = true;
    CGraphics::SetFog(ERglFogMode::PerspLin, 0.f, 75.f, zeus::skBlack);
    mgr.SetupFogForArea3XRange(GetAreaIdAlways());
  }

  CPatterned::Render(mgr);

  if (mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::EPlayerVisor::XRay) {
    CGraphics::SetFog(ERglFogMode::PerspLin, 0.f, 75.f, zeus::skBlack);
    GetModelData()->GetAnimationData()->GetParticleDB().RenderSystemsToBeDrawnLast();
    mgr.SetupFogForArea(GetAreaIdAlways());
    CElementGen::SetSubtractBlend(false);
    CElementGen::g_ParticleSystemInitialized = false;
  }
}

void CChozoGhost::Touch(CActor& act, CStateManager& mgr) { CPatterned::Touch(act, mgr); }

EWeaponCollisionResponseTypes CChozoGhost::GetCollisionResponseType(const zeus::CVector3f& pos,
                                                                    const zeus::CVector3f& dir, const CWeaponMode& mode,
                                                                    EProjectileAttrib attrib) const {
  return CAi::GetCollisionResponseType(pos, dir, mode, attrib);
}

void CChozoGhost::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type, float dt) {
  CPatterned::DoUserAnimEvent(mgr, node, type, dt);
}

void CChozoGhost::KnockBack(const zeus::CVector3f& dir, CStateManager& mgr, const CDamageInfo& info,
                            EKnockBackType type, bool inDeferred, float magnitude) {
  CPatterned::KnockBack(dir, mgr, info, type, inDeferred, magnitude);
}

bool CChozoGhost::CanBeShot(const CStateManager& mgr, int w1) { return IsVisibleEnough(mgr); }

void CChozoGhost::Dead(CStateManager& mgr, EStateMsg msg, float arg) { CPatterned::Dead(mgr, msg, arg); }

void CChozoGhost::SelectTarget(CStateManager& mgr, EStateMsg msg, float arg) { CAi::SelectTarget(mgr, msg, arg); }

void CChozoGhost::Run(CStateManager& mgr, EStateMsg msg, float arg) { CAi::Run(mgr, msg, arg); }

void CChozoGhost::Generate(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x330_stateMachineState.SetDelay(x56c_fadeOutDelay);
    x32c_animState = EAnimState::Ready;
    x664_27_ = false;
    CRayCastResult res = mgr.RayStaticIntersection(GetTranslation(), zeus::skDown, 100.f,
                                                   CMaterialFilter::MakeInclude({EMaterialTypes::Floor}));
    if (res.IsInvalid()) {
      x678_ = mgr.GetPlayer().GetTranslation().z();
    } else
      x678_ = res.GetPoint().z();
    x3e8_alphaDelta = 1.f;
    x664_29_ = true;
    if (x56c_fadeOutDelay > 0.f) {
      x6c8_ = x56c_fadeOutDelay;
      FindNearestSolid(mgr, zeus::skDown);
    }
  } else if (msg == EStateMsg::Update) {
    TryCommand(mgr, pas::EAnimationState::Jump, &CPatterned::TryJump, 0);
    if (x32c_animState == EAnimState::Over) {
      x68c_boneTracking.SetActive(true);
      x68c_boneTracking.SetTarget(mgr.GetPlayer().GetUniqueId());
      FloatToLevel(x678_, arg);
    } else if (x32c_animState == EAnimState::Repeat) {
      x450_bodyController->SetLocomotionType(pas::ELocomotionType::Crouch);
      if (!x664_27_) {
        zeus::CVector3f pos = GetTranslation();
        SetTranslation({pos.x(), pos.y(), x678_ + x668_});
        x664_27_ = true;
      }
    }
  } else if (msg == EStateMsg::Deactivate) {
    x32c_animState = EAnimState::NotReady;
    x665_24_ = false;
    x664_27_ = false;
  }
}

void CChozoGhost::Deactivate(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x68c_boneTracking.SetActive(false);
    ReleaseCoverPoint(mgr, x674_coverPoint);
    x32c_animState = EAnimState::Ready;
    x665_24_ = true;
  } else if (msg == EStateMsg::Update) {
    TryCommand(mgr, pas::EAnimationState::Generate, &CPatterned::TryGenerate, 1);
    if (x32c_animState == EAnimState::Repeat)
      GetBodyController()->SetLocomotionType(pas::ELocomotionType::Relaxed);
  } else if (msg == EStateMsg::Deactivate) {
    x32c_animState = EAnimState::NotReady;
  }
}

void CChozoGhost::Attack(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    CTeamAiMgr::AddAttacker(CTeamAiMgr::EAttackType::Melee, mgr, x6c4_teamMgr, GetUniqueId());
    x32c_animState = EAnimState::Ready;
    if (x6d8_ == 1)
      x67c_ = 3;
    else if (x6d8_ == 2)
      x67c_ = 4;
    else if (x6d8_ == 3)
      x67c_ = 5;

    if (x665_26_ && mgr.RayStaticIntersection(GetTranslation() + (zeus::skUp * 0.5f), zeus::skUp, x670_,
                                              CMaterialFilter::MakeInclude({EMaterialTypes::Solid}))
                        .IsValid()) {
      x67c_ = 2;
      x460_knockBackController.SetAvailableState(EKnockBackAnimationState::KnockBack, false);
    }
    x150_momentum.zeroOut();
    xfc_constantForce.zeroOut();
  } else if (msg == EStateMsg::Update) {
    TryCommand(mgr, pas::EAnimationState::MeleeAttack, &CPatterned::TryMeleeAttack, x67c_);
    GetBodyController()->GetCommandMgr().SetTargetVector(mgr.GetPlayer().GetTranslation() - GetTranslation());
    if (x67c_ != 2)
      FloatToLevel(x678_, arg);
  } else if (msg == EStateMsg::Deactivate) {
    x32c_animState = EAnimState::NotReady;
    x665_26_ = false;
    x460_knockBackController.SetAvailableState(EKnockBackAnimationState::KnockBack, true);
    CTeamAiMgr::ResetTeamAiRole(CTeamAiMgr::EAttackType::Ranged, mgr, x6c4_teamMgr, GetUniqueId(), true);
  }
}

void CChozoGhost::Shuffle(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg != EStateMsg::Activate)
    return;

  const CBehaveChance& chance1 = ChooseBehaveChanceRange(mgr);
  CTeamAiRole* role = CTeamAiMgr::GetTeamAiRole(mgr, x6c4_teamMgr, GetUniqueId());
  if (role && role->GetTeamAiRole() == CTeamAiRole::ETeamAiRole::Ranged &&
      !CTeamAiMgr::CanAcceptAttacker(CTeamAiMgr::EAttackType::Ranged, mgr, x6c4_teamMgr, GetUniqueId())) {
    x680_behaveType = EBehaveType::Attack;
  }

  const CBehaveChance& chance2 = ChooseBehaveChanceRange(mgr);
  x680_behaveType = chance2.GetBehave(x680_behaveType, mgr);
  if (x680_behaveType == EBehaveType::Lurk)
    x684_lurkDelay = chance1.GetLurkTime();
  else if (x680_behaveType == EBehaveType::Attack) {
    x665_25_ = mgr.GetActiveRandom()->Float() < chance1.GetChargeAttack();
    const int rnd = mgr.GetActiveRandom()->Next();
    x6d8_ = (rnd - (rnd / chance1.GetNumBolts()) * chance1.GetNumBolts()) + 1;
  }
  x664_31_ = false;
  x665_27_playerInLeashRange = false;
}

void CChozoGhost::InActive(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    if (!x450_bodyController->GetActive())
      x450_bodyController->Activate(mgr);

    if (x63c_ == 3) {
      x450_bodyController->SetLocomotionType(pas::ELocomotionType::Crouch);
      x42c_color.a() = 1.f;
    } else {
      x450_bodyController->SetLocomotionType(pas::ELocomotionType::Relaxed);
      x42c_color.a() = 0.f;
    }

    RemoveMaterial(EMaterialTypes::Solid, mgr);
    x150_momentum.zeroOut();
    x665_24_ = true;
  }
}

void CChozoGhost::Taunt(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x32c_animState = EAnimState::Ready;
  } else if (msg == EStateMsg::Update) {
    TryCommand(mgr, pas::EAnimationState::Taunt, &CPatterned::TryTaunt, 0);
    FloatToLevel(x678_, arg);
  } else {
    x32c_animState = EAnimState::NotReady;
    x665_26_ = false;
  }
}

void CChozoGhost::Hurled(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x328_25_verticalMovement = false;
    x664_27_ = false;
    x665_24_ = true;
  } else if (msg == EStateMsg::Update) {
    x3e8_alphaDelta = 2.f;
    if (x664_27_)
      return;

    if (x138_velocity.z() < 0.f) {
      CRayCastResult res = mgr.RayStaticIntersection(GetTranslation() + zeus::skUp, zeus::skDown, 2.f,
                                                     CMaterialFilter::MakeInclude({EMaterialTypes::Floor}));
      if (res.IsValid()) {
        x664_27_ = true;
        x150_momentum.zeroOut();
        zeus::CVector3f velNoZ = x138_velocity;
        velNoZ.z() = 0.f;
        SetVelocityWR(velNoZ);
        x678_ = res.GetPoint().z();
        x330_stateMachineState.SetCodeTrigger();
      }
    }
    if (!x664_27_ && x638_hurlRecoverTime < x330_stateMachineState.GetTime()) {
      GetBodyController()->GetCommandMgr().DeliverCmd(CBodyStateCmd(EBodyStateCmd::ExitState));
      GetBodyController()->SetLocomotionType(pas::ELocomotionType::Lurk);
      x330_stateMachineState.SetCodeTrigger();
    }
  } else if (msg == EStateMsg::Deactivate) {
    x328_25_verticalMovement = true;
    x150_momentum.zeroOut();
  }
}

void CChozoGhost::WallDetach(CStateManager& mgr, EStateMsg msg, float arg) { CAi::WallDetach(mgr, msg, arg); }

void CChozoGhost::Growth(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x330_stateMachineState.SetDelay(x56c_fadeOutDelay);
    GetBodyController()->SetLocomotionType(pas::ELocomotionType::Crouch);
    x3e8_alphaDelta = 1.f;
    x664_29_ = true;
    if (x56c_fadeOutDelay > 0.f) {
      x6c8_ = x56c_fadeOutDelay;
      FindNearestSolid(mgr, zeus::skUp);
    }
  } else if (msg == EStateMsg::Deactivate) {
    x665_24_ = false;
    x68c_boneTracking.SetActive(false);
    x68c_boneTracking.SetTarget(mgr.GetPlayer().GetUniqueId());
  }
}

void CChozoGhost::Land(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Update) {
    FloatToLevel(x678_, arg);
    if (std::fabs(x678_ - GetTranslation().z()) < 0.05f) {
      x330_stateMachineState.SetCodeTrigger();
    }
  }
}

void CChozoGhost::Lurk(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x330_stateMachineState.SetDelay(x684_lurkDelay);
  } else if (msg == EStateMsg::Update) {
    FloatToLevel(x678_, arg);
  }
}

bool CChozoGhost::Leash(CStateManager& mgr, float arg) {
  return x665_27_playerInLeashRange || CPatterned::Leash(mgr, arg);
}

bool CChozoGhost::InRange(CStateManager& mgr, float arg) { return x665_28_inRange; }

bool CChozoGhost::InPosition(CStateManager& mgr, float arg) { return x680_behaveType == EBehaveType::Attack; }

bool CChozoGhost::AggressionCheck(CStateManager& mgr, float arg) { return x665_29_aggressive; }

bool CChozoGhost::ShouldTaunt(CStateManager& mgr, float arg) { return x680_behaveType == EBehaveType::One; }

bool CChozoGhost::ShouldFlinch(CStateManager& mgr, float arg) { return x664_25_flinch; }

bool CChozoGhost::ShouldMove(CStateManager& mgr, float arg) { return x680_behaveType == EBehaveType::Move; }

bool CChozoGhost::AIStage(CStateManager& mgr, float arg) { return arg == x63c_; }

u8 CChozoGhost::GetModelAlphau8(const CStateManager& mgr) const {
  // if (mgr.GetPlayerState()->GetActiveVisor(mgr) != CPlayerState::EPlayerVisor::XRay || !IsAlive())
  //  return u8(x42c_color.a() * 255);

  return 255;
}

bool CChozoGhost::IsOnGround() const { return x664_24_onGround; }

CProjectileInfo* CChozoGhost::GetProjectileInfo() { return x67c_ == 2 ? &x578_ : &x5a0_; }

void CChozoGhost::AddToTeam(CStateManager& mgr) {
  if (x6c4_teamMgr == kInvalidUniqueId)
    x6c4_teamMgr = CTeamAiMgr::GetTeamAiMgr(*this, mgr);

  if (x6c4_teamMgr == kInvalidUniqueId)
    return;

  if (TCastToPtr<CTeamAiMgr> teamMgr = mgr.ObjectById(x6c4_teamMgr))
    teamMgr->AssignTeamAiRole(*this, CTeamAiRole::ETeamAiRole::Ranged, CTeamAiRole::ETeamAiRole::Unknown,
                              CTeamAiRole::ETeamAiRole::Invalid);
}

void CChozoGhost::RemoveFromTeam(CStateManager& mgr) {
  if (x6c4_teamMgr == kInvalidUniqueId)
    return;

  if (TCastToPtr<CTeamAiMgr> teamMgr = mgr.ObjectById(x6c4_teamMgr)) {
    if (teamMgr->IsPartOfTeam(GetUniqueId())) {
      teamMgr->RemoveTeamAiRole(GetUniqueId());
      x6c4_teamMgr = kInvalidUniqueId;
    }
  }
}

void CChozoGhost::FloatToLevel(float f1, float f2) {
  zeus::CVector3f pos = GetTranslation();
  pos.z() = 4.f * (f1 - pos.z()) * f2 + pos.z();
  SetTranslation(pos);
}

const CChozoGhost::CBehaveChance& CChozoGhost::ChooseBehaveChanceRange(CStateManager& mgr) {
  const float dist = (GetTranslation() - mgr.GetPlayer().GetTranslation()).magnitude();
  if (x654_ <= dist && x658_ > dist)
    return x5e8_;
  else if (x658_ <= dist)
    return x608_;
  else
    return x5c8_;
}
void CChozoGhost::FindNearestSolid(CStateManager& mgr, const zeus::CVector3f& dir) {
  CRayCastResult res = mgr.RayStaticIntersection(GetBoundingBox().center() + (dir * 8.f), -dir, 8.f,
                                                 CMaterialFilter::MakeInclude({EMaterialTypes::Solid}));
  if (res.IsInvalid()) {
    x6cc_ = GetBoundingBox().center() + dir;
  } else
    x6cc_ = res.GetPoint();
}

} // namespace urde::MP1