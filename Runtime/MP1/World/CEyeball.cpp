#include "Runtime/MP1/World/CEyeball.hpp"

#include "Runtime/CStateManager.hpp"
#include "Runtime/Weapon/CBeamInfo.hpp"
#include "Runtime/Weapon/CGameProjectile.hpp"
#include "Runtime/Weapon/CPlasmaProjectile.hpp"
#include "Runtime/World/CGameArea.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "Runtime/World/CWorld.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde::MP1 {
constexpr std::string_view skEyeLocator = "Laser_LCTR"sv;

CEyeball::CEyeball(TUniqueId uid, std::string_view name, CPatterned::EFlavorType flavor, const CEntityInfo& info,
                   const zeus::CTransform& xf, CModelData&& mData, const CPatternedInfo& pInfo, float attackDelay,
                   float attackStartTime, CAssetId wpscId, const CDamageInfo& dInfo, CAssetId beamContactFxId,
                   CAssetId beamPulseFxId, CAssetId beamTextureId, CAssetId beamGlowTextureId, u32 anim0, u32 anim1,
                   u32 anim2, u32 anim3, u32 beamSfx, bool attackDisabled, const CActorParameters& actParms)
: CPatterned(ECharacter::EyeBall, uid, name, flavor, info, xf, std::move(mData), pInfo, EMovementType::Flyer,
             EColliderType::Zero, EBodyType::Restricted, actParms, EKnockBackVariant::Medium)
, x568_attackDelay(attackDelay)
, x56c_attackStartTime(attackStartTime)
, x570_boneTracking(*GetModelData()->GetAnimationData(), "Eye"sv, zeus::degToRad(45.f), zeus::degToRad(180.f),
                    EBoneTrackingFlags::NoParent)
, x5b4_projectileInfo(wpscId, dInfo)
, x5dc_beamContactFxId(beamContactFxId)
, x5e0_beamPulseFxId(beamPulseFxId)
, x5e4_beamTextureId(beamTextureId)
, x5e8_beamGlowTextureId(beamGlowTextureId)
, x5f4_animIdxs{static_cast<s32>(anim0), static_cast<s32>(anim1), static_cast<s32>(anim2), static_cast<s32>(anim3)}
, x604_beamSfxId(CSfxManager::TranslateSFXID(beamSfx))
, x60c_27_attackDisabled(attackDisabled) {
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

void CEyeball::Think(float dt, CStateManager& mgr) {
  CPatterned::Think(dt, mgr);

  if (!GetActive()) {
    return;
  }

  const CPlayer& player = mgr.GetPlayer();
  const zeus::CVector3f direction = (player.GetTranslation() - GetTranslation()).normalized();

  // Used to be directly calculated as std::cos(zeus::degToRad(45.f));
  // but was converted into the exact constant to avoid unnecessary runtime initialization.
  constexpr float minAngle = 0.707106769f;

  x60c_25_playerInRange = (player.GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Morphed &&
                           direction.dot(GetTransform().frontVector()) > minAngle);

  if (x60c_25_playerInRange) {
    x570_boneTracking.SetActive(true);
    x5a8_targetPosition = player.GetTranslation() - (0.5f * player.GetVelocity());
    x570_boneTracking.SetTargetPosition(x5a8_targetPosition);
    x570_boneTracking.Update(dt);
    GetModelData()->GetAnimationData()->PreRender();
    x570_boneTracking.PreRender(mgr, *GetModelData()->GetAnimationData(), GetTransform(), GetModelData()->GetScale(),
                                *x450_bodyController);
  } else {
    x570_boneTracking.SetActive(false);
  }

  if (GetActive()) {
    CPlasmaProjectile* projectile = static_cast<CPlasmaProjectile*>(mgr.ObjectById(x5ec_projectileId));
    if (projectile && projectile->GetActive())
      projectile->UpdateFx(GetLctrTransform(skEyeLocator), dt, mgr);
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

  CBeamInfo beamInfo(3, x5dc_beamContactFxId, x5e0_beamPulseFxId, x5e4_beamTextureId, x5e8_beamGlowTextureId, 50, 0.05f,
                     1.f, 2.f, 20.f, 1.f, 1.f, 2.f, zeus::CColor(1.f, 1.f, 1.f, 0.f), zeus::CColor(0.f, 1.f, 0.5f, 0.f),
                     150.f);
  x5ec_projectileId = mgr.AllocateUniqueId();
  mgr.AddObject(new CPlasmaProjectile(x5b4_projectileInfo.Token(), "EyeBall_Beam"sv, EWeaponType::AI, beamInfo,
                                      zeus::CTransform(), EMaterialTypes::Immovable,
                                      x5b4_projectileInfo.GetDamage(), x5ec_projectileId, GetAreaIdAlways(),
                                      GetUniqueId(), {}, false, EProjectileAttrib::KeepInCinematic));
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
    if (x330_stateMachineState.GetTime() > x56c_attackStartTime)
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
  if (std::fabs(GetModelData()->GetAnimationData()->GetAnimTimeRemaining("Whole Body"sv) - 0.f) >= 0.00001f) {
    return;
  }

  x5f0_currentAnim = (x5f0_currentAnim + 1) & 3;
  for (size_t i = 0; i < x5f4_animIdxs.size(); ++i) {
    if (x5f4_animIdxs[x5f0_currentAnim] != -1) {
      break;
    }

    x5f0_currentAnim = (x5f0_currentAnim + 1) & 3;
  }
  const s32 animIdx = x5f4_animIdxs[x5f0_currentAnim];
  if (animIdx != -1) {
    x450_bodyController->GetCommandMgr().DeliverCmd(CBCScriptedCmd(animIdx, false, false, 0.f));
  }
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
  x570_boneTracking.PreRender(mgr, *GetModelData()->GetAnimationData(), GetTransform(), GetModelData()->GetScale(),
                              *x450_bodyController);
}

void CEyeball::Death(CStateManager& mgr, const zeus::CVector3f& pos, EScriptObjectState state) {
  zeus::CTransform oldXf = GetTransform();
  CPatterned::Death(mgr, pos, state);
  SetTransform(oldXf);
}
} // namespace urde::MP1