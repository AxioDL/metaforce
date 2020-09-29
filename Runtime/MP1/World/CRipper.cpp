#include "Runtime/MP1/World/CRipper.hpp"

#include "Runtime/CStateManager.hpp"
#include "Runtime/Collision/CCollidableOBBTreeGroup.hpp"
#include "Runtime/Collision/CGameCollision.hpp"
#include "Runtime/Weapon/CPlayerGun.hpp"
#include "Runtime/World/CActorParameters.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "Runtime/World/CScriptGrapplePoint.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde::MP1 {
CRipper::CRipper(TUniqueId uid, std::string_view name, EFlavorType type, const CEntityInfo& info,
                 const zeus::CTransform& xf, CModelData&& mData, const CPatternedInfo& pInfo,
                 const CActorParameters& actParms, const CGrappleParameters& grappleParms)
: CPatterned(ECharacter::Ripper, uid, name, type, info, xf, std::move(mData), pInfo, EMovementType::Flyer,
             EColliderType::One, EBodyType::Flyer, actParms, EKnockBackVariant::Medium)
, x568_grappleParams(grappleParms) {
  SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
      {EMaterialTypes::Solid},
      {EMaterialTypes::NoStaticCollision, EMaterialTypes::NoPlatformCollision, EMaterialTypes::Platform}));
  x460_knockBackController.SetAutoResetImpulse(false);
  x460_knockBackController.SetAnimationStateRange(EKnockBackAnimationState::Flinch,
                                                  EKnockBackAnimationState::KnockBack);
}

void CRipper::Think(float dt, CStateManager& mgr) {
  if (!GetActive())
    return;

  ProcessGrapplePoint(mgr);
  const CPlayer& pl = mgr.GetPlayer();
  CGrappleArm::EArmState armState = pl.GetPlayerGun()->GetGrappleArm().GetAnimState();
  if (x598_grapplePoint != kInvalidUniqueId && pl.GetOrbitTargetId() == x598_grapplePoint &&
      pl.GetGrappleState() != CPlayer::EGrappleState::None) {
    if (pl.GetGrappleState() != CPlayer::EGrappleState::Firing && (armState > CGrappleArm::EArmState::Three)) {
      Stop();
      if (!x59c_24_muted) {
        SetMuted(true);
        x59c_24_muted = true;
      }
    } else {
      CPatterned::Think(dt, mgr);
    }
  } else {
    CPatterned::Think(dt, mgr);
    if (x59c_24_muted) {
      SetMuted(false);
      x59c_24_muted = false;
    }
  }
}

void CRipper::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  CPatterned::AcceptScriptMsg(msg, uid, mgr);

  switch (msg) {
  case EScriptObjectMessage::Deleted:
  case EScriptObjectMessage::Deactivate: {
    RemoveGrapplePoint(mgr);
    RemovePlatform(mgr);
    break;
  }
  case EScriptObjectMessage::Activate: {
    AddGrapplePoint(mgr);
    AddPlatform(mgr);
    break;
  }
  case EScriptObjectMessage::Registered: {
    x450_bodyController->Activate(mgr);
    AddMaterial(EMaterialTypes::Immovable, mgr);
    RemoveMaterial(EMaterialTypes::Solid, mgr);
    if (x3fc_flavor == EFlavorType::One) {
      AddGrapplePoint(mgr);
      RemoveMaterial(EMaterialTypes::Orbit, mgr);
    }

    AddPlatform(mgr);
    break;
  }
  default:
    break;
  }
}

void CRipper::KnockBack(const zeus::CVector3f& dir, CStateManager& mgr, const CDamageInfo& dInfo, EKnockBackType kb,
                        bool inDeferred, float mag) {
  CPatterned::KnockBack(dir, mgr, dInfo, kb, inDeferred, mag);
  x450_bodyController->GetCommandMgr().DeliverCmd(CBCKnockBackCmd(-dir, pas::ESeverity::One));
}

void CRipper::Patrol(CStateManager& mgr, EStateMsg msg, float arg) {
  x450_bodyController->GetCommandMgr().SetSteeringBlendMode(ESteeringBlendMode::FullSpeed);
  x450_bodyController->GetCommandMgr().SetSteeringSpeedRange(1.f, 1.f);
  CPatterned::Patrol(mgr, msg, arg);
}

void CRipper::ProcessGrapplePoint(CStateManager& mgr) {
  if (x3fc_flavor != EFlavorType::One || x598_grapplePoint == kInvalidUniqueId)
    return;

  if (TCastToPtr<CScriptGrapplePoint> gp = mgr.ObjectById(x598_grapplePoint)) {
    gp->SetTransform(GetTransform());
  }
}

void CRipper::AddGrapplePoint(CStateManager& mgr) {
  if (x598_grapplePoint != kInvalidUniqueId)
    return;

  x598_grapplePoint = mgr.AllocateUniqueId();
  mgr.AddObject(new CScriptGrapplePoint(x598_grapplePoint, "RipperGrapplePoint"sv,
                                        CEntityInfo(GetAreaIdAlways(), NullConnectionList), GetTransform(), true,
                                        x568_grappleParams));
}

void CRipper::RemoveGrapplePoint(CStateManager& mgr) {
  if (x598_grapplePoint == kInvalidUniqueId)
    return;
  mgr.FreeScriptObject(x598_grapplePoint);
}

void CRipper::AddPlatform(CStateManager& mgr) {
  if (x59a_platformId != kInvalidUniqueId)
    return;

  x59a_platformId = mgr.AllocateUniqueId();
  const zeus::CAABox bounds = GetModelData()->GetBounds(GetTransform().getRotation());

  const auto& platform = new CRipperControlledPlatform(x59a_platformId, GetUniqueId(), "Ripper Controlled Platform"sv,
                                                       CEntityInfo(GetAreaIdAlways(), NullConnectionList),
                                                       GetTransform(), bounds, GetActive(), {});
  mgr.AddObject(platform);
  platform->AddMaterial(EMaterialTypes::ProjectilePassthrough);
}

void CRipper::RemovePlatform(CStateManager& mgr) {
  if (x59a_platformId == kInvalidUniqueId)
    return;
  mgr.FreeScriptObject(x59a_platformId);
  x59a_platformId = kInvalidUniqueId;
}

CRipperControlledPlatform::CRipperControlledPlatform(
    TUniqueId uid, TUniqueId owner, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
    const zeus::CAABox& bounds, bool active, const std::optional<TLockedToken<CCollidableOBBTreeGroup>>& colTree)
: CScriptPlatform(uid, name, info, xf, CModelData::CModelDataNull(), CActorParameters::None(), bounds, 0.f, false, 1.f,
                  active, CHealthInfo(FLT_MAX, 10.f), CDamageVulnerability::ImmuneVulnerabilty(), colTree, false, 1, 1)
, x358_owner(owner)
, x35c_yaw(GetYaw()) {}

constexpr float RCP_2PI = 0.15915494f;
constexpr float M_2PI = 6.2831855f;

zeus::CQuaternion CRipperControlledPlatform::Move(float arg, CStateManager& mgr) {
  if (const auto* actor = static_cast<CActor*>(mgr.ObjectById(x358_owner))) {
    MoveToWR(GetTranslation() + (actor->GetTranslation() - GetTranslation()), arg);
    float yawDiff = actor->GetYaw() - x35c_yaw;
    float zRot = yawDiff - static_cast<float>(static_cast<int>(yawDiff * RCP_2PI)) * M_2PI;
    if (zRot < 0.f) {
      zRot += M_2PI;
    }
    if (zRot > M_PIF) {
      zRot -= M_2PI;
    }
    const auto quat = zeus::CQuaternion::fromAxisAngle({0.0f, 0.0f, 1.0f}, zRot);
    RotateToOR(quat, arg);

    rstl::reserved_vector<TUniqueId, 1024> nearList;
    rstl::reserved_vector<TUniqueId, 1024> filteredNearList;
    mgr.BuildColliderList(nearList, *this, GetMotionVolume(arg));
    for (const auto& id : nearList) {
      if (!IsRider(id) && !IsSlave(id)) {
        filteredNearList.push_back(id);
      }
    }

    xf8_24_movable = true;
    CGameCollision::Move(mgr, *this, arg, &filteredNearList);
    xf8_24_movable = false;
    x35c_yaw = GetYaw();
    return quat;
  }
  return {};
}
} // namespace urde::MP1
