#include "CGameOptions.hpp"
#include "GameGlobalObjects.hpp"
#include "CMemoryCardSys.hpp"
#include "CSimplePool.hpp"
#include "CSaveWorld.hpp"
#include "CGameHintInfo.hpp"

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
    xd0_24_ = stream.ReadEncoded(1);
    xd0_25_hasHardMode = stream.ReadEncoded(1);
    xd0_26_hardModeBeat = stream.ReadEncoded(1);
    xd0_27_ = stream.ReadEncoded(1);
    xd0_28_hasFusion = stream.ReadEncoded(1);
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
    w.WriteEncoded(xd0_24_, 1);
    w.WriteEncoded(xd0_25_hasHardMode, 1);
    w.WriteEncoded(xd0_26_hardModeBeat, 1);
    w.WriteEncoded(xd0_27_, 1);
    w.WriteEncoded(xd0_28_hasFusion, 1);
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

    x4c_ = stream.ReadEncoded(6);
    x50_ = stream.ReadEncoded(6);
    x54_ = stream.ReadEncoded(5);
    x58_ = stream.ReadEncoded(7);
    x5c_musicVol = stream.ReadEncoded(7);
    x60_ = stream.ReadEncoded(8);
    x64_ = stream.ReadEncoded(8);

    x68_24_ = stream.ReadEncoded(1);
    x68_28_ = stream.ReadEncoded(1);
    x68_25_ = stream.ReadEncoded(1);
    x68_26_ = stream.ReadEncoded(1);
    x68_27_ = stream.ReadEncoded(1);
}

void CGameOptions::PutTo(CBitStreamWriter& writer) const
{
    for (int b=0 ; b<64 ; ++b)
        writer.WriteEncoded(x0_[b], 1);

    writer.WriteEncoded(u32(x44_soundMode), 2);
    writer.WriteEncoded(x48_, 4);

    writer.WriteEncoded(x4c_, 6);
    writer.WriteEncoded(x50_, 6);
    writer.WriteEncoded(x54_, 5);
    writer.WriteEncoded(x58_, 7);
    writer.WriteEncoded(x5c_musicVol, 7);
    writer.WriteEncoded(x60_, 8);
    writer.WriteEncoded(x64_, 8);

    writer.WriteEncoded(x68_24_, 1);
    writer.WriteEncoded(x68_28_, 1);
    writer.WriteEncoded(x68_25_, 1);
    writer.WriteEncoded(x68_26_, 1);
    writer.WriteEncoded(x68_27_, 1);
}

CGameOptions::CGameOptions()
{
    x68_24_ = true;
    x68_26_ = true;
    x68_28_ = true;
    InitSoundMode();
}

void CGameOptions::InitSoundMode()
{
    /* If system is mono, force x44 to mono, otherwise honor user preference */
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
