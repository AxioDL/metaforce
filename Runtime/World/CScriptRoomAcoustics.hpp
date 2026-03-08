#pragma once

#include <string_view>

#include "Runtime/RetroTypes.hpp"
#include "Runtime/World/CEntity.hpp"

#include <musyx/musyx.h>

namespace metaforce {

class CScriptRoomAcoustics : public CEntity {
  u32 x34_volumeScale;

  bool x38_revHi;
  bool x39_revHiDis;
  SND_AUX_REVERBHI x3c_revHiInfo{};

  bool x54_chorus;
  SND_AUX_CHORUS x58_chorusInfo{};

  bool x64_revStd;
  bool x65_revStdDis;
  SND_AUX_REVERBSTD x68_revStdInfo{};

  bool x7c_delay;
  SND_AUX_DELAY x80_delayInfo{};

public:
  DEFINE_ENTITY
  CScriptRoomAcoustics(TUniqueId uid, std::string_view name, const CEntityInfo& info, bool active, u32 volScale,
                       bool revHi, bool revHiDis, float revHiColoration, float revHiMix, float revHiTime,
                       float revHiDamping, float revHiPreDelay, float revHiCrosstalk, bool chorus, float baseDelay,
                       float variation, float period, bool revStd, bool revStdDis, float revStdColoration,
                       float revStdMix, float revStdTime, float revStdDamping, float revStdPreDelay, bool delay,
                       u32 delayL, u32 delayR, u32 delayS, u32 feedbackL, u32 feedbackR, u32 feedbackS, u32 outputL,
                       u32 outputR, u32 outputS);
  void Think(float dt, CStateManager& stateMgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr) override;
  void Accept(IVisitor& visitor) override;
  void EnableAuxCallbacks();

  static void DisableAuxCallbacks();
};

} // namespace metaforce
