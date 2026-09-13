#ifndef _CSCRIPTBEAM
#define _CSCRIPTBEAM

#include "MetroidPrime/CActor.hpp"

#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/Weapons/CBeamInfo.hpp"

class CWeaponDescription;
class CScriptBeam : public CActor {
  TLockedToken< CWeaponDescription > xe8_weaponDescription;
  CBeamInfo xf4_beamInfo;
  CDamageInfo x138_damageInfo;
  TUniqueId x154_projectileId;

public:
  CScriptBeam(TUniqueId, const rstl::string&, const CEntityInfo&, const CTransform4f&, bool,
              const TToken< CWeaponDescription >&, const CBeamInfo&, const CDamageInfo&);

  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void Think(float, CStateManager&) override;
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
};

#endif // _CSCRIPTBEAM
