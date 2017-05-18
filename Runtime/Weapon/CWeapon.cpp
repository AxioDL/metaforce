#include "CWeapon.hpp"
#include "World/CActorParameters.hpp"
#include "TCastTo.hpp"

namespace urde
{

CWeapon::CWeapon(TUniqueId uid, TAreaId aid, bool active, TUniqueId, EWeaponType, const std::string& name,
                 const zeus::CTransform& xf, const CMaterialFilter& filter, const CMaterialList& mList, const CDamageInfo&,
                 EProjectileAttrib, CModelData&& mData)
: CActor(uid, active, name, CEntityInfo(aid, CEntity::NullConnectionList), xf, std::move(mData), mList,
         CActorParameters::None(), kInvalidUniqueId)
, xf8_(filter)
{
}

void CWeapon::Accept(urde::IVisitor& visitor)
{
    visitor.Visit(this);
}

}
