#pragma once

#include "CWeapon.hpp"
#include "CToken.hpp"

namespace urde {

class CElementGen;
class CBomb : public CWeapon {

  zeus::CVector3f x158_velocity;
  zeus::CVector3f x164_acceleration;
  zeus::CVector3f x170_prevLocation;
  float x17c_fuseTime;
  std::unique_ptr<CElementGen> x180_particle1;
  std::unique_ptr<CElementGen> x184_particle2;
  TUniqueId x188_lightId = kInvalidUniqueId;
  const CGenDescription* x18c_particle2Obj;
  bool x190_24_isNotDetonated : 1;
  bool x190_25_beingDragged : 1;
  bool x190_26_disableFuse : 1;

public:
  CBomb(const TCachedToken<CGenDescription>& particle1, const TCachedToken<CGenDescription>& particle2, TUniqueId uid,
        TAreaId aid, TUniqueId playerId, float f1, const zeus::CTransform& xf, const CDamageInfo& dInfo);

  void Accept(IVisitor&);
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);
  void Think(float, CStateManager&);
  void AddToRenderer(const zeus::CFrustum&, const CStateManager&) const;
  void Render(const CStateManager&) const {}
  void Touch(CActor&, CStateManager&);
  void Explode(const zeus::CVector3f&, CStateManager&);
  void UpdateLight(float, CStateManager&);
  std::optional<zeus::CAABox> GetTouchBounds() const;
  void SetVelocityWR(const zeus::CVector3f& vel) { x158_velocity = vel; }
  void SetConstantAccelerationWR(const zeus::CVector3f& acc) { x164_acceleration = acc; }
  void SetFuseDisabled(bool b) { x190_26_disableFuse = false; }
  void SetIsBeingDragged(bool b) { x190_25_beingDragged = b; }
  bool IsBeingDragged() const { return x190_25_beingDragged; }
};

} // namespace urde
