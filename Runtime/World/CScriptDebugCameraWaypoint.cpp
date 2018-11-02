#include "CScriptDebugCameraWaypoint.hpp"
#include "CActorParameters.hpp"
#include "TCastTo.hpp"

namespace urde
{

CScriptDebugCameraWaypoint::CScriptDebugCameraWaypoint(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                                                       const zeus::CTransform& xf, u32 w1)
: CActor(uid, true, name, info, xf, CModelData::CModelDataNull(), {EMaterialTypes::NoStepLogic},
         CActorParameters::None(), kInvalidUniqueId), xe8_w1(w1)
{
}

void CScriptDebugCameraWaypoint::Accept(IVisitor& visitor)
{
    visitor.Visit(this);
}

}
