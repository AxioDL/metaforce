#pragma once

#include <array>
#include <string_view>
#include <vector>

#include "Runtime/CSaveWorld.hpp"
#include "Runtime/RetroTypes.hpp"
#include "Runtime/Audio/CAudioSys.hpp"

namespace urde {
struct CFinalInput;
class CStateManager;

/** Options presented in UI */
enum class EGameOption {
  VisorOpacity,
  HelmetOpacity,
  HUDLag,
  HintSystem,
  ScreenBrightness,
  ScreenOffsetX,
  ScreenOffsetY,
  ScreenStretch,
  SFXVolume,
  MusicVolume,
  SoundMode,
  ReverseYAxis,
  Rumble,
  SwapBeamControls,
  RestoreDefaults
};

/** Option UI type */
enum class EOptionType { Float, DoubleEnum, TripleEnum, RestoreDefaults };

/** Option UI presentation information */
struct SGameOption {
  EGameOption option;
  u32 stringId;
  float minVal, maxVal, increment;
  EOptionType type;
};

/** Static registry of Option UI presentation information */
extern const std::array<std::pair<size_t, const SGameOption*>, 5> GameOptionsRegistry;
extern const std::array<std::pair<size_t, const SGameOption*>, 5> GameOptionsRegistryNew;

/** Options tracked persistently between game sessions */
class CPersistentOptions {
  friend class CGameState;
  std::array<u8, 98> x0_nesState{};
  std::array<bool, 64> x68_{};
  std::vector<std::pair<CAssetId, TEditorId>> xac_cinematicStates; /* (MLVL, Cinematic) */
  u32 xbc_autoMapperKeyState = 0;
  u32 xc0_frozenFpsCount = 0;
  u32 xc4_frozenBallCount = 0;
  u32 xc8_powerBombAmmoCount = 0;
  u32 xcc_logScanPercent = 0;
  bool xd0_24_fusionLinked : 1 = false;
  bool xd0_25_normalModeBeat : 1 = false;
  bool xd0_26_hardModeBeat : 1 = false;
  bool xd0_27_fusionBeat : 1 = false;
  bool xd0_28_fusionSuitActive : 1 = false;
  bool xd0_29_allItemsCollected : 1 = false;

public:
  CPersistentOptions() = default;
  explicit CPersistentOptions(CBitStreamReader& stream);

  bool GetCinematicState(CAssetId mlvlId, TEditorId cineId) const;
  void SetCinematicState(CAssetId mlvlId, TEditorId cineId, bool state);
  u32 GetAutoMapperKeyState() const { return xbc_autoMapperKeyState; }
  void SetAutoMapperKeyState(u32 state) { xbc_autoMapperKeyState = state; }
  bool GetPlayerLinkedFusion() const { return xd0_24_fusionLinked; }
  void SetPlayerLinkedFusion(bool fusionLinked) { xd0_24_fusionLinked = fusionLinked; }
  bool GetPlayerBeatNormalMode() const { return xd0_25_normalModeBeat; }
  void SetPlayerBeatNormalMode(bool normalModeBeat) { xd0_25_normalModeBeat = normalModeBeat; }
  bool GetPlayerBeatHardMode() const { return xd0_26_hardModeBeat; }
  void SetPlayerBeatHardMode(bool hardModeBeat) { xd0_26_hardModeBeat = hardModeBeat; }
  bool GetPlayerBeatFusion() const { return xd0_27_fusionBeat; }
  void SetPlayerBeatFusion(bool fusionBeat) { xd0_27_fusionBeat = fusionBeat; }
  bool GetPlayerFusionSuitActive() const { return xd0_28_fusionSuitActive; }
  void SetPlayerFusionSuitActive(bool fusionSuitActive) { xd0_28_fusionSuitActive = fusionSuitActive; }
  bool GetAllItemsCollected() const { return xd0_29_allItemsCollected; }
  void SetAllItemsCollected(bool allItemsCollected) { xd0_29_allItemsCollected = allItemsCollected; }
  u32 GetLogScanPercent() const { return xcc_logScanPercent; }
  void SetLogScanPercent(u32 percent) { xcc_logScanPercent = percent; }
  void IncrementFrozenFpsCount() { xc0_frozenFpsCount = std::min(int(xc0_frozenFpsCount + 1), 3); }
  bool GetShowFrozenFpsMessage() const { return xc0_frozenFpsCount != 3; }
  void IncrementFrozenBallCount() { xc4_frozenBallCount = std::min(int(xc4_frozenBallCount + 1), 3); }
  bool GetShowFrozenBallMessage() const { return xc4_frozenBallCount != 3; }
  bool GetShowPowerBombAmmoMessage() const { return xc8_powerBombAmmoCount != 1; }
  void IncrementPowerBombAmmoCount() { xc8_powerBombAmmoCount = std::min<u32>(1, xc8_powerBombAmmoCount + 1); }

  void PutTo(CBitStreamWriter& w) const;

  u8* GetNESState() { return x0_nesState.data(); }
  const u8* GetNESState() const { return x0_nesState.data(); }
};

/** Options tracked per game session */
class CGameOptions {
  std::array<u8, 64> x0_{};
  CAudioSys::ESurroundModes x44_soundMode = CAudioSys::ESurroundModes::Stereo;
  u32 x48_screenBrightness = 4;
  s32 x4c_screenXOffset = 0;
  s32 x50_screenYOffset = 0;
  s32 x54_screenStretch = 0;
  u32 x58_sfxVol = 0x7f;
  u32 x5c_musicVol = 0x7f;
  u32 x60_hudAlpha = 0xff;
  u32 x64_helmetAlpha = 0xff;
  bool x68_24_hudLag : 1;
  bool x68_25_invertY : 1;
  bool x68_26_rumble : 1;
  bool x68_27_swapBeamsControls : 1;
  bool x68_28_hintSystem : 1;
  std::vector<std::pair<CAssetId, CAssetId>> x6c_controlTxtrMap;

  s32 m_gamma = 0;

public:
  CGameOptions();
  explicit CGameOptions(CBitStreamReader& stream);
  void ResetToDefaults();
  void InitSoundMode();
  void EnsureSettings();
  void PutTo(CBitStreamWriter& writer) const;

  float TuneScreenBrightness() const;
  void SetScreenBrightness(s32 value, bool apply);
  s32 GetScreenBrightness() const { return x48_screenBrightness; }
  void ApplyGamma();
  void SetGamma(s32 value, bool apply);
  s32 GetGamma() const { return m_gamma; }
  void SetScreenPositionX(s32 position, bool apply);
  s32 GetScreenPositionX() const { return x4c_screenXOffset; }
  void SetScreenPositionY(s32 position, bool apply);
  s32 GetScreenPositionY() const { return x50_screenYOffset; }
  void SetScreenStretch(s32 stretch, bool apply);
  s32 GetScreenStretch() const { return x54_screenStretch; }
  void SetSfxVolume(s32 volume, bool apply);
  s32 GetSfxVolume() const { return x58_sfxVol; }
  void SetMusicVolume(s32 volume, bool apply);
  s32 GetMusicVolume() const { return x5c_musicVol; }
  void SetHUDAlpha(u32 alpha);
  u32 GetHUDAlpha() const { return x60_hudAlpha; }
  void SetHelmetAlpha(u32 alpha);
  u32 GetHelmetAlpha() const { return x64_helmetAlpha; }
  void SetHUDLag(bool lag);
  bool GetHUDLag() const { return x68_24_hudLag; }
  void SetSurroundMode(int mode, bool apply);
  CAudioSys::ESurroundModes GetSurroundMode() const;
  void SetInvertYAxis(bool invert);
  bool GetInvertYAxis() const { return x68_25_invertY; }
  void SetIsRumbleEnabled(bool rumble);
  bool GetIsRumbleEnabled() const { return x68_26_rumble; }
  void SetSwapBeamControls(bool swap);
  bool GetSwapBeamControls() const { return x68_27_swapBeamsControls; }
  void SetIsHintSystemEnabled(bool hints);
  bool GetIsHintSystemEnabled() const { return x68_28_hintSystem; }
  void SetControls(int controls);
  void ResetControllerAssets(int controls);
  const std::vector<std::pair<CAssetId, CAssetId>>& GetControlTXTRMap() const { return x6c_controlTxtrMap; }

  static void TryRestoreDefaults(const CFinalInput& input, int category, int option, bool frontend, bool forceRestore);
  static void SetOption(EGameOption option, int value);
  static int GetOption(EGameOption option);
};

class CHintOptions {
public:
  enum class EHintState { Zero, Waiting, Displaying, Delayed };
  struct SHintState {
    EHintState x0_state = EHintState::Zero;
    float x4_time = 0.f;
    bool x8_dismissed = false;

    SHintState() = default;
    SHintState(EHintState state, float time, bool flag) : x0_state(state), x4_time(time), x8_dismissed(flag) {}

    bool CanContinue() const { return x4_time / 3.f <= 1.f; }
  };

private:
  std::vector<SHintState> x0_hintStates;
  u32 x10_nextHintIdx = -1;

public:
  CHintOptions() = default;
  explicit CHintOptions(CBitStreamReader& stream);
  void PutTo(CBitStreamWriter& writer) const;
  void SetNextHintTime();
  void InitializeMemoryState();
  const SHintState* GetCurrentDisplayedHint() const;
  void DelayHint(std::string_view name);
  void ActivateImmediateHintTimer(std::string_view name);
  void ActivateContinueDelayHintTimer(std::string_view name);
  void DismissDisplayedHint();
  u32 GetNextHintIdx() const;
  const std::vector<SHintState>& GetHintStates() const { return x0_hintStates; }
  void Update(float dt, const CStateManager& stateMgr);
};

} // namespace urde
