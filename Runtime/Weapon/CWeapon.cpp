#include "CWeapon.hpp"
#include "World/CActorParameters.hpp"

namespace urde
{

CWeapon::CWeapon(TUniqueId uid, TAreaId aid, bool active, TUniqueId, EWeaponType, const std::string& name,
                 const zeus::CTransform& xf, const CMaterialFilter&, const CMaterialList& mList, const CDamageInfo&,
                 EProjectileAttrib, CModelData&& mData)
: CActor(uid, active, name, CEntityInfo(aid, CEntity::NullConnectionList), xf, std::move(mData), mList,
         CActorParameters::None(), kInvalidUniqueId)
{
}
}
