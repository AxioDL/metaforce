#include "MetroidPrime/ScriptObjects/CScriptDebugCameraWaypoint.hpp"
#include "MetroidPrime/CActorParameters.hpp"

CScriptDebugCameraWaypoint::CScriptDebugCameraWaypoint(TUniqueId uid, const rstl::string& name,
                                                       const CEntityInfo& info,
                                                       const CTransform4f& xf, uint value)
: CActor(uid, true, name, info, xf, CModelData::CModelDataNull(), CMaterialList(kMT_NoStepLogic),
         CActorParameters::None(), kInvalidUniqueId)
, mValue(value) {}

ENTITY_ACCEPT_IMPL(CScriptDebugCameraWaypoint)

CScriptDebugCameraWaypoint::~CScriptDebugCameraWaypoint() {}
