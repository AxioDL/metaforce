#include "MetroidPrime/ScriptObjects/CScriptRipple.hpp"

#include "MetroidPrime/CFluidPlaneCPU.hpp"
#include "MetroidPrime/CRipple.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"

CScriptRipple::CScriptRipple(const TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                             const CVector3f& vec, const bool active, const float f1)
: CEntity(uid, info, active, name)
, x34_magnitude(f1 >= 0.f ? f1 : CRipple::kDefaultScale)
, x38_center(vec) {}

CScriptRipple::~CScriptRipple() {}

void CScriptRipple::AcceptScriptMsg(const EScriptObjectMessage msg, const TUniqueId uid, CStateManager& mgr) {
  switch (msg) {
  case (kSM_Play): {
    if (!GetActive()) {
      return;
    }

    rstl::vector< SConnection >::const_iterator conn = GetConnectionList().begin();
    for (; conn != GetConnectionList().end(); ++conn) {
      if (conn->x0_state != kSS_Active || conn->x4_msg != kSM_Next) {
        continue;
      }

      CStateManager::TIdListResult search = mgr.GetIdListForScript(conn->x8_objId);
      if (search.first != search.second) {
        if (CScriptWater* water =
                TCastToPtr< CScriptWater >(mgr.ObjectById(search.first->second))) {
          water->FluidPlane().AddRipple(x34_magnitude, GetUniqueId(), x38_center, *water, mgr);
        }
      }
    }
    break;
  }
  default:
    CEntity::AcceptScriptMsg(msg, uid, mgr);
    break;
  }
}

void CScriptRipple::Think(float, CStateManager&) {}

ENTITY_ACCEPT_IMPL(CScriptRipple)
