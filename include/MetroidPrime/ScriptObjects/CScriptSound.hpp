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

  ushort GetSoundId() const { return mSoundId; }

private:
  static bool sFirstInFrame;

  float mOccUpdateTimer;
  CSfxHandle mSfxHandle;
  short mMaxVol;
  short mMaxVolUpd;
  short mMaxVolUpdDelta;
  float mUpdateTimer;
  float mStartDelay;
  ushort mSoundId;
  float mMaxDist;
  float mDistComp;
  short mMinVol;
  short mVol;
  short x110_;
  short mPrio;
  short mPan;
  short x116_;
  int mPitch;
  bool mPlayRequested : 1;
  bool mLooped : 1;
  bool mNonEmitter : 1;
  bool mAutoStart : 1;
  bool mOcclusionTest : 1;
  bool mAcoustics : 1;
  bool mWorldSfx : 1;
  bool mSelfFree : 1;
  bool mAllowDuplicates : 1;
  bool mProcessedThisFrame : 1;
};

CHECK_SIZEOF(CScriptSound, (VERSION >= VERSION_GM8E_02 ? 0x130 : 0x120))

#endif // _CSCRIPTSOUND
