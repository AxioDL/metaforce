#ifndef _CSCRIPTTRIGGER
#define _CSCRIPTTRIGGER

#include "MetroidPrime/CActor.hpp"

#include "MetroidPrime/CDamageInfo.hpp"

#include "rstl/list.hpp"

enum ETriggerFlags {
  kTFL_None = 0,
  kTFL_DetectPlayer = (1 << 0),
  kTFL_DetectAI = (1 << 1),
  kTFL_DetectProjectiles1 = (1 << 2),
  kTFL_DetectProjectiles2 = (1 << 3),
  kTFL_DetectProjectiles3 = (1 << 4),
  kTFL_DetectProjectiles4 = (1 << 5),
  kTFL_DetectBombs = (1 << 6),
  kTFL_DetectPowerBombs = (1 << 7),
  kTFL_DetectProjectiles5 = (1 << 8),
  kTFL_DetectProjectiles6 = (1 << 9),
  kTFL_DetectProjectiles7 = (1 << 10),
  kTFL_KillOnEnter = (1 << 11),
  kTFL_DetectMorphedPlayer = (1 << 12),
  kTFL_UseCollisionImpulses = (1 << 13),
  kTFL_DetectCamera = (1 << 14),
  kTFL_UseBooleanIntersection = (1 << 15),
  kTFL_DetectUnmorphedPlayer = (1 << 16),
  kTFL_BlockEnvironmentalEffects = (1 << 17)
};

class CScriptTrigger : public CActor {
public:
  class CObjectTracker {
    TUniqueId x0_id;

  public:
    explicit CObjectTracker(TUniqueId id) : x0_id(id) {}

    TUniqueId GetObjectId() const { return x0_id; }
    bool operator==(const CObjectTracker& other) const { return x0_id == other.x0_id; }
  };

  rstl::list< CObjectTracker > xe8_inhabitants;
  CDamageInfo x100_damageInfo;
  CVector3f x11c_forceField;
  float x128_forceMagnitude;
  uint x12c_flags;
  CAABox x130_bounds;
  bool x148_24_detectCamera : 1;
  bool x148_25_camSubmerged : 1;
  bool x148_26_deactivateOnEntered : 1;
  bool x148_27_deactivateOnExited : 1;
  bool x148_28_playerTriggerProc : 1;
  bool x148_29_didPhazonDamage : 1;

public:
  CScriptTrigger(const TUniqueId, const rstl::string& name, const CEntityInfo& info,
                 const CVector3f& pos, const CAABox&, const CDamageInfo& dInfo,
                 const CVector3f& orientedForce, const uint triggerFlags, bool active,
                 bool deactivateOnEntered, const bool deactivateOnExited);
  ~CScriptTrigger();

  void Touch(CActor&, CStateManager&) override;
  CAABox GetTriggerBoundsWR() const;
  const CAABox& GetTriggerBounds() const { return x130_bounds; }
  uint GetTriggerFlags() const { return x12c_flags; }
  const CDamageInfo& GetDamageInfo() const { return x100_damageInfo; }
  float GetForceMagnitude() const { return x128_forceMagnitude; }
  const CVector3f& GetForceField() const { return x11c_forceField; }
  void SetForceField(const CVector3f& force) {
    x11c_forceField = force;
    x128_forceMagnitude = force.Magnitude();
  }
  bool GetPlayerInside() const { return x148_28_playerTriggerProc; }
  void SetTriggerBounds(const CAABox& bounds) { x130_bounds = bounds; }
  bool CameraInside() const { return x148_25_camSubmerged; }
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
  void Think(float, CStateManager&) override;
  void UpdateInhabitants(float, CStateManager&);
  const rstl::list< CObjectTracker >& GetInhabitants() const;
  const CScriptTrigger::CObjectTracker* FindObject(TUniqueId);
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  virtual void InhabitantAdded(CActor&, CStateManager&);
  virtual void InhabitantIdle(CActor&, CStateManager&);
  virtual void InhabitantExited(CActor&, CStateManager&);
  virtual void InhabitantRejected(CActor&, CStateManager&);

private:
  void ActivatePlayer(CStateManager&);
  void DeactivatePlayer(CStateManager&);
};

#endif // _CSCRIPTTRIGGER
