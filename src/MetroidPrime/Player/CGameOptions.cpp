#include "MetroidPrime/Player/CGameOptions.hpp"

#include "MetroidPrime/Tweaks/CTweaks.hpp"

#include "Kyoto/Audio/CStreamAudioManager.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CMoviePlayer.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/Streams/CMemoryStreamOut.hpp"

#include "rstl/algorithm.hpp"

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
#if VERSION >= VERSION_GM8E_02
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
#if VERSION >= VERSION_GM8E_02
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
#if VERSION >= VERSION_GM8E_02
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
#if VERSION >= VERSION_GM8E_02
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
#if VERSION >= VERSION_GM8E_02
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
#if VERSION >= VERSION_GM8E_02
  SetHudAlpha(mHudAlpha);
#endif
  SetHelmetAlpha(mHelmetAlpha);
  SetHUDLag(mHudLag);
  SetInvertYAxis(mInvertY);
  SetIsRumbleEnabled(mRumble);
  SetIsHintSystemEnabled(mHintSystem);
  ToggleControls(mSwapBeamsControls);
#if VERSION >= VERSION_GM8E_02
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
  return f / 4.f * 0.375f + 1.f;
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
  mScreenYOffset = CMath::Clamp(-30, position, 30);
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

#if VERSION >= VERSION_GM8E_02

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
