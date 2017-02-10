#include "CGameOptions.hpp"
#include "GameGlobalObjects.hpp"
#include "CMemoryCardSys.hpp"
#include "CSimplePool.hpp"
#include "CSaveWorld.hpp"
#include "CGameHintInfo.hpp"
#include "GameGlobalObjects.hpp"
#include "CGameState.hpp"
#include "Input/CFinalInput.hpp"
#include "Audio/CSfxManager.hpp"
#include "Audio/CStreamAudioManager.hpp"
#include "Graphics/CMoviePlayer.hpp"

namespace urde
{

static const SGameOption VisorOpts[] =
{
    {EGameOption::VisorOpacity, 21, 0.f, 255.f, 1.f, EOptionType::Float},
    {EGameOption::HelmetOpacity, 22, 0.f, 255.f, 1.f, EOptionType::Float},
    {EGameOption::HUDLag, 23, 0.f, 1.f, 1.f, EOptionType::DoubleEnum},
    {EGameOption::HintSystem, 24, 0.f, 1.f, 1.f, EOptionType::DoubleEnum},
    {EGameOption::RestoreDefaults, 35, 0.f, 1.f, 1.f, EOptionType::RestoreDefaults}
};

static const SGameOption DisplayOpts[] =
{
    {EGameOption::ScreenBrightness, 25, 0.f, 8.f, 1.f, EOptionType::Float},
    {EGameOption::ScreenOffsetX, 26, -30.f, 30.f, 1.f, EOptionType::Float},
    {EGameOption::ScreenOffsetY, 27, -30.f, 30.f, 1.f, EOptionType::Float},
    {EGameOption::ScreenStretch, 28, -10.f, 10.f, 1.f, EOptionType::Float},
    {EGameOption::RestoreDefaults, 35, 0.f, 1.f, 1.f, EOptionType::RestoreDefaults}
};

static const SGameOption SoundOpts[] =
{
    {EGameOption::SFXVolume, 29, 0.f, 127.f, 1.f, EOptionType::Float},
    {EGameOption::MusicVolume, 30, 0.f, 127.f, 1.f, EOptionType::Float},
    {EGameOption::SoundMode, 31, 0.f, 1.f, 1.f, EOptionType::TripleEnum},
    {EGameOption::RestoreDefaults, 35, 0.f, 1.f, 1.f, EOptionType::RestoreDefaults}
};

static const SGameOption ControllerOpts[] =
{
    {EGameOption::ReverseYAxis, 32, 0.f, 1.f, 1.f, EOptionType::DoubleEnum},
    {EGameOption::Rumble, 33, 0.f, 1.f, 1.f, EOptionType::DoubleEnum},
    {EGameOption::SwapBeamControls, 34, 0.f, 1.f, 1.f, EOptionType::DoubleEnum},
    {EGameOption::RestoreDefaults, 35, 0.f, 1.f, 1.f, EOptionType::RestoreDefaults}
};

const std::pair<int, const SGameOption*> GameOptionsRegistry[] =
{
    {5, VisorOpts},
    {5, DisplayOpts},
    {4, SoundOpts},
    {4, ControllerOpts}
};

CPersistentOptions::CPersistentOptions(CBitStreamReader& stream)
{
    for (int b=0 ; b<98 ; ++b)
        x0_[b] = stream.ReadEncoded(1);

    for (int b=0 ; b<64 ; ++b)
        x68_[b] = stream.ReadEncoded(1);

    xc0_ = stream.ReadEncoded(2);
    xc4_ = stream.ReadEncoded(2);
    xc8_ = stream.ReadEncoded(1);
    xcc_logScanCount = stream.ReadEncoded(7);
    xd0_24_fusionLinked = stream.ReadEncoded(1);
    xd0_25_normalModeBeat = stream.ReadEncoded(1);
    xd0_26_hardModeBeat = stream.ReadEncoded(1);
    xd0_27_fusionBeat = stream.ReadEncoded(1);
    xd0_28_fusionSuitActive = stream.ReadEncoded(1);
    xd0_29_allItemsCollected = stream.ReadEncoded(1);
    xbc_ = stream.ReadEncoded(2);

    auto& memWorlds = g_MemoryCardSys->GetMemoryWorlds();
    size_t cinematicCount = 0;
    for (const auto& world : memWorlds)
    {
        TLockedToken<CSaveWorld> saveWorld =
            g_SimplePool->GetObj(SObjectTag{FOURCC('SAVW'), world.second.GetSaveWorldAssetId()});
        cinematicCount += saveWorld->GetCinematicCount();
    }

    std::vector<bool> cinematicStates;
    cinematicStates.reserve(cinematicCount);
    for (size_t i=0 ; i<cinematicCount ; ++i)
        cinematicStates.push_back(stream.ReadEncoded(1));

    for (const auto& world : memWorlds)
    {
        TLockedToken<CSaveWorld> saveWorld =
            g_SimplePool->GetObj(SObjectTag{FOURCC('SAVW'), world.second.GetSaveWorldAssetId()});

        auto stateIt = cinematicStates.cbegin();
        for (TEditorId cineId : saveWorld->GetCinematics())
            if (*stateIt++)
                SetCinematicState(world.first, cineId, true);
    }
}

void CPersistentOptions::PutTo(CBitStreamWriter& w) const
{
    for (int b=0 ; b<98 ; ++b)
        w.WriteEncoded(x0_[b], 1);

    for (int b=0 ; b<64 ; ++b)
        w.WriteEncoded(x68_[b], 1);

    w.WriteEncoded(xc0_, 2);
    w.WriteEncoded(xc4_, 2);
    w.WriteEncoded(xc8_, 1);
    w.WriteEncoded(xcc_logScanCount, 7);
    w.WriteEncoded(xd0_24_fusionLinked, 1);
    w.WriteEncoded(xd0_25_normalModeBeat, 1);
    w.WriteEncoded(xd0_26_hardModeBeat, 1);
    w.WriteEncoded(xd0_27_fusionBeat, 1);
    w.WriteEncoded(xd0_28_fusionSuitActive, 1);
    w.WriteEncoded(xd0_29_allItemsCollected, 1);
    w.WriteEncoded(xbc_, 2);

    auto& memWorlds = g_MemoryCardSys->GetMemoryWorlds();
    for (const auto& world : memWorlds)
    {
        TLockedToken<CSaveWorld> saveWorld =
                g_SimplePool->GetObj(SObjectTag{FOURCC('SAVW'), world.second.GetSaveWorldAssetId()});

        for (TEditorId cineId : saveWorld->GetCinematics())
            w.WriteEncoded(GetCinematicState(world.first, cineId), 1);
    }
}

bool CPersistentOptions::GetCinematicState(ResId mlvlId, TEditorId cineId) const
{
    auto existing = std::find_if(xac_cinematicStates.cbegin(), xac_cinematicStates.cend(),
                                 [&](const std::pair<ResId, TEditorId>& pair) -> bool
    {
        return pair.first == mlvlId && pair.second == cineId;
    });

    return existing != xac_cinematicStates.cend();
}

void CPersistentOptions::SetCinematicState(ResId mlvlId, TEditorId cineId, bool state)
{
    auto existing = std::find_if(xac_cinematicStates.cbegin(), xac_cinematicStates.cend(),
                                 [&](const std::pair<ResId, TEditorId>& pair) -> bool
    {
        return pair.first == mlvlId && pair.second == cineId;
    });

    if (state && existing == xac_cinematicStates.cend())
        xac_cinematicStates.emplace_back(mlvlId, cineId);
    else if (!state && existing != xac_cinematicStates.cend())
        xac_cinematicStates.erase(existing);
}

CGameOptions::CGameOptions(CBitStreamReader& stream)
{
    for (int b=0 ; b<64 ; ++b)
        x0_[b] = stream.ReadEncoded(8);

    x44_soundMode = CAudioSys::ESurroundModes(stream.ReadEncoded(2));
    x48_screenBrightness = stream.ReadEncoded(4);

    x4c_screenXOffset = stream.ReadEncoded(6) - 30;
    x50_screenYOffset = stream.ReadEncoded(6) - 30;
    x54_screenStretch = stream.ReadEncoded(5) - 10;
    x58_sfxVol = stream.ReadEncoded(7);
    x5c_musicVol = stream.ReadEncoded(7);
    x60_hudAlpha = stream.ReadEncoded(8);
    x64_helmetAlpha = stream.ReadEncoded(8);

    x68_24_hudLag = stream.ReadEncoded(1);
    x68_28_hintSystem = stream.ReadEncoded(1);
    x68_25_invertY = stream.ReadEncoded(1);
    x68_26_rumble = stream.ReadEncoded(1);
    x68_27_swapBeamsControls = stream.ReadEncoded(1);
}

void CGameOptions::ResetToDefaults()
{
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

void CGameOptions::PutTo(CBitStreamWriter& writer) const
{
    for (int b=0 ; b<64 ; ++b)
        writer.WriteEncoded(x0_[b], 8);

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
{
    x68_24_hudLag = true;
    x68_26_rumble = true;
    x68_28_hintSystem = true;
    InitSoundMode();
}

float CGameOptions::TuneScreenBrightness()
{
    return (0.375f * 1.f) + (float(x48_screenBrightness) * 0.25f);
}

void CGameOptions::InitSoundMode()
{
    /* If system is mono, force x44 to mono, otherwise honor user preference */
}
static float BrightnessCopyFilter = 0.f;
void CGameOptions::SetScreenBrightness(s32 val, bool b)
{
    x48_screenBrightness = zeus::clamp(0, val, 8);

    if (b)
        BrightnessCopyFilter = TuneScreenBrightness();
}

void CGameOptions::SetScreenPositionX(s32 pos, bool apply)
{
    x4c_screenXOffset = zeus::clamp(-30, pos, 30);

    if (apply)
    {
        /* TOOD: CGraphics related funcs */
    }
}

void CGameOptions::SetScreenPositionY(s32 pos, bool apply)
{
    x50_screenYOffset = zeus::clamp(-30, pos, 30);

    if (apply)
    {
        /* TOOD: CGraphics related funcs */
    }
}

void CGameOptions::SetScreenStretch(s32 st, bool apply)
{
    x54_screenStretch = zeus::clamp(-10, st, 10);

    if (apply)
    {
        /* TOOD: CGraphics related funcs */
    }
}

void CGameOptions::SetSfxVolume(s32 vol, bool apply)
{
    x58_sfxVol = zeus::clamp(0, vol, 0x7f);

    if (apply)
    {
        CAudioSys::SysSetSfxVolume(x58_sfxVol, 1, 1, 1);
        CStreamAudioManager::SetSfxVolume(x58_sfxVol);
        CMoviePlayer::SetSfxVolume(x58_sfxVol);
    }
}

void CGameOptions::SetMusicVolume(s32 vol, bool apply)
{
    x5c_musicVol = zeus::clamp(0, vol, 0x7f);
    if (apply)
        CStreamAudioManager::SetMusicVolume(x5c_musicVol);
}

void CGameOptions::SetHUDAlpha(u32 alpha)
{
    x60_hudAlpha = alpha;
}

void CGameOptions::SetHelmetAlpha(u32 alpha)
{
    x64_helmetAlpha = alpha;
}

void CGameOptions::SetHUDLag(bool lag)
{
    x68_24_hudLag = lag;
}

void CGameOptions::SetSurroundMode(int mode, bool apply)
{
    x44_soundMode = CAudioSys::ESurroundModes(zeus::clamp(0, mode, 2));
    if (apply)
        CAudioSys::SetSurroundMode(x44_soundMode);
}

CAudioSys::ESurroundModes CGameOptions::GetSurroundMode() const
{
    return x44_soundMode;
}

void CGameOptions::SetInvertYAxis(bool invert)
{
    x68_25_invertY = invert;
}

void CGameOptions::SetIsRumbleEnabled(bool rumble)
{
    x68_26_rumble = rumble;
}

void CGameOptions::SetSwapBeamControls(bool swap)
{
    x68_27_swapBeamsControls = swap;
    if (!swap)
        SetControls(0);
    else
        SetControls(1);
}

void CGameOptions::SetIsHintSystemEnabled(bool hints)
{
    x68_28_hintSystem = hints;
}

void CGameOptions::SetControls(s32 controls)
{
    if (controls == 0)
        g_currentPlayerControl = g_tweakPlayerControl;
    else
        g_currentPlayerControl = g_tweakPlayerControlAlt;

    ResetControllerAssets();
}

void CGameOptions::ResetControllerAssets()
{
}

void CGameOptions::EnsureSettings()
{
    SetScreenBrightness(x48_screenBrightness, true);
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

void CGameOptions::TryRestoreDefaults(const CFinalInput& input, int category,
                                      int option, bool frontend, bool forceRestore)
{
    const std::pair<int, const SGameOption*>& options = GameOptionsRegistry[category];
    if (!options.first)
        return;

    if (options.second[option].option != EGameOption::RestoreDefaults)
        return;

    if (!forceRestore && !input.PA())
        return;

    if (frontend)
    {
        CSfxManager::SfxStart(1096, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
        CSfxManager::SfxStart(1091, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
    }
    else
    {
        CSfxManager::SfxStart(1432, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
    }

    CGameOptions& gameOptions = g_GameState->GameOptions();
    switch (category)
    {
    case 0:
        gameOptions.SetHelmetAlpha(0xff);
        gameOptions.SetHUDLag(true);
        gameOptions.SetIsHintSystemEnabled(true);
        break;

    case 1:
        gameOptions.SetScreenBrightness(4, true);
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

    default: break;
    }
}

void CGameOptions::SetOption(EGameOption option, int value)
{
    CGameOptions& options = g_GameState->GameOptions();

    switch (option)
    {
    case EGameOption::VisorOpacity:
        options.SetHUDAlpha(value);
        break;
    case EGameOption::HelmetOpacity:
        options.SetHelmetAlpha(value);
        break;
    case EGameOption::HUDLag:
        options.SetHUDLag(value);
        break;
    case EGameOption::HintSystem:
        options.SetIsHintSystemEnabled(value);
        break;
    case EGameOption::ScreenBrightness:
        options.SetScreenBrightness(value, true);
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
        options.SetInvertYAxis(value);
        break;
    case EGameOption::Rumble:
        options.SetIsRumbleEnabled(value);
        break;
    case EGameOption::SwapBeamControls:
        options.SetSwapBeamControls(value);
        break;
    default: break;
    }
}

int CGameOptions::GetOption(EGameOption option)
{
    const CGameOptions& options = g_GameState->GameOptions();

    switch (option)
    {
    case EGameOption::VisorOpacity:
        return options.GetHUDAlpha();
    case EGameOption::HelmetOpacity:
        return options.GetHelmetAlpha();
    case EGameOption::HUDLag:
        return options.GetHUDLag();
    case EGameOption::HintSystem:
        return options.GetIsHintSystemEnabled();
    case EGameOption::ScreenBrightness:
        return options.GetScreenBrightness();
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
        return options.GetInvertYAxis();
    case EGameOption::Rumble:
        return options.GetIsRumbleEnabled();
    case EGameOption::SwapBeamControls:
        return options.GetSwapBeamControls();
    default: break;
    }

    return 0;
}

CHintOptions::CHintOptions(CBitStreamReader& stream)
{
    const auto& hints = g_MemoryCardSys->GetHints();
    x0_hintStates.reserve(hints.size());

    u32 hintIdx = 0;
    for (const auto& hint : hints)
    {
        EHintState state = EHintState(stream.ReadEncoded(2));
        u32 timeBits = stream.ReadEncoded(32);
        float time = reinterpret_cast<float&>(timeBits);
        if (state == EHintState::Zero)
            time = 0.f;

        x0_hintStates.emplace_back(state, time, false);

        if (x10_nextHintIdx == -1 && state == EHintState::Two)
            x10_nextHintIdx = hintIdx;
        ++hintIdx;
    }
}

void CHintOptions::PutTo(CBitStreamWriter& writer) const
{
    for (const SHintState& hint : x0_hintStates)
    {
        writer.WriteEncoded(u32(hint.x0_state), 2);
        writer.WriteEncoded(reinterpret_cast<const u32&>(hint.x4_time), 32);
    }
}

void CHintOptions::SetNextHintTime()
{
    if (x10_nextHintIdx == -1)
        return;
    x0_hintStates[x10_nextHintIdx].x4_time =
            g_MemoryCardSys->GetHints()[x10_nextHintIdx].GetTime() + 5.f;
}

void CHintOptions::InitializeMemoryState()
{
    const auto& hints = g_MemoryCardSys->GetHints();
    x0_hintStates.resize(hints.size());
}

}
