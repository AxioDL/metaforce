#include "MetroidPrime/Player/CGameOptions.hpp"

#include "MetroidPrime/Tweaks/CTweaks.hpp"

#include "Kyoto/Audio/CStreamAudioManager.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CMoviePlayer.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/Streams/CMemoryStreamOut.hpp"

#include "rstl/algorithm.hpp"

#if VERSION >= VERSION_R3IJ_00

#include "MetroidPrime/CMain.hpp"
#include "MetroidPrime/CTrilogyUtil.hpp"
#include "MetroidPrime/Player/CTrilogyState.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayerControl.hpp"

static CAssetId GetControlAssetId(const char* name) {
  return gpResourceFactory->GetResourceIdByName(name)->GetId();
}

static int AdjustVolumeForTrilogy(int volume) {
  if (CTrilogyUtil::GetGameType() == CTrilogyUtil::kGT_Trilogy) {
    return 0.9f * volume;
  }
  return volume;
}

CGameOptions::CGameOptions() : mPersistentData(uchar(0)) {}

CGameOptions::CGameOptions(CInputStream& in) : mPersistentData(uchar(0)) {
  for (int i = 0; i < mPersistentData.size(); ++i) {
    mPersistentData[i] = in.ReadBits(8);
  }
}

void CGameOptions::PutTo(COutputStream& out) {
  for (int i = 0; i < mPersistentData.size(); ++i) {
    out.WriteBits(mPersistentData[i], 8);
  }
}

void CGameOptions::EnsureOptions() {
  if (gpTrilogyState == nullptr) {
    return;
  }

  SetScreenBrightness(gpTrilogyState->GetOptions().mScreenBrightness, true);
  SetScreenPositionX(gpTrilogyState->GetOptions().mScreenPositionX, true);
  SetScreenPositionY(gpTrilogyState->GetOptions().mScreenPositionY, true);
  SetSfxVolume(gpTrilogyState->GetOptions().mSfxVolume, true);
  SetMusicVolume(gpTrilogyState->GetOptions().mMusicVolume, true);
  SetHudAlpha(gpTrilogyState->GetOptions().mHudAlpha);
  SetHelmetAlpha(gpTrilogyState->GetOptions().mHelmetAlpha);
  SetIsHudLag(gpTrilogyState->GetOptions().mHudLag);
  SetIsControllerRumble(gpTrilogyState->GetOptions().mControllerRumble);
  SetIsRedundantHintSystem(gpTrilogyState->GetOptions().mRedundantHintSystem);
  SetIsHudEnglish(gpTrilogyState->GetOptions().mHudEnglish);
  SetIsFireAndJumpSwapped(gpTrilogyState->GetOptions().mFireAndJumpSwapped);
  SetIsSwitchVisorBeamControls(gpTrilogyState->GetOptions().mSwitchVisorBeamControls);
  SetIsLockOnFreeAim(gpTrilogyState->GetOptions().mLockOnFreeAim);
  SetControlPreset(gpTrilogyState->GetOptions().mControlPreset);
  UpdateAssetRemapList();
}

void CGameOptions::SetScreenBrightness(int value, bool apply) {
  gpTrilogyState->Options().SetScreenBrightness(value);
  if (apply) {
    CGraphics::SetBrightness(TuneScreenBrightness());
  }
}

const float CGameOptions::TuneScreenBrightness() {
  float brightness = (gpTrilogyState->GetOptions().mScreenBrightness - 50) / 50.f;
  return brightness * 0.375f + 1.f;
}

void CGameOptions::SetScreenPositionX(int value, bool apply) {
  gpTrilogyState->Options().SetScreenPositionX(value);
  if (apply) {
    int stretch, x, y;
    CGraphics::GetScreenPosition(&stretch, &x, &y);
    CGraphics::SetScreenPosition(stretch, value, y);
  }
}

void CGameOptions::SetScreenPositionY(int value, bool apply) {
  gpTrilogyState->Options().SetScreenPositionY(value);
  if (apply) {
    int stretch, x, y;
    CGraphics::GetScreenPosition(&stretch, &x, &y);
    CGraphics::SetScreenPosition(stretch, x, value);
  }
}

void CGameOptions::SetSfxVolume(int value, bool apply) {
  gpTrilogyState->Options().SetSfxVolume(value);
  if (apply) {
    int volume = AdjustVolumeForTrilogy(gpTrilogyState->GetOptions().mSfxVolume);
    CAudioSys::SysSetSfxVolume(volume, 1, true, true);
    CStreamAudioManager::SetSfxVolume(volume);
    CMoviePlayer::SetSfxVolume(volume);
  }
}

void CGameOptions::SetMusicVolume(int value, bool apply) {
  gpTrilogyState->Options().SetMusicVolume(value);
  if (apply) {
    CStreamAudioManager::SetMusicVolume(AdjustVolumeForTrilogy(value));
  }
}

int CGameOptions::GetHudAlphaRaw() const { return gpTrilogyState->GetOptions().mHudAlpha; }

void CGameOptions::SetHudAlpha(int value) { gpTrilogyState->Options().SetHudAlpha(value); }

const float CGameOptions::GetHudAlpha() const {
  return gpTrilogyState->GetOptions().mHudAlpha * 0.003921569f;
}

void CGameOptions::SetHelmetAlpha(int value) { gpTrilogyState->Options().SetHelmetAlpha(value); }

int CGameOptions::GetHelmetAlphaRaw() const { return gpTrilogyState->GetOptions().mHelmetAlpha; }

const float CGameOptions::GetHelmetAlpha() const {
  return gpTrilogyState->GetOptions().mHelmetAlpha * 0.003921569f;
}

void CGameOptions::SetIsHudLag(bool enabled) {
  gpTrilogyState->Options().mHudLag = enabled;
}

void CGameOptions::SetIsRedundantHintSystem(bool enabled) {
  gpTrilogyState->Options().mRedundantHintSystem = enabled;
}

void CGameOptions::SetIsLockOnFreeAim(bool enabled) {
  gpTrilogyState->Options().mLockOnFreeAim = enabled;
}

void CGameOptions::SetIsHudEnglish(bool enabled) {
  gpTrilogyState->Options().mHudEnglish = enabled;
}

void CGameOptions::SetIsControllerRumble(bool enabled) {
  gpTrilogyState->Options().mControllerRumble = enabled;
}

void CGameOptions::SetIsSwitchVisorBeamControls(bool enabled) {
  gpTrilogyState->Options().mSwitchVisorBeamControls = enabled;
  UpdateAssetRemapList();
}

void CGameOptions::SetIsFireAndJumpSwapped(bool enabled) {
  gpTrilogyState->Options().mFireAndJumpSwapped = enabled;
  UpdateAssetRemapList();
}

void CGameOptions::SetNotifyAchievementEarned(bool enabled) {
  gpTrilogyState->Options().mNotifyAchievementEarned = enabled;
}

void CGameOptions::UpdateAssetRemapList() {
  int count = 0;
  if (GetIsFireAndJumpSwapped()) {
    count += 2;
  }
  if (GetIsSwitchVisorBeamControls()) {
    count += 2;
  }

  mControlTxtrMap.clear();
  mControlTxtrMap.reserve(count);
  if (GetIsFireAndJumpSwapped()) {
    CAssetId jump = GetControlAssetId("TXTR_Jump");
    CAssetId fire = GetControlAssetId("TXTR_Fire");
    mControlTxtrMap.push_back_unsafe(rstl::pair< CAssetId, CAssetId >(jump, fire));
    mControlTxtrMap.push_back_unsafe(rstl::pair< CAssetId, CAssetId >(fire, jump));
  }
  if (GetIsSwitchVisorBeamControls()) {
    CAssetId visor = GetControlAssetId("TXTR_Visor");
    CAssetId beam = GetControlAssetId("TXTR_Beam");
    mControlTxtrMap.push_back_unsafe(rstl::pair< CAssetId, CAssetId >(visor, beam));
    mControlTxtrMap.push_back_unsafe(rstl::pair< CAssetId, CAssetId >(beam, visor));
  }
  rstl::sort_by_key(mControlTxtrMap);
}

void CGameOptions::SetControlPreset(CTrilogyOptions::EControlPreset preset) {
  if (gpTrilogyState != nullptr) {
    gpTrilogyState->Options().mControlPreset = preset;
  }

  int index = 13;
  switch (preset) {
  case CTrilogyOptions::kCP_Basic:
    index = 12;
    break;
  case CTrilogyOptions::kCP_Standard:
    index = 13;
    break;
  case CTrilogyOptions::kCP_Advanced:
    index = 14;
    break;
  }
  gpMain->RegisterControlTweak(index, rs_new CTweakPlayerControl(preset));
}

int CGameOptions::GetScreenBrightness() const {
  return gpTrilogyState->GetOptions().mScreenBrightness;
}

int CGameOptions::GetSfxVolume() const { return gpTrilogyState->GetOptions().mSfxVolume; }

int CGameOptions::GetMusicVolume() const { return gpTrilogyState->GetOptions().mMusicVolume; }

int CGameOptions::GetMusicVolumeAdjustedForTrilogy() const {
  return AdjustVolumeForTrilogy(GetMusicVolume());
}

bool CGameOptions::GetIsHudLag() const {
  return gpTrilogyState->GetOptions().mHudLag;
}

bool CGameOptions::GetIsRedundantHintSystem() const {
  return gpTrilogyState->GetOptions().mRedundantHintSystem;
}

bool CGameOptions::GetIsLockOnFreeAim() const {
  return gpTrilogyState->GetOptions().mLockOnFreeAim;
}

bool CGameOptions::GetIsHudEnglish() const {
  return gpTrilogyState->GetOptions().mHudEnglish;
}

bool CGameOptions::GetIsControllerRumble() const {
  return gpTrilogyState->GetOptions().mControllerRumble;
}

bool CGameOptions::GetIsSwitchVisorBeamControls() const {
  return gpTrilogyState->GetOptions().mSwitchVisorBeamControls;
}

bool CGameOptions::GetIsFireAndJumpSwapped() const {
  return gpTrilogyState->GetOptions().mFireAndJumpSwapped;
}

bool CGameOptions::GetNotifyAchievementEarned() const {
  return gpTrilogyState->GetOptions().mNotifyAchievementEarned;
}

const rstl::vector< rstl::pair< CAssetId, CAssetId > >& CGameOptions::GetAssetRemapList() {
  return mControlTxtrMap;
}

CTrilogyOptions::EControlPreset CGameOptions::GetControlPreset() const {
  if (gpTrilogyState != nullptr) {
    return gpTrilogyState->GetOptions().mControlPreset;
  }
  return CTrilogyOptions::kDefaultControlPreset;
}

#else

#include "dolphin/os.h"

const bool CGameOptions::skDefaultHudLag = true;
const bool CGameOptions::skDefaultInvertY = false;
const bool CGameOptions::skDefaultRumble = true;
const bool CGameOptions::skDefaultSwapBeamsControls = false;
const bool CGameOptions::skDefaultHintSystem = true;
const bool CGameOptions::skDefaultPalFlag = false;

int CalculateBits(int i) {
  int result = 0;
  for (uint j = i; j != 0; j = j >> 1) {
    result += 1;
  }
  return result;
}

inline void WriteValue(COutputStream& out, uint value, int maxSize) {
  out.WriteBits(value, CalculateBits(maxSize));
}

void CGameOptions::InitSoundMode() {
  if (OSGetSoundMode() == 0) {
    mSoundMode = 0;
  } else {
    mSoundMode = (mSoundMode != 0) ? mSoundMode : 1;
  }
}

CGameOptions::CGameOptions()
: x0_(0)
, mSoundMode(1)
, mScreenBrightness(4)
, mScreenXOffset(0)
, mScreenYOffset(0)
, mScreenStretch(0)
, mSfxVol(0x7f)
, mMusicVol(0x7f)
, mHudAlpha(0xff)
, mHelmetAlpha(0xff)
, mHudLag(skDefaultHudLag)
, mInvertY(skDefaultInvertY)
, mRumble(skDefaultRumble)
, mSwapBeamsControls(skDefaultSwapBeamsControls)
, mHintSystem(skDefaultHintSystem)
#if VERSION >= VERSION_GM8P_00
, mPalExclusive(false)
#endif
{
  InitSoundMode();
}

CGameOptions::CGameOptions(CInputStream& in)
: x0_(0)
, mSoundMode(1)
, mScreenBrightness(4)
, mScreenXOffset(0)
, mScreenYOffset(0)
, mScreenStretch(0)
, mSfxVol(0x7f)
, mMusicVol(0x7f)
, mHudAlpha(0xff)
, mHelmetAlpha(0xff)
, mHudLag(skDefaultHudLag)
, mInvertY(skDefaultInvertY)
, mRumble(skDefaultRumble)
, mSwapBeamsControls(skDefaultSwapBeamsControls)
, mHintSystem(skDefaultHintSystem)
#if VERSION >= VERSION_GM8P_00
, mPalExclusive(false)
#endif
{

  for (int i = 0; i < x0_.size(); ++i) {
    x0_[i] = in.ReadBits(8);
  }
  mSoundMode = in.ReadBits(CalculateBits(2));
  mScreenBrightness = in.ReadBits(CalculateBits(8));
  mScreenXOffset = in.ReadBits(CalculateBits(0x3c)) - 0x1e;
  mScreenYOffset = in.ReadBits(CalculateBits(0x3c)) - 0x1e;
#if VERSION == VERSION_GM8E_02 || VERSION == VERSION_GM8J_00
  mScreenYOffset = CMath::Clamp(-19, mScreenYOffset, 19);
#endif
  mScreenStretch = in.ReadBits(CalculateBits(0x14)) - 10;
  mSfxVol = in.ReadBits(CalculateBits(0x7f));
  mMusicVol = in.ReadBits(CalculateBits(0x7f));
  mHudAlpha = in.ReadBits(CalculateBits(0xff));
  mHelmetAlpha = in.ReadBits(CalculateBits(0xff));

  mHudLag = in.ReadPackedBool();
  mHintSystem = in.ReadPackedBool();
  mInvertY = in.ReadPackedBool();
  mRumble = in.ReadPackedBool();
  mSwapBeamsControls = in.ReadPackedBool();
#if VERSION >= VERSION_GM8P_00
  mPalExclusive = in.ReadPackedBool();
#endif

  InitSoundMode();
}

void CGameOptions::PutTo(COutputStream& out) {
  for (int i = 0; i < x0_.size(); ++i) {
    out.WriteBits(x0_[i], 8);
  }

  WriteValue(out, mSoundMode, 2);
  WriteValue(out, mScreenBrightness, 8);
  WriteValue(out, mScreenXOffset + 0x1e, 0x3c);
  WriteValue(out, mScreenYOffset + 0x1e, 0x3c);
  WriteValue(out, mScreenStretch + 10, 0x14);
  WriteValue(out, mSfxVol, 0x7f);
  WriteValue(out, mMusicVol, 0x7f);
  WriteValue(out, mHudAlpha, 0xff);
  WriteValue(out, mHelmetAlpha, 0xff);

  out.WriteBits(mHudLag != false, 1);
  out.WriteBits(mHintSystem != false, 1);
  out.WriteBits(mInvertY != false, 1);
  out.WriteBits(mRumble != false, 1);
  out.WriteBits(mSwapBeamsControls != false, 1);
#if VERSION >= VERSION_GM8P_00
  out.WriteBits(mPalExclusive != false, 1);
#endif
}

void CGameOptions::ResetToDefaults() {
  mScreenBrightness = 4;
  mScreenXOffset = 0;
  mScreenYOffset = 0;
  mScreenStretch = 0;
  mSfxVol = 0x7f;
  mMusicVol = 0x7f;
  mSoundMode = CAudioSys::kSM_Stereo;
  mHudAlpha = 0xff;
  mHelmetAlpha = 0xff;
  mHudLag = skDefaultHudLag;
  mInvertY = skDefaultInvertY;
  mRumble = skDefaultRumble;
  mSwapBeamsControls = skDefaultSwapBeamsControls;
  mHintSystem = skDefaultHintSystem;
#if VERSION >= VERSION_GM8P_00
  mPalExclusive = false;
#endif
  InitSoundMode();
  EnsureOptions();
}

void CGameOptions::EnsureOptions() {
  SetScreenBrightness(mScreenBrightness, true);
  SetScreenPositionX(mScreenXOffset, true);
  SetScreenPositionY(mScreenYOffset, true);
  SetScreenStretch(mScreenStretch, true);
  SetSfxVolume(mSfxVol, true);
  SetMusicVolume(mMusicVol, true);
  SetSurroundMode(CAudioSys::ESurroundModes(mSoundMode), true);
#if VERSION >= VERSION_GM8P_00
  SetHudAlpha(mHudAlpha);
#endif
  SetHelmetAlpha(mHelmetAlpha);
  SetHUDLag(mHudLag);
  SetInvertYAxis(mInvertY);
  SetIsRumbleEnabled(mRumble);
  SetIsHintSystemEnabled(mHintSystem);
  ToggleControls(mSwapBeamsControls);
#if VERSION >= VERSION_GM8P_00
  fn_80200564(mPalExclusive);
#endif
}

void CGameOptions::SetScreenBrightness(int value, bool apply) {
  mScreenBrightness = CMath::Clamp(0, value, 8);
  if (apply) {
    CGraphics::SetBrightness(TuneScreenBrightness());
  }
}

const float CGameOptions::TuneScreenBrightness() {
  float f = mScreenBrightness - 4;
#if VERSION >= VERSION_GM8J_00
  return f / 4.f * 0.375f + 1.125f;
#else
  return f / 4.f * 0.375f + 1.f;
#endif
}

void CGameOptions::SetScreenPositionX(int position, bool apply) {
  mScreenXOffset = CMath::Clamp(-30, position, 30);
  if (apply) {
    int a, b, c;
    CGraphics::GetScreenPosition(&a, &b, &c);
    CGraphics::SetScreenPosition(a, mScreenXOffset, c);
  }
}

void CGameOptions::SetScreenPositionY(int position, bool apply) {
#if VERSION == VERSION_GM8E_02 || VERSION == VERSION_GM8J_00
  mScreenYOffset = CMath::Clamp(-19, position, 19);
#else
  mScreenYOffset = CMath::Clamp(-30, position, 30);
#endif
  if (apply) {
    int a, b, c;
    CGraphics::GetScreenPosition(&a, &b, &c);
    CGraphics::SetScreenPosition(a, b, mScreenYOffset);
  }
}

void CGameOptions::SetScreenStretch(int value, bool apply) {
  mScreenStretch = CMath::Clamp(-10, value, 10);

  if (apply) {
    int a, b, c;
    CGraphics::GetScreenPosition(&a, &b, &c);
    CGraphics::SetScreenPosition(mScreenStretch, b, c);
  }
}

void CGameOptions::SetSfxVolume(int value, bool apply) {
  mSfxVol = CMath::Clamp(0, value, 0x7f);
  if (apply) {
    CAudioSys::SysSetSfxVolume(mSfxVol, 1, true, true);
    CStreamAudioManager::SetSfxVolume(mSfxVol);
    CMoviePlayer::SetSfxVolume(mSfxVol);
  }
}

void CGameOptions::SetMusicVolume(int value, bool apply) {
  mMusicVol = CMath::Clamp(0, value, 0x7f);
  if (apply) {
    CStreamAudioManager::SetMusicVolume(mMusicVol);
  }
}

void CGameOptions::SetSurroundMode(CAudioSys::ESurroundModes mode, bool apply) {
  mSoundMode = CMath::Clamp< int >(0, mode, 2);
  if (apply) {
    CAudioSys::SetSurroundMode(CAudioSys::ESurroundModes(mSoundMode));
  }
}

int CGameOptions::GetHudAlphaRaw() const {
  return mHudAlpha;
}

void CGameOptions::SetHudAlpha(int hudAlpha) {
  mHudAlpha = hudAlpha;
}

const float CGameOptions::GetHudAlpha() const { return mHudAlpha * 0.003921569f; }

#if VERSION >= VERSION_GM8J_00

void CGameOptions::SetHelmetAlpha(const int alpha) { mHelmetAlpha = alpha; }

int CGameOptions::GetHelmetAlphaRaw() const { return GetHudAlphaRaw(); }

const float CGameOptions::GetHelmetAlpha() const { return GetHudAlpha(); }

#elif VERSION >= VERSION_GM8P_00

void CGameOptions::SetHelmetAlpha(const int alpha) { mHelmetAlpha = alpha; }

int CGameOptions::GetHelmetAlphaRaw() const {
  return mHelmetAlpha;
}

const float CGameOptions::GetHelmetAlpha() const { return mHelmetAlpha * 0.003921569f; }

#else

const float CGameOptions::GetHelmetAlpha() const { return mHelmetAlpha * 0.003921569f; }

void CGameOptions::SetHelmetAlpha(const int alpha) { mHelmetAlpha = alpha; }

#endif

void CGameOptions::SetHUDLag(const bool flag) { mHudLag = flag; }

void CGameOptions::SetIsHintSystemEnabled(bool flag) { mHintSystem = flag; }

void CGameOptions::fn_80200564(const bool flag) {
#if VERSION >= VERSION_GM8E_02
  mPalExclusive = flag;
#endif
}

void CGameOptions::SetInvertYAxis(const bool flag) { mInvertY = flag; }

void CGameOptions::SetIsRumbleEnabled(const bool flag) { mRumble = flag; }

void CGameOptions::ToggleControls(const bool flag) {
  mSwapBeamsControls = flag;
  if (flag) {
    SetControls(1);
  } else {
    SetControls(0);
  }
}

void CGameOptions::ResetControllerAssets(int controls) {
  switch (controls) {
  case 0: {
    mControlTxtrMap = rstl::vector< rstl::pair< CAssetId, CAssetId > >();
    break;
  }
  case 1: {
    if (mControlTxtrMap.empty()) {
      const rstl::pair< CAssetId, CAssetId > CStickToDPadRemap[] = {
          rstl::pair< CAssetId, CAssetId >(0x2A13C23Eu, 0xF13452F8u),
          rstl::pair< CAssetId, CAssetId >(0xA91A7703u, 0xC042EC91u),
          rstl::pair< CAssetId, CAssetId >(0x12A12131u, 0x5F556002u),
          rstl::pair< CAssetId, CAssetId >(0xA9798329u, 0xB306E26Fu),
          rstl::pair< CAssetId, CAssetId >(0xCD7B1ACAu, 0x8ADA8184u),
      };

      const rstl::pair< CAssetId, CAssetId > CStickOutlineToDPadRemap[] = {
          rstl::pair< CAssetId, CAssetId >(0x1A29C0E6u, 0xF13452F8u),
          rstl::pair< CAssetId, CAssetId >(0x5D9F9796u, 0xC042EC91u),
          rstl::pair< CAssetId, CAssetId >(0x951546A8u, 0x5F556002u),
          rstl::pair< CAssetId, CAssetId >(0x7946C4C5u, 0xB306E26Fu),
          rstl::pair< CAssetId, CAssetId >(0x409AA72Eu, 0x8ADA8184u),
      };

      mControlTxtrMap.reserve(15);

      for (int i = 0; i < 5; ++i) {
        mControlTxtrMap.push_back(rstl::pair< CAssetId, CAssetId >(CStickToDPadRemap[i].first,
                                                                      CStickToDPadRemap[i].second));
        mControlTxtrMap.push_back(rstl::pair< CAssetId, CAssetId >(CStickToDPadRemap[i].second,
                                                                      CStickToDPadRemap[i].first));
      }

      for (int i = 0; i < 5; ++i) {
        rstl::pair< CAssetId, CAssetId > value(CStickOutlineToDPadRemap[i]);
        mControlTxtrMap.push_back(value);
      }

      rstl::sort_by_key(mControlTxtrMap);
    }
    break;
  }
  }
}

void CGameOptions::SetControls(int controls) {
  switch (controls) {
  case 0:
    gpTweakPlayerControlCurrent = gpTweakPlayerControl1;
    break;
  case 1:
    gpTweakPlayerControlCurrent = gpTweakPlayerControl2;
    break;
  }
  ResetControllerAssets(controls);
}

#endif
