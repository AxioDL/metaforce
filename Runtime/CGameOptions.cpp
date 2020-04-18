#include "Runtime/CGameOptions.hpp"

#include <cstring>

#include "Runtime/CGameHintInfo.hpp"
#include "Runtime/CGameState.hpp"
#include "Runtime/CMemoryCardSys.hpp"
#include "Runtime/CSaveWorld.hpp"
#include "Runtime/CSimplePool.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Audio/CSfxManager.hpp"
#include "Runtime/Audio/CStreamAudioManager.hpp"
#include "Runtime/Graphics/CMoviePlayer.hpp"
#include "Runtime/Input/CFinalInput.hpp"

#include <hecl/CVarManager.hpp>

namespace urde {

constexpr std::array<SGameOption, 5> VisorOpts{{
    {EGameOption::VisorOpacity, 21, 0.f, 255.f, 1.f, EOptionType::Float},
    {EGameOption::HelmetOpacity, 22, 0.f, 255.f, 1.f, EOptionType::Float},
    {EGameOption::HUDLag, 23, 0.f, 1.f, 1.f, EOptionType::DoubleEnum},
    {EGameOption::HintSystem, 24, 0.f, 1.f, 1.f, EOptionType::DoubleEnum},
    {EGameOption::RestoreDefaults, 35, 0.f, 1.f, 1.f, EOptionType::RestoreDefaults},
}};

constexpr std::array<SGameOption, 5> DisplayOpts{{
    //{EGameOption::ScreenBrightness, 25, 0.f, 8.f, 1.f, EOptionType::Float},
    {EGameOption::ScreenBrightness, 25, -100.f, 100.f, 1.f, EOptionType::Float},
    {EGameOption::ScreenOffsetX, 26, -30.f, 30.f, 1.f, EOptionType::Float},
    {EGameOption::ScreenOffsetY, 27, -30.f, 30.f, 1.f, EOptionType::Float},
    {EGameOption::ScreenStretch, 28, -10.f, 10.f, 1.f, EOptionType::Float},
    {EGameOption::RestoreDefaults, 35, 0.f, 1.f, 1.f, EOptionType::RestoreDefaults},
}};

constexpr std::array<SGameOption, 4> SoundOpts{{
    {EGameOption::SFXVolume, 29, 0.f, 127.f, 1.f, EOptionType::Float},
    {EGameOption::MusicVolume, 30, 0.f, 127.f, 1.f, EOptionType::Float},
    {EGameOption::SoundMode, 31, 0.f, 1.f, 1.f, EOptionType::TripleEnum},
    {EGameOption::RestoreDefaults, 35, 0.f, 1.f, 1.f, EOptionType::RestoreDefaults},
}};

constexpr std::array<SGameOption, 4> ControllerOpts{{
    {EGameOption::ReverseYAxis, 32, 0.f, 1.f, 1.f, EOptionType::DoubleEnum},
    {EGameOption::Rumble, 33, 0.f, 1.f, 1.f, EOptionType::DoubleEnum},
    {EGameOption::SwapBeamControls, 34, 0.f, 1.f, 1.f, EOptionType::DoubleEnum},
    {EGameOption::RestoreDefaults, 35, 0.f, 1.f, 1.f, EOptionType::RestoreDefaults},
}};

constexpr std::array<SGameOption, 5> VisorOptsNew{{
    {EGameOption::VisorOpacity, 23, 0.f, 255.f, 1.f, EOptionType::Float},
    {EGameOption::HelmetOpacity, 24, 0.f, 255.f, 1.f, EOptionType::Float},
    {EGameOption::HUDLag, 25, 0.f, 1.f, 1.f, EOptionType::DoubleEnum},
    {EGameOption::HintSystem, 26, 0.f, 1.f, 1.f, EOptionType::DoubleEnum},
    {EGameOption::RestoreDefaults, 38, 0.f, 1.f, 1.f, EOptionType::RestoreDefaults},
}};

constexpr std::array<SGameOption, 5> DisplayOptsNew{{
    //{EGameOption::ScreenBrightness, 25, 0.f, 8.f, 1.f, EOptionType::Float},
    {EGameOption::ScreenBrightness, 28, -100.f, 100.f, 1.f, EOptionType::Float},
    {EGameOption::ScreenOffsetX, 29, -30.f, 30.f, 1.f, EOptionType::Float},
    {EGameOption::ScreenOffsetY, 30, -30.f, 30.f, 1.f, EOptionType::Float},
    {EGameOption::ScreenStretch, 31, -10.f, 10.f, 1.f, EOptionType::Float},
    {EGameOption::RestoreDefaults, 38, 0.f, 1.f, 1.f, EOptionType::RestoreDefaults},
}};

constexpr std::array<SGameOption, 4> SoundOptsNew{{
    {EGameOption::SFXVolume, 32, 0.f, 127.f, 1.f, EOptionType::Float},
    {EGameOption::MusicVolume, 33, 0.f, 127.f, 1.f, EOptionType::Float},
    {EGameOption::SoundMode, 34, 0.f, 1.f, 1.f, EOptionType::TripleEnum},
    {EGameOption::RestoreDefaults, 38, 0.f, 1.f, 1.f, EOptionType::RestoreDefaults},
}};

constexpr std::array<SGameOption, 4> ControllerOptsNew{{
    {EGameOption::ReverseYAxis, 35, 0.f, 1.f, 1.f, EOptionType::DoubleEnum},
    {EGameOption::Rumble, 37, 0.f, 1.f, 1.f, EOptionType::DoubleEnum},
    {EGameOption::SwapBeamControls, 37, 0.f, 1.f, 1.f, EOptionType::DoubleEnum},
    {EGameOption::RestoreDefaults, 38, 0.f, 1.f, 1.f, EOptionType::RestoreDefaults},
}};

constexpr std::array<std::pair<size_t, const SGameOption*>, 5> GameOptionsRegistry{{
    {VisorOpts.size(), VisorOpts.data()},
    {DisplayOpts.size(), DisplayOpts.data()},
    {SoundOpts.size(), SoundOpts.data()},
    {ControllerOpts.size(), ControllerOpts.data()},
    {0, nullptr},
}};

constexpr std::array<std::pair<size_t, const SGameOption*>, 5> GameOptionsRegistryNew{{
    {VisorOptsNew.size(), VisorOptsNew.data()},
    {DisplayOptsNew.size(), DisplayOptsNew.data()},
    {SoundOptsNew.size(), SoundOptsNew.data()},
    {ControllerOptsNew.size(), ControllerOptsNew.data()},
    {0, nullptr},
}};

CPersistentOptions::CPersistentOptions(CBitStreamReader& stream) {
  for (u8& entry : x0_nesState) {
    entry = stream.ReadEncoded(8);
  }

  for (bool& entry : x68_) {
    entry = stream.ReadEncoded(8) != 0;
  }

  xc0_frozenFpsCount = stream.ReadEncoded(2);
  xc4_frozenBallCount = stream.ReadEncoded(2);
  xc8_powerBombAmmoCount = stream.ReadEncoded(1);
  xcc_logScanPercent = stream.ReadEncoded(7);
  xd0_24_fusionLinked = stream.ReadEncoded(1) != 0;
  xd0_25_normalModeBeat = stream.ReadEncoded(1) != 0;
  xd0_26_hardModeBeat = stream.ReadEncoded(1) != 0;
  xd0_27_fusionBeat = stream.ReadEncoded(1) != 0;
  xd0_28_fusionSuitActive = false;
  xd0_29_allItemsCollected = stream.ReadEncoded(1) != 0;
  xbc_autoMapperKeyState = stream.ReadEncoded(2);

  auto& memWorlds = g_MemoryCardSys->GetMemoryWorlds();
  size_t cinematicCount = 0;
  for (const auto& world : memWorlds) {
    TLockedToken<CSaveWorld> saveWorld =
        g_SimplePool->GetObj(SObjectTag{FOURCC('SAVW'), world.second.GetSaveWorldAssetId()});
    cinematicCount += saveWorld->GetCinematicCount();
  }

  std::vector<bool> cinematicStates;
  cinematicStates.reserve(cinematicCount);
  for (size_t i = 0; i < cinematicCount; ++i) {
    cinematicStates.push_back(stream.ReadEncoded(1) != 0);
  }

  for (const auto& world : memWorlds) {
    TLockedToken<CSaveWorld> saveWorld =
        g_SimplePool->GetObj(SObjectTag{FOURCC('SAVW'), world.second.GetSaveWorldAssetId()});

    auto stateIt = cinematicStates.cbegin();
    for (TEditorId cineId : saveWorld->GetCinematics())
      if (*stateIt++)
        SetCinematicState(world.first, cineId, true);
  }
}

void CPersistentOptions::PutTo(CBitStreamWriter& w) const {
  for (const u8 entry : x0_nesState) {
    w.WriteEncoded(entry, 8);
  }

  for (const bool entry : x68_) {
    w.WriteEncoded(u32(entry), 8);
  }

  w.WriteEncoded(xc0_frozenFpsCount, 2);
  w.WriteEncoded(xc4_frozenBallCount, 2);
  w.WriteEncoded(xc8_powerBombAmmoCount, 1);
  w.WriteEncoded(xcc_logScanPercent, 7);
  w.WriteEncoded(xd0_24_fusionLinked, 1);
  w.WriteEncoded(xd0_25_normalModeBeat, 1);
  w.WriteEncoded(xd0_26_hardModeBeat, 1);
  w.WriteEncoded(xd0_27_fusionBeat, 1);
  w.WriteEncoded(xd0_29_allItemsCollected, 1);
  w.WriteEncoded(xbc_autoMapperKeyState, 2);

  auto& memWorlds = g_MemoryCardSys->GetMemoryWorlds();
  for (const auto& world : memWorlds) {
    const TLockedToken<CSaveWorld> saveWorld =
        g_SimplePool->GetObj(SObjectTag{FOURCC('SAVW'), world.second.GetSaveWorldAssetId()});

    for (const TEditorId cineId : saveWorld->GetCinematics()) {
      w.WriteEncoded(u32(GetCinematicState(world.first, cineId)), 1);
    }
  }
}

bool CPersistentOptions::GetCinematicState(CAssetId mlvlId, TEditorId cineId) const {
  auto existing = std::find_if(xac_cinematicStates.cbegin(), xac_cinematicStates.cend(),
                               [&](const std::pair<CAssetId, TEditorId>& pair) -> bool {
                                 return pair.first == mlvlId && pair.second == cineId;
                               });

  return existing != xac_cinematicStates.cend();
}

void CPersistentOptions::SetCinematicState(CAssetId mlvlId, TEditorId cineId, bool state) {
  auto existing = std::find_if(xac_cinematicStates.cbegin(), xac_cinematicStates.cend(),
                               [&](const std::pair<CAssetId, TEditorId>& pair) -> bool {
                                 return pair.first == mlvlId && pair.second == cineId;
                               });

  if (state && existing == xac_cinematicStates.cend())
    xac_cinematicStates.emplace_back(mlvlId, cineId);
  else if (!state && existing != xac_cinematicStates.cend())
    xac_cinematicStates.erase(existing);
}

CGameOptions::CGameOptions(CBitStreamReader& stream) {
  for (u8& entry : x0_)
    entry = stream.ReadEncoded(8);

  x44_soundMode = CAudioSys::ESurroundModes(stream.ReadEncoded(2));
  x48_screenBrightness = stream.ReadEncoded(4);

  x4c_screenXOffset = stream.ReadEncoded(6) - 30;
  x50_screenYOffset = stream.ReadEncoded(6) - 30;
  x54_screenStretch = stream.ReadEncoded(5) - 10;
  x58_sfxVol = stream.ReadEncoded(7);
  x5c_musicVol = stream.ReadEncoded(7);
  x60_hudAlpha = stream.ReadEncoded(8);
  x64_helmetAlpha = stream.ReadEncoded(8);

  x68_24_hudLag = stream.ReadEncoded(1) != 0;
  x68_28_hintSystem = stream.ReadEncoded(1) != 0;
  x68_25_invertY = stream.ReadEncoded(1) != 0;
  x68_26_rumble = stream.ReadEncoded(1) != 0;
  x68_27_swapBeamsControls = stream.ReadEncoded(1) != 0;
}

void CGameOptions::ResetToDefaults() {
  x48_screenBrightness = 4;
  x4c_screenXOffset = 0;
  x50_screenYOffset = 0;
  x54_screenStretch = 0;
  x58_sfxVol = 0x7f;
  x5c_musicVol = 0x7f;
  x44_soundMode = CAudioSys::ESurroundModes::Stereo;
  x60_hudAlpha = 0xFF;
  x64_helmetAlpha = 0xFF;
  x68_24_hudLag = true;
  x68_25_invertY = false;
  x68_26_rumble = true;
  x68_27_swapBeamsControls = false;
  x68_28_hintSystem = true;
  InitSoundMode();
  EnsureSettings();
}

void CGameOptions::PutTo(CBitStreamWriter& writer) const {
  for (const u8 entry : x0_)
    writer.WriteEncoded(entry, 8);

  writer.WriteEncoded(u32(x44_soundMode), 2);
  writer.WriteEncoded(x48_screenBrightness, 4);

  writer.WriteEncoded(x4c_screenXOffset + 30, 6);
  writer.WriteEncoded(x50_screenYOffset + 30, 6);
  writer.WriteEncoded(x54_screenStretch + 10, 5);
  writer.WriteEncoded(x58_sfxVol, 7);
  writer.WriteEncoded(x5c_musicVol, 7);
  writer.WriteEncoded(x60_hudAlpha, 8);
  writer.WriteEncoded(x64_helmetAlpha, 8);

  writer.WriteEncoded(x68_24_hudLag, 1);
  writer.WriteEncoded(x68_28_hintSystem, 1);
  writer.WriteEncoded(x68_25_invertY, 1);
  writer.WriteEncoded(x68_26_rumble, 1);
  writer.WriteEncoded(x68_27_swapBeamsControls, 1);
}

CGameOptions::CGameOptions()
: x68_24_hudLag(true)
, x68_25_invertY(false)
, x68_26_rumble(true)
, x68_27_swapBeamsControls(false)
, x68_28_hintSystem(true) {
  InitSoundMode();
}

float CGameOptions::TuneScreenBrightness() const { return (0.375f * 1.f) + (float(x48_screenBrightness) * 0.25f); }

void CGameOptions::InitSoundMode() { /* If system is mono, force x44 to mono, otherwise honor user preference */
}
static float BrightnessCopyFilter = 0.f;
void CGameOptions::SetScreenBrightness(s32 value, bool apply) {
  x48_screenBrightness = zeus::clamp(0, value, 8);

  if (!apply) {
    return;
  }

  BrightnessCopyFilter = TuneScreenBrightness();
}

void CGameOptions::ApplyGamma() {
  float gammaT = -m_gamma / 100.f + 1.f;
  if (gammaT < 1.f)
    gammaT = gammaT * 0.5f + 0.5f;
  if (zeus::close_enough(gammaT, 1.f, 0.05f))
    gammaT = 1.f;
  CGraphics::g_BooFactory->setDisplayGamma(gammaT);
}

void CGameOptions::SetGamma(s32 value, bool apply) {
  m_gamma = zeus::clamp(-100, value, 100);

  if (!apply) {
    return;
  }

  ApplyGamma();
}

void CGameOptions::SetScreenPositionX(s32 position, bool apply) {
  x4c_screenXOffset = zeus::clamp(-30, position, 30);

  if (apply) {
    /* TOOD: CGraphics related funcs */
  }
}

void CGameOptions::SetScreenPositionY(s32 position, bool apply) {
  x50_screenYOffset = zeus::clamp(-30, position, 30);

  if (apply) {
    /* TOOD: CGraphics related funcs */
  }
}

void CGameOptions::SetScreenStretch(s32 stretch, bool apply) {
  x54_screenStretch = zeus::clamp(-10, stretch, 10);

  if (apply) {
    /* TOOD: CGraphics related funcs */
  }
}

void CGameOptions::SetSfxVolume(s32 volume, bool apply) {
  x58_sfxVol = zeus::clamp(0, volume, 0x7f);

  if (!apply) {
    return;
  }

  CAudioSys::SysSetSfxVolume(x58_sfxVol, 1, true, true);
  CStreamAudioManager::SetSfxVolume(x58_sfxVol);
  CMoviePlayer::SetSfxVolume(x58_sfxVol);
}

void CGameOptions::SetMusicVolume(s32 volume, bool apply) {
  x5c_musicVol = zeus::clamp(0, volume, 0x7f);

  if (!apply) {
    return;
  }

  CStreamAudioManager::SetMusicVolume(x5c_musicVol);
}

void CGameOptions::SetHUDAlpha(u32 alpha) { x60_hudAlpha = alpha; }

void CGameOptions::SetHelmetAlpha(u32 alpha) { x64_helmetAlpha = alpha; }

void CGameOptions::SetHUDLag(bool lag) { x68_24_hudLag = lag; }

void CGameOptions::SetSurroundMode(int mode, bool apply) {
  x44_soundMode = CAudioSys::ESurroundModes(zeus::clamp(0, mode, 2));
  if (apply)
    CAudioSys::SetSurroundMode(x44_soundMode);
}

CAudioSys::ESurroundModes CGameOptions::GetSurroundMode() const { return x44_soundMode; }

void CGameOptions::SetInvertYAxis(bool invert) { x68_25_invertY = invert; }

void CGameOptions::SetIsRumbleEnabled(bool rumble) { x68_26_rumble = rumble; }

void CGameOptions::SetSwapBeamControls(bool swap) {
  x68_27_swapBeamsControls = swap;
  if (!swap)
    SetControls(0);
  else
    SetControls(1);
}

void CGameOptions::SetIsHintSystemEnabled(bool hints) { x68_28_hintSystem = hints; }

void CGameOptions::SetControls(int controls) {
  if (controls == 0)
    g_currentPlayerControl = g_tweakPlayerControl;
  else
    g_currentPlayerControl = g_tweakPlayerControlAlt;

  ResetControllerAssets(controls);
}

constexpr std::array<std::pair<CAssetId, CAssetId>, 5> CStickToDPadRemap{{
    {0x2A13C23E, 0xF13452F8},
    {0xA91A7703, 0xC042EC91},
    {0x12A12131, 0x5F556002},
    {0xA9798329, 0xB306E26F},
    {0xCD7B1ACA, 0x8ADA8184},
}};

constexpr std::array<std::pair<CAssetId, CAssetId>, 5> CStickOutlineToDPadRemap{{
    {0x1A29C0E6, 0xF13452F8},
    {0x5D9F9796, 0xC042EC91},
    {0x951546A8, 0x5F556002},
    {0x7946C4C5, 0xB306E26F},
    {0x409AA72E, 0x8ADA8184},
}};

void CGameOptions::ResetControllerAssets(int controls) {
  if (controls != 1) {
    x6c_controlTxtrMap.clear();
  } else if (x6c_controlTxtrMap.empty()) {
    x6c_controlTxtrMap.reserve(15);

    for (const auto& entry : CStickToDPadRemap) {
      const auto& emplaced = x6c_controlTxtrMap.emplace_back(entry);
      x6c_controlTxtrMap.emplace_back(emplaced.second, emplaced.first);
    }

    for (const auto& entry : CStickOutlineToDPadRemap)
      x6c_controlTxtrMap.emplace_back(entry);

    std::sort(x6c_controlTxtrMap.begin(), x6c_controlTxtrMap.end(),
              [](const std::pair<CAssetId, CAssetId>& a, const std::pair<CAssetId, CAssetId>& b) {
                return a.first < b.first;
              });
  }
}

void CGameOptions::EnsureSettings() {
  SetScreenBrightness(x48_screenBrightness, true);
  SetGamma(m_gamma, true);
  SetScreenPositionX(x4c_screenXOffset, true);
  SetScreenPositionY(x50_screenYOffset, true);
  SetScreenStretch(x54_screenStretch, true);
  SetSfxVolume(x58_sfxVol, true);
  SetMusicVolume(x5c_musicVol, true);
  SetSurroundMode(int(x44_soundMode), true);
  SetHelmetAlpha(x64_helmetAlpha);
  SetHUDLag(x68_24_hudLag);
  SetInvertYAxis(x68_25_invertY);
  SetIsRumbleEnabled(x68_26_rumble);
  SetIsHintSystemEnabled(x68_28_hintSystem);
  SetSwapBeamControls(x68_27_swapBeamsControls);
}

void CGameOptions::TryRestoreDefaults(const CFinalInput& input, int category, int option, bool frontend,
                                      bool forceRestore) {
  const auto& options = GameOptionsRegistry[category];
  if (options.first == 0)
    return;

  if (options.second[option].option != EGameOption::RestoreDefaults)
    return;

  if (!forceRestore && !input.PA() && !input.PSpecialKey(boo::ESpecialKey::Enter))
    return;

  if (frontend) {
    CSfxManager::SfxStart(SFXfnt_advance_L, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
    CSfxManager::SfxStart(SFXfnt_advance_R, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
  } else {
    CSfxManager::SfxStart(SFXui_advance, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
  }

  CGameOptions& gameOptions = g_GameState->GameOptions();
  switch (category) {
  case 0:
    gameOptions.SetHelmetAlpha(0xff);
    gameOptions.SetHUDLag(true);
    gameOptions.SetIsHintSystemEnabled(true);
    break;

  case 1:
    gameOptions.SetScreenBrightness(4, true);
    gameOptions.SetGamma(0, true);
    gameOptions.SetScreenPositionX(0, true);
    gameOptions.SetScreenPositionY(0, true);
    gameOptions.SetScreenStretch(0, true);
    break;

  case 2:
    gameOptions.SetSfxVolume(0x7f, true);
    gameOptions.SetMusicVolume(0x7f, true);
    gameOptions.SetSurroundMode(1, true);
    break;

  case 3:
    gameOptions.SetInvertYAxis(false);
    gameOptions.SetIsRumbleEnabled(true);
    gameOptions.SetSwapBeamControls(false);
    break;

  default:
    break;
  }
}

void CGameOptions::SetOption(EGameOption option, int value) {
  CGameOptions& options = g_GameState->GameOptions();

  switch (option) {
  case EGameOption::VisorOpacity:
    options.SetHUDAlpha(value);
    break;
  case EGameOption::HelmetOpacity:
    options.SetHelmetAlpha(value);
    break;
  case EGameOption::HUDLag:
    options.SetHUDLag(value != 0);
    break;
  case EGameOption::HintSystem:
    options.SetIsHintSystemEnabled(value != 0);
    break;
  case EGameOption::ScreenBrightness:
    options.SetGamma(value, true);
    break;
  case EGameOption::ScreenOffsetX:
    options.SetScreenPositionX(value, true);
    break;
  case EGameOption::ScreenOffsetY:
    options.SetScreenPositionY(value, true);
    break;
  case EGameOption::ScreenStretch:
    options.SetScreenStretch(value, true);
    break;
  case EGameOption::SFXVolume:
    options.SetSfxVolume(value, true);
    break;
  case EGameOption::MusicVolume:
    options.SetMusicVolume(value, true);
    break;
  case EGameOption::SoundMode:
    options.SetSurroundMode(value, true);
    break;
  case EGameOption::ReverseYAxis:
    options.SetInvertYAxis(value != 0);
    break;
  case EGameOption::Rumble:
    options.SetIsRumbleEnabled(value != 0);
    break;
  case EGameOption::SwapBeamControls:
    options.SetSwapBeamControls(value != 0);
    break;
  default:
    break;
  }
}

int CGameOptions::GetOption(EGameOption option) {
  const CGameOptions& options = g_GameState->GameOptions();

  switch (option) {
  case EGameOption::VisorOpacity:
    return options.GetHUDAlpha();
  case EGameOption::HelmetOpacity:
    return options.GetHelmetAlpha();
  case EGameOption::HUDLag:
    return int(options.GetHUDLag());
  case EGameOption::HintSystem:
    return int(options.GetIsHintSystemEnabled());
  case EGameOption::ScreenBrightness:
    return options.GetGamma();
  case EGameOption::ScreenOffsetX:
    return options.GetScreenPositionX();
  case EGameOption::ScreenOffsetY:
    return options.GetScreenPositionY();
  case EGameOption::ScreenStretch:
    return options.GetScreenStretch();
  case EGameOption::SFXVolume:
    return options.GetSfxVolume();
  case EGameOption::MusicVolume:
    return options.GetMusicVolume();
  case EGameOption::SoundMode:
    return int(options.GetSurroundMode());
  case EGameOption::ReverseYAxis:
    return int(options.GetInvertYAxis());
  case EGameOption::Rumble:
    return int(options.GetIsRumbleEnabled());
  case EGameOption::SwapBeamControls:
    return int(options.GetSwapBeamControls());
  default:
    break;
  }

  return 0;
}

CHintOptions::CHintOptions(CBitStreamReader& stream) {
  const auto& hints = g_MemoryCardSys->GetHints();
  x0_hintStates.reserve(hints.size());

  u32 hintIdx = 0;
  for ([[maybe_unused]] const auto& hint : hints) {
    const auto state = EHintState(stream.ReadEncoded(2));
    const s32 timeBits = stream.ReadEncoded(32);
    float time;
    std::memcpy(&time, &timeBits, sizeof(s32));
    if (state == EHintState::Zero) {
      time = 0.f;
    }

    x0_hintStates.emplace_back(state, time, false);

    if (x10_nextHintIdx == -1 && state == EHintState::Displaying) {
      x10_nextHintIdx = hintIdx;
    }
    ++hintIdx;
  }
}

void CHintOptions::PutTo(CBitStreamWriter& writer) const {
  for (const SHintState& hint : x0_hintStates) {
    writer.WriteEncoded(u32(hint.x0_state), 2);

    u32 timeBits;
    std::memcpy(&timeBits, &hint.x4_time, sizeof(timeBits));

    writer.WriteEncoded(timeBits, 32);
  }
}

void CHintOptions::SetNextHintTime() {
  if (x10_nextHintIdx == -1)
    return;
  x0_hintStates[x10_nextHintIdx].x4_time = g_MemoryCardSys->GetHints()[x10_nextHintIdx].GetTextTime() + 5.f;
}

void CHintOptions::InitializeMemoryState() {
  const auto& hints = g_MemoryCardSys->GetHints();
  x0_hintStates.resize(hints.size());
}

const CHintOptions::SHintState* CHintOptions::GetCurrentDisplayedHint() const {
  if (!g_GameState->GameOptions().GetIsHintSystemEnabled())
    return nullptr;

  if (x10_nextHintIdx == -1)
    return nullptr;

  const SHintState& hintState = x0_hintStates[x10_nextHintIdx];
  const CGameHintInfo::CGameHint& hint = g_MemoryCardSys->GetHints()[x10_nextHintIdx];
  if (hintState.x4_time >= hint.GetTextTime())
    return nullptr;

  if (hintState.x4_time < 3.f)
    return &hintState;

  if (!hintState.x8_dismissed)
    return &hintState;

  return nullptr;
}

void CHintOptions::DelayHint(std::string_view name) {
  const int idx = CGameHintInfo::FindHintIndex(name);
  if (idx == -1) {
    return;
  }

  if (x10_nextHintIdx == idx) {
    for (SHintState& state : x0_hintStates) {
      state.x4_time += 60.f;
    }
  }

  x0_hintStates[idx].x0_state = EHintState::Delayed;
}

void CHintOptions::ActivateImmediateHintTimer(std::string_view name) {
  const int idx = CGameHintInfo::FindHintIndex(name);
  if (idx == -1) {
    return;
  }

  SHintState& hintState = x0_hintStates[idx];
  const CGameHintInfo::CGameHint& hint = g_MemoryCardSys->GetHints()[idx];
  if (hintState.x0_state != EHintState::Zero) {
    return;
  }

  hintState.x0_state = EHintState::Waiting;
  hintState.x4_time = hint.GetImmediateTime();
}

void CHintOptions::ActivateContinueDelayHintTimer(std::string_view name) {
  int idx = x10_nextHintIdx;
  if (idx != 0) {
    idx = CGameHintInfo::FindHintIndex(name);
  }
  if (idx == -1) {
    return;
  }

  SHintState& hintState = x0_hintStates[idx];
  const CGameHintInfo::CGameHint& hint = g_MemoryCardSys->GetHints()[idx];
  if (hintState.x0_state != EHintState::Displaying) {
    return;
  }

  hintState.x4_time = hint.GetTextTime();
}

void CHintOptions::DismissDisplayedHint() {
  if (x10_nextHintIdx == -1)
    return;
  const CGameHintInfo::CGameHint& hint = g_MemoryCardSys->GetHints()[x10_nextHintIdx];
  SHintState& hintState = x0_hintStates[x10_nextHintIdx];
  if (hintState.x4_time >= hint.GetTextTime())
    return;
  hintState.x4_time = hint.GetNormalTime();
  hintState.x8_dismissed = true;
}

u32 CHintOptions::GetNextHintIdx() const {
  if (g_GameState->GameOptions().GetIsHintSystemEnabled())
    return x10_nextHintIdx;
  return -1;
}

void CHintOptions::Update(float dt, const CStateManager& stateMgr) {
  x10_nextHintIdx = -1;
  int idx = 0;
  auto memIt = g_MemoryCardSys->GetHints().begin();
  for (SHintState& state : x0_hintStates) {
    switch (state.x0_state) {
    case EHintState::Waiting:
      state.x4_time -= dt;
      if (state.x4_time <= 0.f) {
        state.x0_state = EHintState::Displaying;
        state.x4_time = memIt->GetTextTime();
      }
      break;
    case EHintState::Displaying:
      if (x10_nextHintIdx == -1)
        x10_nextHintIdx = idx;
      break;
    default:
      break;
    }
    ++memIt;
    ++idx;
  }

  if (x10_nextHintIdx == -1)
    return;

  SHintState& state = x0_hintStates[x10_nextHintIdx];
  const CGameHintInfo::CGameHint& data = g_MemoryCardSys->GetHints()[x10_nextHintIdx];

  state.x4_time = std::max(0.f, state.x4_time - dt);
  if (state.x4_time < data.GetTextTime()) {
    for (const CGameHintInfo::SHintLocation& loc : data.GetLocations()) {
      if (loc.x0_mlvlId == stateMgr.GetWorld()->IGetWorldAssetId() && loc.x8_areaId == stateMgr.GetNextAreaId()) {
        state.x4_time = data.GetNormalTime();
        state.x8_dismissed = true;
      }
    }
  }
}

} // namespace urde
