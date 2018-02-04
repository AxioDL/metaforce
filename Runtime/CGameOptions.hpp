#ifndef __URDE_CGAMEOPTIONS_HPP__
#define __URDE_CGAMEOPTIONS_HPP__

#include "RetroTypes.hpp"
#include "Audio/CAudioSys.hpp"
#include "CSaveWorld.hpp"

namespace urde
{
struct CFinalInput;
class CStateManager;

/** Options presented in UI */
enum class EGameOption
{
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
enum class EOptionType
{
    Float,
    DoubleEnum,
    TripleEnum,
    RestoreDefaults
};

/** Option UI presentation information */
struct SGameOption
{
    EGameOption option;
    u32 stringId;
    float minVal, maxVal, increment;
    EOptionType type;
};

/** Static registry of Option UI presentation information */
extern const std::pair<int, const SGameOption*> GameOptionsRegistry[];

/** Options tracked persistently between game sessions */
class CPersistentOptions
{
    friend class CGameState;
    u8 x0_[98] = {};
    bool x68_[64] = {};
    std::vector<std::pair<CAssetId, TEditorId>> xac_cinematicStates; /* (MLVL, Cinematic) */
    u32 xbc_autoMapperKeyState = 0;
    u32 xc0_frozenFpsCount = 0;
    u32 xc4_frozenBallCount = 0;
    u32 xc8_powerBombAmmoCount = 0;
    u32 xcc_logScanPercent = 0;

    union
    {
        struct
        {
            bool xd0_24_fusionLinked : 1;
            bool xd0_25_normalModeBeat : 1;
            bool xd0_26_hardModeBeat : 1;
            bool xd0_27_fusionBeat : 1;
            bool xd0_28_fusionSuitActive : 1;
            bool xd0_29_allItemsCollected : 1;
        };
        u16 _dummy = 0;
    };

public:
    CPersistentOptions() = default;
    CPersistentOptions(CBitStreamReader& stream);

    bool GetCinematicState(CAssetId mlvlId, TEditorId cineId) const;
    void SetCinematicState(CAssetId mlvlId, TEditorId cineId, bool state);
    u32 GetAutoMapperKeyState() const { return xbc_autoMapperKeyState; }
    void SetAutoMapperKeyState(u32 s) { xbc_autoMapperKeyState = s; }
    bool GetPlayerLinkedFusion() const { return xd0_24_fusionLinked; }
    void SetPlayerLinkedFusion(bool v) { xd0_24_fusionLinked = v; }
    bool GetPlayerBeatNormalMode() const { return xd0_25_normalModeBeat; }
    void SetPlayerBeatNormalMode(bool v) { xd0_25_normalModeBeat = v; }
    bool GetPlayerBeatHardMode() const { return xd0_26_hardModeBeat; }
    void SetPlayerBeatHardMode(bool v) { xd0_26_hardModeBeat = v; }
    bool GetPlayerBeatFusion() const { return xd0_27_fusionBeat; }
    void SetPlayerBeatFusion(bool v) { xd0_27_fusionBeat = v; }
    bool GetPlayerFusionSuitActive() const { return xd0_28_fusionSuitActive; }
    void SetPlayerFusionSuitActive(bool v) { xd0_28_fusionSuitActive = v; }
    bool GetAllItemsCollected() const { return xd0_29_allItemsCollected; }
    void SetAllItemsCollected(bool v) { xd0_29_allItemsCollected = v; }
    u32 GetLogScanPercent() const { return xcc_logScanPercent; }
    void SetLogScanPercent(u32 v) { xcc_logScanPercent = v; }
    void IncrementFrozenFpsCount() { xc0_frozenFpsCount = std::min(int(xc0_frozenFpsCount + 1), 3); }
    bool GetShowFrozenFpsMessage() const { return xc0_frozenFpsCount != 3; }
    void IncrementFrozenBallCount() { xc4_frozenBallCount = std::min(int(xc4_frozenBallCount + 1), 3); }
    bool GetShowFrozenBallMessage() const { return xc4_frozenBallCount != 3; }
    bool GetShowPowerBombAmmoMessage() const { return xc8_powerBombAmmoCount != 1; }
    void IncrementPowerBombAmmoCount()
    {
        xc8_powerBombAmmoCount = std::min<u32>(1, xc8_powerBombAmmoCount + 1);
    }

    void PutTo(CBitStreamWriter& w) const;

    u8* GetNESState() { return x0_; }
};

/** Options tracked per game session */
class CGameOptions
{
    u8 x0_[64] = {};
    CAudioSys::ESurroundModes x44_soundMode = CAudioSys::ESurroundModes::Stereo;
    u32 x48_screenBrightness = 4;
    s32 x4c_screenXOffset = 0;
    s32 x50_screenYOffset = 0;
    s32 x54_screenStretch = 0;
    u32 x58_sfxVol = 0x7f;
    u32 x5c_musicVol = 0x7f;
    u32 x60_hudAlpha = 0xff;
    u32 x64_helmetAlpha = 0xff;

    s32 m_gamma = 0;

    union
    {
        struct
        {
            bool x68_24_hudLag : 1;
            bool x68_25_invertY : 1;
            bool x68_26_rumble : 1;
            bool x68_27_swapBeamsControls : 1;
            bool x68_28_hintSystem : 1;
        };
        u16 _dummy = 0;
    };

    std::vector<std::pair<CAssetId, CAssetId>> x6c_controlTxtrMap;

public:
    CGameOptions();
    CGameOptions(CBitStreamReader& stream);
    void ResetToDefaults();
    void InitSoundMode();
    void EnsureSettings();
    void PutTo(CBitStreamWriter& writer) const;

    float TuneScreenBrightness();
    void SetScreenBrightness(s32, bool);
    s32 GetScreenBrightness() const { return x48_screenBrightness; }
    void ApplyGamma();
    void SetGamma(s32, bool);
    s32 GetGamma() const { return m_gamma; }
    void SetScreenPositionX(s32, bool);
    s32 GetScreenPositionX() const { return x4c_screenXOffset; }
    void SetScreenPositionY(s32, bool);
    s32 GetScreenPositionY() const { return x50_screenYOffset; }
    void SetScreenStretch(s32, bool);
    s32 GetScreenStretch() const { return x54_screenStretch; }
    void SetSfxVolume(s32, bool);
    s32 GetSfxVolume() const { return x58_sfxVol; }
    void SetMusicVolume(s32, bool);
    s32 GetMusicVolume() const { return x5c_musicVol; }
    void SetHUDAlpha(u32);
    u32 GetHUDAlpha() const { return x60_hudAlpha; }
    void SetHelmetAlpha(u32);
    u32 GetHelmetAlpha() const { return x64_helmetAlpha; }
    void SetHUDLag(bool);
    bool GetHUDLag() const { return x68_24_hudLag; }
    void SetSurroundMode(int mode, bool apply);
    CAudioSys::ESurroundModes GetSurroundMode() const;
    void SetInvertYAxis(bool);
    bool GetInvertYAxis() const { return x68_25_invertY; }
    void SetIsRumbleEnabled(bool);
    bool GetIsRumbleEnabled() const { return x68_26_rumble; }
    void SetSwapBeamControls(bool);
    bool GetSwapBeamControls() const { return x68_27_swapBeamsControls; }
    void SetIsHintSystemEnabled(bool);
    bool GetIsHintSystemEnabled() const { return x68_28_hintSystem; }
    void SetControls(int controls);
    void ResetControllerAssets(int controls);
    const std::vector<std::pair<CAssetId, CAssetId>>& GetControlTXTRMap() const { return x6c_controlTxtrMap; }

    static void TryRestoreDefaults(const CFinalInput& input, int category,
                                   int option, bool frontend, bool forceRestore);
    static void SetOption(EGameOption option, int value);
    static int GetOption(EGameOption option);
};

class CHintOptions
{
public:
    enum class EHintState
    {
        Zero,
        Waiting,
        Displaying,
        Delayed
    };
    struct SHintState
    {
        EHintState x0_state = EHintState::Zero;
        float x4_time = 0.f;
        bool x8_dismissed = false;

        SHintState() = default;
        SHintState(EHintState state, float time, bool flag)
        : x0_state(state), x4_time(time), x8_dismissed(flag) {}

        bool CanContinue() const { return x4_time / 3.f <= 1.f; }
    };
private:
    std::vector<SHintState> x0_hintStates;
    u32 x10_nextHintIdx = -1;
public:
    CHintOptions() = default;
    CHintOptions(CBitStreamReader& stream);
    void PutTo(CBitStreamWriter& writer) const;
    void SetNextHintTime();
    void InitializeMemoryState();
    const SHintState* GetCurrentDisplayedHint() const;
    void DelayHint(const char* name);
    void ActivateImmediateHintTimer(const char* name);
    void ActivateContinueDelayHintTimer(const char* name);
    void DismissDisplayedHint();
    u32 GetNextHintIdx() const;
    const std::vector<SHintState>& GetHintStates() const { return x0_hintStates; }
    void Update(float dt, const CStateManager& stateMgr);
};

}

#endif // __URDE_CGAMEOPTIONS_HPP__
