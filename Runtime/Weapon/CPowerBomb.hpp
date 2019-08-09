#pragma once

#include "CWeapon.hpp"
#include "CToken.hpp"

namespace urde {

class CElementGen;
class CPowerBomb : public CWeapon {
  static const zeus::CColor kFadeColor;
  bool x158_24_canStartFilter : 1;
  bool x158_25_filterEnabled : 1;
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
  void AddToRenderer(const zeus::CFrustum&, const CStateManager&) const override;
  void Render(const CStateManager&) const override {}
  std::optional<zeus::CAABox> GetTouchBounds() const override { return {}; }
  void Touch(CActor&, CStateManager&) override { /*x158_24_canStartFilter; */
  }
  float GetCurTime() const { return x15c_curTime; }
  void ApplyDynamicDamage(const zeus::CVector3f&, CStateManager&);
};

} // namespace urde
