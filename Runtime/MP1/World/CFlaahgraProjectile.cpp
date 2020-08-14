#include "Runtime/MP1/World/CFlaahgraProjectile.hpp"

#include "Runtime/CStateManager.hpp"
#include "Runtime/World/CPlayer.hpp"

namespace urde::MP1 {

CFlaahgraProjectile::CFlaahgraProjectile(bool bigStrike, const TToken<CWeaponDescription>& desc,
                                         const zeus::CTransform& xf, const CDamageInfo& damage, TUniqueId uid,
                                         TAreaId aid, TUniqueId owner)
: CEnergyProjectile(true, desc, EWeaponType::AI, xf, EMaterialTypes::Character, damage, uid, aid, owner,
                    kInvalidUniqueId, EProjectileAttrib::BigProjectile, false, zeus::skOne3f, {}, 0xffff, false)
, x3d8_bigStrike(bigStrike) {
  if (x3d8_bigStrike) {
    xe8_projectileAttribs |= EProjectileAttrib::BigStrike;
    x150_damageDuration = 2.f;
  }
}

void CFlaahgraProjectile::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) {
  CEnergyProjectile::AcceptScriptMsg(msg, sender, mgr);
  if (x3d8_bigStrike && msg == EScriptObjectMessage::Deleted && mgr.GetPlayer().GetUniqueId() == x2c2_lastResolvedObj) {
    if (auto* ent = mgr.ObjectById(GetOwnerId())) {
      mgr.SendScriptMsg(ent, GetUniqueId(), EScriptObjectMessage::Action);
    }
  }
}

} // namespace urde::MP1
