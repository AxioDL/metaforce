#ifndef __URDE_CSCRIPTBEAM_HPP__
#define __URDE_CSCRIPTBEAM_HPP__

#include "CActor.hpp"
#include "Weapon/CBeamInfo.hpp"
#include "CDamageInfo.hpp"

namespace urde
{
class CWeaponDescription;
class CScriptBeam : public CActor
{
    TCachedToken<CWeaponDescription> xe8_weaponDescription;
    CBeamInfo xf4_beamInfo;
    CDamageInfo x138_damageInfo;
    TUniqueId x154_projectileId;
public:
    CScriptBeam(TUniqueId, const std::string&, const CEntityInfo&, const zeus::CTransform&, bool,
                const TToken<CWeaponDescription>&, const CBeamInfo&, const CDamageInfo&);

    void Accept(IVisitor& visitor);
    void Think(float, CStateManager &);
    void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager &);
};
}
#endif // __URDE_CSCRIPTBEAM_HPP__
