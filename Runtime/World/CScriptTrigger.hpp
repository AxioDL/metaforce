#pragma once

#include <list>
#include <string_view>

#include "Runtime/World/CActor.hpp"
#include "Runtime/World/CDamageInfo.hpp"

#include <zeus/CAABox.hpp>
#include <zeus/CVector3f.hpp>

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
    explicit CObjectTracker(TUniqueId id) : x0_id(id) {}

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
  bool x148_24_detectCamera : 1 = false;
  bool x148_25_camSubmerged : 1 = false;
  bool x148_26_deactivateOnEntered : 1;
  bool x148_27_deactivateOnExited : 1;
  bool x148_28_playerTriggerProc : 1 = false;
  bool x148_29_didPhazonDamage : 1 = false;

public:
  CScriptTrigger(TUniqueId, std::string_view name, const CEntityInfo& info, const zeus::CVector3f& pos,
                 const zeus::CAABox&, const CDamageInfo& dInfo, const zeus::CVector3f& orientedForce,
                 ETriggerFlags triggerFlags, bool, bool, bool);

  void Accept(IVisitor& visitor) override;
  void Think(float, CStateManager&) override;
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
  virtual void InhabitantRejected(CActor&, CStateManager&) {}
  virtual void InhabitantExited(CActor&, CStateManager&) {}
  virtual void InhabitantIdle(CActor&, CStateManager&) {}
  virtual void InhabitantAdded(CActor&, CStateManager&) {}
  CObjectTracker* FindObject(TUniqueId);
  void UpdateInhabitants(float, CStateManager&);
  std::list<CObjectTracker>& GetInhabitants();
  std::optional<zeus::CAABox> GetTouchBounds() const override;
  void Touch(CActor&, CStateManager&) override;
  const zeus::CAABox& GetTriggerBoundsOR() const { return x130_bounds; }
  zeus::CAABox GetTriggerBoundsWR() const;
  const CDamageInfo& GetDamageInfo() const { return x100_damageInfo; }
  ETriggerFlags GetTriggerFlags() const { return x12c_flags; }
  float GetForceMagnitude() const { return x128_forceMagnitude; }
  const zeus::CVector3f& GetForceVector() const { return x11c_forceField; }
  void SetForceVector(const zeus::CVector3f& force) {
    x11c_forceField = force;
    x128_forceMagnitude = x11c_forceField.magnitude();
  }
  bool IsPlayerTriggerProc() const { return x148_28_playerTriggerProc; }
};
} // namespace urde
