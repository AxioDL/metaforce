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
    xcc_ = stream.ReadEncoded(7);
    xd0_24_ = stream.ReadEncoded(1);
    xd0_25_ = stream.ReadEncoded(1);
    xd0_26_ = stream.ReadEncoded(1);
    xd0_27_ = stream.ReadEncoded(1);
    xd0_28_ = stream.ReadEncoded(1);
    xd0_29_ = stream.ReadEncoded(1);
    xbc_ = stream.ReadEncoded(2);

    auto& memWorlds = g_MemoryCardSys->GetMemoryWorlds();
    size_t cinematicCount = 0;
    for (const CSaveWorldMemory& world : memWorlds)
    {
        TLockedToken<CSaveWorld> saveWorld =
            g_SimplePool->GetObj(SObjectTag{FOURCC('SAVW'), world.GetSaveWorldAssetId()});
        cinematicCount += saveWorld->GetCinematicCount();
    }

    std::vector<bool> cinematicStates;
    cinematicStates.reserve(cinematicCount);
    for (size_t i=0 ; i<cinematicCount ; ++i)
        cinematicStates.push_back(stream.ReadEncoded(1));

    for (const CSaveWorldMemory& world : memWorlds)
    {
        TLockedToken<CSaveWorld> saveWorld =
            g_SimplePool->GetObj(SObjectTag{FOURCC('SAVW'), world.GetSaveWorldAssetId()});

        auto stateIt = cinematicStates.cbegin();
        for (TEditorId cineId : saveWorld->GetCinematics())
            if (*stateIt++)
                SetCinematicState(world.GetWorldAssetId(), cineId, true);
    }
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
    x5c_ = stream.ReadEncoded(7);
    x60_ = stream.ReadEncoded(8);
    x64_ = stream.ReadEncoded(8);

    x68_24_ = stream.ReadEncoded(1);
    x68_28_ = stream.ReadEncoded(1);
    x68_25_ = stream.ReadEncoded(1);
    x68_26_ = stream.ReadEncoded(1);
    x68_27_ = stream.ReadEncoded(1);
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

}
