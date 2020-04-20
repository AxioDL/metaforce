#pragma once

#include <optional>
#include <string_view>

#include "Runtime/GCNTypes.hpp"
#include "Runtime/Character/CharacterCommon.hpp"
#include "Runtime/World/CActor.hpp"

#include <zeus/CAABox.hpp>

namespace urde {
class CScriptCoverPoint : public CActor {
  bool xe8_26_landHere : 1;
  bool xe8_27_wallHang : 1;
  bool xe8_28_stay : 1;
  bool xe8_29_ : 1;
  bool xe8_30_attackDirection : 1;
  float xec_cosHorizontalAngle;
  float xf0_sinVerticalAngle;
  float xf4_coverTime;
  bool xf8_24_crouch : 1;
  bool xf8_25_inUse : 1 = false;
  TUniqueId xfa_occupant = kInvalidUniqueId;
  TUniqueId xfc_retreating = kInvalidUniqueId;
  std::optional<zeus::CAABox> x100_touchBounds;
  float x11c_timeLeft = 0.f;

public:
  CScriptCoverPoint(TUniqueId uid, std::string_view name, const CEntityInfo& info, zeus::CTransform xf, bool active,
                    u32 flags, bool crouch, float horizontalAngle, float verticalAngle, float coverTime);

  void Accept(IVisitor& visitor) override;
  void Think(float, CStateManager&) override;
  void AddToRenderer(const zeus::CFrustum&, CStateManager&) override {}
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
  void Render(CStateManager&) override {}
  std::optional<zeus::CAABox> GetTouchBounds() const override;
  void SetInUse(bool inUse);
  bool GetInUse(TUniqueId uid) const;
  bool ShouldLandHere() const { return xe8_26_landHere; }
  bool ShouldWallHang() const { return xe8_27_wallHang; }
  bool ShouldStay() const { return xe8_28_stay; }
  bool ShouldCrouch() const { return xf8_24_crouch; }
  bool Blown(const zeus::CVector3f& pos) const;
  float GetSinSqVerticalAngle() const;
  float GetCosHorizontalAngle() const { return xec_cosHorizontalAngle; }
  pas::ECoverDirection GetAttackDirection() const {
    return xe8_30_attackDirection ? pas::ECoverDirection::Right : pas::ECoverDirection::Left;
  }
  void Reserve(TUniqueId id) { xfa_occupant = id; }
};
} // namespace urde
