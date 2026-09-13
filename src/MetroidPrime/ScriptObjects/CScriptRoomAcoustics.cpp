#include "MetroidPrime/ScriptObjects/CScriptRoomAcoustics.hpp"

#include "Kyoto/Audio/CAudioSys.hpp"

static TAreaId s_ActiveAcousticsAreaId = kInvalidAreaId;

CScriptRoomAcoustics::CScriptRoomAcoustics(
    TUniqueId uid, const rstl::string& name, const CEntityInfo& info, const bool active,
    const uint volScale, const bool revHi, const bool revHiDis, const float revHiTime,
    const float revHiPreDelay, const float revHiDamping, const float revHiColoration,
    const float revHiCrosstalk, const float revHiMix, const bool chorus, const float baseDelay,
    const float variation, const float period, const bool revStd, const bool revStdDis,
    const float revStdTime, const float revStdPreDelay, const float revStdDamping,
    const float revStdColoration, const float revStdMix, const bool delay, const int delayL,
    const int delayR, const int delayS, const int feedbackL, const int feedbackR,
    const int feedbackS, const int outputL, const int outputR, const int outputS)
: CEntity(uid, info, active, name)
, x34_volumeScale(volScale)
, x38_revHi(revHi)
, x39_revHiDis(revHiDis)
, x3c_revHiInfo(revHiTime, revHiPreDelay, revHiDamping, revHiColoration, revHiCrosstalk, revHiMix)
, x54_chorus(chorus)
, x58_chorusInfo(baseDelay, variation, period)
, x64_revStd(revStd)
, x65_revStdDis(revStdDis)
, x68_revStdInfo(revStdTime, revStdPreDelay, revStdDamping, revStdColoration, revStdMix)
, x7c_delay(delay)
, x80_delayInfo(delayL, delayR, delayS, feedbackL, feedbackR, feedbackS, outputL, outputR,
                outputS) {}

ENTITY_ACCEPT_IMPL(CScriptRoomAcoustics)

void CScriptRoomAcoustics::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId,
                                           CStateManager& stateMgr) {
  CEntity::AcceptScriptMsg(msg, objId, stateMgr);

  switch (msg) {
  case kSM_Activate:
    EnableAuxCallbacks();
    break;
  case kSM_Deactivate:
    if (s_ActiveAcousticsAreaId == GetCurrentAreaId()) {
      CSfxManager::DisableAuxProcessing();
      s_ActiveAcousticsAreaId = kInvalidAreaId;
      CAudioSys::SetVolumeScale(CAudioSys::GetDefaultVolumeScale());
    }
    break;
  default:
    break;
  }
}

void CScriptRoomAcoustics::Think(float dt, CStateManager& stateMgr) {
  if (!GetActive()) {
    return;
  }
}

void CScriptRoomAcoustics::EnableAuxCallbacks() {
  if (!GetActive()) {
    return;
  }

  int applied = 0;

  if (x38_revHi && applied <= 0) {
    SND_AUX_REVERBHI reverb;
    reverb.tempDisableFX = x39_revHiDis;
    reverb.time = x3c_revHiInfo.time;
    reverb.preDelay = x3c_revHiInfo.preDelay;
    reverb.damping = x3c_revHiInfo.damping;
    reverb.coloration = x3c_revHiInfo.coloration;
    reverb.crosstalk = x3c_revHiInfo.crosstalk;
    reverb.mix = x3c_revHiInfo.mix;
    applied++;
    CSfxManager::PrepareReverbHiCallback(reverb);
  }
  if (x54_chorus && applied < 1) {
    SND_AUX_CHORUS chorus;
    chorus.baseDelay = x58_chorusInfo.baseDelay;
    chorus.variation = x58_chorusInfo.variation;
    chorus.period = x58_chorusInfo.period;
    applied++;
    CSfxManager::PrepareChorusCallback(chorus);
  }
  if (x64_revStd && applied < 1) {
    SND_AUX_REVERBSTD reverbStd;
    reverbStd.tempDisableFX = x65_revStdDis;
    reverbStd.time = x68_revStdInfo.time;
    reverbStd.preDelay = x68_revStdInfo.preDelay;
    reverbStd.damping = x68_revStdInfo.damping;
    reverbStd.coloration = x68_revStdInfo.coloration;
    reverbStd.mix = x68_revStdInfo.mix;
    applied++;
    CSfxManager::PrepareReverbStdCallback(reverbStd);
  }
  if (x7c_delay && applied < 1) {
    SND_AUX_DELAY delay;
    delay.delay[0] = x80_delayInfo.delayL;
    delay.delay[1] = x80_delayInfo.delayR;
    delay.delay[2] = x80_delayInfo.delayS;
    delay.feedback[0] = x80_delayInfo.feedbackL;
    delay.feedback[1] = x80_delayInfo.feedbackR;
    delay.feedback[2] = x80_delayInfo.feedbackS;
    delay.output[0] = x80_delayInfo.outputL;
    delay.output[1] = x80_delayInfo.outputR;
    delay.output[2] = x80_delayInfo.outputS;
    applied++;
    CSfxManager::PrepareDelayCallback(delay);
  }

  if (applied > 0) {
    CAudioSys::SetVolumeScale(x34_volumeScale);
  }
  s_ActiveAcousticsAreaId = GetCurrentAreaId();
}

void CScriptRoomAcoustics::DisableAuxCallbacks() {
  CSfxManager::DisableAuxProcessing();
  s_ActiveAcousticsAreaId = kInvalidAreaId;
  CAudioSys::SetVolumeScale(CAudioSys::GetDefaultVolumeScale());
}

CScriptRoomAcoustics::~CScriptRoomAcoustics() {}
