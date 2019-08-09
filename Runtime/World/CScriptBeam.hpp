#pragma once

#include "CActor.hpp"
#include "Weapon/CBeamInfo.hpp"
#include "CDamageInfo.hpp"

namespace urde {
class CWeaponDescription;
class CScriptBeam : public CActor {
  TCachedToken<CWeaponDescription> xe8_weaponDescription;
  CBeamInfo xf4_beamInfo;
  CDamageInfo x138_damageInfo;
  TUniqueId x154_projectileId;

public:
  CScriptBeam(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&, bool,
              const TToken<CWeaponDescription>&, const CBeamInfo&, const CDamageInfo&);

  void Accept(IVisitor& visitor) override;
  void Think(float, CStateManager&) override;
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
};
} // namespace urde
