#include "CScriptRoomAcoustics.hpp"
#include "TCastTo.hpp"
#include "Audio/CSfxManager.hpp"

namespace urde {

static TAreaId s_ActiveAcousticsAreaId = kInvalidAreaId;

CScriptRoomAcoustics::CScriptRoomAcoustics(TUniqueId uid, std::string_view name, const CEntityInfo& info, bool active,
                                           u32 volScale, bool revHi, bool revHiDis, float revHiColoration,
                                           float revHiMix, float revHiTime, float revHiDamping, float revHiPreDelay,
                                           float revHiCrosstalk, bool chorus, float baseDelay, float variation,
                                           float period, bool revStd, bool revStdDis, float revStdColoration,
                                           float revStdMix, float revStdTime, float revStdDamping, float revStdPreDelay,
                                           bool delay, u32 delayL, u32 delayR, u32 delayS, u32 feedbackL, u32 feedbackR,
                                           u32 feedbackS, u32 outputL, u32 outputR, u32 outputS)
: CEntity(uid, info, active, name)
, x34_volumeScale(volScale)
, x38_revHi(revHi)
, x39_revHiDis(revHiDis)
, x3c_revHiInfo(revHiColoration, revHiMix, revHiTime, revHiDamping, revHiPreDelay, revHiCrosstalk)
, x54_chorus(chorus)
, x58_chorusInfo(baseDelay, variation, period)
, x64_revStd(revStd)
, x65_revStdDis(revStdDis)
, x68_revStdInfo(revStdColoration, revStdMix, revStdTime, revStdDamping, revStdPreDelay)
, x7c_delay(delay)
, x80_delayInfo(delayL, delayR, delayS, feedbackL, feedbackR, feedbackS, outputL, outputR, outputS) {}

void CScriptRoomAcoustics::DisableAuxCallbacks() {
  CSfxManager::DisableAuxProcessing();
  s_ActiveAcousticsAreaId = kInvalidAreaId;
  CAudioSys::SetVolumeScale(CAudioSys::GetDefaultVolumeScale());
}

void CScriptRoomAcoustics::EnableAuxCallbacks() {
  if (!x30_24_active)
    return;

  bool applied = true;
  if (x38_revHi)
    CSfxManager::PrepareReverbHiCallback(x3c_revHiInfo);
  else if (x54_chorus)
    CSfxManager::PrepareChorusCallback(x58_chorusInfo);
  else if (x64_revStd)
    CSfxManager::PrepareReverbStdCallback(x68_revStdInfo);
  else if (x7c_delay)
    CSfxManager::PrepareDelayCallback(x80_delayInfo);
  else
    applied = false;

  if (applied)
    CAudioSys::SetVolumeScale(x34_volumeScale);
  s_ActiveAcousticsAreaId = x4_areaId;
}

void CScriptRoomAcoustics::Think(float dt, CStateManager& stateMgr) { /* Intentionally empty */ }

void CScriptRoomAcoustics::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr) {
  CEntity::AcceptScriptMsg(msg, objId, stateMgr);

  switch (msg) {
  case EScriptObjectMessage::Activate:
    EnableAuxCallbacks();
    break;
  case EScriptObjectMessage::Deactivate:
    if (s_ActiveAcousticsAreaId == x4_areaId) {
      s_ActiveAcousticsAreaId = kInvalidAreaId;
      CSfxManager::DisableAuxProcessing();
      CAudioSys::SetVolumeScale(CAudioSys::GetDefaultVolumeScale());
    }
    break;
  default:
    break;
  }
}

void CScriptRoomAcoustics::Accept(IVisitor& visitor) { visitor.Visit(this); }

} // namespace urde
