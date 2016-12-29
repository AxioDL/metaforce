#include "CSaveUI.hpp"
#include "CSimplePool.hpp"
#include "GameGlobalObjects.hpp"
#include "CMemoryCardSys.hpp"
#include "GuiSys/CGuiFrame.hpp"
#include "GuiSys/CGuiTableGroup.hpp"
#include "GuiSys/CGuiTextPane.hpp"
#include "GuiSys/CGuiWidgetDrawParms.hpp"
#include "Audio/CSfxManager.hpp"

namespace urde
{
namespace MP1
{

using EState = CMemoryCardDriver::EState;
using EError = CMemoryCardDriver::EError;

void CSaveUI::ResetCardDriver()
{
    x92_ = false;
    x6c_cardDriver.reset();
    bool importState = (x0_instIdx == 0 && !x90_needsDriverReset);
    x6c_cardDriver = ConstructCardDriver(importState);
    x6c_cardDriver->StartCardProbe();
    x10_uiType = UIType::Empty;
    SetUIText();
}

CIOWin::EMessageReturn CSaveUI::Update(float dt)
{
    if (PumpLoad())
        return CIOWin::EMessageReturn::Normal;

    x50_loadedFrame->Update(dt);
    x6c_cardDriver->Update();

    if (x6c_cardDriver->x10_state == EState::DriverClosed)
    {
        if (x90_needsDriverReset)
        {
            ResetCardDriver();
            x90_needsDriverReset = false;
        }
        else
            x80_iowRet = CIOWin::EMessageReturn::Exit;
    }
    else if (x6c_cardDriver->x10_state == EState::CardCheckDone && x10_uiType != UIType::NotOriginalCard)
    {
        if (x6c_cardDriver->x28_cardSerial != x8_serial)
        {
            if (x93_secondaryInst)
            {
                x10_uiType = UIType::NotOriginalCard;
                x91_uiTextDirty = true;
            }
            else
            {
                x8_serial = x6c_cardDriver->x28_cardSerial;
                x6c_cardDriver->IndexFiles();
            }
        }
    }
    else if (x6c_cardDriver->x10_state == EState::Ready)
    {
        if (x90_needsDriverReset)
            x6c_cardDriver->StartFileCreateTransactional();
    }

    if (x80_iowRet != CIOWin::EMessageReturn::Normal)
        return x80_iowRet;

    UIType oldTp = x10_uiType;
    x10_uiType = SelectUIType();
    if (oldTp != x10_uiType || x91_uiTextDirty)
        SetUIText();

    if (x6c_cardDriver->x10_state == EState::NoCard)
    {
        auto res = CMemoryCardSys::CardProbe(kabufuda::ECardSlot::SlotA);
        if (res.x0_error == CMemoryCardSys::ECardResult::READY ||
            res.x0_error == CMemoryCardSys::ECardResult::WRONGDEVICE)
            ResetCardDriver();
    }
    else if (x6c_cardDriver->x10_state == EState::CardFormatted)
    {
        ResetCardDriver();
    }
    else if (x6c_cardDriver->x10_state == EState::FileBad &&
             x6c_cardDriver->x14_error == EError::FileMissing)
    {
        x6c_cardDriver->StartFileCreate();
    }

    return CIOWin::EMessageReturn::Normal;
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
        x6c_cardDriver->StartCardProbe();

    x10_uiType = SelectUIType();
    SetUIText();
    return true;
}

CSaveUI::UIType CSaveUI::SelectUIType() const
{
    if (x6c_cardDriver->x10_state == EState::NoCard)
        return UIType::NoCardFound;

    switch (x10_uiType)
    {
    case UIType::ProgressWillBeLost:
    case UIType::NotOriginalCard:
    case UIType::AllDataWillBeLost:
        return x10_uiType;
    default: break;
    }

    if (CMemoryCardDriver::IsCardBusy(x6c_cardDriver->x10_state))
    {
        if (CMemoryCardDriver::IsCardWriting(x6c_cardDriver->x10_state))
            return UIType::BusyWriting;
        return UIType::BusyReading;
    }

    if (x6c_cardDriver->x10_state == EState::Ready)
    {
        if (x6c_cardDriver->x14_error == CMemoryCardDriver::EError::CardStillFull)
            return UIType::StillInsufficientSpace;
        return UIType::SaveProgress;
    }

    if (x6c_cardDriver->x14_error == CMemoryCardDriver::EError::CardBroken)
        return UIType::NeedsFormatBroken;

    if (x6c_cardDriver->x14_error == CMemoryCardDriver::EError::CardWrongCharacterSet)
        return UIType::NeedsFormatEncoding;

    if (x6c_cardDriver->x14_error == CMemoryCardDriver::EError::CardWrongDevice)
        return UIType::WrongDevice;

    if (x6c_cardDriver->x14_error == CMemoryCardDriver::EError::CardFull)
    {
        if (x6c_cardDriver->x10_state == EState::CardCheckFailed)
            return UIType::InsufficientSpaceBadCheck;
        return UIType::InsufficientSpaceOKCheck;
    }

    if (x6c_cardDriver->x14_error == CMemoryCardDriver::EError::CardNon8KSectors)
        return UIType::IncompatibleCard;

    if (x6c_cardDriver->x14_error == CMemoryCardDriver::EError::FileCorrupted)
        return UIType::SaveCorrupt;

    if (x6c_cardDriver->x14_error == CMemoryCardDriver::EError::CardIOError)
        return UIType::CardDamaged;

    return UIType::Empty;
}

void CSaveUI::SetUIText()
{
    x91_uiTextDirty = false;

    u32 msgA = -1;
    u32 msgB = -1;
    u32 opt0 = -1;
    u32 opt1 = -1;
    u32 opt2 = -1;

    switch (x10_uiType)
    {
    case UIType::BusyReading:
        msgB = 24; // Reading
        break;
    case UIType::BusyWriting:
        msgB = 25; // Writing
        break;
    case UIType::NoCardFound:
        msgB = 0; // No card found
        opt0 = 17; // Continue without saving
        opt1 = 18; // Retry
        break;
    case UIType::NeedsFormatBroken:
        msgB = 1; // Needs format (card broken)
        opt0 = 17; // Continue without saving
        opt1 = 18; // Retry
        opt2 = 20; // Format
        break;
    case UIType::NeedsFormatEncoding:
        msgB = 2; // Needs format (wrong char set)
        opt0 = 17; // Continue without saving
        opt1 = 18; // Retry
        opt2 = 20; // Format
        break;
    case UIType::CardDamaged:
        msgB = 3; // Damaged
        opt0 = 17; // Continue without saving
        opt1 = 18; // Retry
        break;
    case UIType::WrongDevice:
        msgB = 5; // Invalid device
        opt0 = 17; // Continue without saving
        opt1 = 18; // Retry
        break;
    case UIType::InsufficientSpaceOKCheck:
        msgB = 6; // Insufficient space (completely filled)
        opt0 = 17; // Continue without saving
        opt1 = 18; // Retry
        opt2 = 19; // Manage memory card
        break;
    case UIType::InsufficientSpaceBadCheck:
        msgB = bool(x0_instIdx) + 9;  // Insufficient space A or B
        opt0 = 17; // Continue without saving
        opt1 = 18; // Retry
        opt2 = 19; // Manage memory card
        break;
    case UIType::IncompatibleCard:
        msgB = 7; // Incompatible card
        opt0 = 17; // Continue without saving
        opt1 = 18; // Retry
        break;
    case UIType::SaveCorrupt:
        msgB = 4; // Save corrupt
        opt0 = 22; // Delete corrupt file
        opt1 = 17; // Continue without saving
        opt2 = 18; // Retry
        break;
    case UIType::StillInsufficientSpace:
        if (x0_instIdx == 1)
        {
            msgB = 10; // Insufficient space B
            opt0 = 17; // Continue without saving
            opt1 = 18; // Retry
            opt2 = 19; // Manage memory card
        }
        else
        {
            msgB = 9; // Insufficient space A
            opt0 = 17; // Continue without saving
            opt1 = 18; // Retry
            opt2 = 19; // Manage memory card
        }
        break;
    case UIType::ProgressWillBeLost:
        msgA = 28; // Warning
        msgB = 11; // Progress will be lost
        opt0 = 21; // Cancel
        opt1 = 16; // Continue
        break;
    case UIType::NotOriginalCard:
        msgA = 28; // Warning
        msgB = 12; // Not the original card
        opt0 = x0_instIdx == 1 ? 21 : 17; // Cancel : continue without saving
        opt1 = 16; // Continue
        break;
    case UIType::AllDataWillBeLost:
        msgA = 28; // Warning
        msgB = 13; // All card data will be erased
        opt0 = 16; // Continue
        opt1 = 21; // Cancel
        break;
    case UIType::SaveProgress:
        if (x0_instIdx == 1)
        {
            msgB = 8; // Save progress?
            opt0 = 14; // Yes
            opt1 = 15; // No
        }
        break;
    default: break;
    }

    std::wstring msgAStr;
    if (msgA != -1)
        msgAStr = x38_strgMemoryCard->GetString(msgA);
    std::wstring msgBStr;
    if (msgB != -1)
        msgBStr = x38_strgMemoryCard->GetString(msgB);
    x54_textpane_message->TextSupport()->SetText(msgAStr + msgBStr);

    std::wstring opt0Str;
    if (opt0 != -1)
        opt0Str = x38_strgMemoryCard->GetString(opt0);
    x5c_textpane_choice0->TextSupport()->SetText(opt0Str);

    std::wstring opt1Str;
    if (opt1 != -1)
        opt1Str = x38_strgMemoryCard->GetString(opt1);
    x60_textpane_choice1->TextSupport()->SetText(opt1Str);

    std::wstring opt2Str;
    if (opt2 != -1)
        opt2Str = x38_strgMemoryCard->GetString(opt2);
    x64_textpane_choice2->TextSupport()->SetText(opt2Str);

    std::wstring opt3Str;
    x68_textpane_choice3->TextSupport()->SetText(opt3Str);

    x5c_textpane_choice0->SetB627(opt0 != -1);
    x60_textpane_choice1->SetB627(opt1 != -1);
    x64_textpane_choice2->SetB627(opt2 != -1);
    x68_textpane_choice3->SetB627(false);

    x58_tablegroup_choices->SetIsActive(opt0 != -1 || opt1 != -1 || opt2 != -1);
    SetUIColors();
}

void CSaveUI::SetUIColors()
{
    x58_tablegroup_choices->SetColors(zeus::CColor::skWhite,
                                      zeus::CColor{0.627450f, 0.627450f, 0.627450f, 0.784313f});
}

void CSaveUI::Draw() const
{
    if (x50_loadedFrame)
        x50_loadedFrame->Draw(CGuiWidgetDrawParms::Default);
}

void CSaveUI::ContinueWithoutSaving()
{
    x80_iowRet = CIOWin::EMessageReturn::RemoveIOWin;
    g_GameState->SetCardSerial(0);
}

void CSaveUI::DoAdvance(CGuiTableGroup* caller)
{
    int userSel = x58_tablegroup_choices->GetUserSelection();
    s32 sfx = -1;

    switch (x10_uiType)
    {
    case UIType::NoCardFound:
    case UIType::CardDamaged:
    case UIType::WrongDevice:
    case UIType::IncompatibleCard:
        if (userSel == 0)
        {
            /* Continue without saving */
            if (x0_instIdx == 1)
                x80_iowRet = CIOWin::EMessageReturn::RemoveIOWinAndExit;
            else
                ContinueWithoutSaving();
            sfx = x8c_navBackSfx;
        }
        else if (userSel == 1)
        {
            /* Retry */
            ResetCardDriver();
            sfx = x84_navConfirmSfx;
        }
        break;

    case UIType::NeedsFormatBroken:
    case UIType::NeedsFormatEncoding:

    case UIType::InsufficientSpaceBadCheck:
    case UIType::InsufficientSpaceOKCheck:

    case UIType::SaveCorrupt:

    case UIType::StillInsufficientSpace:

    case UIType::NotOriginalCard:

    case UIType::AllDataWillBeLost:

    case UIType::SaveProgress:


    default: break;
    }

    if (sfx >= 0)
        CSfxManager::SfxStart(sfx, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
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

CSaveUI::CSaveUI(u32 instIdx, u64 serial)
: x0_instIdx(instIdx), x8_serial(serial)
{
    x14_txtrSaveBanner = g_SimplePool->GetObj("TXTR_SaveBanner");
    x20_txtrSaveIcon0 = g_SimplePool->GetObj("TXTR_SaveIcon0");
    x2c_txtrSaveIcon1 = g_SimplePool->GetObj("TXTR_SaveIcon1");
    x38_strgMemoryCard = g_SimplePool->GetObj("STRG_MemoryCard");
    x44_frmeGenericMenu = g_SimplePool->GetObj("FRME_GenericMenu");

    x6c_cardDriver = ConstructCardDriver(x0_instIdx);

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

std::unique_ptr<CMemoryCardDriver> CSaveUI::ConstructCardDriver(bool importState)
{
    return std::make_unique<CMemoryCardDriver>(kabufuda::ECardSlot::SlotA,
        g_ResFactory->GetResourceIdByName("TXTR_SaveBanner")->id,
        g_ResFactory->GetResourceIdByName("TXTR_SaveIcon0")->id,
        g_ResFactory->GetResourceIdByName("TXTR_SaveIcon1")->id, importState);
}

}
}
