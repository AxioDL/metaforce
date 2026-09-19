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
    x44_soundMode = 0;
  } else {
    x44_soundMode = (x44_soundMode != 0) ? x44_soundMode : 1;
  }
}

CGameOptions::CGameOptions()
: x0_(0)
, x44_soundMode(1)
, x48_screenBrightness(4)
, x4c_screenXOffset(0)
, x50_screenYOffset(0)
, x54_screenStretch(0)
, x58_sfxVol(0x7f)
, x5c_musicVol(0x7f)
, x60_hudAlpha(0xff)
, x64_helmetAlpha(0xff)
, x68_24_hudLag(skDefaultHudLag)
, x68_25_invertY(skDefaultInvertY)
, x68_26_rumble(skDefaultRumble)
, x68_27_swapBeamsControls(skDefaultSwapBeamsControls)
, x68_28_hintSystem(skDefaultHintSystem)
#if VERSION >= VERSION_GM8P_00
, x68_29_palExclusive(false)
#endif
{
  InitSoundMode();
}

CGameOptions::CGameOptions(CInputStream& in)
: x0_(0)
, x44_soundMode(1)
, x48_screenBrightness(4)
, x4c_screenXOffset(0)
, x50_screenYOffset(0)
, x54_screenStretch(0)
, x58_sfxVol(0x7f)
, x5c_musicVol(0x7f)
, x60_hudAlpha(0xff)
, x64_helmetAlpha(0xff)
, x68_24_hudLag(skDefaultHudLag)
, x68_25_invertY(skDefaultInvertY)
, x68_26_rumble(skDefaultRumble)
, x68_27_swapBeamsControls(skDefaultSwapBeamsControls)
, x68_28_hintSystem(skDefaultHintSystem)
#if VERSION >= VERSION_GM8P_00
, x68_29_palExclusive(false)
#endif
{

  for (int i = 0; i < x0_.size(); ++i) {
    x0_[i] = in.ReadBits(8);
  }
  x44_soundMode = in.ReadBits(CalculateBits(2));
  x48_screenBrightness = in.ReadBits(CalculateBits(8));
  x4c_screenXOffset = in.ReadBits(CalculateBits(0x3c)) - 0x1e;
  x50_screenYOffset = in.ReadBits(CalculateBits(0x3c)) - 0x1e;
  x54_screenStretch = in.ReadBits(CalculateBits(0x14)) - 10;
  x58_sfxVol = in.ReadBits(CalculateBits(0x7f));
  x5c_musicVol = in.ReadBits(CalculateBits(0x7f));
  x60_hudAlpha = in.ReadBits(CalculateBits(0xff));
  x64_helmetAlpha = in.ReadBits(CalculateBits(0xff));

  x68_24_hudLag = in.ReadPackedBool();
  x68_28_hintSystem = in.ReadPackedBool();
  x68_25_invertY = in.ReadPackedBool();
  x68_26_rumble = in.ReadPackedBool();
  x68_27_swapBeamsControls = in.ReadPackedBool();
#if VERSION >= VERSION_GM8P_00
  x68_29_palExclusive = in.ReadPackedBool();
#endif

  InitSoundMode();
}

void CGameOptions::PutTo(COutputStream& out) {
  for (int i = 0; i < x0_.size(); ++i) {
    out.WriteBits(x0_[i], 8);
  }

  WriteValue(out, x44_soundMode, 2);
  WriteValue(out, x48_screenBrightness, 8);
  WriteValue(out, x4c_screenXOffset + 0x1e, 0x3c);
  WriteValue(out, x50_screenYOffset + 0x1e, 0x3c);
  WriteValue(out, x54_screenStretch + 10, 0x14);
  WriteValue(out, x58_sfxVol, 0x7f);
  WriteValue(out, x5c_musicVol, 0x7f);
  WriteValue(out, x60_hudAlpha, 0xff);
  WriteValue(out, x64_helmetAlpha, 0xff);

  out.WriteBits(x68_24_hudLag != false, 1);
  out.WriteBits(x68_28_hintSystem != false, 1);
  out.WriteBits(x68_25_invertY != false, 1);
  out.WriteBits(x68_26_rumble != false, 1);
  out.WriteBits(x68_27_swapBeamsControls != false, 1);
#if VERSION >= VERSION_GM8P_00
  out.WriteBits(x68_29_palExclusive != false, 1);
#endif
}

void CGameOptions::ResetToDefaults() {
  x48_screenBrightness = 4;
  x4c_screenXOffset = 0;
  x50_screenYOffset = 0;
  x54_screenStretch = 0;
  x58_sfxVol = 0x7f;
  x5c_musicVol = 0x7f;
  x44_soundMode = CAudioSys::kSM_Stereo;
  x60_hudAlpha = 0xff;
  x64_helmetAlpha = 0xff;
  x68_24_hudLag = skDefaultHudLag;
  x68_25_invertY = skDefaultInvertY;
  x68_26_rumble = skDefaultRumble;
  x68_27_swapBeamsControls = skDefaultSwapBeamsControls;
  x68_28_hintSystem = skDefaultHintSystem;
#if VERSION >= VERSION_GM8P_00
  x68_29_palExclusive = false;
#endif
  InitSoundMode();
  EnsureOptions();
}

void CGameOptions::EnsureOptions() {
  SetScreenBrightness(x48_screenBrightness, true);
  SetScreenPositionX(x4c_screenXOffset, true);
  SetScreenPositionY(x50_screenYOffset, true);
  SetScreenStretch(x54_screenStretch, true);
  SetSfxVolume(x58_sfxVol, true);
  SetMusicVolume(x5c_musicVol, true);
  SetSurroundMode(CAudioSys::ESurroundModes(x44_soundMode), true);
#if VERSION >= VERSION_GM8P_00
  SetHudAlpha(x60_hudAlpha);
#endif
  SetHelmetAlpha(x64_helmetAlpha);
  SetHUDLag(x68_24_hudLag);
  SetInvertYAxis(x68_25_invertY);
  SetIsRumbleEnabled(x68_26_rumble);
  SetIsHintSystemEnabled(x68_28_hintSystem);
  ToggleControls(x68_27_swapBeamsControls);
#if VERSION >= VERSION_GM8P_00
  fn_80200564(x68_29_palExclusive);
#endif
}

void CGameOptions::SetScreenBrightness(int value, bool apply) {
  x48_screenBrightness = CMath::Clamp(0, value, 8);
  if (apply) {
    CGraphics::SetBrightness(TuneScreenBrightness());
  }
}

const float CGameOptions::TuneScreenBrightness() {
  float f = x48_screenBrightness - 4;
  return f / 4.f * 0.375f + 1.f;
}

void CGameOptions::SetScreenPositionX(int position, bool apply) {
  x4c_screenXOffset = CMath::Clamp(-30, position, 30);
  if (apply) {
    int a, b, c;
    CGraphics::GetScreenPosition(&a, &b, &c);
    CGraphics::SetScreenPosition(a, x4c_screenXOffset, c);
  }
}

void CGameOptions::SetScreenPositionY(int position, bool apply) {
  x50_screenYOffset = CMath::Clamp(-30, position, 30);
  if (apply) {
    int a, b, c;
    CGraphics::GetScreenPosition(&a, &b, &c);
    CGraphics::SetScreenPosition(a, b, x50_screenYOffset);
  }
}

void CGameOptions::SetScreenStretch(int value, bool apply) {
  x54_screenStretch = CMath::Clamp(-10, value, 10);

  if (apply) {
    int a, b, c;
    CGraphics::GetScreenPosition(&a, &b, &c);
    CGraphics::SetScreenPosition(x54_screenStretch, b, c);
  }
}

void CGameOptions::SetSfxVolume(int value, bool apply) {
  x58_sfxVol = CMath::Clamp(0, value, 0x7f);
  if (apply) {
    CAudioSys::SysSetSfxVolume(x58_sfxVol, 1, true, true);
    CStreamAudioManager::SetSfxVolume(x58_sfxVol);
    CMoviePlayer::SetSfxVolume(x58_sfxVol);
  }
}

void CGameOptions::SetMusicVolume(int value, bool apply) {
  x5c_musicVol = CMath::Clamp(0, value, 0x7f);
  if (apply) {
    CStreamAudioManager::SetMusicVolume(x5c_musicVol);
  }
}

void CGameOptions::SetSurroundMode(CAudioSys::ESurroundModes mode, bool apply) {
  x44_soundMode = CMath::Clamp< int >(0, mode, 2);
  if (apply) {
    CAudioSys::SetSurroundMode(CAudioSys::ESurroundModes(x44_soundMode));
  }
}

int CGameOptions::GetHudAlphaRaw() const {
  return x60_hudAlpha;
}

void CGameOptions::SetHudAlpha(int hudAlpha) {
  x60_hudAlpha = hudAlpha;
}

const float CGameOptions::GetHudAlpha() const { return x60_hudAlpha * 0.003921569f; }

#if VERSION >= VERSION_GM8P_00

void CGameOptions::SetHelmetAlpha(const int alpha) { x64_helmetAlpha = alpha; }

int CGameOptions::GetHelmetAlphaRaw() const {
  return x64_helmetAlpha;
}

const float CGameOptions::GetHelmetAlpha() const { return x64_helmetAlpha * 0.003921569f; }

#else

const float CGameOptions::GetHelmetAlpha() const { return x64_helmetAlpha * 0.003921569f; }

void CGameOptions::SetHelmetAlpha(const int alpha) { x64_helmetAlpha = alpha; }

#endif

void CGameOptions::SetHUDLag(const bool flag) { x68_24_hudLag = flag; }

void CGameOptions::SetIsHintSystemEnabled(bool flag) { x68_28_hintSystem = flag; }

void CGameOptions::fn_80200564(const bool flag) {
#if VERSION >= VERSION_GM8P_00
  x68_29_palExclusive = flag;
#endif
}

void CGameOptions::SetInvertYAxis(const bool flag) { x68_25_invertY = flag; }

void CGameOptions::SetIsRumbleEnabled(const bool flag) { x68_26_rumble = flag; }

void CGameOptions::ToggleControls(const bool flag) {
  x68_27_swapBeamsControls = flag;
  if (flag) {
    SetControls(1);
  } else {
    SetControls(0);
  }
}

void CGameOptions::ResetControllerAssets(int controls) {
  switch (controls) {
  case 0: {
    x6c_controlTxtrMap = rstl::vector< rstl::pair< CAssetId, CAssetId > >();
    break;
  }
  case 1: {
    if (x6c_controlTxtrMap.empty()) {
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

      x6c_controlTxtrMap.reserve(15);

      for (int i = 0; i < 5; ++i) {
        x6c_controlTxtrMap.push_back(rstl::pair< CAssetId, CAssetId >(CStickToDPadRemap[i].first,
                                                                      CStickToDPadRemap[i].second));
        x6c_controlTxtrMap.push_back(rstl::pair< CAssetId, CAssetId >(CStickToDPadRemap[i].second,
                                                                      CStickToDPadRemap[i].first));
      }

      for (int i = 0; i < 5; ++i) {
        rstl::pair< CAssetId, CAssetId > value(CStickOutlineToDPadRemap[i]);
        x6c_controlTxtrMap.push_back(value);
      }

      rstl::sort_by_key(x6c_controlTxtrMap);
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
