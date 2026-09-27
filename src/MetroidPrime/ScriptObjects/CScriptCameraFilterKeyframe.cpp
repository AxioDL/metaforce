#include "MetroidPrime/ScriptObjects/CScriptCameraFilterKeyframe.hpp"

#include "MetroidPrime/CStateManager.hpp"

CScriptCameraFilterKeyframe::CScriptCameraFilterKeyframe(
    TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
    CCameraFilterPass::EFilterType type, CCameraFilterPass::EFilterShape shape,
    CStateManager::ECameraFilterStage filterIdx, uint unk, float colorR, float colorG, float colorB,
    float colorA, float timeIn, float timeOut, CAssetId txtr, bool active)
: CEntity(uid, info, active, name)
, mType(type)
, mShape(shape)
, mFilterIdx(filterIdx)
, x40_(unk)
, mColor(colorR, colorG, colorB, colorA)
, mTimeIn(timeIn)
, mTimeOut(timeOut)
, mTxtr(txtr) {}

CScriptCameraFilterKeyframe::~CScriptCameraFilterKeyframe() {}

void CScriptCameraFilterKeyframe::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId,
                                                  CStateManager& stateMgr) {
  switch (msg) {
  case kSM_Increment:
    if (GetActive()) {
      stateMgr.CameraFilterPass(mFilterIdx)
          .SetFilter(mType, mShape, mTimeIn, mColor, mTxtr);
    }
    break;
  case kSM_Decrement:
    if (GetActive()) {
      stateMgr.CameraFilterPass(mFilterIdx).DisableFilter(mTimeOut);
    }
    break;
  case kSM_Deactivate:
    if (GetActive()) {
      stateMgr.CameraFilterPass(mFilterIdx).DisableFilter(0.f);
    }
    break;
  default:
    break;
  }

  CEntity::AcceptScriptMsg(msg, objId, stateMgr);
}

ENTITY_ACCEPT_IMPL(CScriptCameraFilterKeyframe)
