#pragma once

#include "Runtime/Weapon/CEnergyProjectile.hpp"

namespace metaforce::MP1 {

class CFlaahgraProjectile : public CEnergyProjectile {
  bool x3d8_bigStrike;

public:
  DEFINE_ENTITY
  CFlaahgraProjectile(bool bigStrike, const TToken<CWeaponDescription>& desc, const zeus::CTransform& xf,
                      const CDamageInfo& damage, TUniqueId uid, TAreaId aid, TUniqueId owner);

  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) override;
};

} // namespace metaforce::MP1
