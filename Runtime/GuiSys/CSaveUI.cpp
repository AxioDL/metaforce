#include "CSaveUI.hpp"
#include "CSimplePool.hpp"
#include "GameGlobalObjects.hpp"
#include "CMemoryCardSys.hpp"

namespace urde
{

CIOWin::EMessageReturn CSaveUI::Update(float dt)
{

}

bool CSaveUI::PumpLoad()
{
    return false;
}

void CSaveUI::ProcessUserInput(const CFinalInput& input)
{

}

void CSaveUI::StartGame(int idx)
{

}

void CSaveUI::EraseGame(int idx)
{

}

void* CSaveUI::GetGameData(int idx) const
{
    return nullptr;
}

CSaveUI::CSaveUI(u32 instIdx, u32 a, u32 b)
: x0_instIdx(instIdx), x8_a(a), xc_b(b)
{
    x14_txtrSaveBanner = g_SimplePool->GetObj("TXTR_SaveBanner");
    x20_txtrSaveIcon0 = g_SimplePool->GetObj("TXTR_SaveIcon0");
    x2c_txtrSaveIcon1 = g_SimplePool->GetObj("TXTR_SaveIcon1");
    x38_strgMemoryCard = g_SimplePool->GetObj("STRG_MemoryCard");
    x44_frmeGenericMenu = g_SimplePool->GetObj("FRME_GenericMenu");

    if (instIdx)
    {
        x84_navConfirmSfx = 1432;
        x88_navMoveSfx = 1436;
        x8c_navBackSfx = 1431;
    }
    x93_secondaryInst = instIdx;

    x70_saveWorlds.reserve(g_MemoryCardSys->GetMemoryWorlds().size());
    for (const std::pair<ResId, CSaveWorldMemory>& wld : g_MemoryCardSys->GetMemoryWorlds())
    {
        x70_saveWorlds.push_back(
            g_SimplePool->GetObj(SObjectTag{FOURCC('SAVW'), wld.second.GetSaveWorldAssetId()}));
    }
}

}
