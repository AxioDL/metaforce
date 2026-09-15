#include "MetroidPrime/ScriptObjects/CScriptCameraShaker.hpp"

#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"

CScriptCameraShaker::CScriptCameraShaker(TUniqueId uid, const rstl::string& name,
                                         const CEntityInfo& info, const bool active,
                                         const CCameraShakeData& shakeData)
: CEntity(uid, info, active, name), x34_shakeData(shakeData) {}

void CScriptCameraShaker::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId,
                                          CStateManager& stateMgr) {
  switch (msg) {
  case kSM_Action: {
    if (GetActive() && GetCurrentAreaId() != kInvalidAreaId) {
      const CGameArea& area = stateMgr.GetWorld()->GetAreaAlways(GetCurrentAreaId());
      if (area.GetOcclusionState() != CGameArea::kOS_Occluded) {
        x34_shakeData.SetId(stateMgr.CameraManager()->AddCameraShaker(x34_shakeData, false));
      }
    }
    break;
  }
  case kSM_Deactivate: {
    if (GetActive())
      stateMgr.CameraManager()->RemoveCameraShaker(x34_shakeData.GetId());
    break;
  }
  default:
    break;
  }
  CEntity::AcceptScriptMsg(msg, objId, stateMgr);
}

ENTITY_ACCEPT_IMPL(CScriptCameraShaker)
