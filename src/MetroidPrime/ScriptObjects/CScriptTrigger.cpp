#include "MetroidPrime/ScriptObjects/CScriptTrigger.hpp"

#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Weapons/CWeapon.hpp"

inline void CScriptTrigger::ActivatePlayer(CStateManager& mgr) {
  if (mPlayerTriggerProc != true) {
    mPlayerTriggerProc = true;

    CPlayer* pl = mgr.Player();
    if (mDidPhazonDamage) {
      pl->DecrementEnvironmentDamage();
      mDidPhazonDamage = false;
    }
    if (!mDamageInfo.HasNoDamage()) {
      const CDamageVulnerability* dVuln = pl->GetDamageVulnerability();
      bool phazonHurt =
          dVuln->WeaponHurts(mDamageInfo.GetWeaponMode(), CDamageVulnerability::kRD_No);
      if (mDamageInfo.GetWeaponMode().GetType() == kWT_Phazon) {
        if (mgr.GetPlayerState()->HasPowerUp(CPlayerState::kIT_PhazonSuit)) {
          phazonHurt = false;
        }
      }
      if (phazonHurt) {
        pl->IncrementEnvironmentDamage();
        mDidPhazonDamage = true;
      }
    }
  }
}

inline void CScriptTrigger::DeactivatePlayer(CStateManager& mgr) {
  mPlayerTriggerProc = false;
  CPlayer* player = mgr.Player();
  if (mDidPhazonDamage) {
    player->DecrementEnvironmentDamage();
    mDidPhazonDamage = false;
  }
  if (mgr.GetLastTriggerId() == GetUniqueId()) {
    mgr.SetLastTriggerId(kInvalidUniqueId);
  }
}

CScriptTrigger::CScriptTrigger(const TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                               const CVector3f& pos, const CAABox& bounds, const CDamageInfo& dInfo,
                               const CVector3f& forceField, const uint triggerFlags, const bool active,
                               const bool deactivateOnEntered, const bool deactivateOnExited)
: CActor(uid, active, name, info, CTransform4f::Translate(pos), CModelData::CModelDataNull(),
         CMaterialList(kMT_Trigger), CActorParameters::None(), kInvalidUniqueId)
, mDamageInfo(dInfo)
, mForceField(forceField)
, mForceMagnitude(forceField.Magnitude())
, mFlags(triggerFlags)
, mBounds(bounds)
, mDetectCamera(false)
, mCamSubmerged(false)
, mDeactivateOnEntered(deactivateOnEntered)
, mDeactivateOnExited(deactivateOnExited)
, mPlayerTriggerProc(false)
, mDidPhazonDamage(false) {
  SetCallTouch(false);
}

CScriptTrigger::~CScriptTrigger() {
  AUTO(it, mInhabitants.begin());
  AUTO(end, mInhabitants.end());
  AUTO(old, it);
  while (it != end) {
    ++it;
    mInhabitants.erase(old);
    old = it;
  }
  if (mFlags & 0x11000) {
    mFlags = mFlags & 0xfffeefff;
    mFlags = mFlags | 1;
  }
}

void CScriptTrigger::Touch(CActor& act, CStateManager& mgr) {
  if (GetActive() && !act.GetMaterialList().HasMaterial(kMT_Trigger)) {
    if (FindObject(act.GetUniqueId()) == nullptr) {
      uint testFlags = kTFL_None;
      CPlayer* const pl = TCastToPtr< CPlayer >(act);
      if (pl) {
        if (mForceMagnitude > 0.f && ((mFlags & kTFL_DetectPlayer) != 0)) {
          if (mgr.GetLastTriggerId() != kInvalidUniqueId) {
            return;
          }
          mgr.SetLastTriggerId(GetUniqueId());
        }

        testFlags |= kTFL_DetectPlayer;
        if (pl->GetMorphballTransitionState() == CPlayer::kMS_Unmorphed) {
          testFlags |= kTFL_DetectUnmorphedPlayer;
        }
        if (pl->GetMorphballTransitionState() == CPlayer::kMS_Morphed) {
          testFlags |= kTFL_DetectMorphedPlayer;
        }
      }
      if (TCastToPtr< CPatterned >(act)) {
        testFlags |= kTFL_DetectAI;
      }
      if (TCastToPtr< CGameProjectile >(act)) {
        testFlags |= kTFL_DetectProjectiles1 | kTFL_DetectProjectiles2 | kTFL_DetectProjectiles3 |
                     kTFL_DetectProjectiles4 | kTFL_DetectProjectiles5 | kTFL_DetectProjectiles6 |
                     kTFL_DetectProjectiles7;
      } else if (TCastToConstPtr< CWeapon >(act)) {
        const CWeapon& weap = static_cast< const CWeapon& >(act);
        if ((weap.GetAttribField() & CWeapon::kPA_Bombs) == CWeapon::kPA_Bombs) {
          testFlags |= kTFL_DetectBombs;
        } else if ((weap.GetAttribField() & CWeapon::kPA_PowerBombs) == CWeapon::kPA_PowerBombs) {
          testFlags |= kTFL_DetectPowerBombs;
        }
      }

      if (testFlags & mFlags) {
        mInhabitants.push_back(CObjectTracker(act.GetUniqueId()));
        InhabitantAdded(act, mgr);

        if ((testFlags & kTFL_DetectPlayer) && pl) {
          ActivatePlayer(mgr);
        }

        SendScriptMsgs(kSS_Entered, mgr, kSM_None);

        if (mDeactivateOnEntered) {
          mgr.SendScriptMsg(GetUniqueId(), mgr.GetEditorIdForUniqueId(GetUniqueId()),
                            kSM_Deactivate, kSS_Entered);
          if (act.HealthInfo(mgr) && mDamageInfo.GetDamage() > 0.f) {
            mgr.ApplyDamage(
                GetUniqueId(), act.GetUniqueId(), GetUniqueId(), mDamageInfo,
                CMaterialFilter::MakeIncludeExclude(CMaterialList(SolidMaterial), CMaterialList(0)),
                CVector3f::Zero());
          }
        }

        if (mFlags & kTFL_KillOnEnter) {
          CHealthInfo* hInfo = act.HealthInfo(mgr);
          if (hInfo) {
            static CWeaponMode sktonOHurtWeaponMode(kWT_Power, false, false, true);
            CDamageInfo dmg(sktonOHurtWeaponMode, 10.f * hInfo->GetHP(), 0.f, 0.f);

            mgr.ApplyDamage(
                GetUniqueId(), act.GetUniqueId(), GetUniqueId(), dmg,
                CMaterialFilter::MakeIncludeExclude(CMaterialList(SolidMaterial), CMaterialList(0)),
                CVector3f::Zero());
          }
        }
      } else {
        InhabitantRejected(act, mgr);
      }
    }
  }
}

CAABox CScriptTrigger::GetTriggerBoundsWR() const {
  return CAABox(mBounds.GetMinPoint() + GetTranslation(),
                mBounds.GetMaxPoint() + GetTranslation());
}

rstl::optional_object< CAABox > CScriptTrigger::GetTouchBounds() const {
  if (GetActive()) {
    return GetTriggerBoundsWR();
  }
  return rstl::optional_object_null();
}

void CScriptTrigger::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  if (GetActive()) {
    if (msg == kSM_Deactivate) {
      mInhabitants.clear();
      mCamSubmerged = false;

      if (mPlayerTriggerProc) {
        DeactivatePlayer(mgr);
      }

    } else if (msg == kSM_Deleted) {
      if (mPlayerTriggerProc) {
        DeactivatePlayer(mgr);
      }
    }
  }

  CActor::AcceptScriptMsg(msg, uid, mgr);
}

void CScriptTrigger::Think(float dt, CStateManager& mgr) {
  if (GetActive()) {
    UpdateInhabitants(dt, mgr);
  }
}

void CScriptTrigger::UpdateInhabitants(float dt, CStateManager& mgr) {
  bool sendInside = false;
  bool sendExited = false;
  rstl::optional_object< CDamageInfo > timedDamage;
  for (rstl::list< CObjectTracker >::iterator it = mInhabitants.begin();
       it != mInhabitants.end();) {
    AUTO(nextIt, it);
    ++nextIt;
#if NONMATCHING
    const TUniqueId objectId = it->GetObjectId();
#endif
    if (CActor* act = TCastToPtr< CActor >(mgr.ObjectById(it->GetObjectId()))) {
      bool playerValid = true;
      if (it->GetObjectId() == mgr.GetPlayer()->GetUniqueId()) {
        if ((mFlags & kTFL_DetectPlayer) == 0) {
          if (mgr.GetPlayer()->GetMorphballTransitionState() == CPlayer::kMS_Morphed) {
            if (mFlags & kTFL_DetectUnmorphedPlayer) {
              playerValid = false;
            }
          }
          if (mgr.GetPlayer()->GetMorphballTransitionState() == CPlayer::kMS_Unmorphed) {
            if (mFlags & kTFL_DetectMorphedPlayer) {
              playerValid = false;
            }
          }
        }
        if (!playerValid) {
          mInhabitants.erase(it);
          sendExited = true;
          if (mPlayerTriggerProc) {
            DeactivatePlayer(mgr);
          }

          InhabitantExited(*act, mgr);
        }
      }
      if (playerValid) {
        rstl::optional_object< CAABox > touchBounds = GetTouchBounds();
        rstl::optional_object< CAABox > actTouchBounds = act->GetTouchBounds();
        if (touchBounds.valid() && actTouchBounds.valid() &&
            touchBounds->DoBoundsOverlap(*actTouchBounds)) {
          sendInside = true;
          InhabitantIdle(*act, mgr);
          if (act->HealthInfo(mgr) && mDamageInfo.GetDamage() > 0.f) {
            if (!timedDamage) {
              timedDamage = mDamageInfo.MakeScaledForTime(dt);
            }
            mgr.ApplyDamage(
                GetUniqueId(), act->GetUniqueId(), GetUniqueId(), *timedDamage,
                CMaterialFilter::MakeIncludeExclude(CMaterialList(SolidMaterial), CMaterialList(0)),
                CVector3f::Zero());
          }

          if (mForceMagnitude > 0.f) {
            if (CPhysicsActor* pact = TCastToPtr< CPhysicsActor >(act)) {
              float forceMult = 1.f;
              if ((mFlags & kTFL_UseBooleanIntersection)) {
                forceMult = touchBounds->GetBooleanIntersection(*actTouchBounds).GetVolume() /
                            actTouchBounds->GetVolume();
              }

              const CVector3f force = forceMult * mForceField;
              if ((mFlags & kTFL_UseCollisionImpulses)) {
                pact->ApplyImpulseWR(force, CAxisAngle::Identity());
                pact->UseCollisionImpulses();
              } else {
                pact->ApplyForceWR(force, CAxisAngle::Identity());
              }
            }
          }
        } else {
          mInhabitants.erase(it);
          sendExited = true;
#if NONMATCHING
          if (objectId == mgr.GetPlayer()->GetUniqueId() && mPlayerTriggerProc) {
#else
          if (it->GetObjectId() == mgr.GetPlayer()->GetUniqueId() && mPlayerTriggerProc) {
#endif
            DeactivatePlayer(mgr);
          }

          InhabitantExited(*act, mgr);
        }
      }
    } else {
      mInhabitants.erase(it);
#if NONMATCHING
      if (objectId == mgr.GetPlayer()->GetUniqueId() && mPlayerTriggerProc) {
#else
      if (it->GetObjectId() == mgr.GetPlayer()->GetUniqueId() && mPlayerTriggerProc) {
#endif
        DeactivatePlayer(mgr);
      }
    }
    it = nextIt;
  }

  if ((mFlags & kTFL_DetectCamera) || mDetectCamera) {
    CGameCamera& cam = mgr.CameraManager()->CurrentCamera(mgr);
    const bool camInTrigger = GetTriggerBoundsWR().PointInside(cam.GetTranslation());
    if (mCamSubmerged) {
      if (!camInTrigger) {
        mCamSubmerged = false;
        if ((mFlags & kTFL_DetectCamera)) {
          sendExited = true;
          InhabitantExited(cam, mgr);
        }
      } else {
        if ((mFlags & kTFL_DetectCamera)) {
          InhabitantIdle(cam, mgr);
          sendInside = true;
        }
      }
    } else {
      if (camInTrigger) {
        mCamSubmerged = true;
        if ((mFlags & kTFL_DetectCamera)) {
          InhabitantAdded(cam, mgr);
          SendScriptMsgs(kSS_Entered, mgr, kSM_None);
        }
      }
    }
  }

  if (sendInside) {
    SendScriptMsgs(kSS_Inside, mgr, kSM_None);
  }

  if (sendExited) {
    SendScriptMsgs(kSS_Exited, mgr, kSM_None);
    if (mDeactivateOnExited) {
      mgr.SendScriptMsg(GetUniqueId(), mgr.GetEditorIdForUniqueId(GetUniqueId()), kSM_Deactivate,
                        kSS_Exited);
    }
  }
}

const rstl::list< CScriptTrigger::CObjectTracker >& CScriptTrigger::GetInhabitants() const {
  return mInhabitants;
}

const CScriptTrigger::CObjectTracker* CScriptTrigger::FindObject(TUniqueId id) {
  for (rstl::list< CScriptTrigger::CObjectTracker >::const_iterator it = GetInhabitants().begin();
       it != GetInhabitants().end(); ++it) {
    if (it->GetObjectId() == id) {
      return &*it;
    }
  }
  return nullptr;
}

ENTITY_ACCEPT_IMPL(CScriptTrigger)

void CScriptTrigger::InhabitantAdded(CActor&, CStateManager&) {}

void CScriptTrigger::InhabitantIdle(CActor&, CStateManager&) {}

void CScriptTrigger::InhabitantExited(CActor&, CStateManager&) {}

void CScriptTrigger::InhabitantRejected(CActor&, CStateManager&) {}
