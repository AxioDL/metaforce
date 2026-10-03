#ifndef _CGAMEOPTIONS
#define _CGAMEOPTIONS

#include "types.h"

#include "rstl/pair.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/vector.hpp"

#include "Kyoto/Audio/CAudioSys.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "MetroidPrime/Player/CTrilogyOptions.hpp"

class CInputStream;
class CMemoryStreamOut;
class CFinalInput;

enum EGameOption {
  kGO_VisorOpacity,
  kGO_HelmetOpacity,
  kGO_HUDLag,
  kGO_HintSystem,
  kGO_ScreenBrightness,
  kGO_ScreenOffsetX,
  kGO_ScreenOffsetY,
  kGO_ScreenStretch,
  kGO_SFXVolume,
  kGO_MusicVolume,
  kGO_SoundMode,
  kGO_ReverseYAxis,
  kGO_Rumble,
  kGO_SwapBeamControls,
  kGO_RestoreDefaults
};

#if VERSION >= VERSION_R3IJ_00

namespace rstl {
template <>
struct is_trivially_destructible< pair< CAssetId, CAssetId > > {
  enum { value = true };
};

template <>
inline void construct< pair< CAssetId, CAssetId > >(void* dest,
                                                 const pair< CAssetId, CAssetId >& src) {
  *static_cast< pair< CAssetId, CAssetId >* >(dest) = src;
}
} // namespace rstl

class CGameOptions {
public:
  CGameOptions();
  CGameOptions(CInputStream& in);
  ~CGameOptions() {}

  void PutTo(COutputStream& out);
  void EnsureOptions();
  void SetScreenBrightness(int value, bool apply);
  const float TuneScreenBrightness();
  void SetScreenPositionX(int value, bool apply);
  void SetScreenPositionY(int value, bool apply);
  void SetSfxVolume(int value, bool apply);
  void SetMusicVolume(int value, bool apply);
  int GetHudAlphaRaw() const;
  void SetHudAlpha(int value);
  const float GetHudAlpha() const;
  void SetHelmetAlpha(int value);
  int GetHelmetAlphaRaw() const;
  const float GetHelmetAlpha() const;
  void SetIsHudLag(bool enabled);
  void SetIsRedundantHintSystem(bool enabled);
  void SetIsLockOnFreeAim(bool enabled);
  void SetIsHudEnglish(bool enabled);
  void SetIsControllerRumble(bool enabled);
  void SetIsSwitchVisorBeamControls(bool enabled);
  void SetIsFireAndJumpSwapped(bool enabled);
  void SetNotifyAchievementEarned(bool enabled);
  void UpdateAssetRemapList();
  void SetControlPreset(CTrilogyOptions::EControlPreset preset);
  int GetScreenBrightness() const;
  int GetSfxVolume() const;
  int GetMusicVolume() const;
  int GetMusicVolumeAdjustedForTrilogy() const;
  bool GetIsHudLag() const;
  bool GetIsRedundantHintSystem() const;
  bool GetIsLockOnFreeAim() const;
  bool GetIsHudEnglish() const;
  bool GetIsControllerRumble() const;
  bool GetIsSwitchVisorBeamControls() const;
  bool GetIsFireAndJumpSwapped() const;
  bool GetNotifyAchievementEarned() const;
  const rstl::vector< rstl::pair< CAssetId, CAssetId > >& GetAssetRemapList();
  CTrilogyOptions::EControlPreset GetControlPreset() const;

private:
  rstl::reserved_vector< uchar, 64 > mPersistentData;
  rstl::vector< rstl::pair< CAssetId, CAssetId > > mControlTxtrMap;
};
CHECK_SIZEOF(CGameOptions, 0x50)

#else

class CGameOptions {
public:
  static const bool skDefaultHudLag;
  static const bool skDefaultInvertY;
  static const bool skDefaultRumble;
  static const bool skDefaultSwapBeamsControls;
  static const bool skDefaultHintSystem;
  static const bool skDefaultPalFlag;

  bool GetIsSwitchVisorBeamControls() const;
  bool GetIsFireAndJumpSwapped() const;

  static int GetOption(EGameOption option);
  static void SetOption(EGameOption option, int value);
  static void TryRestoreDefaults(const CFinalInput& input, int category, int option, bool frontEnd);

  CGameOptions();
  CGameOptions(CInputStream& in);
  ~CGameOptions() {}

  void PutTo(COutputStream&);

  void InitSoundMode();
  void ResetToDefaults();
  void EnsureOptions();

  void SetScreenBrightness(const int, const bool);
  const float TuneScreenBrightness();
  void SetScreenPositionX(const int, const bool);
  void SetScreenPositionY(const int, const bool);
  void SetScreenStretch(const int, const bool);
  void SetSfxVolume(const int, const bool);
  void SetMusicVolume(const int, const bool);
  void SetSurroundMode(CAudioSys::ESurroundModes, bool);
  void SetHudAlpha(const int);

  const rstl::vector< rstl::pair< CAssetId, CAssetId > >& GetControlTXTRMap() const {
    return mControlTxtrMap;
  }
  int GetMusicVolume() const { return mMusicVol; }

  int GetHudAlphaRaw() const;
  const float GetHudAlpha() const;
  int GetHUDAlpha() const { return mHudAlpha; }
  int GetHelmetAlphaRaw() const;
  const float GetHelmetAlpha() const;
  void SetHelmetAlpha(const int);
  void SetHUDLag(const bool);
  bool GetHUDLag() const { return mHudLag; }
  void SetIsHintSystemEnabled(bool);
  void ToggleControls(const bool);
  void fn_80200564(const bool);
  void ResetControllerAssets(const int);
  void SetControls(const int);

  void SetInvertYAxis(const bool invert);
  const bool GetInvertYAxis() const { return mInvertY; }
  void SetIsRumbleEnabled(const bool rumble);
  const bool GetIsRumbleEnabled() const { return mRumble; }
  bool GetIsHintSystemEnabled() const { return mHintSystem; }
  bool GetSwapBeamControls() const { return mSwapBeamsControls; }

public:
  rstl::reserved_vector< uchar, 64 > x0_;
  int mSoundMode;
  int mScreenBrightness;
  int mScreenXOffset;
  int mScreenYOffset;
  int mScreenStretch;
  uint mSfxVol;
  uint mMusicVol;
  int mHudAlpha;
  int mHelmetAlpha;
  bool mHudLag : 1;
  bool mInvertY : 1;
  bool mRumble : 1;
  bool mSwapBeamsControls : 1;
  bool mHintSystem : 1;
  bool mPalExclusive : 1; // seems unused
  rstl::vector< rstl::pair< CAssetId, CAssetId > > mControlTxtrMap;
};

#if VERSION < VERSION_R3IJ_00
CHECK_SIZEOF(CGameOptions, 0x7c)
#endif

#endif

#endif // _CGAMEOPTIONS
