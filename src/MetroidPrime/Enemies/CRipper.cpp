#include "MetroidPrime/Enemies/CRipper.hpp"

#include "Collision/CMaterialList.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/CEntityInfo.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/Enemies/CRipperControlledPlatform.hpp"
#include "MetroidPrime/Player/CGrappleArm.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerGun.hpp"
#include "MetroidPrime/ScriptObjects/CScriptGrapplePoint.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/string.hpp"

#include <Collision/CMaterialFilter.hpp>

const uint CRipper::skNumProperties = 8;

CRipper::CRipper(TUniqueId uid, const rstl::string& name, EFlavorType type, const CEntityInfo& info,
                 const CTransform4f& transform, const CModelData& modelData,
                 const CPatternedInfo& patternedInfo, const CActorParameters& actorParams,
                 const CGrappleParameters& grappleParams)
: CPatterned(kC_Ripper, uid, name, type, info, transform, modelData, patternedInfo, kMT_Flyer,
             kCT_One, kBT_Flyer, actorParams, kCS_Medium)
, mGrappleParams(grappleParams)
, mGrapplePoint(kInvalidUniqueId)
, mPlatform(kInvalidUniqueId)
, mMuted(false) {

  SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
      CMaterialList(kMT_Solid),
      CMaterialList(kMT_NoStaticCollision, kMT_NoPlatformCollision, kMT_Platform)));
  KnockBackCtrl().SetAutoResetImpulse(false);
  KnockBackCtrl().SetAnimationStateRange(kAR_Flinch, kAR_KnockBack);
}

CRipper::~CRipper() {}

ENTITY_ACCEPT_IMPL(CRipper)

void CRipper::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  ProcessGrapplePoint(mgr);
  const CPlayer* player = mgr.GetPlayer();
  CGrappleArm::EArmState armState = player->GetPlayerGun()->GetGrappleArm().GetAnimState();
  if (mGrapplePoint == kInvalidUniqueId || player->GetOrbitTargetId() != mGrapplePoint ||
      player->GetGrappleState() == CPlayer::kGS_None) {
    CPatterned::Think(dt, mgr);
    if (mMuted) {
      SetMuted(false);
      mMuted = false;
    }

    return;
  }

  if (player->GetGrappleState() == CPlayer::kGS_Firing &&
      ((uint)armState <= (uint)CGrappleArm::kAS_FireGrapple ||
       armState == CGrappleArm::kAS_Three)) {
    CPatterned::Think(dt, mgr);
  } else {
    Stop();
    if (!mMuted) {
      SetMuted(true);
      mMuted = true;
    }
  }
}

void CRipper::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) {
  CPatterned::AcceptScriptMsg(msg, sender, mgr);

  switch (msg) {
  case kSM_Registered: {
    BodyCtrl()->Activate(mgr);
    AddMaterial(kMT_Immovable, mgr);
    RemoveMaterial(kMT_Solid, mgr);

    if (GetFlavorType() == kFT_One) {
      AddGrapplePoint(mgr);
      RemoveMaterial(kMT_Orbit, mgr);
    }
    AddPlatform(mgr);
    break;
  }
  case kSM_Deleted: {
    RemoveGrapplePoint(mgr);
    RemovePlatform(mgr);
    break;
  }
  case kSM_Activate:
    AddGrapplePoint(mgr);
    AddPlatform(mgr);
    break;
  case kSM_Deactivate: {
    RemoveGrapplePoint(mgr);
    RemovePlatform(mgr);
    break;
  }
  default:
    break;
  }
}
EWeaponCollisionResponseTypes CRipper::GetCollisionResponseType(const CVector3f& position,
                                                                const CVector3f& direction,
                                                                const CWeaponMode& mode,
                                                                int projectileAtrib) const {
  EWeaponCollisionResponseTypes ret = kWCR_Unknown32;
  if (!GetDamageVulnerability()->WeaponHits(mode, CDamageVulnerability::kRD_No)) {
    ret = kWCR_Unknown82;
  }

  return ret;
}

void CRipper::KnockBack(const CVector3f& direction, CStateManager& mgr, const CDamageInfo& damage,
                        float mag, bool direct, const bool inDeferred) {
  CPatterned::KnockBack(direction, mgr, damage, mag, direct, inDeferred);
  BodyCtrl()->CommandMgr().DeliverCmd(CBCKnockBackCmd(-direction, pas::kS_One));
}

bool CRipper::PathOver(CStateManager& mgr, float arg) { return false; }

void CRipper::Patrol(CStateManager& mgr, EStateMsg msg, float arg) {
  BodyCtrl()->CommandMgr().SetSteeringBlendMode(kSBM_FullSpeed);
  BodyCtrl()->CommandMgr().SetSteeringSpeedRange(1.f, 1.f);
  CPatterned::Patrol(mgr, msg, arg);
}

void CRipper::AddGrapplePoint(CStateManager& mgr) {
  if (mGrapplePoint != kInvalidUniqueId) {
    return;
  }

  mGrapplePoint = mgr.AllocateUniqueId();
  mgr.AddObject(
      rs_new CScriptGrapplePoint(mGrapplePoint, rstl::string_l("RipperGrapplePoint"),
                                 CEntityInfo(GetCurrentAreaId(), CEntity::NullConnectionList),
                                 GetTransform(), true, mGrappleParams));
}

void CRipper::RemoveGrapplePoint(CStateManager& mgr) {
  if (mGrapplePoint == kInvalidUniqueId) {
    return;
  }

  mgr.DeleteObjectRequest(mGrapplePoint);
  mGrapplePoint = kInvalidUniqueId;
}

void CRipper::ProcessGrapplePoint(CStateManager& mgr) {
  if (GetFlavorType() == kFT_One && mGrapplePoint != kInvalidUniqueId) {
    if (CScriptGrapplePoint* gp =
            TCastToPtr< CScriptGrapplePoint >(mgr.ObjectById(mGrapplePoint))) {
      gp->SetTransform(GetTransform());
    }
  }
}

void CRipper::AddPlatform(CStateManager& mgr) {
  if (mPlatform != kInvalidUniqueId) {
    return;
  }
  mPlatform = mgr.AllocateUniqueId();
  CAABox bounds = GetModelData()->GetBounds(GetTransform().GetRotation());
  CRipperControlledPlatform* platform = rs_new CRipperControlledPlatform(
      mPlatform, GetUniqueId(), rstl::string_l("Ripper Controlled Platform"),
      CEntityInfo(GetCurrentAreaId(), CEntity::NullConnectionList), GetTransform(), bounds,
      GetActive(), rstl::optional_object_null());

  if (!platform) {
    return;
  }
  mgr.AddObject(platform);
  platform->AddMaterial(kMT_ProjectilePassthrough, mgr);
}

void CRipper::RemovePlatform(CStateManager& mgr) {
  if (mPlatform == kInvalidUniqueId) {
    return;
  }

  mgr.DeleteObjectRequest(mPlatform);
  mPlatform = kInvalidUniqueId;
}
