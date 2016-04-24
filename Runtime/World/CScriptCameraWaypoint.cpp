#include "CScriptCameraWaypoint.hpp"
#include "CActorParameters.hpp"

namespace urde
{

CScriptCameraWaypoint::CScriptCameraWaypoint(TUniqueId uid, const std::string& name, const CEntityInfo& info,
                                             const zeus::CTransform& xf, bool active, float, u32)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), CMaterialList(EMaterialTypes::Zero),
         CActorParameters::None(), kInvalidUniqueId)
{

}

}
