#include "CEyeball.hpp"
#include "Weapon/CBeamInfo.hpp"
#include "Weapon/CGameProjectile.hpp"
#include "Weapon/CPlasmaProjectile.hpp"
#include "World/CPlayer.hpp"
#include "World/CWorld.hpp"
#include "World/CGameArea.hpp"
#include "CStateManager.hpp"
#include "TCastTo.hpp"
namespace urde::MP1 {
CEyeball::CEyeball(TUniqueId uid, std::string_view name, CPatterned::EFlavorType flavor, const CEntityInfo& info,
                   const zeus::CTransform& xf, CModelData&& mData, const CPatternedInfo& pInfo, float f1, float f2,
                   CAssetId aId1, const CDamageInfo& dInfo, CAssetId aId2, CAssetId aId3, CAssetId aId4, CAssetId aId5,
                   u32 w1, u32 w2, u32 w3, u32 w4, u32 w5, bool b1, const CActorParameters& actParms)
: CPatterned(ECharacter::EyeBall, uid, name, flavor, info, xf, std::move(mData), pInfo, EMovementType::Flyer,
             EColliderType::Zero, EBodyType::Restricted, actParms, EKnockBackVariant::Medium)
, x568_attackDelay(f1)
, x56c_maxAttackDelay(f2)
, x570_boneTracking(*GetModelData()->GetAnimationData(), "Eye"sv, zeus::degToRad(45.f), zeus::degToRad(180.f), true)
, x5b4_projectileInfo(aId1, dInfo)
, x5dc_(aId2)
, x5e0_(aId3)
, x5e4_(aId4)
, x5e8_(aId5)
, x604_beamSfxId(CSfxManager::TranslateSFXID(w5))
, x60c_24_canAttack(false)
, x60c_25_playerInRange(false)
, x60c_26_alert(false)
, x60c_27_attackDisabled(b1)
, x60c_28_firingBeam(false) {
  x5f4_animIdxs[0] = w1;
  x5f4_animIdxs[1] = w2;
  x5f4_animIdxs[2] = w3;
  x5f4_animIdxs[3] = w4;

  x460_knockBackController.SetAutoResetImpulse(false);
}

void CEyeball::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CEyeball::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  switch (msg) {
  case EScriptObjectMessage::ProjectileCollide:
  case EScriptObjectMessage::InvulnDamage: {
    if (TCastToConstPtr<CGameProjectile> proj = mgr.GetObjectById(uid)) {
      if (proj->GetOwnerId() == mgr.GetPlayer().GetUniqueId())
        if (GetDamageVulnerability()->GetVulnerability(proj->GetDamageInfo().GetWeaponMode(), false) !=
            EVulnerability::Deflect)
          x400_24_hitByPlayerProjectile = true;
    }
    return;
  }
  case EScriptObjectMessage::Alert:
    x60c_26_alert = true;
    break;
  case EScriptObjectMessage::Registered: {
    RemoveMaterial(EMaterialTypes::Orbit, EMaterialTypes::Target, mgr);
    x450_bodyController->Activate(mgr);
    x330_stateMachineState.SetDelay(0.f);
    CreateShadow(false);
    CreateBeam(mgr);
    break;
  }
  case EScriptObjectMessage::Deleted: {
    if (x5ec_projectileId != kInvalidUniqueId) {
      mgr.FreeScriptObject(x5ec_projectileId);
      if (x608_beamSfx) {
        CSfxManager::RemoveEmitter(x608_beamSfx);
        x608_beamSfx.reset();
      }
    }
    x5ec_projectileId = kInvalidUniqueId;
    break;
  }
  default:
    break;
  }
  CPatterned::AcceptScriptMsg(msg, uid, mgr);
}

static float kMinAngle = std::cos(zeus::degToRad(45.f));
void CEyeball::Think(float dt, CStateManager& mgr) {
  CPatterned::Think(dt, mgr);
  if (!GetActive())
    return;

  CPlayer& player = mgr.GetPlayer();
  zeus::CVector3f direction = (player.GetTranslation() - GetTranslation()).normalized();

  x60c_25_playerInRange = (player.GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Morphed &&
                           direction.dot(GetTransform().frontVector()) > kMinAngle);

  if (x60c_25_playerInRange) {
    x570_boneTracking.SetActive(true);
    x5a8_targetPosition = player.GetTranslation() - (0.5f * player.GetVelocity());
    x570_boneTracking.SetTargetPosition(x5a8_targetPosition);
    x570_boneTracking.Update(dt);
    ModelData()->AnimationData()->PreRender();
    x570_boneTracking.PreRender(mgr, *ModelData()->AnimationData(), GetTransform(), GetModelData()->GetScale(),
                                *x450_bodyController.get());
  } else
    x570_boneTracking.SetActive(false);

  if (GetActive()) {
    CPlasmaProjectile* projectile = static_cast<CPlasmaProjectile*>(mgr.ObjectById(x5ec_projectileId));
    if (projectile && projectile->GetActive())
      projectile->UpdateFX(GetLctrTransform(skEyeLocator), dt, mgr);
  }

  if (x60c_28_firingBeam) {
    const CGameArea* area = mgr.GetWorld()->GetAreaAlways(GetAreaIdAlways());
    if (area->GetActive() && area->IsPostConstructed() &&
        area->GetPostConstructed()->x10dc_occlusionState == CGameArea::EOcclusionState::Occluded)
      ResetBeamState(mgr);
  }
}

void CEyeball::CreateBeam(CStateManager& mgr) {
  if (x5ec_projectileId != kInvalidUniqueId)
    return;

  CBeamInfo beamInfo(3, x5dc_, x5e0_, x5e4_, x5e8_, 50, .5f, 1.f, 2.f, 20.f, 1.f, 1.f, 2.f,
                     zeus::CColor(1.f, 1.f, 1.f, 0.f), zeus::CColor(0.f, 1.f, 0.5f, 0.f), 150.f);
  x5ec_projectileId = mgr.AllocateUniqueId();
  mgr.AddObject(new CPlasmaProjectile(x5b4_projectileInfo.Token(), "EyeBall_Beam"sv, EWeaponType::AI, beamInfo,
                                      zeus::CTransform::Identity(), EMaterialTypes::Immovable,
                                      x5b4_projectileInfo.GetDamage(), x5ec_projectileId, GetAreaIdAlways(),
                                      GetUniqueId(), 8, false, EProjectileAttrib::KeepInCinematic));
}

void CEyeball::InActive(CStateManager&, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate)
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Relaxed);
}

void CEyeball::Cover(CStateManager&, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Lurk);
    x60c_24_canAttack = false;
    x330_stateMachineState.SetDelay(x568_attackDelay);
  }
}

void CEyeball::Flinch(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x32c_animState = EAnimState::Ready;
    x330_stateMachineState.SetDelay(x568_attackDelay);
  } else if (msg == EStateMsg::Update)
    TryCommand(mgr, pas::EAnimationState::KnockBack, CPatternedTryFunc(&CEyeball::TryFlinch), 0);
  else if (msg == EStateMsg::Deactivate)
    x32c_animState = EAnimState::NotReady;
}

void CEyeball::TryFlinch(CStateManager&, int arg) {
  x450_bodyController->GetCommandMgr().DeliverCmd(CBCKnockBackCmd(GetTransform().basis[0], pas::ESeverity(arg)));
}

void CEyeball::Active(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    x400_24_hitByPlayerProjectile = 0;
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Combat);
    x60c_24_canAttack = false;
  } else if (msg == EStateMsg::Update) {
    if (x330_stateMachineState.GetDelay() > x56c_maxAttackDelay)
      x60c_24_canAttack = true;

    UpdateAnimation();
  } else if (msg == EStateMsg::Deactivate) {
    x330_stateMachineState.SetDelay(x568_attackDelay);
    if (CPlasmaProjectile* proj = static_cast<CPlasmaProjectile*>(mgr.ObjectById(x5ec_projectileId)))
      proj->ResetBeam(mgr, true);

    x60c_24_canAttack = false;

    CSfxManager::RemoveEmitter(x608_beamSfx);
    x608_beamSfx.reset();
  }
}

void CEyeball::UpdateAnimation() {
  if (std::fabs(GetModelData()->GetAnimationData()->GetAnimTimeRemaining("Whole Body"sv) - 0.f) >= 0.00001f)
    return;

  x5f0_currentAnim = (x5f0_currentAnim + 1) & 3;
  for (u32 i = 0; i < 4; ++i) {
    if (x5f4_animIdxs[x5f0_currentAnim] != -1)
      break;

    x5f0_currentAnim = (x5f0_currentAnim + 1) & 3;
  }
  s32 animIdx = x5f4_animIdxs[x5f0_currentAnim];
  if (animIdx != -1)
    x450_bodyController->GetCommandMgr().DeliverCmd(CBCScriptedCmd(animIdx, false, false, 0.f));
}

void CEyeball::ResetBeamState(CStateManager& mgr) {
  if (CPlasmaProjectile* projectile = static_cast<CPlasmaProjectile*>(mgr.ObjectById(x5ec_projectileId)))
    projectile->ResetBeam(mgr, true);

  x60c_28_firingBeam = false;
  if (x608_beamSfx) {
    CSfxManager::RemoveEmitter(x608_beamSfx);
    x608_beamSfx.reset();
  }
}

void CEyeball::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type, float dt) {
  if (type == EUserEventType::DamageOff)
    ResetBeamState(mgr);
  else if (type == EUserEventType::DamageOn && x60c_24_canAttack)
    FireBeam(mgr, GetLctrTransform(node.GetLocatorName()));
  else
    CPatterned::DoUserAnimEvent(mgr, node, type, dt);
}

void CEyeball::FireBeam(CStateManager& mgr, const zeus::CTransform& xf) {
  if (CPlasmaProjectile* projectile = static_cast<CPlasmaProjectile*>(mgr.ObjectById(x5ec_projectileId))) {
    if (!projectile->GetActive()) {
      projectile->Fire(xf, mgr, false);
      x60c_28_firingBeam = true;
      if (!x608_beamSfx) {
        CAudioSys::C3DEmitterParmData parmData{
            GetTranslation(), {}, 50.f, 0.1f, 0x1, x604_beamSfxId, 1.f /* 127 */, 0.15f /* 20 / 127 */, false, 127};
        x608_beamSfx = CSfxManager::AddEmitter(parmData, true, 127, true, GetAreaIdAlways());
      }
    }
  }
}

void CEyeball::PreRender(CStateManager& mgr, const zeus::CFrustum& frustum) {
  CPatterned::PreRender(mgr, frustum);
  x570_boneTracking.PreRender(mgr, *ModelData()->AnimationData(), GetTransform(), GetModelData()->GetScale(),
                              *x450_bodyController);
}

void CEyeball::Death(CStateManager& mgr, const zeus::CVector3f& pos, EScriptObjectState state) {
  zeus::CTransform oldXf = GetTransform();
  CPatterned::Death(mgr, pos, state);
  SetTransform(oldXf);
}
} // namespace urde::MP1