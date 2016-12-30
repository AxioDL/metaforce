#include "CScriptTrigger.hpp"
#include "Character/CModelData.hpp"
#include "CActorParameters.hpp"
#include "Collision/CMaterialList.hpp"
#include "CStateManager.hpp"

namespace urde
{

CScriptTrigger::CScriptTrigger(TUniqueId uid, const std::string& name, const CEntityInfo& info,
                               const zeus::CVector3f& pos, const zeus::CAABox& bounds,
                               const CDamageInfo& dInfo, const zeus::CVector3f& forceField,
                               ETriggerFlags triggerFlags, bool active, bool b2, bool b3)
: CActor(uid, active, name, info, zeus::CTransform::Translate(pos), CModelData::CModelDataNull(),
         CMaterialList(EMaterialTypes::Trigger), CActorParameters::None(), kInvalidUniqueId),
  x100_damageInfo(dInfo),
  x11c_forceField(forceField),
  x128_forceMagnitude(forceField.magnitude()),
  x12c_flags(triggerFlags),
  x130_bounds(bounds),
  x148_26_(b2),
  x148_27_(b3)
{
}

CScriptTrigger::CObjectTracker* CScriptTrigger::FindInhabitant(TUniqueId id)
{
    const auto& iter = std::find(xe8_inhabitants.begin(), xe8_inhabitants.end(), id);

    if (iter != xe8_inhabitants.end())
        return &(*iter);
    return nullptr;
}

void CScriptTrigger::UpdateInhabitants(CStateManager& mgr)
{
}

const std::list<CScriptTrigger::CObjectTracker>&CScriptTrigger::GetInhabitants() const
{
    return xe8_inhabitants;
}

rstl::optional_object<zeus::CAABox> CScriptTrigger::GetTouchBounds() const
{
    if (x30_24_active)
        return {GetTriggerBoundsWR()};
    return {};
}

zeus::CAABox CScriptTrigger::GetTriggerBoundsWR() const
{
    return {x130_bounds.min + x34_transform.origin, x130_bounds.max + x34_transform.origin};
}

}
