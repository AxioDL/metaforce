#ifndef _CRUMBLEVOICE
#define _CRUMBLEVOICE

#include "types.h"

#include "rstl/reserved_vector.hpp"
#include "rstl/vector.hpp"

enum ERumbleFxId {
  kRFX_Zero = 0,
  kRFX_One = 1,
  kRFX_CameraShake = 6,
  kRFX_EscapeSequenceShake = 7,
  kRFX_PlayerBump = 11,
  kRFX_PlayerGunCharge = 12,
  kRFX_PlayerMissileFire = 13,
  kRFX_PlayerGrappleFire = 14,
  kRFX_PlayerLand = 15,
  kRFX_PlayerGrappleSwoosh = 17,
  kRFX_IntroBossProjectile = 19,
  kRFX_Twenty = 20,
  kRFX_TwentyOne = 21,
  kRFX_TwentyTwo = 22,
  kRFX_TwentyThree = 23
};

enum ERumblePriority {
  kRP_None = 0,
  kRP_One = 1,
  kRP_Two = 2,
  kRP_Three = 3,
};

struct SAdsrData {
  SAdsrData();
  SAdsrData(float attackGain, float autoReleaseDur, float attackDur, float decayDur,
            float sustainGain, float releaseDur, bool hasSustain, bool autoRelease);

  float mAttackGain;
  float mAutoReleaseDur;
  float mAttackDur;
  float mDecayDur;
  float mSustainGain;
  float mReleaseDur;
  bool mHasSustain : 1;
  bool mAutoRelease : 1;
};

struct SAdsrDelta {
  enum EPhase {
    kP_Stop,
    kP_PrePulse,
    kP_Attack,
    kP_Decay,
    kP_Sustain,
    kP_Release,
  };

  SAdsrDelta(EPhase phase, ERumblePriority priority);
  SAdsrDelta(EPhase phase);
  static SAdsrDelta Stopped();
  static SAdsrDelta Start(ERumblePriority priority, bool prePulse);

  float mCurIntensity;
  float mAttackTime;
  float mDecayTime;
  float mReleaseTime;
  float mAutoReleaseTime;
  float mAttackIntensity;
  float mSustainIntensity;
  ERumblePriority mPriority;
  EPhase mPhase;
};

class CRumbleVoice {
private:
  static float EnvelopeLerp(float t, float start, float end);
  static void UpdateStage(SAdsrDelta::EPhase& phase, float& intensity, float& time,
                          float start, float end, float duration, SAdsrDelta::EPhase nextPhase,
                          float dt);

  rstl::vector< SAdsrData > mDatas;
  rstl::vector< SAdsrDelta > mDeltas;
  rstl::reserved_vector< ushort, 4 > mHandleIds;
  ushort mUsedChannels;
  uchar mLastId;

public:
  CRumbleVoice();

  short Activate(const SAdsrData& data, ushort idx, float gain, ERumblePriority prio);
  void Deactivate(short id, bool b1);
  void HardReset();
  bool UpdateChannel(SAdsrDelta& delta, const SAdsrData& data, float dt);
  bool Update(float dt);
  ushort GetFreeChannel() const;
  float GetIntensity() const;
  bool OwnsSustained(short id) const;
  short CreateRumbleHandle(ushort idx);
  ushort GetChannelId(short handle) const { return handle & 0xf; }
  ushort GetOwnerId(short handle) const { return ((handle >> 8) & 0xFF); }
  ERumblePriority GetPriority(uint idx) { return mDeltas[idx].mPriority; }
};

#endif // _CRUMBLEVOICE
