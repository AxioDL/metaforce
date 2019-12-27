#include "Runtime/World/CScriptCameraFilterKeyframe.hpp"

#include "Runtime/CStateManager.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde {
CScriptCameraFilterKeyframe::CScriptCameraFilterKeyframe(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                                                         EFilterType type, EFilterShape shape, u32 filterIdx, u32 unk,
                                                         const zeus::CColor& color, float timeIn, float timeOut,
                                                         CAssetId txtr, bool active)
: CEntity(uid, info, active, name)
, x34_type(type)
, x38_shape(shape)
, x3c_filterIdx(filterIdx)
, x40_(unk)
, x44_color(color)
, x48_timeIn(timeIn)
, x4c_timeOut(timeOut)
, x50_txtr(txtr) {}

void CScriptCameraFilterKeyframe::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr) {
  switch (msg) {
  case EScriptObjectMessage::Increment:
    if (GetActive())
      stateMgr.GetCameraFilterPass(x3c_filterIdx).SetFilter(x34_type, x38_shape, x48_timeIn, x44_color, x50_txtr);
    break;
  case EScriptObjectMessage::Decrement:
    if (GetActive())
      stateMgr.GetCameraFilterPass(x3c_filterIdx).DisableFilter(x4c_timeOut);
    break;
  case EScriptObjectMessage::Deactivate:
    if (GetActive())
      stateMgr.GetCameraFilterPass(x3c_filterIdx).DisableFilter(0.f);
    break;
  default:
    break;
  }

  CEntity::AcceptScriptMsg(msg, objId, stateMgr);
}

void CScriptCameraFilterKeyframe::Accept(IVisitor& visitor) { visitor.Visit(this); }

} // namespace urde
