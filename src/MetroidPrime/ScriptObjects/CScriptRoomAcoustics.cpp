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
, mVolumeScale(volScale)
, mRevHi(revHi)
, mRevHiDis(revHiDis)
, mRevHiInfo(revHiTime, revHiPreDelay, revHiDamping, revHiColoration, revHiCrosstalk, revHiMix)
, mChorus(chorus)
, mChorusInfo(baseDelay, variation, period)
, mRevStd(revStd)
, mRevStdDis(revStdDis)
, mRevStdInfo(revStdTime, revStdPreDelay, revStdDamping, revStdColoration, revStdMix)
, mDelay(delay)
, mDelayInfo(delayL, delayR, delayS, feedbackL, feedbackR, feedbackS, outputL, outputR,
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

  if (mRevHi && applied <= 0) {
    SND_AUX_REVERBHI reverb;
    reverb.tempDisableFX = mRevHiDis;
    reverb.time = mRevHiInfo.time;
    reverb.preDelay = mRevHiInfo.preDelay;
    reverb.damping = mRevHiInfo.damping;
    reverb.coloration = mRevHiInfo.coloration;
    reverb.crosstalk = mRevHiInfo.crosstalk;
    reverb.mix = mRevHiInfo.mix;
    applied++;
    CSfxManager::PrepareReverbHiCallback(reverb);
  }
  if (mChorus && applied < 1) {
    SND_AUX_CHORUS chorus;
    chorus.baseDelay = mChorusInfo.baseDelay;
    chorus.variation = mChorusInfo.variation;
    chorus.period = mChorusInfo.period;
    applied++;
    CSfxManager::PrepareChorusCallback(chorus);
  }
  if (mRevStd && applied < 1) {
    SND_AUX_REVERBSTD reverbStd;
    reverbStd.tempDisableFX = mRevStdDis;
    reverbStd.time = mRevStdInfo.time;
    reverbStd.preDelay = mRevStdInfo.preDelay;
    reverbStd.damping = mRevStdInfo.damping;
    reverbStd.coloration = mRevStdInfo.coloration;
    reverbStd.mix = mRevStdInfo.mix;
    applied++;
    CSfxManager::PrepareReverbStdCallback(reverbStd);
  }
  if (mDelay && applied < 1) {
    SND_AUX_DELAY delay;
    delay.delay[0] = mDelayInfo.delayL;
    delay.delay[1] = mDelayInfo.delayR;
    delay.delay[2] = mDelayInfo.delayS;
    delay.feedback[0] = mDelayInfo.feedbackL;
    delay.feedback[1] = mDelayInfo.feedbackR;
    delay.feedback[2] = mDelayInfo.feedbackS;
    delay.output[0] = mDelayInfo.outputL;
    delay.output[1] = mDelayInfo.outputR;
    delay.output[2] = mDelayInfo.outputS;
    applied++;
    CSfxManager::PrepareDelayCallback(delay);
  }

  if (applied > 0) {
    CAudioSys::SetVolumeScale(mVolumeScale);
  }
  s_ActiveAcousticsAreaId = GetCurrentAreaId();
}

void CScriptRoomAcoustics::DisableAuxCallbacks() {
  CSfxManager::DisableAuxProcessing();
  s_ActiveAcousticsAreaId = kInvalidAreaId;
  CAudioSys::SetVolumeScale(CAudioSys::GetDefaultVolumeScale());
}
