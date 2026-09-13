#ifndef _CSCRIPTSOUND
#define _CSCRIPTSOUND

#include <MetroidPrime/CActor.hpp>

class CScriptSound : public CActor {
public:
  CScriptSound(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
               const CTransform4f& xf, ushort soundId, bool active, float maxDist, float distComp,
               float startDelay, uint minVol, uint vol, uint w3, uint prio, uint pan, uint w6,
               bool looped, bool nonEmitter, bool autoStart, bool occlusionTest, bool acoustics,
               bool worldSfx, bool allowDuplicates, int pitch);
  ~CScriptSound();

  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
  void PreThink(float dt, CStateManager& mgr) override;
  void Think(float dt, CStateManager&) override;
  void AddToRenderer(const CFrustumPlanes& planes, const CStateManager& mgr) const override;
  DECLARE_TYPES_MATCH_OR_ACCEPT;

  void PlaySound(CStateManager& mgr);
  void StopSound(CStateManager& mgr);

  static float GetOccludedVolumeAmount(const CVector3f& pos, const CStateManager& mgr);

  ushort GetSoundId() const { return x100_soundId; }

private:
  static bool sFirstInFrame;

  float xe8_occUpdateTimer;
  CSfxHandle xec_sfxHandle;
  short xf0_maxVol;
  short xf2_maxVolUpd;
  short xf4_maxVolUpdDelta;
  float xf8_updateTimer;
  float xfc_startDelay;
  ushort x100_soundId;
  float x104_maxDist;
  float x108_distComp;
  short x10c_minVol;
  short x10e_vol;
  short x110_;
  short x112_prio;
  short x114_pan;
  short x116_;
  int x118_pitch;
  bool x11c_24_playRequested : 1;
  bool x11c_25_looped : 1;
  bool x11c_26_nonEmitter : 1;
  bool x11c_27_autoStart : 1;
  bool x11c_28_occlusionTest : 1;
  bool x11c_29_acoustics : 1;
  bool x11c_30_worldSfx : 1;
  bool x11c_31_selfFree : 1;
  bool x11d_24_allowDuplicates : 1;
  bool x11d_25_processedThisFrame : 1;
};

CHECK_SIZEOF(CScriptSound, 0x120)

#endif // _CSCRIPTSOUND
