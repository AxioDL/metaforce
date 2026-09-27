#include "MetroidPrime/Enemies/CMetaree.hpp"

#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/SFX/Metaree.h"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CGameProjectile.hpp"

#include "Kyoto/Audio/CSfxManager.hpp"

#include "Collision/CCollisionInfoList.hpp"

CMetaree::CMetaree(TUniqueId uid, const rstl::string& name, EFlavorType flavor,
                   const CEntityInfo& info, const CTransform4f& xf, const CModelData& mData,
                   const CPatternedInfo& pInfo, const CDamageInfo& dInfo, float f1,
                   const CVector3f& v1, float f2, EBodyType bodyType, float f3, float f4,
                   const CActorParameters& aParms)
: CPatterned(kC_Metaree, uid, name, flavor, info, xf, mData, pInfo, kMT_Flyer, kCT_Zero, bodyType,
             aParms, kCS_Small)
, mDelay(f3)
, mHaltDelay(f4)
, mDropHeight(f1)
, mOffset(v1)
, mAttackSpeed(f2)
, mLookPos(CVector3f::Zero())
, mProjectileDelta(0.f, 0.f, 0.f)
, mVelocity(CVector3f::Zero())
, x5a8_(0)
, mDamageInfo(dInfo)
, mAttackSfx(SFXmtr_a_scream_01)
, x5ca_24_(true)
, mStarted(false)
, mDeactivated(false) {}

ENTITY_ACCEPT_IMPL(CMetaree)

void CMetaree::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  CPatterned::AcceptScriptMsg(msg, uid, mgr);

  switch (msg) {
    break;
  case kSM_Registered:
    BodyCtrl()->Activate(mgr);
    break;
  case kSM_Start:
    mStarted = true;
    break;
  case kSM_Activate:
  default:
    break;
  }
}

void CMetaree::ThinkAboutMove(float) {}

void CMetaree::CollidedWith(const TUniqueId& id, const CCollisionInfoList& colList,
                            CStateManager& mgr) {
  if (IsAlive() && colList.GetCount() > 0) {
    mgr.ApplyDamageToWorld(GetUniqueId(), *this, GetTranslation(), mDamageInfo,
                           CMaterialFilter::MakeInclude(CMaterialList(kMT_Player)));
    SendScriptMsgs(kSS_Arrived, mgr, kSM_None);
    MassiveDeath(mgr);
  }
}

void CMetaree::Touch(CActor& act, CStateManager& mgr) {
  if (!IsAlive()) {
    return;
  }

  if (CGameProjectile* projectile = TCastToPtr< CGameProjectile >(act)) {
    if (projectile->GetOwnerId() != mgr.GetPlayer()->GetUniqueId()) {
      return;
    }

    SetWasHit(true);
    mProjectileDelta = projectile->GetTranslation() - projectile->GetPreviousPos();
  }
}

bool CMetaree::ShouldAttack(CStateManager&, float) {
  return GetTranslation().GetZ() < mLookPos.GetZ();
}

bool CMetaree::InRange(CStateManager& mgr, float arg) {
  return mStarted || CPatterned::InRange(mgr, arg);
}

void CMetaree::InActive(CStateManager&, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    if (!mDeactivated) {
      BodyCtrl()->SetLocomotionType(pas::kLT_Relaxed);
    } else {
      BodyCtrl()->SetLocomotionType(pas::kLT_Crouch);
    }
    break;
  case kStateMsg_Update:
    break;
  case kStateMsg_Deactivate:
    mDeactivated = true;
    break;
  }
}

void CMetaree::Active(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate: {
    SetWasHit(false);
    const CVector3f translation = GetTranslation();
    const CVector3f dropVector(0.f, 0.f, mDropHeight);
    mLookPos = GetTranslation() - dropVector;
    BodyCtrl()->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_Zero, mLookPos, true));
    SetMomentumWR(CVector3f(0.f, 0.f, -GetGravityConstant() * GetMass()));
    break;
  }
  case kStateMsg_Update:
    BodyCtrl()->CommandMgr().DeliverTargetVector(
        (mgr.GetPlayer()->GetTranslation() - GetTranslation()).AsNormalized());
    break;
  case kStateMsg_Deactivate:
    SetMomentumWR(CVector3f::Zero());
    break;
  }
}

void CMetaree::Halt(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate: {
    Stop();
    SetVelocityWR(CVector3f::Zero());
    SetMomentumWR(CVector3f::Zero());
    BodyCtrl()->SetLocomotionType(pas::kLT_Lurk);
    mLookPos = mgr.GetPlayer()->GetTranslation() + mOffset;
    SetTransform(CTransform4f::LookAt(GetTranslation(), mLookPos));
    StateMachineState().SetDelay(mHaltDelay);
    break;
  }
  case kStateMsg_Update:
  case kStateMsg_Deactivate:
    break;
  }
}

void CMetaree::Attack(CStateManager&, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate: {
    x5a8_ = 0;
    CVector3f dir = (mLookPos - GetTranslation()).AsNormalized();
    SetVelocityWR(mAttackSpeed * dir);
    CSfxManager::AddEmitter(mAttackSfx, GetTranslation(), CVector3f::Zero(), true, false);
    BodyCtrl()->SetLocomotionType(pas::kLT_Combat);
    mVelocity = mAttackSpeed * dir;
    break;
  }
  case kStateMsg_Update:
    if (GetBodyCtrl()->GetPercentageFrozen() == 0.f) {
      SetVelocityWR(mVelocity);
    } else {
      Stop();
      SetVelocityWR(CVector3f::Zero());
    }
    break;
  case kStateMsg_Deactivate:
    break;
  }
}

void CMetaree::Dead(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    mgr.ApplyDamageToWorld(
        GetUniqueId(), *this, GetTranslation(), mDamageInfo,
        CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Player), CMaterialList()));
    DeathDelete(mgr);
    break;
  default:
    break;
  }
}

void CMetaree::Flee(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate: {
    CVector3f ang =
        GetMass() * CVector3f(mProjectileDelta.GetX(), mProjectileDelta.GetY(), 0.f) * 5.f;
    ApplyImpulseWR(ang, CAxisAngle::Identity());

    SetMomentumWR(CVector3f(0.f, 0.f, -GetGravityConstant() * GetMass()));
    SetTransform(CTransform4f::Translate(GetTranslation()));
    x5a8_ = 0;
    break;
  }
  case kStateMsg_Update: {
    switch (x5a8_) {
    case 0:
      if (GetBodyCtrl()->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_LieOnGround) {
        x5a8_ = 1;
      } else {
        BodyCtrl()->CommandMgr().DeliverCmd(
            CBCKnockDownCmd(CVector3f(0.f, 1.f, 0.f), pas::kS_Zero));
      }
      break;
    default:
      break;
    }

    break;
  }
  case kStateMsg_Deactivate:
    break;
  }
}

void CMetaree::Explode(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    mgr.ApplyDamage(GetUniqueId(), mgr.GetPlayer()->GetUniqueId(), GetUniqueId(), mDamageInfo,
                    CMaterialFilter::MakeIncludeExclude(CMaterialList(SolidMaterial), CMaterialList()),
                    CVector3f::Zero());
    MassiveDeath(mgr);
    break;
  default:
    break;
  }
}

void CMetaree::Think(float dt, CStateManager& mgr) {
  SetTargetable((mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::kPV_Thermal ||
                 mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::kPV_Scan) ||
                mDeactivated);
  CPatterned::Think(dt, mgr);
}

bool CMetaree::Delay(CStateManager&, float) {
  return GetStateMachineState().GetTime() > mDelay;
}
