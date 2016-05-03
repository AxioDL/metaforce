#include "CScriptDamageableTrigger.hpp"
#include "CActorParameters.hpp"

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
        return CMaterialList(EMaterialTypes::FourtyOne, EMaterialTypes::ThirtyFour, EMaterialTypes::FourtyThree,
                             EMaterialTypes::Fifty, EMaterialTypes::FiftySix);
    return CMaterialList(EMaterialTypes::ThirtyFour, EMaterialTypes::FourtyThree,
                         EMaterialTypes::Fifty, EMaterialTypes::FiftySix);
}


CScriptDamageableTrigger::CScriptDamageableTrigger(TUniqueId uid, const std::string& name, const CEntityInfo& info,
                                                   const zeus::CVector3f& position, const zeus::CVector3f& extent, const CHealthInfo&,
                                                   const CDamageVulnerability&, u32, ResId, ResId, ResId,
                                                   CScriptDamageableTrigger::ECanOrbit canOrbit, bool active, const CVisorParameters& vParams)
    : CActor(uid, active, name, info, zeus::CTransform::Translate(position), CModelData::CModelDataNull(),
             MakeDamageableTriggerMaterial(canOrbit), MakeDamageableTriggerActorParms(CActorParameters::None(), vParams),
             kInvalidUniqueId),
      x14c_bounds(-extent * 0.5f, extent * 0.5f)
{
}

}
