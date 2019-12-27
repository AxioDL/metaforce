#include "Runtime/World/CScriptDebugCameraWaypoint.hpp"

#include "Runtime/World/CActorParameters.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde {

CScriptDebugCameraWaypoint::CScriptDebugCameraWaypoint(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                                                       const zeus::CTransform& xf, u32 w1)
: CActor(uid, true, name, info, xf, CModelData::CModelDataNull(), {EMaterialTypes::NoStepLogic},
         CActorParameters::None(), kInvalidUniqueId)
, xe8_w1(w1) {}

void CScriptDebugCameraWaypoint::Accept(IVisitor& visitor) { visitor.Visit(this); }

} // namespace urde
