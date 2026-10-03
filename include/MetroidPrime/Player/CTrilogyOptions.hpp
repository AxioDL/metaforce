#ifndef _CTRILOGYOPTIONS
#define _CTRILOGYOPTIONS

#include "types.h"

class CInputStream;
class COutputStream;

class CTrilogyOptions {
public:
  enum EControlPreset {
    kCP_Basic,
    kCP_Standard,
    kCP_Advanced
  };

  CTrilogyOptions();
  CTrilogyOptions(CInputStream& in);
  void SetScreenBrightness(int value);
  void SetScreenPositionX(int value);
  void SetScreenPositionY(int value);
  void SetSfxVolume(int value);
  void SetMusicVolume(int value);
  void SetHudAlpha(int value);
  void SetHelmetAlpha(int value);

  static EControlPreset kDefaultControlPreset;

private:
  friend class CGameOptions;

  int mScreenBrightness;
  int mScreenPositionX;
  int mScreenPositionY;
  int mSfxVolume;
  int mMusicVolume;
  int mVoiceVolume;
  int mHudAlpha;
  int mHelmetAlpha;
  bool mHudLag : 1;
  bool mControllerRumble : 1;
  bool mRedundantHintSystem : 1;
  bool mHudEnglish : 1;
  bool mFireAndJumpSwapped : 1;
  bool mSwitchVisorBeamControls : 1;
  bool mLockOnFreeAim : 1;
  bool mReservedFlag : 1;
  bool mNotifyAchievementEarned : 1;
  EControlPreset mControlPreset;
};
CHECK_SIZEOF(CTrilogyOptions, 0x28)

#endif // _CTRILOGYOPTIONS
