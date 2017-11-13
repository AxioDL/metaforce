#include "CScriptWaypoint.hpp"
#include "Character/CModelData.hpp"
#include "Collision/CMaterialList.hpp"
#include "CActorParameters.hpp"
#include "TCastTo.hpp"

namespace urde
{

CScriptWaypoint::CScriptWaypoint(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                                 const zeus::CTransform& xf, bool active, float, float,
                                 u32, u32, u32, u32, u32, u32, u32)
: CActor(uid, active, name, info, xf, CModelData(), CMaterialList(),
         CActorParameters::None(), kInvalidUniqueId)
{
}

void CScriptWaypoint::Accept(IVisitor& visitor)
{
    visitor.Visit(this);
}

}
