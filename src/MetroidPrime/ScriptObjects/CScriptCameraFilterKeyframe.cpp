#include "MetroidPrime/ScriptObjects/CScriptCameraFilterKeyframe.hpp"

#include "MetroidPrime/CStateManager.hpp"

CScriptCameraFilterKeyframe::CScriptCameraFilterKeyframe(
    TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
    CCameraFilterPass::EFilterType type, CCameraFilterPass::EFilterShape shape,
    CStateManager::ECameraFilterStage filterIdx, uint unk, float colorR, float colorG, float colorB,
    float colorA, float timeIn, float timeOut, CAssetId txtr, bool active)
: CEntity(uid, info, active, name)
, x34_type(type)
, x38_shape(shape)
, x3c_filterIdx(filterIdx)
, x40_(unk)
, x44_color(colorR, colorG, colorB, colorA)
, x48_timeIn(timeIn)
, x4c_timeOut(timeOut)
, x50_txtr(txtr) {}

CScriptCameraFilterKeyframe::~CScriptCameraFilterKeyframe() {}

void CScriptCameraFilterKeyframe::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId,
                                                  CStateManager& stateMgr) {
  switch (msg) {
  case kSM_Increment:
    if (GetActive()) {
      stateMgr.CameraFilterPass(x3c_filterIdx)
          .SetFilter(x34_type, x38_shape, x48_timeIn, x44_color, x50_txtr);
    }
    break;
  case kSM_Decrement:
    if (GetActive()) {
      stateMgr.CameraFilterPass(x3c_filterIdx).DisableFilter(x4c_timeOut);
    }
    break;
  case kSM_Deactivate:
    if (GetActive()) {
      stateMgr.CameraFilterPass(x3c_filterIdx).DisableFilter(0.f);
    }
    break;
  default:
    break;
  }

  CEntity::AcceptScriptMsg(msg, objId, stateMgr);
}

ENTITY_ACCEPT_IMPL(CScriptCameraFilterKeyframe)
