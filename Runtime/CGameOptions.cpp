#include "CGameOptions.hpp"
#include "GameGlobalObjects.hpp"
#include "CMemoryCardSys.hpp"
#include "CSimplePool.hpp"
#include "CSaveWorld.hpp"
#include "CGameHintInfo.hpp"
//#include "Audio/CStreamedAudioManager.hpp"

namespace urde
{

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
                g_SimplePool->GetObj(SObjectTag{FOURCC('SAVW'), world.first});
        cinematicCount += saveWorld->GetCinematicCount();
    }

    std::vector<bool> cinematicStates;
    cinematicStates.reserve(cinematicCount);
    for (size_t i=0 ; i<cinematicCount ; ++i)
        cinematicStates.push_back(stream.ReadEncoded(1));

    for (const auto& world : memWorlds)
    {
        TLockedToken<CSaveWorld> saveWorld =
                g_SimplePool->GetObj(SObjectTag{FOURCC('SAVW'), world.first});

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
                g_SimplePool->GetObj(SObjectTag{FOURCC('SAVW'), world.first});

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
        x0_[b] = stream.ReadEncoded(1);

    x44_soundMode = ESoundMode(stream.ReadEncoded(2));
    x48_ = stream.ReadEncoded(4);

    x4c_screenXOffset = stream.ReadEncoded(6);
    x50_screenYOffset = stream.ReadEncoded(6);
    x54_screenStretch = stream.ReadEncoded(5);
    x58_sfxVol = stream.ReadEncoded(7);
    x5c_musicVol = stream.ReadEncoded(7);
    x60_helmetAlpha = stream.ReadEncoded(8);
    x64_hudAlpha = stream.ReadEncoded(8);

    x68_24_hudLag = stream.ReadEncoded(1);
    x68_28_hintSystem = stream.ReadEncoded(1);
    x68_25_invertY = stream.ReadEncoded(1);
    x68_26_rumble = stream.ReadEncoded(1);
    x68_27_swapBeamsControls = stream.ReadEncoded(1);
}

void CGameOptions::ResetToDefaults()
{
    x48_ = 4;
    x4c_screenXOffset = 0;
    x50_screenYOffset = 0;
    x54_screenStretch = 0;
    x58_sfxVol = 0x7f;
    x5c_musicVol = 0x7f;
    x44_soundMode = ESoundMode::Stereo;
    x60_helmetAlpha = 0xFF;
    x64_hudAlpha = 0xFF;
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
        writer.WriteEncoded(x0_[b], 1);

    writer.WriteEncoded(u32(x44_soundMode), 2);
    writer.WriteEncoded(x48_, 4);

    writer.WriteEncoded(x4c_screenXOffset, 6);
    writer.WriteEncoded(x50_screenYOffset, 6);
    writer.WriteEncoded(x54_screenStretch, 5);
    writer.WriteEncoded(x58_sfxVol, 7);
    writer.WriteEncoded(x5c_musicVol, 7);
    writer.WriteEncoded(x60_helmetAlpha, 8);
    writer.WriteEncoded(x64_hudAlpha, 8);

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

float CGameOptions::sub8020F054()
{
    return (0.375f * 1.f) + (float(x48_) * 0.25f);
}

void CGameOptions::InitSoundMode()
{
    /* If system is mono, force x44 to mono, otherwise honor user preference */
}
static float flt805A8844 = 0.f;
void CGameOptions::sub8020F098(int val, bool b)
{
    x48_ = zeus::clamp(0, val, 8);

    if (b)
        flt805A8844 = sub8020F054();
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

#if 0
    if (apply)
    {
        CAudioSys::SysSetSfxVolume(x58_sfxVol, 1, 1, 1);
        CStreamedAudioManager::SetSfxVolume(x58_sfxVol);
        CMoviePlayer::SetSfxVolume(x58_sfxVol);
    }
#endif
}

void CGameOptions::SetMusicVolume(s32 vol, bool apply)
{
    x5c_musicVol = zeus::clamp(0, vol, 0x7f);
# if 0
    if (apply)
        CStreamedAudioManager::SetGlobalVolume(x5c_musicVol);
#endif
}

void CGameOptions::SetHUDAlpha(u32 alpha)
{
    x64_hudAlpha = alpha;
}

u32 CGameOptions::GetHUDAlpha() const
{
    return x64_hudAlpha;
}

void CGameOptions::SetHelmetAlpha(u32 alpha)
{
    x60_helmetAlpha = alpha;
}

u32 CGameOptions::GetHelmetAlpha() const
{
    return x60_helmetAlpha;
}

void CGameOptions::SetHUDLag(bool lag)
{
    x68_24_hudLag = lag;
}

bool CGameOptions::GetHUDLag() const
{
    return x68_24_hudLag;
}

void CGameOptions::SetInvertYAxis(bool invert)
{
    x68_25_invertY = invert;
}

bool CGameOptions::GetInvertYAxis() const
{
    return x68_25_invertY;
}

void CGameOptions::SetIsRumbleEnabled(bool rumble)
{
    x68_26_rumble = rumble;
}

bool CGameOptions::IsRumbleEnabled() const
{
    return x68_26_rumble;
}

void CGameOptions::ToggleControls(bool swap)
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

bool CGameOptions::IsHintSystemEnabled() const
{
    return x68_28_hintSystem;
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
    sub8020F098(x48_, true);
    SetScreenPositionX(x4c_screenXOffset, true);
    SetScreenPositionY(x50_screenYOffset, true);
    SetScreenStretch(x54_screenStretch, true);
    SetSfxVolume(x58_sfxVol, true);
    SetMusicVolume(x5c_musicVol, true);
    //SetSurroundMode(x44_soundMode, true);
    SetHUDAlpha(x64_hudAlpha);
    SetHUDLag(x68_24_hudLag);
    SetInvertYAxis(x68_25_invertY);
    SetIsRumbleEnabled(x68_26_rumble);
    SetIsHintSystemEnabled(x68_28_hintSystem);
    ToggleControls(x68_27_swapBeamsControls);
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

}
