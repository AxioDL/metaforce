#include "CScriptTrigger.hpp"
#include "Character/CModelData.hpp"
#include "CActorParameters.hpp"
#include "Collision/CMaterialList.hpp"

namespace urde
{

CScriptTrigger::CScriptTrigger(TUniqueId uid, const std::string& name, const CEntityInfo& info,
                               const zeus::CVector3f& pos, const zeus::CAABox&,
                               const CDamageInfo& dInfo, const zeus::CVector3f& orientedForce,
                               u32, bool active, bool, bool)
: CActor(uid, active, name, info, zeus::CTransform::Translate(pos), CModelData::CModelDataNull(),
         CMaterialList(34), CActorParameters::None(), kInvalidUniqueId)
{
}

}
