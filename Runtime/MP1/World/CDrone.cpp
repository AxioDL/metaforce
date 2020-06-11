#include "Runtime/MP1/World/CDrone.hpp"

#include "Runtime/Audio/CSfxManager.hpp"
#include "Runtime/Collision/CGameCollision.hpp"
#include "Runtime/CSimplePool.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Particle/CWeaponDescription.hpp"
#include "Runtime/Weapon/CGameProjectile.hpp"
#include "Runtime/Weapon/CWeapon.hpp"
#include "Runtime/World/CGameLight.hpp"
#include "Runtime/World/CPatternedInfo.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "Runtime/World/CScriptWater.hpp"
#include "Runtime/World/CTeamAiMgr.hpp"
#include "Runtime/World/CWorld.hpp"

#include "DataSpec/DNAMP1/SFX/Drones.h"

#include "TCastTo.hpp" // Generated file, do not modify include path

#include <algorithm>

namespace urde::MP1 {
CDrone::CDrone(TUniqueId uid, std::string_view name, EFlavorType flavor, const CEntityInfo& info,
               const zeus::CTransform& xf, float f1, CModelData&& mData, const CPatternedInfo& pInfo,
               const CActorParameters& actParms, EMovementType movement, EColliderType colliderType, EBodyType bodyType,
               const CDamageInfo& dInfo1, CAssetId aId1, const CDamageInfo& dInfo2, CAssetId aId2,
               std::vector<CVisorFlare::CFlareDef> flares, float f2, float f3, float f4, float f5, float f6, float f7,
               float f8, float f9, float f10, float f11, float f12, float f13, float f14, float f15, float f16,
               float f17, float f18, float f19, float f20, CAssetId crscId, float f21, float f22, float f23, float f24,
               s32 sId, bool b1)
: CPatterned(ECharacter::Drone, uid, name, flavor, info, xf, std::move(mData), pInfo, movement, colliderType, bodyType,
             actParms, EKnockBackVariant(flavor == EFlavorType::Zero))
, x568_(aId1)
, x56c_(g_SimplePool->GetObj({SBIG('CRSC'), crscId}))
, x57c_flares(std::move(flares))
, x590_(dInfo1)
, x5ac_(dInfo2)
, x5e4_(f23)
, x5ec_turnSpeed(f1)
, x5f0_(f2)
, x5f4_(f3)
, x5f8_(f4)
, x5fc_(f5)
, x600_(f11)
, x608_(f6)
, x60c_(f7)
, x610_(f8)
, x614_(f9)
, x618_(f10)
, x61c_(f12)
, x620_(f20)
, x63c_(f13)
, x640_(f14)
, x648_(f15)
, x64c_(f16)
, x650_(f17)
, x654_(f18)
, x658_(f19)
, x65c_(f21)
, x660_(f22)
, x664_(f24)
, x690_(zeus::CSphere({0.f, 0.f, 1.8f}, 1.1f), CActor::GetMaterialList())
, x6b0_pathFind(nullptr, 3 + int(b1), pInfo.GetPathfindingIndex(), 1.f, 2.4f)
, x7cc_(CSfxManager::TranslateSFXID(sId))
, x82c_shieldModel(std::make_unique<CModelData>(CStaticRes{aId2, zeus::skOne3f}))
, x832_a(0)
, x832_b(0)
, x834_24_(false)
, x834_25_(false)
, x834_26_(false)
, x834_27_(false)
, x834_28_(false)
, x834_29_codeTrigger(false)
, x834_30_visible(false)
, x834_31_(false)
, x835_24_(false)
, x835_25_(b1)
, x835_26_(false) {
  UpdateTouchBounds(pInfo.GetHalfExtent());
  x460_knockBackController.SetEnableShock(true);
  x460_knockBackController.SetAvailableState(EKnockBackAnimationState::Hurled, false);
  x460_knockBackController.SetLocomotionDuringElectrocution(true);
  MakeThermalColdAndHot();
  CreateShadow(flavor != EFlavorType::One);
}

void CDrone::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CDrone::Think(float dt, CStateManager& mgr) {
  if (x3fc_flavor != EFlavorType::One) {
    if (mgr.GetPlayerState()->GetActiveVisor(mgr) != CPlayerState::EPlayerVisor::XRay) {
      x42c_color.a() = std::max(0.f, x428_damageCooldownTimer / 0.33f);
    }
  }

  if (GetBodyController()->IsElectrocuting() && (x824_[0] || x824_[1])) {
    x824_[0] = false;
    x824_[1] = false;
    UpdateLaser(mgr, 0, false);
    UpdateLaser(mgr, 1, false);
    SetVisorFlareEnabled(mgr, false);
  }
  CPatterned::Think(dt, mgr);

  if (!GetActive())
    return;

  x5c8_ -= dt;
  if (x7c4_ > 0.f) {
    x7c4_ -= dt;
  }

  if (x5d0_ > 0.f) {
    x5d0_ -= (mgr.GetPlayer().GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Morphed ? 3.f * dt : dt);
  }

  if (x624_ > 0.f) {
    x624_ -= dt;
  }

  if (x644_ > 0.f) {
    x644_ -= dt;
  }

  if (x824_[0] || (x824_[1] && IsAlive())) {
    sub_80163c40(dt, mgr);
    UpdateVisorFlare(mgr);
  }

  if (x834_25_ && IsAlive()) {
    UpdateScanner(mgr, dt);
  }

  const float dist = (mgr.GetPlayer().GetTranslation() - GetTranslation()).magSquared();
  if (x834_28_ && dist < x60c_ * x60c_) {
    mgr.GetPlayerState()->GetStaticInterference().RemoveSource(GetUniqueId());
    mgr.GetPlayerState()->GetStaticInterference().AddSource(
        GetUniqueId(), std::max(0.f, mgr.GetPlayerState()->GetStaticInterference().GetTotalInterference() - x608_),
        0.2f);
  }

  if (!x834_28_ && dist < x614_ * x614_) {
    mgr.GetPlayerState()->GetStaticInterference().RemoveSource(GetUniqueId());
    mgr.GetPlayerState()->GetStaticInterference().AddSource(
        GetUniqueId(), std::max(0.f, mgr.GetPlayerState()->GetStaticInterference().GetTotalInterference() - x610_),
        0.2f);
  }

  if (!x834_28_ && IsAlive() && !x835_25_) {
    x5e0_ -= dt;
    if (x5e0_ < 0.f) {
      sub_801633a8(mgr);
      x5e0_ = 0.1f;
    }
  }

  const float healthDiff = x604_ - HealthInfo(mgr)->GetHP();
  if (!zeus::close_enough(x600_, 0.f)) {
    x5d0_ -= healthDiff / x600_;
    x624_ -= healthDiff / x600_;
  }
  x604_ = HealthInfo(mgr)->GetHP();
  if (x3fc_flavor == EFlavorType::One) {
    if (!x834_30_visible) {
      x5dc_ = zeus::max(0.f, x5dc_ - (3.f * dt));
    } else {
      x5dc_ = zeus::max(0.f, x5dc_ + (3.f * dt));
    }
    x5e8_shieldTime = zeus::max(0.f, x5e8_shieldTime - dt);

    if (zeus::close_enough(x5dc_, 0.f) && x7d0_) {
      CSfxManager::RemoveEmitter(x7d0_);
      x7d0_.reset();
    } else if (!x7d0_ && IsAlive()) {
      x7d0_ = CSfxManager::AddEmitter(SFXsfx00DD, GetTranslation(), zeus::skZero3f, true, true, 127, GetAreaIdAlways());
    }
  }
  sub_8015f25c(dt, mgr);
  sub_8015f158(dt);

  if (!x835_25_) {
    CGameCollision::AvoidStaticCollisionWithinRadius(mgr, *this, 8, dt, 0.25f, 3.5f * GetModelData()->GetScale().y(),
                                                     3000.f, 0.5f);
  }
  if (x66c_ > 0.f) {
    x66c_ -= dt;
  } else {
    x668_ = mgr.RayStaticIntersection(GetTranslation(), zeus::skDown, 1000.f,
                                      CMaterialFilter::MakeInclude({EMaterialTypes::Solid}))
                .GetT();
    x66c_ = 0.f;
  }

  if (IsAlive() && x835_25_) {
    zeus::CAABox box = GetBoundingBox();
    box.accumulateBounds(20.f * zeus::skDown);
    rstl::reserved_vector<TUniqueId, 1024> nearList;
    mgr.BuildNearList(nearList, GetBoundingBox(), CMaterialFilter::MakeInclude({EMaterialTypes::Trigger}), this);
    for (TUniqueId id : nearList) {
      if (const TCastToConstPtr<CScriptWater> water = mgr.GetObjectById(id)) {
        zeus::CAABox waterBox = water->GetTriggerBoundsWR();
        if (waterBox.max.z() - GetTranslation().z() < 3.f) {
          float z = 20.f;
          if (waterBox.max.z() - GetTranslation().z() < 1.5f) {
            z = 60.f;
          }
          ApplyImpulseWR(GetMoveToORImpulseWR(GetTransform().transposeRotate(z * zeus::skDown), dt),
                         zeus::CAxisAngle());
        }
      }
    }
  }
  if (IsAlive() && x668_ < x664_) {
    ApplyImpulseWR(GetMoveToORImpulseWR(GetTransform().transposeRotate(dt * (1.f * zeus::skUp)), dt),
                   zeus::CAxisAngle());
    xe7_31_targetable = IsAlive();
  }
}

void CDrone::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) {
  CPatterned::AcceptScriptMsg(msg, sender, mgr);

  switch (msg) {
  case EScriptObjectMessage::Activate: {
    SetLightEnabled(mgr, true);
    AddToTeam(mgr);
    break;
  }
  case EScriptObjectMessage::Deactivate:
  case EScriptObjectMessage::Deleted: {
    for (TUniqueId& unkId : x7d4_) {
      if (unkId != kInvalidUniqueId) {
        mgr.FreeScriptObject(unkId);
        unkId = kInvalidUniqueId;
      }
    }
    RemoveFromTeam(mgr);
    mgr.GetPlayerState()->GetStaticInterference().RemoveSource(GetUniqueId());
    if (x578_lightId != kInvalidUniqueId) {
      mgr.FreeScriptObject(x578_lightId);
    }
    if (x57a_ != kInvalidUniqueId) {
      mgr.FreeScriptObject(x57a_);
    }

    if (x7d0_) {
      CSfxManager::RemoveEmitter(x7d0_);
      x7d0_.reset();
    }
    break;
  }
  case EScriptObjectMessage::Alert:
    x834_29_codeTrigger = true;
    break;
  case EScriptObjectMessage::OnFloor:
    if (!x835_26_ && x834_24_) {
      x835_26_ = true;
      MassiveFrozenDeath(mgr);
    }
    break;
  case EScriptObjectMessage::Registered:
    x450_bodyController->Activate(mgr);
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Lurk);
    x450_bodyController->BodyStateInfo().SetMaximumPitch(0.f);
    x5cc_ = 0.f;
    x460_knockBackController.SetEnableFreeze(false);
    AddMaterial(EMaterialTypes::AIJoint, mgr);
    x578_lightId = mgr.AllocateUniqueId();
    mgr.AddObject(new CGameLight(x578_lightId, GetAreaIdAlways(), GetActive(), "LaserLight"sv, {}, GetUniqueId(),
                                 CLight::BuildPoint(zeus::skZero3f, zeus::skRed), 0, 0, 0.f));
    break;
  case EScriptObjectMessage::InitializedInArea: {
    x6b0_pathFind.SetArea(mgr.GetWorld()->GetAreaAlways(GetAreaIdAlways())->GetPostConstructed()->x10bc_pathArea);
    if (x688_teamMgr == kInvalidUniqueId) {
      x688_teamMgr = CTeamAiMgr::GetTeamAiMgr(*this, mgr);
      if (GetActive()) {
        AddToTeam(mgr);
      }
    }

    x604_ = HealthInfo(mgr)->GetHP();
    x55c_moveScale = 1.f / GetModelData()->GetScale();
    if (x835_25_)
      SetSoundEventPitchBend(0);
    break;
  }
  default:
    break;
  }
}

void CDrone::PreRender(CStateManager& mgr, const zeus::CFrustum& frustum) {
  CPatterned::PreRender(mgr, frustum);
  if (x3fc_flavor == EFlavorType::One) {
    if (HasModelData() && GetModelData()->HasAnimData()) {
      if (GetModelAlphau8(mgr) == 0)
        GetModelData()->GetAnimationData()->BuildPose();
    }
  }
}

void CDrone::Render(CStateManager& mgr) {
  bool isOne = x3fc_flavor == EFlavorType::One;
  if (!isOne || GetModelAlphau8(mgr) != 0) {
    if (isOne && mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::EPlayerVisor::XRay) {
      CGraphics::SetFog(ERglFogMode::PerspLin, 0.f, 75.f, zeus::skBlack);
      GetModelData()->GetAnimationData()->GetParticleDB().RenderSystemsToBeDrawnFirst();
      mgr.SetupFogForArea3XRange(GetAreaIdAlways());
    }
    CPatterned::Render(mgr);
    if (isOne && mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::EPlayerVisor::XRay) {
      CGraphics::SetFog(ERglFogMode::PerspLin, 0.f, 75.f, zeus::skBlack);
      GetModelData()->GetAnimationData()->GetParticleDB().RenderSystemsToBeDrawnLast();
      mgr.SetupFogForArea3XRange(GetAreaIdAlways());
    }

    if (isOne && zeus::close_enough(x5dc_, 0)) {
      x82c_shieldModel->Render(
          mgr, GetLctrTransform("Shield_LCTR"sv), GetActorLights(),
          CModelFlags{8, 0, 3, zeus::CColor::lerp({1.f, 0.f, 0.f, 1.f}, zeus::skWhite, x5e8_shieldTime)});
    }
  }
}

bool CDrone::CanRenderUnsorted(const CStateManager& mgr) const {
  if (zeus::close_enough(x5dc_, 0.f))
    return false;
  return CPatterned::CanRenderUnsorted(mgr);
}

const CDamageVulnerability* CDrone::GetDamageVulnerability(const zeus::CVector3f&, const zeus::CVector3f& dir,
                                                           const CDamageInfo&) const {
  if (x3fc_flavor == EFlavorType::One && HitShield(-dir)) {
    return &CDamageVulnerability::ImmuneVulnerabilty();
  }
  return CAi::GetDamageVulnerability();
}

void CDrone::Touch(CActor& act, CStateManager& mgr) {
  CPatterned::Touch(act, mgr);
  if (TCastToPtr<CWeapon> weapon = act) {
    if (IsAlive()) {
      x834_24_ = weapon->GetType() == EWeaponType::Wave;
      if (HitShield(weapon->GetTranslation() - GetTranslation())) {
        x5e8_shieldTime = 1.f;
      }
    }
  }
}

EWeaponCollisionResponseTypes CDrone::GetCollisionResponseType(const zeus::CVector3f&, const zeus::CVector3f& dir,
                                                               const CWeaponMode&, EProjectileAttrib) const {
  if (x3fc_flavor == EFlavorType::One && HitShield(-dir)) {
    return EWeaponCollisionResponseTypes::Unknown86;
  }
  return EWeaponCollisionResponseTypes::Unknown36;
}

void CDrone::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type, float dt) {
  // TODO: Finish
  switch (type) {
  case EUserEventType::Projectile:
    // sub80165984(mgr, GetLctrTransform(node.GetLocatorName()));
    return;
  case EUserEventType::Delete:
    if (x7d0_) {
      CSfxManager::RemoveEmitter(x7d0_);
      x7d0_.reset();
    }
    MassiveDeath(mgr);
    break;
  case EUserEventType::DamageOn: {
    if (IsAlive() && x835_24_) {
      UpdateLaser(mgr, 0, true);
      x824_[0] = true;
      SetVisorFlareEnabled(mgr, true);
    } else if (x3fc_flavor == EFlavorType::One) {
      UpdateLaser(mgr, 1, true);
      x824_[1] = true;
    }
    return;
  }
  case EUserEventType::DamageOff: {
    if (x824_[0]) {
      UpdateLaser(mgr, 0, false);
      x824_[0] = false;
      SetVisorFlareEnabled(mgr, false);
    } else if (x3fc_flavor == EFlavorType::One) {
      UpdateLaser(mgr, 1, false);
      x824_[1] = false;
    }
    return;
  }
  case EUserEventType::FadeIn: {
    if (x3fc_flavor == EFlavorType::One)
      x834_30_visible = true;
    return;
  }
  case EUserEventType::FadeOut: {
    if (x3fc_flavor == EFlavorType::One)
      x834_30_visible = false;
    return;
  }
  default:
    break;
  }
  CPatterned::DoUserAnimEvent(mgr, node, type, dt);
}

const CCollisionPrimitive* CDrone::GetCollisionPrimitive() const {
  if (!x834_28_)
    return &x690_;
  return CPatterned::GetCollisionPrimitive();
}

void CDrone::Death(CStateManager& mgr, const zeus::CVector3f& direction, EScriptObjectState state) {
  if (!IsAlive())
    return;

  x824_[0] = false;
  x824_[1] = false;
  UpdateLaser(mgr, 0, false);
  UpdateLaser(mgr, 1, false);
  SetVisorFlareEnabled(mgr, false);

  if (x3e4_lastHP - HealthInfo(mgr)->GetHP() < x3d8_xDamageThreshold || x834_24_) {
    x330_stateMachineState.SetState(mgr, *this, GetStateMachine(), "Dead"sv);
  } else {
    x834_28_ = true;
    if (x3e0_xDamageDelay <= 0.f) {
      SetTransform(zeus::lookAt(GetTranslation(), GetTranslation() - direction) *
                   zeus::CTransform::RotateX(zeus::degToRad(45.f)));
    }

    if (x450_bodyController->GetPercentageFrozen() > 0.f) {
      x450_bodyController->UnFreeze();
    }

    x400_25_alive = false;
    SendScriptMsgs(state, mgr, EScriptObjectMessage::None);
  }
}

void CDrone::KnockBack(const zeus::CVector3f& backVec, CStateManager& mgr, const CDamageInfo& info, EKnockBackType type,
                       bool inDeferred, float magnitude) {
  if (!IsAlive())
    return;
  CPatterned::KnockBack(backVec, mgr, info, type, inDeferred, magnitude);
  if (GetKnockBackController().GetActiveParms().x0_animState == EKnockBackAnimationState::Invalid)
    return;
  x630_ = 0.5f;
  x634_ = 1.f;
}

void CDrone::Patrol(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Lurk);
    SetLightEnabled(mgr, true);
    x834_25_ = true;
  } else if (msg == EStateMsg::Update) {
    rstl::reserved_vector<TUniqueId, 1024> nearList;
    BuildNearList(EMaterialTypes::Character, EMaterialTypes::Player, nearList, 5.f, mgr);
    if (nearList.empty())
      return;
    zeus::CVector3f sep = x45c_steeringBehaviors.Separation(
        *this, static_cast<const CActor*>(mgr.GetObjectById(nearList[0]))->GetTranslation(), 5.f);
    if (!sep.isZero()) {
      x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(sep, zeus::skZero3f, 0.5f));
    }
  } else if (msg == EStateMsg::Deactivate) {
    SetLightEnabled(mgr, false);
    x834_25_ = false;
  }
  CPatterned::Patrol(mgr, msg, dt);
}

void CDrone::PathFind(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    zeus::CVector3f searchOff = GetTranslation() + zeus::CVector3f{0.f, 0.f, x664_};
    CPathFindSearch::EResult res = GetSearchPath()->Search(GetTranslation(), searchOff);
    if (res != CPathFindSearch::EResult::Success &&
        (res == CPathFindSearch::EResult::NoDestPoint || res == CPathFindSearch::EResult::NoPath)) {
      if (GetSearchPath()->FindClosestReachablePoint(GetTranslation(), searchOff) ==
          CPathFindSearch::EResult::Success) {
        GetSearchPath()->Search(GetTranslation(), searchOff);
        SetDestPos(searchOff);
      }
    }
    x834_30_visible = true;
  } else if (msg == EStateMsg::Update) {
    CPatterned::PathFind(mgr, msg, dt);
    x450_bodyController->GetCommandMgr().BlendSteeringCmds();
    zeus::CVector3f moveVec = x450_bodyController->GetCommandMgr().GetMoveVector();
    if (moveVec.canBeNormalized()) {
      moveVec.normalize();
      ApplyImpulseWR(GetMass() * (x5e4_ * moveVec), {});
      zeus::CVector3f target = (mgr.GetPlayer().GetAimPosition(mgr, 0.f) - GetTranslation());
      x450_bodyController->GetCommandMgr().DeliverCmd(
          CBCLocomotionCmd(FLT_EPSILON * GetTransform().basis[1], target.normalized(), 1.f));
      x450_bodyController->GetCommandMgr().DeliverTargetVector(target);
      StrafeFromCompanions(mgr);
      if (x630_ <= 0.f) {
        x634_ = 0.333333f;
      }
    } else if (x630_ <= 0.f) {
      x634_ = 0.f;
    }
  } else if (msg == EStateMsg::Deactivate) {
    CPatterned::PathFind(mgr, msg, dt);
  }
}

void CDrone::TargetPlayer(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    x3b8_turnSpeed = x5ec_turnSpeed;
    if (x450_bodyController->GetLocomotionType() != pas::ELocomotionType::Combat)
      x450_bodyController->SetLocomotionType(pas::ELocomotionType::Combat);
    SetDestPos(mgr.GetPlayer().GetAimPosition(mgr, 0.f));
    x400_24_hitByPlayerProjectile = false;
    if (x3fc_flavor == EFlavorType::One)
      x834_30_visible = true;
    x330_stateMachineState.SetDelay(std::max(0.f, x624_));
  } else if (msg == EStateMsg::Update) {
    zeus::CVector3f target = (mgr.GetPlayer().GetAimPosition(mgr, 0.f) - GetTranslation()).normalized();
    x450_bodyController->GetCommandMgr().DeliverCmd(
        CBCLocomotionCmd(FLT_EPSILON * GetTransform().basis[1], target, 1.f));
    x450_bodyController->GetCommandMgr().DeliverTargetVector(target);
    StrafeFromCompanions(mgr);
    if (x630_ <= 0.f)
      x634_ = 0.f;
  } else if (msg == EStateMsg::Deactivate) {
    SetDestPos(mgr.GetPlayer().GetTranslation() + zeus::CVector3f{0.f, 0.f, x664_});
  }
}

void CDrone::TargetCover(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg != EStateMsg::Update) {
    return;
  }

  // Don't ask I have no idea....
  const zeus::CVector3f vec{1.f * x5e4_ * 0.f, 1.f * x5e4_ * 0.f, 1.f * x5e4_ * 1.f};
  ApplyImpulseWR(GetMoveToORImpulseWR(GetTransform().transposeRotate(vec), 1.f), {});
}

void CDrone::Deactivate(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg != EStateMsg::Activate)
    return;
  DeathDelete(mgr);
}

void CDrone::Attack(CStateManager& mgr, EStateMsg msg, float dt) {
  // TODO: Finish
}

void CDrone::Active(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    x330_stateMachineState.SetDelay(x5f0_);
    GetBodyController()->SetLocomotionType(pas::ELocomotionType::Relaxed);
  } else if (msg == EStateMsg::Deactivate) {
    x5d0_ = x5f8_;
  }
}

void CDrone::Flee(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    x7c8_ = 0;
    x832_b = 0;
    if (mgr.RayStaticIntersection(GetTranslation(), -GetTransform().frontVector(), 4.f,
                                  CMaterialFilter::MakeInclude({EMaterialTypes::Solid}))
            .IsValid()) {
      x832_b = mgr.GetActiveRandom()->Float() > 0.5f ? 1 : 2;
    }
  } else if (msg == EStateMsg::Update) {
    if (x7c8_ == 0) {
      if (GetBodyController()->GetBodyStateInfo().GetCurrentStateId() == pas::EAnimationState::Step) {
        x7c8_ = 1;
      } else {
        if (x832_b == 0) {
          GetBodyController()->GetCommandMgr().DeliverCmd(
              CBCStepCmd(pas::EStepDirection::Backward, pas::EStepType::BreakDodge));
        } else if (x832_b == 1) {
          GetBodyController()->GetCommandMgr().DeliverCmd(
              CBCStepCmd(pas::EStepDirection::Left, pas::EStepType::Normal));
        } else if (x832_b == 2) {
          GetBodyController()->GetCommandMgr().DeliverCmd(
              CBCStepCmd(pas::EStepDirection::Right, pas::EStepType::Normal));
        }
      }
    } else if (x7c8_ == 1 &&
               GetBodyController()->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::Step) {
      x7c8_ = 2;
    }
    GetBodyController()->GetCommandMgr().DeliverTargetVector(
        (mgr.GetPlayer().GetTranslation() - GetTranslation()).normalized());
  }
}

void CDrone::ProjectileAttack(CStateManager& mgr, EStateMsg msg, float dt) {
  // TODO: Finish
}

void CDrone::TelegraphAttack(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    x7c8_ = 0;
  } else if (msg == EStateMsg::Update) {
    if (x7c8_ == 1 && x450_bodyController->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::Taunt) {
      x7c8_ = 2;
    } else if (x7c8_ == 0) {
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::EAnimationState::Taunt) {
        x7c8_ = 1;
      } else {
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCTauntCmd(pas::ETauntType::One));
      }
    }
  } else if (msg == EStateMsg::Deactivate) {
    SendScriptMsgs(EScriptObjectState::Zero, mgr, EScriptObjectMessage::None);
  }
}

void CDrone::Dodge(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    x7c8_ = 0;
    x630_ = 0.5f;
    x634_ = 1.f;
    if (x3fc_flavor == EFlavorType::One)
      x834_30_visible = true;
  } else if (msg == EStateMsg::Update) {
    if (x7c8_ == 0) {
      GetBodyController()->GetCommandMgr().DeliverCmd(CBodyStateCmd(EBodyStateCmd::NextState));
      if (x58c_ == 5) {
        GetBodyController()->GetCommandMgr().DeliverCmd(CBCStepCmd(pas::EStepDirection::Left, pas::EStepType::Dodge));
        x58c_ = 2;
      } else if (x58c_ == 4) {
        GetBodyController()->GetCommandMgr().DeliverCmd(CBCStepCmd(pas::EStepDirection::Down, pas::EStepType::Dodge));
        x58c_ = 5;
      } else if (x58c_ == 3) {
        GetBodyController()->GetCommandMgr().DeliverCmd(CBCStepCmd(pas::EStepDirection::Up, pas::EStepType::Dodge));
        x58c_ = 4;
      } else if (x58c_ == 2) {
        GetBodyController()->GetCommandMgr().DeliverCmd(CBCStepCmd(pas::EStepDirection::Right, pas::EStepType::Dodge));
        x58c_ = 3;
      }
      x7c8_ = 1;
    } else if (x7c8_ == 1 &&
               GetBodyController()->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::Step) {
      x7c8_ = 2;
    }
    GetBodyController()->GetCommandMgr().DeliverTargetVector(
        (mgr.GetPlayer().GetTranslation() + zeus::CVector3f{0.f, 0.f, 1.f}) - GetTranslation());
  }
}

void CDrone::Retreat(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    x7c8_ = 0;
    if (x3fc_flavor == EFlavorType::One) {
      x834_30_visible = true;
    }
    x330_stateMachineState.SetDelay(x65c_);
  } else if (msg == EStateMsg::Update) {
    if (x7c8_ == 0) {
      if (GetBodyController()->GetBodyStateInfo().GetCurrentStateId() == pas::EAnimationState::Step) {
        x7c8_ = 1;
      } else {
        GetBodyController()->GetCommandMgr().DeliverCmd(
            CBCStepCmd(pas::EStepDirection::Backward, pas::EStepType::Normal));
      }
    } else if (x7c8_ == 1 &&
               GetBodyController()->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::Step) {
      x7c8_ = 2;
    } else if (x7c8_ == 2) {
      x7c8_ = 0;
    }

    GetBodyController()->GetCommandMgr().DeliverTargetVector(
        (mgr.GetPlayer().GetTranslation() - GetTranslation()).normalized());
    if (x630_ <= 0.f) {
      x634_ = 0.333333; // 1/3
    }
  }
}

void CDrone::Cover(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    x67c_ = zeus::skZero3f;
    x670_ = GetTranslation();
    for (int i = 0; i < 4; ++i) {
      float dVar11 = (x64c_ - x648_) * mgr.GetActiveRandom()->Float() + x648_;
      int v = mgr.GetActiveRandom()->Next();
      float angle = 0.f;
      if (((v >> 3) & 1) == 0) {
        const float angleMin = 270.f - x654_;
        const float angleMax = 270.f + x650_;
        angle = zeus::degToRad((angleMax - angleMin) * x648_ + angleMin);
      } else {
        const float angleMin = 90.f - x654_;
        const float angleMax = 90.f + x650_;
        angle = zeus::degToRad((angleMax - angleMin) * x648_ + angleMin);
      }
      zeus::CQuaternion quat;
      quat.rotateZ(angle);
      const zeus::CVector3f end =
          GetTranslation() +
          quat.transform((dVar11 * (mgr.GetPlayer().GetAimPosition(mgr, 0.f) - GetTranslation()).normalized()));
      if (mgr.RayCollideWorld(GetTranslation(), end, CMaterialFilter::MakeInclude({EMaterialTypes::Solid}), this)) {
        x670_ = end;
        x67c_ = end - GetTranslation();
        if (x67c_.canBeNormalized())
          x67c_.normalize();
      }
    }
  } else if (msg == EStateMsg::Update) {
    ApplyImpulseWR(GetMoveToORImpulseWR(GetTransform().transposeRotate(dt * (x658_ * x67c_)), dt), zeus::CAxisAngle());
    x450_bodyController->GetCommandMgr().DeliverCmd(
        CBCLocomotionCmd(FLT_EPSILON * GetTransform().basis[1],
                         (mgr.GetPlayer().GetAimPosition(mgr, 0.f) - GetTranslation()).normalized(), 1.f));
  } else if (msg == EStateMsg::Deactivate) {
    x644_ = (x640_ - x63c_) * mgr.GetActiveRandom()->Float() + x63c_;
  }
}

void CDrone::SpecialAttack(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    if (x3fc_flavor == EFlavorType::One) {
      x834_30_visible = true;
    }
    x330_stateMachineState.SetDelay(x660_);
    GetBodyController()->SetLocomotionType(pas::ELocomotionType::Internal10);
  } else if (msg == EStateMsg::Update) {
    GetBodyController()->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(GetTransform().basis[1], zeus::skZero3f, 1.f));
    zeus::CVector3f local_74 =
        (mgr.GetPlayer().GetAimPosition(mgr, 0.f) + mgr.GetPlayer().GetTranslation()) - GetTranslation();
    if (((x668_ < x664_ && local_74.z() > 0.f) || (x668_ > x664_)) && local_74.canBeNormalized()) {
      ApplyImpulseWR(GetMoveToORImpulseWR(GetTransform().transposeRotate(dt * (x5e4_ * local_74.normalized())), dt),
                     zeus::CAxisAngle());
    }
  }
}

void CDrone::PathFindEx(CStateManager& mgr, EStateMsg msg, float dt) {
  CPatterned::PathFind(mgr, msg, dt);
  if (msg == EStateMsg::Activate) {
    zeus::CVector3f searchOff = GetTranslation() + zeus::CVector3f{0.f, 0.f, x664_};
    CPathFindSearch::EResult res = GetSearchPath()->Search(GetTranslation(), searchOff);
    if (res != CPathFindSearch::EResult::Success &&
        (res == CPathFindSearch::EResult::NoDestPoint || res == CPathFindSearch::EResult::NoPath)) {
      if (GetSearchPath()->FindClosestReachablePoint(GetTranslation(), searchOff) ==
          CPathFindSearch::EResult::Success) {
        GetSearchPath()->Search(GetTranslation(), searchOff);
        SetDestPos(searchOff);
      }
    }
  }
}

bool CDrone::Leash(CStateManager& mgr, float arg) {
  return (mgr.GetPlayer().GetTranslation() - GetTranslation()).magSquared() < x3c8_leashRadius * x3c8_leashRadius;
}

bool CDrone::InRange(CStateManager& mgr, float arg) {
  return (mgr.GetPlayer().GetTranslation() - GetTranslation()).magSquared() < x300_maxAttackRange * x300_maxAttackRange;
}

bool CDrone::SpotPlayer(CStateManager& mgr, float arg) {
  if ((mgr.GetPlayer().GetTranslation() - GetTranslation()).magSquared() > x3bc_detectionRange)
    return false;

  if (!LineOfSight(mgr, arg))
    return false;

  return (GetTransform().basis[1] + x5cc_ * GetTransform().basis[0])
             .normalized()
             .dot((mgr.GetPlayer().GetAimPosition(mgr, 0.f) - GetTranslation()).normalized()) > 0.5f;
}

bool CDrone::AnimOver(CStateManager& mgr, float arg) { return x7c8_ == 2; }

bool CDrone::ShouldAttack(CStateManager& mgr, float arg) {
  if (x5d0_ > 0.f)
    return false;
  if (TCastToPtr<CTeamAiMgr> teamMgr = mgr.ObjectById(x688_teamMgr)) {
    if (teamMgr->HasTeamAiRole(GetUniqueId()))
      teamMgr->AddRangedAttacker(GetUniqueId());
  }
  return true;
}

bool CDrone::HearShot(CStateManager& mgr, float arg) {
  rstl::reserved_vector<TUniqueId, 1024> nearList;
  BuildNearList(EMaterialTypes::Projectile, EMaterialTypes::Player, nearList, 10.f, mgr);
  return std::find_if(nearList.begin(), nearList.end(), [&mgr](TUniqueId uid) {
           if (TCastToConstPtr<CWeapon> wp = mgr.GetObjectById(uid))
             return wp->GetType() != EWeaponType::AI;
           return false;
         }) != nearList.end();
}

bool CDrone::CoverCheck(CStateManager& mgr, float arg) {
  if (!zeus::close_enough(x67c_, zeus::skZero3f)) {
    const zeus::CVector3f diff = x670_ - GetTranslation();
    return x67c_.dot(diff) < 0.0f || diff.magSquared() < 0.25f;
  }

  return true;
}
bool CDrone::LineOfSight(CStateManager& mgr, float arg) {
  return mgr.RayCollideWorld(
      GetTranslation(), mgr.GetPlayer().GetAimPosition(mgr, 0.f),
      CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid, EMaterialTypes::Character},
                                          {EMaterialTypes::Player, EMaterialTypes::ProjectilePassthrough}),
      this);
}

bool CDrone::ShouldMove(CStateManager& mgr, float arg) { return x644_ <= 0.f; }

bool CDrone::CodeTrigger(CStateManager& mgr, float arg) { return x834_29_codeTrigger; }

void CDrone::Burn(float duration, float damage) { /* Intentionally empty */ }

CPathFindSearch* CDrone::GetSearchPath() { return &x6b0_pathFind; }

void CDrone::BuildNearList(EMaterialTypes includeMat, EMaterialTypes excludeMat,
                           rstl::reserved_vector<TUniqueId, 1024>& listOut, float radius, CStateManager& mgr) {
  const zeus::CVector3f pos = GetTranslation();
  mgr.BuildNearList(listOut, zeus::CAABox(pos - radius, pos + radius),
                    CMaterialFilter::MakeIncludeExclude({includeMat}, {excludeMat}), nullptr);
}

void CDrone::SetLightEnabled(CStateManager& mgr, bool activate) {
  mgr.SendScriptMsgAlways(x578_lightId, GetUniqueId(),
                          activate ? EScriptObjectMessage::Activate : EScriptObjectMessage::Deactivate);
}

void CDrone::SetVisorFlareEnabled(CStateManager& mgr, bool activate) {}

void CDrone::UpdateVisorFlare(CStateManager& mgr) {
  // TODO: Finish
}

void CDrone::UpdateTouchBounds(float radius) {
  const zeus::CTransform xf = GetLctrTransform("Skeleton_Root"sv);
  const zeus::CVector3f diff = xf.origin - GetTranslation();
  SetBoundingBox(zeus::CAABox{diff - radius, diff + radius});
  x6b0_pathFind.SetCharacterRadius(0.25f + radius);
}

bool CDrone::HitShield(const zeus::CVector3f& dir) const {
  if (x3fc_flavor == EFlavorType::One && zeus::close_enough(x5dc_, 0.f)) {
    return GetLctrTransform("Shield_LCTR"sv).basis[1].dot(dir.normalized()) > 0.85f;
  }

  return false;
}

void CDrone::AddToTeam(CStateManager& mgr) const {
  if (x688_teamMgr == kInvalidUniqueId) {
    return;
  }

  if (TCastToPtr<CTeamAiMgr> teamMgr = mgr.ObjectById(x688_teamMgr)) {
    teamMgr->AssignTeamAiRole(*this, CTeamAiRole::ETeamAiRole::Ranged, CTeamAiRole::ETeamAiRole::Melee,
                              CTeamAiRole::ETeamAiRole::Invalid);
  }
}

void CDrone::RemoveFromTeam(CStateManager& mgr) const {
  if (TCastToPtr<CTeamAiMgr> teamMgr = mgr.ObjectById(x688_teamMgr)) {
    if (teamMgr->IsPartOfTeam(GetUniqueId())) {
      teamMgr->RemoveTeamAiRole(GetUniqueId());
    }
  }
}

void CDrone::UpdateLaser(CStateManager& mgr, u32 laserIdx, bool b1) {
  // TODO: Finish
}

void CDrone::FireProjectile(CStateManager& mgr, const zeus::CTransform& xf, const TToken<CWeaponDescription>& weapon) {}

void CDrone::StrafeFromCompanions(CStateManager& mgr) {
  if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::EAnimationState::Step)
    return;
  rstl::reserved_vector<TUniqueId, 1024> nearList;
  BuildNearList(EMaterialTypes::Character, EMaterialTypes::Player, nearList, x61c_, mgr);
  if (nearList.empty())
    return;

  float minDist = FLT_MAX;
  zeus::CVector3f nearestPos;
  for (TUniqueId uid : nearList) {
    if (const CActor* act = static_cast<const CActor*>(mgr.GetObjectById(uid))) {
      const float dist = (act->GetTranslation() - GetTranslation()).magSquared();
      if (uid != GetUniqueId() && dist < minDist) {
        minDist = dist;
        nearestPos = act->GetTranslation();
      }
    }
  }

  if (nearestPos.isZero())
    return;

  zeus::CVector3f off = nearestPos - GetTranslation();
  const float rightOff = GetTransform().basis[0].dot(off);
  if (rightOff > -0.2f && rightOff < 0.2f) {
    x450_bodyController->GetCommandMgr().DeliverCmd(CBCStepCmd(pas::EStepDirection::Left, pas::EStepType::Normal));
  } else {
    x450_bodyController->GetCommandMgr().DeliverCmd(CBCStepCmd(pas::EStepDirection::Right, pas::EStepType::Normal));
  }
}

void CDrone::UpdateScanner(CStateManager& mgr, float dt) {
  x5d4_ = zeus::CRelAngle::MakeRelativeAngle(1.2f * dt + x5d4_);
  x5d8_ = zeus::CRelAngle::MakeRelativeAngle(x5d8_);
  float angle = zeus::clamp(0.f, 0.5f * (1.f + std::sin(x5d4_)), 1.f);
  if (std::fpclassify(angle) != FP_SUBNORMAL)
    x5d8_ += 0.03f * std::pow(angle, 5.f);
  zeus::CVector3f vec =
      GetTransform().rotate(zeus::CVector3f(0.5f * std::cos(x5d8_), 1.f, 0.5f * std::sin(2.05f * x5d8_)).normalized());
  TUniqueId id;
  rstl::reserved_vector<TUniqueId, 1024> nearList;
  nearList.push_back(GetUniqueId());
  auto res = mgr.RayWorldIntersection(
      id, GetLctrTransform("Beacon_LCTR"sv).origin + (0.2f * vec), vec, 10000.f,
      CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {EMaterialTypes::ProjectilePassthrough}), nearList);
  if (res.IsValid() && x578_lightId != kInvalidUniqueId) {
    if (TCastToPtr<CGameLight> light = mgr.ObjectById(x578_lightId)) {
      light->SetTranslation(res.GetPoint());
      x7ac_lightPos = res.GetPoint();
    }
  }
}

void CDrone::sub_80163c40(float, CStateManager& mgr) {}

void CDrone::sub_801633a8(CStateManager& mgr) {}

void CDrone::sub_8015f25c(float dt, CStateManager& mgr) {}

void CDrone::sub_8015f158(float dt) {}

} // namespace urde::MP1
