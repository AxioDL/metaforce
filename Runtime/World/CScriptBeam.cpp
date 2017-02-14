#include "CScriptBeam.hpp"
#include "CActorParameters.hpp"
#include "Particle/CWeaponDescription.hpp"
#include "Weapon/CPlasmaProjectile.hpp"
#include "CStateManager.hpp"
#include "TCastTo.hpp"

namespace urde
{

CScriptBeam::CScriptBeam(TUniqueId uid, const std::string& name, const CEntityInfo& info, const zeus::CTransform& xf,
                         bool active, const TToken<CWeaponDescription>& weaponDesc, const CBeamInfo& bInfo,
                         const CDamageInfo& dInfo)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), CMaterialList(), CActorParameters::None(),
         kInvalidUniqueId)
, xe8_weaponDescription(weaponDesc)
, xf4_beamInfo(bInfo)
, x138_damageInfo(dInfo)
{
}

void CScriptBeam::Accept(IVisitor& visitor)
{
    visitor.Visit(this);
}

void CScriptBeam::Think(float dt, CStateManager& mgr)
{
#if 0
    TCastToPtr<CGameProjectile> proj{mgr.GetObjectById(x154_projectileId)};
    if (proj)
    {
        if (proj->GetActive())
            proj->UpdateFx(x34_transform, dt, mgr);
    }
    else
        x154_projectileId = kInvalidUniqueId;
#endif
}

void CScriptBeam::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& mgr)
{
    if (msg == EScriptObjectMessage::Increment)
    {
    }
    else if (msg == EScriptObjectMessage::Decrement)
    {
    }
    else if (msg == EScriptObjectMessage::InternalMessage11)
    {
        x154_projectileId = mgr.AllocateUniqueId();
        mgr.AddObject(new CPlasmaProjectile(xe8_weaponDescription, x10_name + "-Projectile",
                                            x138_damageInfo.GetWeaponMode().GetType(), xf4_beamInfo, x34_transform,
                                            EMaterialTypes::Projectile, x138_damageInfo, x8_uid, x4_areaId,
                                            x154_projectileId, 8, false, 2));
    }
    else if (msg == EScriptObjectMessage::Deleted)
    {
    }

    CActor::AcceptScriptMsg(msg, objId, mgr);
}
}
