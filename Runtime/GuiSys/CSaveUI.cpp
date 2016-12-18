#include "CSaveUI.hpp"
#include "CSimplePool.hpp"
#include "GameGlobalObjects.hpp"
#include "CMemoryCardSys.hpp"
#include "GuiSys/CGuiFrame.hpp"
#include "GuiSys/CGuiTableGroup.hpp"
#include "GuiSys/CGuiTextPane.hpp"
#include "MP1/MP1.hpp"

namespace urde
{

CSaveUI::SBannerInfo::SMemoryCardSlotInfo::SMemoryCardSlotInfo(CMemoryCardSys::EMemoryCardPort parentIdx,
                                                               const std::string& name)
: x0_cardPort(parentIdx), x14_name(name)
{}

CSaveUI::SBannerInfo::SBannerInfo(CMemoryCardSys::EMemoryCardPort cardPort, ResId saveBanner,
                                  ResId saveIcon0, ResId saveIcon1, bool flag)
: x0_cardPort(cardPort), x4_saveBanner(saveBanner),
  x8_saveIcon0(saveIcon0), xc_saveIcon1(saveIcon1), x19d_flag(flag)
{
    x100_mcSlotInfos.reserve(2);
    x100_mcSlotInfos.emplace_back(0, SMemoryCardSlotInfo(x0_cardPort, "MetroidPrime A"));
    x100_mcSlotInfos.emplace_back(0, SMemoryCardSlotInfo(x0_cardPort, "MetroidPrime B"));
}

void CSaveUI::SBannerInfo::FinishedLoading()
{
    x10_state = EState::TwentyFive;
    x14_error = EError::Zero;
    FinishedLoading2();
}

void CSaveUI::SBannerInfo::FinishedLoading2()
{
    auto result = CMemoryCardSys::CardProbe(x0_cardPort);
    switch (result.x0_error)
    {
    case CMemoryCardSys::ECardResult::CARD_RESULT_READY:
        if (result.x8_sectorSize != 0x2000)
        {
            x10_state = EState::Twelve;
            x14_error = EError::Seven;
            return;
        }
        x10_state = EState::Five;
        MountCard();
        break;
    case CMemoryCardSys::ECardResult::CARD_RESULT_BUSY:
        break;
    case CMemoryCardSys::ECardResult::CARD_RESULT_WRONGDEVICE:
        x10_state = EState::Twelve;
        x14_error = EError::Four;
        break;
    default:
        NoCardFound();
        break;
    }
}

void CSaveUI::SBannerInfo::NoCardFound()
{
    x10_state = EState::Two;
    static_cast<MP1::CMain*>(g_Main)->SetCardInserted(false);
}

void CSaveUI::SBannerInfo::MountCard()
{
    x10_state = EState::TwentySix;
    x14_error = EError::Zero;
    CMemoryCardSys::ECardResult result = CMemoryCardSys::MountCard(x0_cardPort);
    if (result != CMemoryCardSys::ECardResult::CARD_RESULT_READY)
        MountCardFailed(result);
}

void CSaveUI::SBannerInfo::MountCardFailed(CMemoryCardSys::ECardResult result)
{
    switch (result)
    {
    case CMemoryCardSys::ECardResult::CARD_RESULT_READY:
        x10_state = EState::Six;
        CheckCard();
        break;
    case CMemoryCardSys::ECardResult::CARD_RESULT_BROKEN:
        x10_state = EState::Six;
        x14_error = EError::One;
        CheckCard();
        break;
    default:
        HandleCardError(result, EState::Thirteen);
        break;
    }
}

void CSaveUI::SBannerInfo::CheckCard()
{
    CMemoryCardSys::ECardResult result = CMemoryCardSys::CheckCard(x0_cardPort);
    if (result != CMemoryCardSys::ECardResult::CARD_RESULT_READY)
        CheckCardFailed(result);
}

void CSaveUI::SBannerInfo::CheckCardFailed(CMemoryCardSys::ECardResult result)
{
    switch (result)
    {
    case CMemoryCardSys::ECardResult::CARD_RESULT_READY:
        x10_state = EState::Seven;
        if (!GetCardFreeBytes())
            return;
        if (CMemoryCardSys::GetSerialNo(x0_cardPort, x28_cardSerial) ==
                CMemoryCardSys::ECardResult::CARD_RESULT_READY)
            return;
        NoCardFound();
        break;
    case CMemoryCardSys::ECardResult::CARD_RESULT_BROKEN:
        x10_state = EState::Fourteen;
        x14_error = EError::One;
        break;
    default:
        HandleCardError(result, EState::Fourteen);
    }
}

bool CSaveUI::SBannerInfo::GetCardFreeBytes()
{
    CMemoryCardSys::ECardResult result = CMemoryCardSys::GetNumFreeBytes(x0_cardPort,
                                                                         x18_cardFreeBytes,
                                                                         x1c_cardFreeFiles);
    if (result == CMemoryCardSys::ECardResult::CARD_RESULT_READY)
        return true;
    NoCardFound();
    return false;
}

void CSaveUI::SBannerInfo::HandleCardError(CMemoryCardSys::ECardResult result, EState state)
{
    switch (result)
    {
    case CMemoryCardSys::ECardResult::CARD_RESULT_WRONGDEVICE:
        x10_state = state;
        x14_error = EError::Four;
        break;
    case CMemoryCardSys::ECardResult::CARD_RESULT_NOCARD:
        NoCardFound();
        break;
    case CMemoryCardSys::ECardResult::CARD_RESULT_IOERROR:
        x10_state = state;
        x14_error = EError::Three;
    case CMemoryCardSys::ECardResult::CARD_RESULT_ENCODING:
        x10_state = state;
        x14_error = EError::Two;
        break;
    default: break;
    }
}

void CSaveUI::SBannerInfo::Update()
{
    auto result = CMemoryCardSys::CardProbe(x0_cardPort);

    if (result.x0_error == CMemoryCardSys::ECardResult::CARD_RESULT_NOCARD)
    {
        if (x10_state != EState::Two)
            NoCardFound();
        static_cast<MP1::CMain*>(g_Main)->SetCardInserted(false);
        return;
    }

    switch (x10_state)
    {
    case EState::TwentySix:
    case EState::TwentySeven:
    case EState::TwentyEight:
    case EState::TwentyNine:
    case EState::Thirty:
    case EState::ThirtyOne:
    case EState::ThirtyTwo:
    case EState::ThirtyThree:
    case EState::ThirtyFour:
    case EState::ThirtyFive:
    case EState::ThirtySix:
    case EState::ThirtySeven:
    default: break;
    }
}

std::unique_ptr<CSaveUI::SBannerInfo> CSaveUI::ConstructBannerInfo(bool flag)
{
    return std::make_unique<SBannerInfo>(CMemoryCardSys::EMemoryCardPort::SlotA,
        g_ResFactory->GetResourceIdByName("TXTR_SaveBanner")->id,
        g_ResFactory->GetResourceIdByName("TXTR_SaveIcon0")->id,
        g_ResFactory->GetResourceIdByName("TXTR_SaveIcon1")->id, flag);
}

CIOWin::EMessageReturn CSaveUI::Update(float dt)
{

}

bool CSaveUI::PumpLoad()
{
    if (x50_loadedFrame)
        return true;
    if (!x14_txtrSaveBanner.IsLoaded())
        return false;
    if (!x20_txtrSaveIcon0.IsLoaded())
        return false;
    if (!x2c_txtrSaveIcon1.IsLoaded())
        return false;
    if (!x38_strgMemoryCard.IsLoaded())
        return false;
    for (TLockedToken<CSaveWorld>& savw : x70_saveWorlds)
        if (!savw.IsLoaded())
            return false;
    if (!x44_frmeGenericMenu.IsLoaded())
        return false;

    x50_loadedFrame = x44_frmeGenericMenu.GetObj();
    x54_textpane_message = static_cast<CGuiTextPane*>(x50_loadedFrame->FindWidget("textpane_message"));
    x58_tablegroup_choices = static_cast<CGuiTableGroup*>(x50_loadedFrame->FindWidget("tablegroup_choices"));
    x5c_textpane_choice0 = static_cast<CGuiTextPane*>(x50_loadedFrame->FindWidget("textpane_choice0"));
    x60_textpane_choice1 = static_cast<CGuiTextPane*>(x50_loadedFrame->FindWidget("textpane_choice1"));
    x64_textpane_choice2 = static_cast<CGuiTextPane*>(x50_loadedFrame->FindWidget("textpane_choice2"));
    x68_textpane_choice3 = static_cast<CGuiTextPane*>(x50_loadedFrame->FindWidget("textpane_choice3"));

    x58_tablegroup_choices->SetMenuAdvanceCallback(
        std::bind(&CSaveUI::DoAdvance, this, std::placeholders::_1));
    x58_tablegroup_choices->SetMenuSelectionChangeCallback(
        std::bind(&CSaveUI::DoSelectionChange, this, std::placeholders::_1));

    if (x0_instIdx == 1)
        x6c_bannerInfo->FinishedLoading();

    x10_uiType = SelectUIType();
    FinishedLoading();
    return true;
}

using EState = CSaveUI::SBannerInfo::EState;

static bool InRange1(EState v)
{
    return v >= EState::TwentySix && v <= EState::ThirtySeven;
}

static bool InRange2(EState v)
{
    if (v < EState::TwentyFive)
        return false;
    if (v == EState::TwentySeven)
        return false;
    if (v == EState::TwentyNine)
        return false;
    return true;
}

CSaveUI::UIType CSaveUI::SelectUIType() const
{
    if (x6c_bannerInfo->x10_state == SBannerInfo::EState::Two)
        return UIType::Three;

    switch (x10_uiType)
    {
    case UIType::Thirteen:
    case UIType::Fourteen:
    case UIType::Fifteen:
        return x10_uiType;
    default: break;
    }

    if (InRange1(x6c_bannerInfo->x10_state))
    {
        if (!InRange2(x6c_bannerInfo->x10_state))
            return UIType::Two;
        return UIType::One;
    }

    if (x6c_bannerInfo->x10_state == SBannerInfo::EState::One)
    {
        if (x6c_bannerInfo->x14_error == SBannerInfo::EError::Six)
            return UIType::Twelve;
        return UIType::Sixteen;
    }

    if (x6c_bannerInfo->x14_error == SBannerInfo::EError::One)
        return UIType::Four;

    if (x6c_bannerInfo->x14_error == SBannerInfo::EError::Two)
        return UIType::Five;

    if (x6c_bannerInfo->x14_error == SBannerInfo::EError::Four)
        return UIType::Seven;

    if (x6c_bannerInfo->x14_error == SBannerInfo::EError::Five)
    {
        if (x6c_bannerInfo->x10_state == SBannerInfo::EState::Fourteen)
            return UIType::Eight;
        return UIType::Nine;
    }

    if (x6c_bannerInfo->x14_error == SBannerInfo::EError::Seven)
        return UIType::Ten;

    if (x6c_bannerInfo->x14_error == SBannerInfo::EError::Nine)
        return UIType::Eleven;

    if (x6c_bannerInfo->x14_error == SBannerInfo::EError::Three)
        return UIType::Six;

    return UIType::Zero;
}

void CSaveUI::FinishedLoading()
{

}

void CSaveUI::Draw() const
{

}

void CSaveUI::DoAdvance(CGuiTableGroup* caller)
{

}

void CSaveUI::DoSelectionChange(CGuiTableGroup* caller)
{

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

    x6c_bannerInfo = ConstructBannerInfo(x0_instIdx / 32);

    if (instIdx == 1)
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
