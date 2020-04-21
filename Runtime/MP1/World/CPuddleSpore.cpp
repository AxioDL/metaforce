#include "Runtime/MP1/World/CPuddleSpore.hpp"

#include "Runtime/CSimplePool.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Collision/CGameCollision.hpp"
#include "Runtime/Weapon/CEnergyProjectile.hpp"
#include "Runtime/Weapon/CGameProjectile.hpp"
#include "Runtime/World/CPatternedInfo.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "Runtime/World/CScriptWater.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde::MP1 {
constexpr u32 kEyeCount = 16;

constexpr std::array kEyeLocators{
    "Glow_1_LCTR"sv,  "Glow_2_LCTR"sv,  "Glow_3_LCTR"sv,  "Glow_4_LCTR"sv,  "Glow_5_LCTR"sv,  "Glow_6_LCTR"sv,
    "Glow_7_LCTR"sv,  "Glow_8_LCTR"sv,  "Glow_9_LCTR"sv,  "Glow_10_LCTR"sv, "Glow_11_LCTR"sv, "Glow_12_LCTR"sv,
    "Glow_13_LCTR"sv, "Glow_14_LCTR"sv, "Glow_15_LCTR"sv, "Glow_16_LCTR"sv,
};

CPuddleSpore::CPuddleSpore(TUniqueId uid, std::string_view name, EFlavorType flavor, const CEntityInfo& info,
                           const zeus::CTransform& xf, CModelData&& mData, const CPatternedInfo& pInfo,
                           EColliderType colType, CAssetId glowFx, float f1, float f2, float f3, float f4, float f5,
                           const CActorParameters& actParms, CAssetId weapon, const CDamageInfo& dInfo)
: CPatterned(ECharacter::PuddleSpore, uid, name, flavor, info, xf, std::move(mData), pInfo, EMovementType::Flyer,
             colType, EBodyType::Restricted, actParms, EKnockBackVariant::Medium)
, x570_(f1)
, x574_(f2)
, x578_(f3)
, x57c_(f4)
, x580_(f5)
, x584_bodyOrigin(pInfo.GetBodyOrigin())
, x590_halfExtent(pInfo.GetHalfExtent())
, x594_height(pInfo.GetHeight())
, x5a0_(CalculateBoundingBox(), GetMaterialList())
, x5d0_(g_SimplePool->GetObj({SBIG('PART'), glowFx}))
, x5ec_projectileInfo(weapon, dInfo) {
  x5dc_elemGens.reserve(kEyeCount);
  for (u32 i = 0; i < kEyeCount; ++i)
    x5dc_elemGens.emplace_back(std::make_unique<CElementGen>(x5d0_));
  x5ec_projectileInfo.Token().Lock();
  x460_knockBackController.SetAutoResetImpulse(false);
}

zeus::CAABox CPuddleSpore::CalculateBoundingBox() const {
  return {
      ((zeus::CVector3f(-x590_halfExtent, -x590_halfExtent, x598_) + x584_bodyOrigin) * 0.5f) +
          (GetBaseBoundingBox().min * 0.95f),
      ((zeus::CVector3f(x590_halfExtent, x590_halfExtent, (x594_height * x59c_) + x598_) + x584_bodyOrigin) * 0.5f) +
          (GetBaseBoundingBox().max * 0.95f)};
}

void CPuddleSpore::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  if (msg == EScriptObjectMessage::InvulnDamage)
    return;
  if (msg == EScriptObjectMessage::Registered)
    x450_bodyController->Activate(mgr);

  CPatterned::AcceptScriptMsg(msg, uid, mgr);
}

bool CPuddleSpore::HitShell(const zeus::CVector3f& point) const {
  if (x5c8_ != 1)
    return true;
  float distance = GetTransform().upVector().dot(zeus::CUnitVector3f(point - GetBoundingBox().center()));
  return (distance <= -0.5f || distance >= 0.5f);
}

void CPuddleSpore::KnockPlayer(CStateManager&, float) {}

void CPuddleSpore::UpdateBoundingState(const zeus::CAABox& box, CStateManager& mgr, float dt) {
  SetBoundingBox(box);
  x5a0_ = CCollidableAABox(box, x68_material);
  zeus::CAABox plBox;
  CPlayer& player = mgr.GetPlayer();
  zeus::CAABox selfBox = GetBoundingBox();
  if (player.GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Morphed) {
    const float ballRad = player.GetMorphBall()->GetBallRadius();
    zeus::CVector3f ballOffset = (player.GetTranslation() + zeus::CVector3f{0.f, 0.f, ballRad});
    plBox = zeus::CAABox(ballOffset - ballRad, ballOffset + ballRad);
  } else {
    plBox = player.GetBoundingBox();
  }

  if (selfBox.intersects(plBox)) {
    float bias = (x5c8_ == 2 ? 0.001f : -0.0001f) + (selfBox.max.z() - plBox.min.z());
    if (bias > 0.f && selfBox.max.z() < plBox.max.z()) {
      bool hadPlayerMat = player.GetMaterialList().HasMaterial(EMaterialTypes::Player);
      if (hadPlayerMat)
        player.RemoveMaterial(EMaterialTypes::GroundCollider, mgr);
      player.RemoveMaterial(EMaterialTypes::Player, mgr);
      CPhysicsState state = player.GetPhysicsState();
      player.MoveToOR(bias * zeus::CVector3f{0.f, 0.f, 1.f}, dt);
      CGameCollision::Move(mgr, player, dt, nullptr);
      state.SetTranslation(player.GetTranslation());
      player.SetPhysicsState(state);
      if (hadPlayerMat)
        player.AddMaterial(EMaterialTypes::GroundCollider, mgr);
      player.AddMaterial(EMaterialTypes::Player, mgr);
    }
  }
}

void CPuddleSpore::PreThink(float dt, CStateManager& mgr) {
  if (x5c8_ == 2)
    AddMaterial(EMaterialTypes::SolidCharacter, mgr);
  else
    RemoveMaterial(EMaterialTypes::SolidCharacter, mgr);

  UpdateBoundingState(CalculateBoundingBox(), mgr, dt);
  CPatterned::PreThink(dt, mgr);
}

void CPuddleSpore::Think(float dt, CStateManager& mgr) {
  if (x614_25)
    x56c_ += dt;
  if (x614_24)
    x568_ += dt;
  HealthInfo(mgr)->SetHP(1000000.0f);
  float t = (x56c_ / x570_) - 1.f >= -0.f ? 1.f : x56c_ / x570_;
  zeus::CColor modColor = zeus::CColor::lerp(zeus::skWhite, zeus::CColor(1.f, 1.f, 1.f, 0.f), t);
  for (size_t i = 0; i < kEyeCount; ++i) {
    const auto& elemGen = x5dc_elemGens[i];
    elemGen->SetModulationColor(modColor);
    elemGen->SetTranslation(GetLctrTransform(kEyeLocators[i]).origin);
    elemGen->Update(dt);
  }

  CPatterned::Think(dt, mgr);
}

void CPuddleSpore::Render(CStateManager& mgr) {
  CPatterned::Render(mgr);
  if (x56c_ > 0.01f) {
    for (const auto& elemGen : x5dc_elemGens)
      elemGen->Render(GetActorLights());
  }
}

void CPuddleSpore::Touch(CActor& act, CStateManager& mgr) {
  if (!x400_25_alive)
    return;

  if (TCastToPtr<CGameProjectile> proj = act) {
    if (proj->GetOwnerId() == mgr.GetPlayer().GetUniqueId())
      x400_24_hitByPlayerProjectile = HitShell(proj->GetTranslation());
  }
}

void CPuddleSpore::FluidFXThink(EFluidState fState, CScriptWater& water, CStateManager& mgr) {
  if (fState != EFluidState::InFluid)
    return;

  CFluidPlaneManager* planeManager = mgr.GetFluidPlaneManager();
  if (planeManager->GetLastRippleDeltaTime(GetUniqueId()) >= 2.9f) {
    zeus::CVector3f point = GetTranslation();
    point.z() = float(water.GetTriggerBoundsWR().max.z());
    water.GetFluidPlane().AddRipple(2.f, GetUniqueId(), point, water, mgr);
  }
}

void CPuddleSpore::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type, float dt) {
  if (type == EUserEventType::Projectile) {
    CProjectileInfo* projInfo = GetProjectileInfo();
    if (projInfo->Token() && projInfo->Token().IsLocked() &&
        mgr.CanCreateProjectile(GetUniqueId(), EWeaponType::AI, 16)) {
      mgr.AddObject(new CEnergyProjectile(
          true, projInfo->Token(), EWeaponType::AI, GetLctrTransform(node.GetLocatorName()), EMaterialTypes::Character,
          projInfo->GetDamage(), mgr.AllocateUniqueId(), GetAreaIdAlways(), GetUniqueId(), kInvalidUniqueId,
          EProjectileAttrib::None, false, zeus::skOne3f, {}, 0xFFFF, false));
    }
  } else
    CPatterned::DoUserAnimEvent(mgr, node, type, dt);
}

bool CPuddleSpore::ShouldTurn(CStateManager& mgr, float) {
  zeus::CAABox plBox = mgr.GetPlayer().GetBoundingBox();
  zeus::CAABox selfBox = GetBoundingBox();

  if (plBox.max.z() >= selfBox.min.z() + selfBox.max.z() * 0.5f || plBox.max.x() < selfBox.min.x() ||
      plBox.max.y() < selfBox.min.y() || selfBox.max.x() < plBox.min.y() || selfBox.max.y() < plBox.min.y() ||
      x450_bodyController->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::Getup)
    return x568_ >= x578_;

  return true;
}

void CPuddleSpore::InActive(CStateManager&, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Relaxed);
    x56c_ = 0.f;
    x598_ = -1.f;
  }
}

void CPuddleSpore::Active(CStateManager&, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Lurk);
    x568_ = 0.f;
    x56c_ = 0.f;
    x598_ = 0.f;
    x614_24 = true;
    x614_25 = true;
  } else if (msg == EStateMsg::Deactivate) {
    x614_24 = false;
    x614_25 = false;
  }
}

void CPuddleSpore::Run(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    x5c8_ = 1;
    x5cc_ = 0;
    x568_ = 0.f;
    x614_24 = false;
    x598_ = 0.f;
    x59c_ = 1.5f;
  } else if (msg == EStateMsg::Update) {
    if (x5cc_ == 0) {
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::EAnimationState::LoopReaction) {
        x5cc_ = 1;
        x614_24 = true;
      } else {
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCLoopReactionCmd(pas::EReactionType::Zero));
      }
    } else if (x5cc_ == 1 &&
               x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::EAnimationState::LoopReaction) {
      x5cc_ = 2;
    }
  } else if (msg == EStateMsg::Deactivate) {
    x450_bodyController->GetCommandMgr().DeliverCmd(CBodyStateCmd(EBodyStateCmd::ExitState));
    x5c8_ = 0;
    x59c_ = 1.f;
    x614_24 = false;
  }
}

void CPuddleSpore::TurnAround(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    x568_ = 0.f;
    x56c_ = 0.f;
    x400_24_hitByPlayerProjectile = false;
    x598_ = -2.5f;
    x5c8_ = 2;
    x5cc_ = 0;
    x614_24 = false;
  } else if (msg == EStateMsg::Update) {
    if (x5cc_ == 0) {
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::EAnimationState::LieOnGround) {
        x5cc_ = 1;
        x614_24 = true;
      } else {
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCKnockDownCmd({1.f, 0.f, 0.f}, pas::ESeverity::One));
      }
    } else if (x5cc_ == 1 &&
               x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::EAnimationState::LieOnGround) {
      x5cc_ = 2;
    }
  } else if (msg == EStateMsg::Deactivate) {
    x614_24 = false;
  }
}

void CPuddleSpore::GetUp(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    x450_bodyController->GetCommandMgr().DeliverCmd(CBCGetupCmd(pas::EGetupType::Zero));
    KnockPlayer(mgr, x580_);
    x56c_ = 0.f;
    x598_ = -1.f;
    x5cc_ = 0;
  } else if (msg == EStateMsg::Update) {
    KnockPlayer(mgr, x580_ * 0.25f);
    if (x5cc_ == 0) {
      x450_bodyController->GetCommandMgr().DeliverCmd(CBCGetupCmd(pas::EGetupType::Zero));
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::EAnimationState::Getup)
        x5cc_ = 1;
    } else if (x5cc_ == 1 &&
               x450_bodyController->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::Getup) {
      x5cc_ = 1;
    }
  } else if (msg == EStateMsg::Deactivate) {
    x5c8_ = 0;
  }
}

void CPuddleSpore::Attack(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    x32c_animState = EAnimState::Ready;
    x598_ = 0.f;
  } else if (msg == EStateMsg::Update) {
    TryCommand(mgr, pas::EAnimationState::MeleeAttack, &CPatterned::TryMeleeAttack, 1);
  } else if (msg == EStateMsg::Deactivate) {
    x32c_animState = EAnimState::NotReady;
  }
}
} // namespace urde::MP1