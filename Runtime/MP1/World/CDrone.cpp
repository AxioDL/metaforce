#include "Runtime/MP1/World/CDrone.hpp"

#include "Runtime/Audio/CSfxManager.hpp"
#include "Runtime/Collision/CGameCollision.hpp"
#include "Runtime/CSimplePool.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/MP1/World/CDroneLaser.hpp"
#include "Runtime/Particle/CWeaponDescription.hpp"
#include "Runtime/Weapon/CGameProjectile.hpp"
#include "Runtime/Weapon/CWeapon.hpp"
#include "Runtime/World/CGameLight.hpp"
#include "Runtime/World/CPatternedInfo.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "Runtime/World/CScriptVisorFlare.hpp"
#include "Runtime/World/CScriptWater.hpp"
#include "Runtime/World/CTeamAiMgr.hpp"
#include "Runtime/World/CWorld.hpp"

#include "Audio/SFX/Drones.h"

#include "TCastTo.hpp" // Generated file, do not modify include path

#include <algorithm>

namespace metaforce::MP1 {
CDrone::CDrone(TUniqueId uid, std::string_view name, EFlavorType flavor, const CEntityInfo& info,
               const zeus::CTransform& xf, float f1, CModelData&& mData, const CPatternedInfo& pInfo,
               const CActorParameters& actParms, EMovementType movement, EColliderType colliderType, EBodyType bodyType,
               const CDamageInfo& dInfo1, CAssetId aId1, const CDamageInfo& dInfo2, CAssetId aId2,
               std::vector<CVisorFlare::CFlareDef> flares, float f2, float f3, float f4, float f5, float f6, float f7,
               float f8, float f9, float f10, float f11, float f12, float f13, float f14, float f15, float f16,
               float f17, float f18, float f19, float f20, CAssetId crscId, float f21, float f22, float f23, float f24,
               s32 sId, bool b1)
: CPatterned(ECharacter::Drone, uid, name, flavor, info, xf, std::move(mData), pInfo, movement, colliderType, bodyType,
             actParms, flavor == EFlavorType::Zero ? EKnockBackVariant::Medium : EKnockBackVariant::Large)
, x568_laserParticlesId(aId1)
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
, x690_colSphere(zeus::CSphere({0.f, 0.f, 1.8f}, 1.1f), CActor::GetMaterialList())
, x6b0_pathFind(nullptr, 3 + int(b1) /* TODO double check */, pInfo.GetPathfindingIndex(), 1.f, 2.4f)
, x7cc_laserSfx(CSfxManager::TranslateSFXID(sId))
, x82c_shieldModel(std::make_unique<CModelData>(CStaticRes{aId2, zeus::skOne3f}))
, x835_25_(b1) {
  UpdateTouchBounds(pInfo.GetHalfExtent());
  x460_knockBackController.SetEnableShock(true);
  x460_knockBackController.SetAvailableState(EKnockBackAnimationState::Hurled, false);
  x460_knockBackController.SetLocomotionDuringElectrocution(true);
  MakeThermalColdAndHot();
  CreateShadow(flavor != EFlavorType::One);
}

void CDrone::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CDrone::Think(float dt, CStateManager& mgr) {
  if (x3fc_flavor == EFlavorType::One) {
    if (mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::EPlayerVisor::XRay) {
      x42c_color.a() = 1.f;
    } else {
      x42c_color.a() = std::max(0.f, x428_damageCooldownTimer / 0.33f);
    }
  }

  x403_25_enableStateMachine = !GetBodyController()->IsElectrocuting();
  if (GetBodyController()->IsElectrocuting() && (x824_activeLasers[0] || x824_activeLasers[1])) {
    x824_activeLasers[0] = false;
    x824_activeLasers[1] = false;
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

  if (x824_activeLasers[0] || (x824_activeLasers[1] && IsAlive())) {
    UpdateLasers(mgr, dt);
    UpdateVisorFlare(mgr);
  }

  if (x834_25_ && IsAlive()) {
    UpdateScanner(mgr, dt);
  }

  const float dist = (mgr.GetPlayer().GetTranslation() - GetTranslation()).magSquared();
  if (x834_28_ && dist < x60c_ * x60c_) {
    mgr.GetPlayerState()->GetStaticInterference().RemoveSource(GetUniqueId());
    mgr.GetPlayerState()->GetStaticInterference().AddSource(
        GetUniqueId(), std::max(0.f, x608_ - mgr.GetPlayerState()->GetStaticInterference().GetTotalInterference()),
        0.2f);
  }

  if (!x834_28_ && dist < x614_ * x614_) {
    mgr.GetPlayerState()->GetStaticInterference().RemoveSource(GetUniqueId());
    mgr.GetPlayerState()->GetStaticInterference().AddSource(
        GetUniqueId(), std::max(0.f, x610_ - mgr.GetPlayerState()->GetStaticInterference().GetTotalInterference()),
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
      x5dc_ = zeus::min(1.f, x5dc_ + (3.f * dt));
    }
    x5e8_shieldTime = zeus::max(0.f, x5e8_shieldTime - dt);

    if (zeus::close_enough(x5dc_, 0.f)) {
      if (x7d0_) {
        CSfxManager::RemoveEmitter(x7d0_);
        x7d0_.reset();
      }
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
    x668_elevation = mgr.RayStaticIntersection(GetTranslation(), zeus::skDown, 10000.f,
                                               CMaterialFilter::MakeInclude({EMaterialTypes::Solid}))
                         .GetT();
    x66c_ = 0.f;
  }

  if (IsAlive() && x835_25_) {
    zeus::CAABox box = GetBoundingBox();
    box.accumulateBounds(GetTranslation() + 20.f * zeus::skDown);
    EntityList nearList;
    mgr.BuildNearList(nearList, GetBoundingBox(), CMaterialFilter::MakeInclude({EMaterialTypes::Trigger}), this);
    for (TUniqueId id : nearList) {
      if (const TCastToConstPtr<CScriptWater> water = mgr.GetObjectById(id)) {
        zeus::CAABox waterBox = water->GetTriggerBoundsWR();
        if (waterBox.max.z() - GetTranslation().z() < 3.f) {
          float z = 20.f;
          if (waterBox.max.z() - GetTranslation().z() < 1.5f) {
            z = 60.f;
          }
          ApplyImpulseWR(GetMoveToORImpulseWR(GetTransform().transposeRotate(z * (dt * zeus::skDown)), dt),
                         zeus::CAxisAngle());
        }
      }
    }
  }
  if (IsAlive() && x668_elevation < x664_) {
    ApplyImpulseWR(GetMoveToORImpulseWR(GetTransform().transposeRotate(dt * (1.f * zeus::skUp)), dt),
                   zeus::CAxisAngle());
  }
  xe7_31_targetable = IsAlive();
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
    for (TUniqueId& unkId : x7d8_laserIds) {
      if (unkId != kInvalidUniqueId) {
        mgr.FreeScriptObject(unkId);
        unkId = kInvalidUniqueId;
      }
    }
    RemoveFromTeam(mgr);
    mgr.GetPlayerState()->GetStaticInterference().RemoveSource(GetUniqueId());
    if (x578_lightId != kInvalidUniqueId) {
      mgr.FreeScriptObject(x578_lightId);
      x578_lightId = kInvalidUniqueId;
    }
    if (x57a_visorFlareId != kInvalidUniqueId) {
      mgr.FreeScriptObject(x57a_visorFlareId);
      x57a_visorFlareId = kInvalidUniqueId;
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
    if (!x835_26_ && x834_24_waveHit && !IsAlive()) {
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

void CDrone::AddToRenderer(const zeus::CFrustum& frustum, CStateManager& mgr) {
  CPatterned::AddToRenderer(frustum, mgr);
}

void CDrone::PreRender(CStateManager& mgr, const zeus::CFrustum& frustum) {
  CPatterned::PreRender(mgr, frustum);
  if (x3fc_flavor == EFlavorType::One) {
    if (HasModelData() && GetModelData()->HasAnimData() && GetModelData()->HasNormalModel()) {
      if (GetModelAlphau8(mgr) == 0)
        GetModelData()->GetAnimationData()->BuildPose();
    }
  }
}

void CDrone::Render(CStateManager& mgr) {
  bool isOne = x3fc_flavor == EFlavorType::One;
  if (!isOne || GetModelAlphau8(mgr) != 0) {
    if (isOne && mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::EPlayerVisor::XRay) {
      CElementGen::SetSubtractBlend(true);
      CElementGen::SetMoveRedToAlphaBuffer(true);
      CGraphics::SetFog(ERglFogMode::PerspLin, 0.f, 75.f, zeus::skBlack);
      GetModelData()->GetAnimationData()->GetParticleDB().RenderSystemsToBeDrawnFirst();
      mgr.SetupFogForArea3XRange(GetAreaIdAlways());
    }
    CPatterned::Render(mgr);
    if (isOne && mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::EPlayerVisor::XRay) {
      CGraphics::SetFog(ERglFogMode::PerspLin, 0.f, 75.f, zeus::skBlack);
      GetModelData()->GetAnimationData()->GetParticleDB().RenderSystemsToBeDrawnLast();
      mgr.SetupFogForArea(GetAreaIdAlways());
      CElementGen::SetSubtractBlend(false);
      CElementGen::SetMoveRedToAlphaBuffer(false);
    }

    if (isOne && !zeus::close_enough(x5dc_, 0)) {
      x82c_shieldModel->Render(
          mgr, GetLctrTransform("Shield_LCTR"sv), GetActorLights(),
          CModelFlags{8, 0, 3, zeus::CColor::lerp({1.f, 1.f, 1.f, x5dc_}, {1.f, 0.f, 0.f, 1.f}, x5e8_shieldTime)});
    }
  }
}

bool CDrone::CanRenderUnsorted(const CStateManager& mgr) const {
  if (!zeus::close_enough(x5dc_, 0.f))
    return false;
  return CPatterned::CanRenderUnsorted(mgr);
}

const CDamageVulnerability* CDrone::GetDamageVulnerability(const zeus::CVector3f&, const zeus::CVector3f& dir,
                                                           const CDamageInfo&) const {
  if (x3fc_flavor == EFlavorType::One && HitShield(-dir)) {
    x5e8_shieldTime = 1.f;
    return &CDamageVulnerability::ReflectVulnerabilty();
  }
  return CAi::GetDamageVulnerability();
}

void CDrone::Touch(CActor& act, CStateManager& mgr) {
  CPatterned::Touch(act, mgr);
  if (TCastToPtr<CWeapon> weapon = act) {
    if (IsAlive()) {
      x834_24_waveHit = weapon->GetType() == EWeaponType::Wave;
      if (x3fc_flavor == CPatterned::EFlavorType::One && HitShield(weapon->GetTranslation() - GetTranslation())) {
        x5e8_shieldTime = 1.f;
      }
    }
  }
}

EWeaponCollisionResponseTypes CDrone::GetCollisionResponseType(const zeus::CVector3f&, const zeus::CVector3f& dir,
                                                               const CWeaponMode&, EProjectileAttrib) const {
  if (x3fc_flavor == EFlavorType::One && HitShield(-dir)) {
    x5e8_shieldTime = 1.f;
    return EWeaponCollisionResponseTypes::Unknown86;
  }
  return EWeaponCollisionResponseTypes::Unknown36;
}

void CDrone::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type, float dt) {
  switch (type) {
  case EUserEventType::Projectile:
    sub_80165984(mgr, GetLctrTransform(node.GetLocatorName()));
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
      if (!x824_activeLasers[0]) {
        UpdateLaser(mgr, 0, true);
        x824_activeLasers[0] = true;
        SetVisorFlareEnabled(mgr, true);
      } else if (x3fc_flavor == EFlavorType::One) {
        UpdateLaser(mgr, 1, true);
        x824_activeLasers[1] = true;
      }
    }
    return;
  }
  case EUserEventType::DamageOff: {
    if (x824_activeLasers[0]) {
      UpdateLaser(mgr, 0, false);
      x824_activeLasers[0] = false;
      SetVisorFlareEnabled(mgr, false);
    } else if (x3fc_flavor == EFlavorType::One) {
      UpdateLaser(mgr, 1, false);
      x824_activeLasers[1] = false;
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
    return &x690_colSphere;
  return CPatterned::GetCollisionPrimitive();
}

void CDrone::Death(CStateManager& mgr, const zeus::CVector3f& direction, EScriptObjectState state) {
  if (!IsAlive())
    return;

  x824_activeLasers[0] = false;
  x824_activeLasers[1] = false;
  UpdateLaser(mgr, 0, false);
  UpdateLaser(mgr, 1, false);
  SetVisorFlareEnabled(mgr, false);

  if (x3e4_lastHP - HealthInfo(mgr)->GetHP() < x3d8_xDamageThreshold || x834_24_waveHit) {
    x330_stateMachineState.SetState(mgr, *this, GetStateMachine(), "Dead"sv);
  } else {
    x834_28_ = true;
    if (x3e0_xDamageDelay <= 0.f) {
      SetTransform(zeus::lookAt(GetTranslation(), GetTranslation() - direction) *
                   zeus::CTransform::RotateX(zeus::degToRad(45.f)));
    }
  }

  if (x450_bodyController->GetPercentageFrozen() > 0.f) {
    x450_bodyController->UnFreeze();
  }

  x400_25_alive = false;
  SendScriptMsgs(state, mgr, EScriptObjectMessage::None);
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
    EntityList nearList;
    BuildNearList(EMaterialTypes::Character, EMaterialTypes::Player, nearList, 5.f, mgr);
    if (!nearList.empty()) {
      zeus::CVector3f sep = x45c_steeringBehaviors.Separation(
          *this, static_cast<const CActor*>(mgr.GetObjectById(nearList[0]))->GetTranslation(), 5.f);
      if (!sep.isZero()) {
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(sep, zeus::skZero3f, 0.5f));
      }
    }
  } else if (msg == EStateMsg::Deactivate) {
    SetLightEnabled(mgr, false);
    x834_25_ = false;
  }
  CPatterned::Patrol(mgr, msg, dt);
}

void CDrone::PathFind(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    auto searchOff = GetDestPos();
    CPathFindSearch::EResult res = GetSearchPath()->Search(GetTranslation(), searchOff);
    if (res != CPathFindSearch::EResult::Success &&
        (res == CPathFindSearch::EResult::NoDestPoint || res == CPathFindSearch::EResult::NoPath)) {
      if (GetSearchPath()->FindClosestReachablePoint(GetTranslation(), searchOff) ==
          CPathFindSearch::EResult::Success) {
        GetSearchPath()->Search(GetTranslation(), searchOff);
        SetDestPos(searchOff);
      }
    }
    if (x3fc_flavor == CPatterned::EFlavorType::One) {
      x834_30_visible = true;
    }
  } else if (msg == EStateMsg::Update) {
    CPatterned::PathFind(mgr, msg, dt);
    x450_bodyController->GetCommandMgr().BlendSteeringCmds();
    zeus::CVector3f moveVec = x450_bodyController->GetCommandMgr().GetMoveVector();
    if (moveVec.canBeNormalized()) {
      moveVec.normalize();
      x450_bodyController->GetCommandMgr().ClearLocomotionCmds();
      ApplyImpulseWR(GetMass() * (x5e4_ * moveVec), {});
      const auto target = (mgr.GetPlayer().GetAimPosition(mgr, 0.f) - GetTranslation()).normalized();
      x450_bodyController->GetCommandMgr().DeliverCmd(
          CBCLocomotionCmd(FLT_EPSILON * GetTransform().frontVector(), target, 1.f));
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
    x450_bodyController->SetTurnSpeed(x5ec_turnSpeed);
    if (x450_bodyController->GetLocomotionType() != pas::ELocomotionType::Combat)
      x450_bodyController->SetLocomotionType(pas::ELocomotionType::Combat);
    x450_bodyController->BodyStateInfo().SetMaximumPitch(zeus::degToRad(60.f));
    SetDestPos(mgr.GetPlayer().GetAimPosition(mgr, 0.f));
    x400_24_hitByPlayerProjectile = false;
    if (x3fc_flavor == EFlavorType::One)
      x834_30_visible = true;
    x330_stateMachineState.SetDelay(std::max(0.3f, x624_));
  } else if (msg == EStateMsg::Update) {
    zeus::CVector3f target = (mgr.GetPlayer().GetAimPosition(mgr, 0.f) - GetTranslation()).normalized();
    x450_bodyController->GetCommandMgr().DeliverCmd(
        CBCLocomotionCmd(FLT_EPSILON * GetTransform().frontVector(), target, 1.f));
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
  if (msg == EStateMsg::Activate) {
    x7c8_ = 0;
    x834_31_attackOver = false;
    const auto playerAimPos = mgr.GetPlayer().GetAimPosition(mgr, 0.f);
    const auto& xf = GetTransform();
    const auto frontVec = xf.frontVector();
    zeus::CVector3f out;
    if ((playerAimPos - GetTranslation()).normalized().dot(frontVec) >= 0.8f) {
      out = playerAimPos;
    } else {
      out = GetTranslation() + 10.f * frontVec;
    }
    s32 state = mgr.GetActiveRandom()->Next() % 4;
    if (state == 0) {
      x7e0_lasersStart[0] = out + (3.f * xf.rightVector()) - (4.f * xf.upVector());
      x7fc_lasersEnd[0] = out - (3.f * xf.rightVector()) + (4.f * xf.upVector());
      x7e0_lasersStart[1] = out - (3.f * xf.rightVector()) - (4.f * xf.upVector());
      x7fc_lasersEnd[1] = out + (3.f * xf.rightVector()) + (4.f * xf.upVector());
    } else if (state == 1) {
      x7e0_lasersStart[0] = out + (3.f * xf.rightVector()) + (4.f * xf.upVector());
      x7fc_lasersEnd[0] = out - (3.f * xf.rightVector()) - (4.f * xf.upVector());
      x7e0_lasersStart[1] = out - (3.f * xf.rightVector()) + (4.f * xf.upVector());
      x7fc_lasersEnd[1] = out + (3.f * xf.rightVector()) - (4.f * xf.upVector());
    } else if (state == 2) {
      x7e0_lasersStart[0] = out - (4.f * xf.rightVector()) - (3.f * xf.upVector());
      x7fc_lasersEnd[0] = out + (4.f * xf.rightVector()) + (3.f * xf.upVector());
      x7e0_lasersStart[1] = out + (4.f * xf.rightVector()) - (3.f * xf.upVector());
      x7fc_lasersEnd[1] = out - (4.f * xf.rightVector()) + (3.f * xf.upVector());
    } else if (state == 3) {
      x7e0_lasersStart[0] = out - (4.f * xf.rightVector()) + (3.f * xf.upVector());
      x7fc_lasersEnd[0] = out + (4.f * xf.rightVector()) - (3.f * xf.upVector());
      x7e0_lasersStart[1] = out + (4.f * xf.rightVector()) + (3.f * xf.upVector());
      x7fc_lasersEnd[1] = out - (4.f * xf.rightVector()) - (3.f * xf.upVector());
    }
    x818_lasersTime[0] = 0.f;
    x818_lasersTime[1] = 0.f;
    x835_24_ = true;
  } else if (msg == EStateMsg::Update) {
    if (x7c8_ == 0) {
      if (GetBodyController()->GetCurrentStateId() == pas::EAnimationState::ProjectileAttack) {
        x7c8_ = 1;
      } else {
        GetBodyController()->GetCommandMgr().DeliverCmd(
            CBCProjectileAttackCmd(pas::ESeverity::Two, mgr.GetPlayer().GetTranslation(), false));
      }
    } else if (x7c8_ == 1) {
      if (GetBodyController()->GetCurrentStateId() != pas::EAnimationState::ProjectileAttack) {
        x7c8_ = 2;
      }
    }
    if (x630_ <= 0.f) {
      x634_ = 0.f;
    }
  } else if (msg == EStateMsg::Deactivate) {
    x824_activeLasers[0] = false;
    x824_activeLasers[1] = false;
    UpdateLaser(mgr, 0, false);
    UpdateLaser(mgr, 1, false);
    SetVisorFlareEnabled(mgr, false);
    x5d0_ = x5f4_;
    x835_24_ = false;
  }
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
      x832_b = mgr.GetActiveRandom()->Float() >= 0.5f ? 1 : 2;
    }
  } else if (msg == EStateMsg::Update) {
    if (x7c8_ == 0) {
      if (GetBodyController()->GetBodyStateInfo().GetCurrentStateId() == pas::EAnimationState::Step) {
        x7c8_ = 1;
      } else if (x832_b == 0) {
        GetBodyController()->GetCommandMgr().DeliverCmd(
            CBCStepCmd(pas::EStepDirection::Backward, pas::EStepType::BreakDodge));
      } else if (x832_b == 1) {
        GetBodyController()->GetCommandMgr().DeliverCmd(CBCStepCmd(pas::EStepDirection::Left, pas::EStepType::Normal));
      } else if (x832_b == 2) {
        GetBodyController()->GetCommandMgr().DeliverCmd(CBCStepCmd(pas::EStepDirection::Right, pas::EStepType::Normal));
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
      if (x58c_prevDodgeDir == pas::EStepDirection::Down) {
        GetBodyController()->GetCommandMgr().DeliverCmd(CBCStepCmd(pas::EStepDirection::Left, pas::EStepType::Dodge));
        x58c_prevDodgeDir = pas::EStepDirection::Left;
      } else if (x58c_prevDodgeDir == pas::EStepDirection::Up) {
        GetBodyController()->GetCommandMgr().DeliverCmd(CBCStepCmd(pas::EStepDirection::Down, pas::EStepType::Dodge));
        x58c_prevDodgeDir = pas::EStepDirection::Down;
      } else if (x58c_prevDodgeDir == pas::EStepDirection::Right) {
        GetBodyController()->GetCommandMgr().DeliverCmd(CBCStepCmd(pas::EStepDirection::Up, pas::EStepType::Dodge));
        x58c_prevDodgeDir = pas::EStepDirection::Up;
      } else if (x58c_prevDodgeDir == pas::EStepDirection::Left) {
        GetBodyController()->GetCommandMgr().DeliverCmd(CBCStepCmd(pas::EStepDirection::Right, pas::EStepType::Dodge));
        x58c_prevDodgeDir = pas::EStepDirection::Right;
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
    GetBodyController()->GetCommandMgr().DeliverCmd(
        CBCLocomotionCmd(GetTransform().frontVector(), zeus::skZero3f, 1.f));
    zeus::CVector3f local_74 =
        0.5f * (mgr.GetPlayer().GetAimPosition(mgr, 0.f) + mgr.GetPlayer().GetTranslation()) - GetTranslation();
    if (((x668_elevation < x664_ && local_74.z() > 0.f) || (x668_elevation > x664_)) && local_74.canBeNormalized()) {
      ApplyImpulseWR(GetMoveToORImpulseWR(GetTransform().transposeRotate(dt * (x5e4_ * local_74.normalized())), dt),
                     zeus::CAxisAngle());
    }
  }
}

void CDrone::PathFindEx(CStateManager& mgr, EStateMsg msg, float dt) {
  CPatterned::PathFind(mgr, msg, dt);
  if (msg == EStateMsg::Activate) {
    auto searchOff = mgr.GetPlayer().GetTranslation() + zeus::CVector3f{0.f, 0.f, x664_};
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
  float mag = (mgr.GetPlayer().GetTranslation() - GetTranslation()).magSquared();
  return mag > x2fc_minAttackRange * x2fc_minAttackRange && mag < x300_maxAttackRange * x300_maxAttackRange;
}

bool CDrone::SpotPlayer(CStateManager& mgr, float arg) {
  if ((mgr.GetPlayer().GetTranslation() - GetTranslation()).magSquared() > x3bc_detectionRange * x3bc_detectionRange)
    return false;

  if (!LineOfSight(mgr, arg))
    return false;

  return (GetTransform().frontVector() + x5cc_ * GetTransform().rightVector())
             .normalized()
             .dot((mgr.GetPlayer().GetAimPosition(mgr, 0.f) - GetTranslation()).normalized()) > 0.5f;
}

bool CDrone::AnimOver(CStateManager& mgr, float arg) { return x7c8_ == 2; }

bool CDrone::AttackOver(CStateManager& mgr, float arg) { return x834_31_attackOver; }

bool CDrone::ShouldAttack(CStateManager& mgr, float arg) {
  if (x5d0_ > 0.f)
    return false;
  if (TCastToPtr<CTeamAiMgr> teamMgr = mgr.ObjectById(x688_teamMgr)) {
    if (teamMgr->HasTeamAiRole(GetUniqueId()))
      return teamMgr->AddRangedAttacker(GetUniqueId());
  }
  return true;
}

bool CDrone::ShouldFire(CStateManager& mgr, float arg) {
  if (mgr.GetPlayer().GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Morphed || x624_ > 0.f) {
    return false;
  }
  const zeus::CVector3f playerAimPos = mgr.GetPlayer().GetAimPosition(mgr, 0.f);
  constexpr auto matFilter =
      CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid, EMaterialTypes::Character},
                                          {EMaterialTypes::Player, EMaterialTypes::ProjectilePassthrough});
  bool result = mgr.RayCollideWorld(GetLctrTransform("R_GUN_TOP_LCTR"sv).origin, playerAimPos, matFilter, this);
  if (!result) {
    return false;
  }
  return mgr.RayCollideWorld(GetLctrTransform("L_GUN_TOP_LCTR"sv).origin, playerAimPos, matFilter, this);
}

bool CDrone::HearShot(CStateManager& mgr, float arg) {
  EntityList nearList;
  BuildNearList(EMaterialTypes::Projectile, EMaterialTypes::Player, nearList, 10.f, mgr);
  return std::any_of(nearList.begin(), nearList.end(), [&mgr](TUniqueId uid) {
    if (TCastToConstPtr<CWeapon> wp = mgr.GetObjectById(uid))
      return wp->GetType() != EWeaponType::AI;
    return false;
  });
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

void CDrone::Burn(float duration, float damage) {
  // Intentionally empty
}

CPathFindSearch* CDrone::GetSearchPath() { return &x6b0_pathFind; }

void CDrone::BuildNearList(EMaterialTypes includeMat, EMaterialTypes excludeMat, EntityList& listOut, float radius,
                           CStateManager& mgr) {
  const zeus::CVector3f pos = GetTranslation();
  mgr.BuildNearList(listOut, zeus::CAABox(pos - radius, pos + radius),
                    CMaterialFilter::MakeIncludeExclude({includeMat}, {excludeMat}), nullptr);
}

void CDrone::SetLightEnabled(CStateManager& mgr, bool activate) {
  mgr.SendScriptMsgAlways(x578_lightId, GetUniqueId(),
                          activate ? EScriptObjectMessage::Activate : EScriptObjectMessage::Deactivate);
}

void CDrone::SetVisorFlareEnabled(CStateManager& mgr, bool activate) {
  if (!IsAlive()) {
    return;
  }
  CScriptVisorFlare* flare = TCastToPtr<CScriptVisorFlare>{mgr.ObjectById(x57a_visorFlareId)};
  if (flare == nullptr && activate) {
    x57a_visorFlareId = mgr.AllocateUniqueId();
    flare = new CScriptVisorFlare(x57a_visorFlareId, "DroneVisorFlare"sv,
                                  CEntityInfo{GetAreaIdAlways(), CEntity::NullConnectionList}, activate,
                                  GetLctrTransform("Beacon_LCTR"sv).origin, CVisorFlare::EBlendMode::Additive, true,
                                  0.1f, 1.f, 2.f, 0, 0, x57c_flares);
    mgr.AddObject(flare);
  }
  mgr.SendScriptMsg(flare, GetUniqueId(), activate ? EScriptObjectMessage::Activate : EScriptObjectMessage::Deactivate);
}

void CDrone::UpdateVisorFlare(CStateManager& mgr) {
  TCastToPtr<CScriptVisorFlare> flare = mgr.ObjectById(x57a_visorFlareId);
  SetVisorFlareEnabled(
      mgr, (mgr.GetPlayer().GetTranslation() - GetTranslation()).normalized().dot(GetTransform().frontVector()) > 0.f);
  if (flare) {
    const auto beaconXf = GetLctrTransform("Beacon_LCTR"sv);
    flare->SetTranslation(beaconXf.origin + (0.1f * beaconXf.frontVector()));
  }
}

void CDrone::UpdateTouchBounds(float radius) {
  const zeus::CTransform xf = GetLctrTransform("Skeleton_Root"sv);
  const zeus::CVector3f diff = xf.origin - GetTranslation();
  x690_colSphere.SetSphereCenter(diff);
  x690_colSphere.SetSphereRadius(radius);
  SetBoundingBox(zeus::CAABox{diff - radius, diff + radius});
  x6b0_pathFind.SetCharacterRadius(0.25f + radius);
}

bool CDrone::HitShield(const zeus::CVector3f& dir) const {
  if (x3fc_flavor == EFlavorType::One && !zeus::close_enough(x5dc_, 0.f)) {
    return GetLctrTransform("Shield_LCTR"sv).frontVector().dot(dir.normalized()) > 0.85f;
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

void CDrone::UpdateLaser(CStateManager& mgr, u32 laserIdx, bool active) {
  if (active && x7d8_laserIds[laserIdx] == kInvalidUniqueId) {
    x7d8_laserIds[laserIdx] = mgr.AllocateUniqueId();
    mgr.AddObject(new CDroneLaser(x7d8_laserIds[laserIdx], GetAreaIdAlways(), GetTransform(), x568_laserParticlesId));
  }
  if (CEntity* ent = mgr.ObjectById(x7d8_laserIds[laserIdx])) {
    mgr.SendScriptMsg(ent, GetUniqueId(), active ? EScriptObjectMessage::Activate : EScriptObjectMessage::Deactivate);
  }
}

void CDrone::FireProjectile(CStateManager& mgr, const zeus::CTransform& xf, const TToken<CWeaponDescription>& weapon) {
  // TODO implement
}

void CDrone::StrafeFromCompanions(CStateManager& mgr) {
  if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::EAnimationState::Step)
    return;
  EntityList nearList;
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

  if (nearestPos.isZero() || minDist > x61c_ * x61c_)
    return;

  const auto off = nearestPos - GetTranslation();
  const float rightOff = GetTransform().rightVector().dot(off);
  if (rightOff < -0.2f) {
    x450_bodyController->GetCommandMgr().DeliverCmd(CBCStepCmd(pas::EStepDirection::Right, pas::EStepType::Normal));
  } else if (rightOff > 0.2f) {
    x450_bodyController->GetCommandMgr().DeliverCmd(CBCStepCmd(pas::EStepDirection::Left, pas::EStepType::Normal));
  }
}

void CDrone::UpdateScanner(CStateManager& mgr, float dt) {
  constexpr float deg360 = zeus::degToRad(360.f);
  x5d4_ = 1.2f * dt + x5d4_;
  if (x5d4_ > deg360) {
    x5d4_ -= deg360;
  }
  if (x5d4_ < 0.f) {
    x5d4_ = 0.f;
  }
  if (x5d8_ > deg360) {
    x5d8_ -= deg360;
  }
  if (x5d8_ < 0.f) {
    x5d8_ = 0.f;
  }
  float angle = zeus::clamp(0.f, 0.5f * (1.f + std::sin(x5d4_)), 1.f);
  if (std::fpclassify(angle) != FP_SUBNORMAL)
    x5d8_ += 0.03f * std::pow(angle, 5.f);
  zeus::CVector3f vec =
      GetTransform().rotate(zeus::CVector3f(0.5f * std::cos(x5d8_), 1.f, 0.5f * std::sin(2.05f * x5d8_)).normalized());
  TUniqueId id;
  EntityList nearList;
  nearList.push_back(mgr.GetPlayer().GetUniqueId());
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

void CDrone::UpdateLasers(CStateManager& mgr, float dt) {
  constexpr auto matFilter =
      CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {EMaterialTypes::ProjectilePassthrough});
  const auto beaconXf = GetLctrTransform("Beacon_LCTR"sv);
  for (size_t i = 0; i < x818_lasersTime.size(); ++i) {
    if (x818_lasersTime[i] >= 1.f || !x824_activeLasers[i]) {
      continue;
    }
    x818_lasersTime[i] += dt;
    const auto vec =
        (x7e0_lasersStart[i] * (1.f - x818_lasersTime[i]) + (x7fc_lasersEnd[i] * x818_lasersTime[i]) - beaconXf.origin)
            .normalized();
    auto box = zeus::skInvertedBox;
    box.accumulateBounds(GetTranslation() + 1000.f * vec);
    box.accumulateBounds(GetTranslation());
    EntityList nearList;
    mgr.BuildNearList(nearList, box, matFilter, nullptr);
    TUniqueId id;
    const auto result = mgr.RayWorldIntersection(id, beaconXf.origin + 2.f * vec, vec, 10000.f, matFilter, nearList);
    if (result.IsInvalid()) {
      continue;
    }
    if (x7d8_laserIds[i] != kInvalidUniqueId) {
      if (auto* laser = static_cast<CDroneLaser*>(mgr.ObjectById(x7d8_laserIds[i]))) {
        laser->SetTransform(beaconXf);
        laser->sub_80167754(mgr, result.GetPoint(), result.GetPlane().normal());
      }
    }
    if (TCastToPtr<CPlayer> player = mgr.ObjectById(id)) {
      if (x420_curDamageRemTime <= 0.f) {
        mgr.ApplyDamage(GetUniqueId(), player->GetUniqueId(), GetUniqueId(), GetContactDamage(),
                        CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {}), zeus::skZero3f);
        x420_curDamageRemTime = x424_damageWaitTime;
        mgr.GetPlayerState()->GetStaticInterference().AddSource(GetUniqueId(), 0.3f, 1.f);
        CSfxManager::AddEmitter(x7cc_laserSfx, result.GetPoint(), zeus::skZero3f, true, false, 127, GetAreaIdAlways());
      }
    }
    if (id != GetUniqueId() && TCastToPtr<CAi>{mgr.ObjectById(id)}) {
      x834_31_attackOver = true;
      float rem = GetModelData()->GetAnimationData()->GetAnimTimeRemaining("Whole Body"sv);
      UpdateAnimation(rem, mgr, true);
    }
  }
}

void CDrone::sub_801633a8(CStateManager& mgr) {
  // TODO implement
}

void CDrone::sub_8015f25c(float dt, CStateManager& mgr) {
  // TODO implement
}

void CDrone::sub_8015f158(float dt) {
  // TODO implement
}

void CDrone::sub_80165984(CStateManager& mgr, const zeus::CTransform& xf) {
  /*constexpr*/ float sin60 = std::sqrt(3.f) / 2.f;
  const auto playerAimPos = mgr.GetPlayer().GetAimPosition(mgr, 0.f);
  const auto distNorm = (playerAimPos - xf.origin).normalized();
  if (distNorm.dot(xf.frontVector()) <= sin60) {
    sub_801656d4(xf, mgr);
  } else {
    zeus::CVector3f vec;
    if (mgr.GetActiveRandom()->Float() > 0.2f) {
      const auto lookAt = zeus::lookAt(xf.origin, playerAimPos);
      vec = zeus::CQuaternion::fromAxisAngle(lookAt.frontVector(), mgr.GetActiveRandom()->Range(0.f, M_PIF))
                .transform(4.f * lookAt.rightVector());
    }
    sub_801656d4(zeus::lookAt(xf.origin, playerAimPos + vec), mgr);
  }
}

void CDrone::sub_801656d4(const zeus::CTransform& xf, CStateManager& mgr) {
  constexpr auto matFilter =
      CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {EMaterialTypes::ProjectilePassthrough});
  EntityList nearList;
  mgr.BuildNearList(nearList, xf.origin, xf.frontVector(), 100000.f, matFilter, this);
  TUniqueId id;
  const auto result = mgr.RayWorldIntersection(id, xf.origin, xf.frontVector(), 100000.f, matFilter, nearList);
  if (result.IsInvalid()) {
    return;
  }
  if (id == mgr.GetPlayer().GetUniqueId()) {
    mgr.ApplyDamage(GetUniqueId(), id, GetUniqueId(), x5ac_,
                    CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {}), zeus::skZero3f);
  }
  mgr.sub_80044098(*x56c_.GetObj(), result, id, x5ac_.GetWeaponMode(), 1, xe6_27_thermalVisorFlags);
}

void CDrone::Dead(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x460_knockBackController.SetAutoResetImpulse(false);
    if (x834_24_waveHit) {
      SetMomentumWR({0.f, 0.f, -GetWeight()});
    } else {
      Stop();
      SetVelocityWR(zeus::skZero3f);
      SetMomentumWR(zeus::skZero3f);
    }
    x401_26_disableMove = true;
    x5c8_ = 0.f;
    SetVisorFlareEnabled(mgr, false);
    x7c8_ = 0;
  } else if (msg == EStateMsg::Update && x7c0_ == 0) {
    if (x834_24_waveHit) {
      GetBodyController()->GetCommandMgr().DeliverCmd(CBCHurledCmd());
      x7c8_ = 1;
    } else {
      GetBodyController()->GetCommandMgr().DeliverCmd(CBCKnockDownCmd(zeus::skZero3f, pas::ESeverity::Zero));
      x7c8_ = 1;
      Stop();
    }
  }
}

} // namespace metaforce::MP1
