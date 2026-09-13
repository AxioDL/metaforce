#ifndef _CSCRIPTROOMACOUSTICS
#define _CSCRIPTROOMACOUSTICS

#include "MetroidPrime/CEntity.hpp"

#include "Kyoto/Audio/CSfxManager.hpp"

struct EffectReverbHiInfo {
  float time;       /**< [0.01, 10.0] time in seconds for reflection decay */
  float preDelay;   /**< [0.0, 0.1] time in seconds before initial reflection heard */
  float damping;    /**< [0.0, 1.0] damping factor influencing low-pass filter of reflections */
  float coloration; /**< [0.0, 1.0] influences filter coefficients to define surface characteristics
                       of a room */
  float crosstalk;  /**< [0.0, 1.0] factor defining how much reflections are allowed to bleed to
                       other channels */
  float mix;        /**< [0.0, 1.0] dry/wet mix factor of reverb effect */

  EffectReverbHiInfo(float time, float preDelay, float damping, float coloration, float crosstalk,
                     float mix)
  : time(time)
  , preDelay(preDelay)
  , damping(damping)
  , coloration(coloration)
  , crosstalk(crosstalk)
  , mix(mix) {}
};
struct EffectChorusInfo {
  float baseDelay;
  float variation;
  float period;

  EffectChorusInfo(float baseDelay, float variation, float period)
  : baseDelay(baseDelay), variation(variation), period(period) {}
};
struct EffectReverbStdInfo {
  float time;       /**< [0.01, 10.0] time in seconds for reflection decay */
  float preDelay;   /**< [0.0, 0.1] time in seconds before initial reflection heard */
  float damping;    /**< [0.0, 1.0] damping factor influencing low-pass filter of reflections */
  float coloration; /**< [0.0, 1.0] influences filter coefficients to define surface characteristics
                       of a room */
  float mix;        /**< [0.0, 1.0] dry/wet mix factor of reverb effect */

  EffectReverbStdInfo(float time, float preDelay, float damping, float coloration, float mix)
  : time(time), preDelay(preDelay), damping(damping), coloration(coloration), mix(mix) {}
};
struct EffectDelayInfo {
  int delayL;
  int delayR;
  int delayS;
  int feedbackL;
  int feedbackR;
  int feedbackS;
  int outputL;
  int outputR;
  int outputS;

  EffectDelayInfo(int delayL, int delayR, int delayS, int feedbackL, int feedbackR, int feedbackS,
                  int outputL, int outputR, int outputS)
  : delayL(delayL)
  , delayR(delayR)
  , delayS(delayS)
  , feedbackL(feedbackL)
  , feedbackR(feedbackR)
  , feedbackS(feedbackS)
  , outputL(outputL)
  , outputR(outputR)
  , outputS(outputS) {}
};

class CScriptRoomAcoustics : public CEntity {
public:
  CScriptRoomAcoustics(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                       bool active, uint volScale, bool revHi, bool revHiDis, float revHiColoration,
                       float revHiMix, float revHiTime, float revHiDamping, float revHiPreDelay,
                       float revHiCrosstalk, bool chorus, float baseDelay, float variation,
                       float period, bool revStd, bool revStdDis, float revStdColoration,
                       float revStdMix, float revStdTime, float revStdDamping, float revStdPreDelay,
                       bool delay, int delayL, int delayR, int delayS, int feedbackL, int feedbackR,
                       int feedbackS, int outputL, int outputR, int outputS);
  virtual ~CScriptRoomAcoustics();

  void Think(float dt, CStateManager& stateMgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr) override;
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void EnableAuxCallbacks();

  static void DisableAuxCallbacks();

private:
  uint x34_volumeScale;

  bool x38_revHi, x39_revHiDis;
  EffectReverbHiInfo x3c_revHiInfo;

  bool x54_chorus;
  EffectChorusInfo x58_chorusInfo;

  bool x64_revStd, x65_revStdDis;
  EffectReverbStdInfo x68_revStdInfo;

  bool x7c_delay;
  EffectDelayInfo x80_delayInfo;
};

#endif // _CSCRIPTROOMACOUSTICS
