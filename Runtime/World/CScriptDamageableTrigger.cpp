#include "CScriptDamageableTrigger.hpp"
#include "CActorParameters.hpp"
#include "TCastTo.hpp"

namespace urde
{
CActorParameters MakeDamageableTriggerActorParms(const CActorParameters& aParams, const CVisorParameters& vParams)
{
    CActorParameters ret = aParams;
    ret.SetVisorParameters(vParams);
    return ret;
}

CMaterialList MakeDamageableTriggerMaterial(CScriptDamageableTrigger::ECanOrbit canOrbit)
{
    if (canOrbit == CScriptDamageableTrigger::ECanOrbit::Orbit)
        return CMaterialList(EMaterialTypes::Orbit, EMaterialTypes::Trigger, EMaterialTypes::Immovable,
                             EMaterialTypes::NonSolidDamageable, EMaterialTypes::ExcludeFromLineOfSightTest);
    return CMaterialList(EMaterialTypes::Trigger, EMaterialTypes::Immovable,
                         EMaterialTypes::NonSolidDamageable, EMaterialTypes::ExcludeFromLineOfSightTest);
}


CScriptDamageableTrigger::CScriptDamageableTrigger(TUniqueId uid, const std::string& name, const CEntityInfo& info,
                                                   const zeus::CVector3f& position, const zeus::CVector3f& extent, const CHealthInfo&,
                                                   const CDamageVulnerability&, u32, CAssetId, CAssetId, CAssetId,
                                                   CScriptDamageableTrigger::ECanOrbit canOrbit, bool active, const CVisorParameters& vParams)
    : CActor(uid, active, name, info, zeus::CTransform::Translate(position), CModelData::CModelDataNull(),
             MakeDamageableTriggerMaterial(canOrbit), MakeDamageableTriggerActorParms(CActorParameters::None(), vParams),
             kInvalidUniqueId),
      x14c_bounds(-extent * 0.5f, extent * 0.5f)
{
}

void CScriptDamageableTrigger::Accept(IVisitor& visitor)
{
    visitor.Visit(this);
}

}
