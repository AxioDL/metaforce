#include "CScriptWaypoint.hpp"
#include "Character/CModelData.hpp"
#include "Collision/CMaterialList.hpp"
#include "CActorParameters.hpp"

namespace urde
{

CScriptWaypoint::CScriptWaypoint(TUniqueId uid, const std::string& name, const CEntityInfo& info,
                                 const zeus::CTransform& xf, bool active, float, float,
                                 u32, u32, u32, u32, u32, u32, u32)
: CActor(uid, active, name, info, xf, CModelData(), CMaterialList(0),
         CActorParameters::None(), kInvalidUniqueId)
{
}

}
