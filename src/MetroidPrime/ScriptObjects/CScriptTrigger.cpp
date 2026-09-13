#include "MetroidPrime/ScriptObjects/CScriptTrigger.hpp"

#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Weapons/CWeapon.hpp"

inline void CScriptTrigger::ActivatePlayer(CStateManager& mgr) {
  if (x148_28_playerTriggerProc != true) {
    x148_28_playerTriggerProc = true;

    CPlayer* pl = mgr.Player();
    if (x148_29_didPhazonDamage) {
      pl->DecrementEnvironmentDamage();
      x148_29_didPhazonDamage = false;
    }
    if (!x100_damageInfo.HasNoDamage()) {
      const CDamageVulnerability* dVuln = pl->GetDamageVulnerability();
      bool phazonHurt =
          dVuln->WeaponHurts(x100_damageInfo.GetWeaponMode(), CDamageVulnerability::kRD_No);
      if (x100_damageInfo.GetWeaponMode().GetType() == kWT_Phazon) {
        if (mgr.GetPlayerState()->HasPowerUp(CPlayerState::kIT_PhazonSuit)) {
          phazonHurt = false;
        }
      }
      if (phazonHurt) {
        pl->IncrementEnvironmentDamage();
        x148_29_didPhazonDamage = true;
      }
    }
  }
}

inline void CScriptTrigger::DeactivatePlayer(CStateManager& mgr) {
  x148_28_playerTriggerProc = false;
  CPlayer* player = mgr.Player();
  if (x148_29_didPhazonDamage) {
    player->DecrementEnvironmentDamage();
    x148_29_didPhazonDamage = false;
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
, x100_damageInfo(dInfo)
, x11c_forceField(forceField)
, x128_forceMagnitude(forceField.Magnitude())
, x12c_flags(triggerFlags)
, x130_bounds(bounds)
, x148_24_detectCamera(false)
, x148_25_camSubmerged(false)
, x148_26_deactivateOnEntered(deactivateOnEntered)
, x148_27_deactivateOnExited(deactivateOnExited)
, x148_28_playerTriggerProc(false)
, x148_29_didPhazonDamage(false) {
  SetCallTouch(false);
}

CScriptTrigger::~CScriptTrigger() {
  AUTO(it, xe8_inhabitants.begin());
  AUTO(end, xe8_inhabitants.end());
  AUTO(old, it);
  while (it != end) {
    ++it;
    xe8_inhabitants.erase(old);
    old = it;
  }
  if (x12c_flags & 0x11000) {
    x12c_flags = x12c_flags & 0xfffeefff;
    x12c_flags = x12c_flags | 1;
  }
}

void CScriptTrigger::Touch(CActor& act, CStateManager& mgr) {
  if (GetActive() && !act.GetMaterialList().HasMaterial(kMT_Trigger)) {
    if (FindObject(act.GetUniqueId()) == nullptr) {
      uint testFlags = kTFL_None;
      CPlayer* const pl = TCastToPtr< CPlayer >(act);
      if (pl) {
        if (x128_forceMagnitude > 0.f && ((x12c_flags & kTFL_DetectPlayer) != 0)) {
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

      if (testFlags & x12c_flags) {
        xe8_inhabitants.push_back(CObjectTracker(act.GetUniqueId()));
        InhabitantAdded(act, mgr);

        if ((testFlags & kTFL_DetectPlayer) && pl) {
          ActivatePlayer(mgr);
        }

        SendScriptMsgs(kSS_Entered, mgr, kSM_None);

        if (x148_26_deactivateOnEntered) {
          mgr.SendScriptMsg(GetUniqueId(), mgr.GetEditorIdForUniqueId(GetUniqueId()),
                            kSM_Deactivate, kSS_Entered);
          if (act.HealthInfo(mgr) && x100_damageInfo.GetDamage() > 0.f) {
            mgr.ApplyDamage(
                GetUniqueId(), act.GetUniqueId(), GetUniqueId(), x100_damageInfo,
                CMaterialFilter::MakeIncludeExclude(CMaterialList(SolidMaterial), CMaterialList(0)),
                CVector3f::Zero());
          }
        }

        if (x12c_flags & kTFL_KillOnEnter) {
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
  return CAABox(x130_bounds.GetMinPoint() + GetTranslation(),
                x130_bounds.GetMaxPoint() + GetTranslation());
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
      xe8_inhabitants.clear();
      x148_25_camSubmerged = false;

      if (x148_28_playerTriggerProc) {
        DeactivatePlayer(mgr);
      }

    } else if (msg == kSM_Deleted) {
      if (x148_28_playerTriggerProc) {
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
  for (rstl::list< CObjectTracker >::iterator it = xe8_inhabitants.begin();
       it != xe8_inhabitants.end();) {
    AUTO(nextIt, it);
    ++nextIt;
#if NONMATCHING
    const TUniqueId objectId = it->GetObjectId();
#endif
    if (CActor* act = TCastToPtr< CActor >(mgr.ObjectById(it->GetObjectId()))) {
      bool playerValid = true;
      if (it->GetObjectId() == mgr.GetPlayer()->GetUniqueId()) {
        if ((x12c_flags & kTFL_DetectPlayer) == 0) {
          if (mgr.GetPlayer()->GetMorphballTransitionState() == CPlayer::kMS_Morphed) {
            if (x12c_flags & kTFL_DetectUnmorphedPlayer) {
              playerValid = false;
            }
          }
          if (mgr.GetPlayer()->GetMorphballTransitionState() == CPlayer::kMS_Unmorphed) {
            if (x12c_flags & kTFL_DetectMorphedPlayer) {
              playerValid = false;
            }
          }
        }
        if (!playerValid) {
          xe8_inhabitants.erase(it);
          sendExited = true;
          if (x148_28_playerTriggerProc) {
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
          if (act->HealthInfo(mgr) && x100_damageInfo.GetDamage() > 0.f) {
            if (!timedDamage) {
              timedDamage = x100_damageInfo.MakeScaledForTime(dt);
            }
            mgr.ApplyDamage(
                GetUniqueId(), act->GetUniqueId(), GetUniqueId(), *timedDamage,
                CMaterialFilter::MakeIncludeExclude(CMaterialList(SolidMaterial), CMaterialList(0)),
                CVector3f::Zero());
          }

          if (x128_forceMagnitude > 0.f) {
            if (CPhysicsActor* pact = TCastToPtr< CPhysicsActor >(act)) {
              float forceMult = 1.f;
              if ((x12c_flags & kTFL_UseBooleanIntersection)) {
                forceMult = touchBounds->GetBooleanIntersection(*actTouchBounds).GetVolume() /
                            actTouchBounds->GetVolume();
              }

              const CVector3f force = forceMult * x11c_forceField;
              if ((x12c_flags & kTFL_UseCollisionImpulses)) {
                pact->ApplyImpulseWR(force, CAxisAngle::Identity());
                pact->UseCollisionImpulses();
              } else {
                pact->ApplyForceWR(force, CAxisAngle::Identity());
              }
            }
          }
        } else {
          xe8_inhabitants.erase(it);
          sendExited = true;
#if NONMATCHING
          if (objectId == mgr.GetPlayer()->GetUniqueId() && x148_28_playerTriggerProc) {
#else
          if (it->GetObjectId() == mgr.GetPlayer()->GetUniqueId() && x148_28_playerTriggerProc) {
#endif
            DeactivatePlayer(mgr);
          }

          InhabitantExited(*act, mgr);
        }
      }
    } else {
      xe8_inhabitants.erase(it);
#if NONMATCHING
      if (objectId == mgr.GetPlayer()->GetUniqueId() && x148_28_playerTriggerProc) {
#else
      if (it->GetObjectId() == mgr.GetPlayer()->GetUniqueId() && x148_28_playerTriggerProc) {
#endif
        DeactivatePlayer(mgr);
      }
    }
    it = nextIt;
  }

  if ((x12c_flags & kTFL_DetectCamera) || x148_24_detectCamera) {
    CGameCamera& cam = mgr.CameraManager()->CurrentCamera(mgr);
    const bool camInTrigger = GetTriggerBoundsWR().PointInside(cam.GetTranslation());
    if (x148_25_camSubmerged) {
      if (!camInTrigger) {
        x148_25_camSubmerged = false;
        if ((x12c_flags & kTFL_DetectCamera)) {
          sendExited = true;
          InhabitantExited(cam, mgr);
        }
      } else {
        if ((x12c_flags & kTFL_DetectCamera)) {
          InhabitantIdle(cam, mgr);
          sendInside = true;
        }
      }
    } else {
      if (camInTrigger) {
        x148_25_camSubmerged = true;
        if ((x12c_flags & kTFL_DetectCamera)) {
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
    if (x148_27_deactivateOnExited) {
      mgr.SendScriptMsg(GetUniqueId(), mgr.GetEditorIdForUniqueId(GetUniqueId()), kSM_Deactivate,
                        kSS_Exited);
    }
  }
}

const rstl::list< CScriptTrigger::CObjectTracker >& CScriptTrigger::GetInhabitants() const {
  return xe8_inhabitants;
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
