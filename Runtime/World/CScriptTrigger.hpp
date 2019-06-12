#pragma once

#include "CActor.hpp"
#include "CDamageInfo.hpp"

namespace urde {

// TODO - Phil: Figure out what each of the DetectProjectiles actually mean
enum class ETriggerFlags : u32 {
  None = 0,
  DetectPlayer = (1 << 0),
  DetectAI = (1 << 1),
  DetectProjectiles1 = (1 << 2),
  DetectProjectiles2 = (1 << 3),
  DetectProjectiles3 = (1 << 4),
  DetectProjectiles4 = (1 << 5),
  DetectBombs = (1 << 6),
  DetectPowerBombs = (1 << 7),
  DetectProjectiles5 = (1 << 8),
  DetectProjectiles6 = (1 << 9),
  DetectProjectiles7 = (1 << 10),
  KillOnEnter = (1 << 11),
  DetectMorphedPlayer = (1 << 12),
  UseCollisionImpulses = (1 << 13),
  DetectCamera = (1 << 14),
  UseBooleanIntersection = (1 << 15),
  DetectUnmorphedPlayer = (1 << 16),
  BlockEnvironmentalEffects = (1 << 17)
};
ENABLE_BITWISE_ENUM(ETriggerFlags)

class CScriptTrigger : public CActor {
public:
  class CObjectTracker {
    TUniqueId x0_id;

  public:
    CObjectTracker(TUniqueId id) : x0_id(id) {}

    TUniqueId GetObjectId() const { return x0_id; }
    bool operator==(const CObjectTracker& other) const { return x0_id == other.x0_id; }
  };

protected:
  std::list<CObjectTracker> xe8_inhabitants;
  CDamageInfo x100_damageInfo;
  zeus::CVector3f x11c_forceField;
  float x128_forceMagnitude;
  ETriggerFlags x12c_flags;
  zeus::CAABox x130_bounds;

  union {
    struct {
      bool x148_24_detectCamera : 1;
      bool x148_25_camSubmerged : 1;
      bool x148_26_deactivateOnEntered : 1;
      bool x148_27_deactivateOnExited : 1;
      bool x148_28_playerTriggerProc : 1;
      bool x148_29_didPhazonDamage : 1;
    };
    u8 dummy = 0;
  };

public:
  CScriptTrigger(TUniqueId, std::string_view name, const CEntityInfo& info, const zeus::CVector3f& pos,
                 const zeus::CAABox&, const CDamageInfo& dInfo, const zeus::CVector3f& orientedForce,
                 ETriggerFlags triggerFlags, bool, bool, bool);

  void Accept(IVisitor& visitor);
  void Think(float, CStateManager&);
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);
  virtual void InhabitantRejected(CActor&, CStateManager&) {}
  virtual void InhabitantExited(CActor&, CStateManager&) {}
  virtual void InhabitantIdle(CActor&, CStateManager&) {}
  virtual void InhabitantAdded(CActor&, CStateManager&) {}
  CObjectTracker* FindObject(TUniqueId);
  void UpdateInhabitants(float, CStateManager&);
  std::list<CObjectTracker>& GetInhabitants();
  std::optional<zeus::CAABox> GetTouchBounds() const;
  void Touch(CActor&, CStateManager&);
  const zeus::CAABox& GetTriggerBoundsOR() const { return x130_bounds; }
  zeus::CAABox GetTriggerBoundsWR() const;
  const CDamageInfo& GetDamageInfo() const { return x100_damageInfo; }
  ETriggerFlags GetTriggerFlags() const { return x12c_flags; }
  float GetForceMagnitude() const { return x128_forceMagnitude; }
  const zeus::CVector3f& GetForceVector() const { return x11c_forceField; }
};
} // namespace urde
