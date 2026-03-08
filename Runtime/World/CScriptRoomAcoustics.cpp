#include "Runtime/World/CScriptRoomAcoustics.hpp"

#include "Runtime/Audio/CSfxManager.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace metaforce {

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
, x54_chorus(chorus)
, x64_revStd(revStd)
, x65_revStdDis(revStdDis)
, x7c_delay(delay) {
  x3c_revHiInfo.coloration = revHiColoration;
  x3c_revHiInfo.mix = revHiMix;
  x3c_revHiInfo.time = revHiTime;
  x3c_revHiInfo.damping = revHiDamping;
  x3c_revHiInfo.preDelay = revHiPreDelay;
  x3c_revHiInfo.crosstalk = revHiCrosstalk;

  x58_chorusInfo.baseDelay = static_cast<u32>(baseDelay);
  x58_chorusInfo.variation = static_cast<u32>(variation);
  x58_chorusInfo.period = static_cast<u32>(period);

  x68_revStdInfo.coloration = revStdColoration;
  x68_revStdInfo.mix = revStdMix;
  x68_revStdInfo.time = revStdTime;
  x68_revStdInfo.damping = revStdDamping;
  x68_revStdInfo.preDelay = revStdPreDelay;

  x80_delayInfo.delay[0] = delayL;
  x80_delayInfo.delay[1] = delayR;
  x80_delayInfo.delay[2] = delayS;
  x80_delayInfo.feedback[0] = feedbackL;
  x80_delayInfo.feedback[1] = feedbackR;
  x80_delayInfo.feedback[2] = feedbackS;
  x80_delayInfo.output[0] = outputL;
  x80_delayInfo.output[1] = outputR;
  x80_delayInfo.output[2] = outputS;
}

void CScriptRoomAcoustics::DisableAuxCallbacks() {
  CSfxManager::DisableAuxProcessing();
  s_ActiveAcousticsAreaId = kInvalidAreaId;
  CAudioSys::SetVolumeScale(CAudioSys::GetDefaultVolumeScale());
}

void CScriptRoomAcoustics::EnableAuxCallbacks() {
  if (!x30_24_active) {
    return;
  }

  bool applied = true;
  if (x38_revHi) {
    CSfxManager::PrepareReverbHiCallback(x3c_revHiInfo);
  } else if (x54_chorus) {
    CSfxManager::PrepareChorusCallback(x58_chorusInfo);
  } else if (x64_revStd) {
    CSfxManager::PrepareReverbStdCallback(x68_revStdInfo);
  } else if (x7c_delay) {
    CSfxManager::PrepareDelayCallback(x80_delayInfo);
  } else {
    applied = false;
  }

  if (applied) {
    CAudioSys::SetVolumeScale(x34_volumeScale);
  }
  s_ActiveAcousticsAreaId = x4_areaId;
}

void CScriptRoomAcoustics::Think(float dt, CStateManager& stateMgr) { /* Intentionally empty */
}

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

} // namespace metaforce
