#pragma once

#include <memory>
#include <optional>

#include "Runtime/CToken.hpp"
#include "Runtime/Weapon/CWeapon.hpp"

#include <zeus/CAABox.hpp>

namespace urde {
class CElementGen;

class CPowerBomb : public CWeapon {
  bool x158_24_canStartFilter : 1 = true;
  bool x158_25_filterEnabled : 1 = false;
  float x15c_curTime = 0.f;
  float x160_curRadius = 0.f;
  float x164_radiusIncrement;
  std::unique_ptr<CElementGen> x168_particle;
  float x16c_radius;

public:
  CPowerBomb(const TToken<CGenDescription>& particle, TUniqueId uid, TAreaId aid, TUniqueId playerId,
             const zeus::CTransform& xf, const CDamageInfo& dInfo);

  void Accept(IVisitor& visitor) override;
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
  void Think(float, CStateManager&) override;
  void AddToRenderer(const zeus::CFrustum&, CStateManager&) override;
  void Render(CStateManager&) override {}
  std::optional<zeus::CAABox> GetTouchBounds() const override { return std::nullopt; }
  void Touch(CActor&, CStateManager&) override { /*x158_24_canStartFilter; */
  }
  float GetCurTime() const { return x15c_curTime; }
  void ApplyDynamicDamage(const zeus::CVector3f&, CStateManager&);
};

} // namespace urde
