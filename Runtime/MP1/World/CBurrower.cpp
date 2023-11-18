#include "Runtime/MP1/World/CBurrower.hpp"

#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Graphics/CCubeRenderer.hpp"
#include "Runtime/Particle/CElementGen.hpp"
#include "Runtime/World/CGameArea.hpp"
#include "Runtime/World/CPatternedInfo.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "Runtime/World/CWorld.hpp"
#include "Runtime/CStateManager.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace metaforce::MP1 {
namespace {
constexpr CDamageVulnerability skVulnerability{
    EVulnerability::Deflect, EVulnerability::Deflect, EVulnerability::Deflect, EVulnerability::Deflect,
    EVulnerability::Normal,  EVulnerability::Normal,  EVulnerability::Deflect, EVulnerability::Deflect,
    EVulnerability::Deflect, EVulnerability::Deflect, EVulnerability::Deflect, EVulnerability::Deflect,
    EVulnerability::Deflect, EVulnerability::Deflect, EVulnerability::Deflect, EDeflectType::None};
} // namespace

CBurrower::CBurrower(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                     CModelData&& mData, const CPatternedInfo& pInfo, const CActorParameters& actParms, CAssetId aId1,
                     CAssetId aId2, CAssetId aId3, const CDamageInfo& damageInfo, CAssetId aId4, u32 sfxId,
                     CAssetId aId5)
: CPatterned(ECharacter::Burrower, uid, name, EFlavorType::Zero, info, xf, std::move(mData), pInfo,
             EMovementType::Ground, EColliderType::One, EBodyType::BiPedal, actParms, EKnockBackVariant::Small)
, x568_pathFindSearch(nullptr, 1, pInfo.GetPathfindingIndex(), 1.f, 1.f)
, x64c_projectileInfo(aId3, damageInfo)
, x6aa_visorSfx(CSfxManager::TranslateSFXID(sfxId)) {
  CreateShadow(false);
  MakeThermalColdAndHot();

  x64c_projectileInfo.Token().Lock();
  if (aId1.IsValid()) {
    x674_jumpParticle =
        std::make_unique<CElementGen>(g_SimplePool->GetObj({SBIG('PART'), aId1}),
                                      CElementGen::EModelOrientationType::One, CElementGen::EOptionalSystemFlags::One);
    x674_jumpParticle->SetGlobalScale(GetModelData()->GetScale());
    x674_jumpParticle->SetParticleEmission(false);
  }

  if (aId2.IsValid()) {
    x678_trailParticle =
        std::make_unique<CElementGen>(g_SimplePool->GetObj({SBIG('PART'), aId2}),
                                      CElementGen::EModelOrientationType::One, CElementGen::EOptionalSystemFlags::One);
    x678_trailParticle->SetGlobalScale(GetModelData()->GetScale());
    x678_trailParticle->SetParticleEmission(false);
  }

  if (aId4.IsValid()) {
    x67c_visorParticle.emplace(g_SimplePool->GetObj({SBIG('PART'), aId4}));
  }

  if (aId5.IsValid()) {
    x68c_deathExplosionParticle.emplace(g_SimplePool->GetObj({SBIG('PART'), aId5}));
  }
}

void CBurrower::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }
  CPatterned::Think(dt, mgr);
  if (x6a4_invulnDamageTime > 0.f) {
    x6a4_invulnDamageTime -= dt;
  }

  if (x6ac_24_doFacePlayer) {
    zeus::CVector3f front = GetTransform().frontVector().normalized();
    zeus::CVector3f diffPos = (mgr.GetPlayer().GetTranslation() - GetTranslation()).normalized();
    if (front.dot(diffPos) < 0.9993) {
      zeus::CQuaternion q = zeus::CQuaternion::lookAt(front, diffPos, zeus::degToRad(360.f * dt));
      q.setImaginary(GetTransform().transposeRotate(q.getImaginary()));
      RotateToOR(q, dt);
    }
  } else if (x69c_attackTime > 0.f) {
    x69c_attackTime -= dt;
  }

  if (x674_jumpParticle) {
    x6a0_lurkTimer -= dt;
    if (!x6ac_25_inAir && x6a0_lurkTimer <= 0.f) {
      if (IsAlive()) {
        x674_jumpParticle->SetParticleEmission(true);
        x674_jumpParticle->SetOrientation(GetTransform().getRotation());
        x674_jumpParticle->SetTranslation(GetTranslation());
        x674_jumpParticle->ForceParticleCreation(1);
        x674_jumpParticle->SetOrientation({});
        x674_jumpParticle->SetParticleEmission(false);
      }
      x6a0_lurkTimer = 0.f;
    }
    x674_jumpParticle->Update(dt);
  }

  if (x678_trailParticle) {
    if (IsAlive() && !x6ac_25_inAir) {
      x678_trailParticle->SetTranslation(GetTranslation());
    }
    x678_trailParticle->Update(dt);
  }
}

void CBurrower::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  CPatterned::AcceptScriptMsg(msg, uid, mgr);
  if (msg == EScriptObjectMessage::Registered) {
    x450_bodyController->Activate(mgr);
  } else if (msg == EScriptObjectMessage::InitializedInArea) {
    x568_pathFindSearch.SetArea(mgr.GetWorld()->GetAreaAlways(GetAreaIdAlways())->GetPostConstructed()->x10bc_pathArea);
    if (!HasPatrolPath(mgr, 0.f)) {
      x678_trailParticle.reset();
    }
  } else if (msg == EScriptObjectMessage::InvulnDamage) {
    x6a4_invulnDamageTime = 1.f;
  }
}

void CBurrower::AddToRenderer(const zeus::CFrustum& frustum, CStateManager& mgr) {
  if (GetActive() && x678_trailParticle) {
    g_Renderer->AddParticleGen(*x678_trailParticle);
  }
  CPatterned::AddToRenderer(frustum, mgr);
}

void CBurrower::Render(CStateManager& mgr) {
  auto* lights = GetActorLights();
  if (lights != nullptr && x674_jumpParticle) {
    lights->ActivateLights();
    x674_jumpParticle->Render();
  }
  CPatterned::Render(mgr);
}

const CDamageVulnerability* CBurrower::GetDamageVulnerability() const {
  if (x6ac_25_inAir) {
    return CAi::GetDamageVulnerability();
  }
  return &skVulnerability;
}

const CDamageVulnerability* CBurrower::GetDamageVulnerability(const zeus::CVector3f& v1, const zeus::CVector3f& v2,
                                                              const CDamageInfo& damageInfo) const {
  if (x6ac_25_inAir) {
    return CAi::GetDamageVulnerability();
  }
  return &skVulnerability;
}

void CBurrower::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type, float dt) {
  if (type == EUserEventType::Landing) {
    x328_25_verticalMovement = false;
    AddMaterial(EMaterialTypes::GroundCollider, mgr);
    x55c_moveScale.splat(1.f);
    return;
  }

  if (type == EUserEventType::Projectile) {
    const zeus::CVector3f aimPos = mgr.GetPlayer().GetAimPosition(mgr, 0.f);
    const zeus::CVector3f gunPos = GetLctrTransform(node.GetLocatorName()).origin;
    const zeus::CVector3f interPos =
        GetProjectileInfo()->PredictInterceptPos(gunPos, aimPos, mgr.GetPlayer(), true, dt);
    const zeus::CTransform gunXf = zeus::lookAt(gunPos, interPos);
    LaunchProjectile(gunXf, mgr, 1, EProjectileAttrib::None, false, x67c_visorParticle, x6aa_visorSfx, false,
                     GetModelData()->GetScale());
    return;
  }

  if (type == EUserEventType::TakeOff) {
    RemoveMaterial(EMaterialTypes::GroundCollider, mgr);
    x328_25_verticalMovement = true;
    x55c_moveScale = GetModelData()->GetScale();
    return;
  }

  CPatterned::DoUserAnimEvent(mgr, node, type, dt);
}

void CBurrower::Death(CStateManager& mgr, const zeus::CVector3f& direction, EScriptObjectState state) {
  if (!IsAlive()) {
    return;
  }

  CPatterned::Death(mgr, direction, state);
  if (x678_trailParticle) {
    x678_trailParticle->SetParticleEmission(false);
  }
}

void CBurrower::Patrol(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    x6ac_25_inAir = false;
    if (x678_trailParticle) {
      x678_trailParticle->SetParticleEmission(true);
    }
  } else if (msg == EStateMsg::Deactivate) {
    x6a8_lastDestObj = x2dc_destObj;
  }

  CPatterned::Patrol(mgr, msg, dt);
}

void CBurrower::TargetPatrol(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg != EStateMsg::Activate) {
    return;
  }

  x2dc_destObj = x6a8_lastDestObj != kInvalidUniqueId
                     ? x6a8_lastDestObj
                     : GetWaypointForState(mgr, EScriptObjectState::Patrol, EScriptObjectMessage::Follow);

  if (const TCastToConstPtr<CActor> act = mgr.GetObjectById(x2dc_destObj)) {
    x2e0_destPos = act->GetTranslation();
    x328_24_inPosition = false;
    x2ec_reflectedDestPos = GetTranslation();
  }
}

void CBurrower::TurnAround(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg != EStateMsg::Activate) {
    return;
  }
  GetBodyController()->GetCommandMgr().DeliverCmd(
      CBCLocomotionCmd{-GetTransform().frontVector(), GetTransform().frontVector(), 1.f});
}

void CBurrower::Active(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    x6ac_24_doFacePlayer = true;
    x32c_animState = EAnimState::Ready;
  } else if (msg == EStateMsg::Update) {
    TryCommand(mgr, pas::EAnimationState::Generate, &CPatterned::TryGenerateNoXf, 0);
  } else if (msg == EStateMsg::Deactivate) {
    x6ac_24_doFacePlayer = false;
    x6ac_25_inAir = true;
    x32c_animState = EAnimState::NotReady;
  }
}

void CBurrower::Lurk(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg != EStateMsg::Activate) {
    return;
  }
  x6ac_25_inAir = false;
  x6a0_lurkTimer = 0.1875f;
}

void CBurrower::ProjectileAttack(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    x6ac_24_doFacePlayer = true;
    x6ac_25_inAir = true;
    if (x678_trailParticle) {
      x678_trailParticle->SetParticleEmission(false);
    }
    x32c_animState = EAnimState::Ready;
  } else if (msg == EStateMsg::Update) {
    TryCommand(mgr, pas::EAnimationState::ProjectileAttack, &CPatterned::TryProjectileAttack, 0);
  } else if (msg == EStateMsg::Deactivate) {
    x32c_animState = EAnimState::NotReady;
    x69c_attackTime = x308_attackTimeVariation * mgr.GetActiveRandom()->Float() + x304_averageAttackTime;
    x328_25_verticalMovement = false;
    AddMaterial(EMaterialTypes::GroundCollider, mgr);
    x55c_moveScale.splat(1.f);
  }
}

void CBurrower::Retreat(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    x32c_animState = EAnimState::Ready;
  } else if (msg == EStateMsg::Update) {
    TryCommand(mgr, pas::EAnimationState::Generate, &CPatterned::TryGenerateNoXf, 1);
  } else if (msg == EStateMsg::Deactivate) {
    x32c_animState = EAnimState::NotReady;
    if (x678_trailParticle) {
      x678_trailParticle->SetParticleEmission(true);
    }
  }
}

bool CBurrower::PathShagged(CStateManager& mgr, float arg) {
  return x568_pathFindSearch.OnPath(GetTranslation()) == CPathFindSearch::EResult::InvalidArea;
}

bool CBurrower::ShouldAttack(CStateManager& mgr, float arg) {
  if (x6a4_invulnDamageTime > 0.f) {
    return false;
  }

  return mgr.CanCreateProjectile(GetUniqueId(), EWeaponType::AI, 1);
}

const std::optional<TLockedToken<CGenDescription>>& CBurrower::GetDeathExplosionParticle() const {
  if (x6ac_25_inAir) {
    return x68c_deathExplosionParticle;
  }
  return x520_deathExplosionParticle;
}

} // namespace metaforce::MP1
