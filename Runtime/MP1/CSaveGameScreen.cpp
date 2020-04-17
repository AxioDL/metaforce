#include "Runtime/MP1/CSaveGameScreen.hpp"

#include "Runtime/CMemoryCardSys.hpp"
#include "Runtime/CSimplePool.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Audio/CSfxManager.hpp"
#include "Runtime/GuiSys/CGuiFrame.hpp"
#include "Runtime/GuiSys/CGuiTableGroup.hpp"
#include "Runtime/GuiSys/CGuiTextPane.hpp"
#include "Runtime/GuiSys/CGuiWidgetDrawParms.hpp"
#include "Runtime/MP1/MP1.hpp"

namespace urde::MP1 {

using EState = CMemoryCardDriver::EState;
using EError = CMemoryCardDriver::EError;

void CSaveGameScreen::ResetCardDriver() {
  x92_savingDisabled = false;
  x6c_cardDriver.reset();
  bool importState = (x0_saveCtx == ESaveContext::FrontEnd && !x90_needsDriverReset);
  x6c_cardDriver = ConstructCardDriver(importState);
  x6c_cardDriver->StartCardProbe();
  x10_uiType = EUIType::Empty;
  SetUIText();
}

CIOWin::EMessageReturn CSaveGameScreen::Update(float dt) {
  if (!PumpLoad())
    return CIOWin::EMessageReturn::Normal;

  x50_loadedFrame->Update(dt);
  x6c_cardDriver->Update();

  if (x6c_cardDriver->x10_state == EState::DriverClosed) {
    if (x90_needsDriverReset) {
      ResetCardDriver();
      x90_needsDriverReset = false;
    } else
      x80_iowRet = CIOWin::EMessageReturn::Exit;
  } else if (x6c_cardDriver->x10_state == EState::CardCheckDone && x10_uiType != EUIType::NotOriginalCard) {
    if (x6c_cardDriver->x28_cardSerial != x8_serial) {
      if (x93_inGame) {
        x10_uiType = EUIType::NotOriginalCard;
        x91_uiTextDirty = true;
      } else {
        x8_serial = x6c_cardDriver->x28_cardSerial;
        x6c_cardDriver->IndexFiles();
      }
    } else {
      x6c_cardDriver->IndexFiles();
    }
  } else if (x6c_cardDriver->x10_state == EState::Ready) {
    if (x90_needsDriverReset)
      x6c_cardDriver->StartFileCreateTransactional();
  }

  if (x80_iowRet != CIOWin::EMessageReturn::Normal)
    return x80_iowRet;

  EUIType oldTp = x10_uiType;
  x10_uiType = SelectUIType();
  if (oldTp != x10_uiType || x91_uiTextDirty)
    SetUIText();

  if (x6c_cardDriver->x10_state == EState::NoCard) {
    auto res = CMemoryCardSys::CardProbe(kabufuda::ECardSlot::SlotA);
    if (res.x0_error == CMemoryCardSys::ECardResult::READY || res.x0_error == CMemoryCardSys::ECardResult::WRONGDEVICE)
      ResetCardDriver();
  } else if (x6c_cardDriver->x10_state == EState::CardFormatted) {
    ResetCardDriver();
  } else if (x6c_cardDriver->x10_state == EState::FileBad && x6c_cardDriver->x14_error == EError::FileMissing) {
    x6c_cardDriver->StartFileCreate();
  }

  return CIOWin::EMessageReturn::Normal;
}

bool CSaveGameScreen::PumpLoad() {
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
  x50_loadedFrame->SetAspectConstraint(1.78f);
  x54_textpane_message = static_cast<CGuiTextPane*>(x50_loadedFrame->FindWidget("textpane_message"));
  x58_tablegroup_choices = static_cast<CGuiTableGroup*>(x50_loadedFrame->FindWidget("tablegroup_choices"));
  x5c_textpane_choice0 = static_cast<CGuiTextPane*>(x50_loadedFrame->FindWidget("textpane_choice0"));
  x60_textpane_choice1 = static_cast<CGuiTextPane*>(x50_loadedFrame->FindWidget("textpane_choice1"));
  x64_textpane_choice2 = static_cast<CGuiTextPane*>(x50_loadedFrame->FindWidget("textpane_choice2"));
  x68_textpane_choice3 = static_cast<CGuiTextPane*>(x50_loadedFrame->FindWidget("textpane_choice3"));

  x58_tablegroup_choices->SetMenuAdvanceCallback([this](CGuiTableGroup* caller) { DoAdvance(caller); });
  x58_tablegroup_choices->SetMenuSelectionChangeCallback(
      [this](CGuiTableGroup* caller, int oldSel) { DoSelectionChange(caller, oldSel); });

  if (x0_saveCtx == ESaveContext::InGame)
    x6c_cardDriver->StartCardProbe();

  x10_uiType = SelectUIType();
  SetUIText();
  return true;
}

CSaveGameScreen::EUIType CSaveGameScreen::SelectUIType() const {
  if (x6c_cardDriver->x10_state == EState::NoCard)
    return EUIType::NoCardFound;

  switch (x10_uiType) {
  case EUIType::ProgressWillBeLost:
  case EUIType::NotOriginalCard:
  case EUIType::AllDataWillBeLost:
    return x10_uiType;
  default:
    break;
  }

  if (CMemoryCardDriver::IsCardBusy(x6c_cardDriver->x10_state)) {
    if (CMemoryCardDriver::IsCardWriting(x6c_cardDriver->x10_state))
      return EUIType::BusyWriting;
    return EUIType::BusyReading;
  }

  if (x6c_cardDriver->x10_state == EState::Ready) {
    if (x6c_cardDriver->x14_error == CMemoryCardDriver::EError::CardStillFull)
      return EUIType::StillInsufficientSpace;
    return EUIType::SaveReady;
  }

  if (x6c_cardDriver->x14_error == CMemoryCardDriver::EError::CardBroken)
    return EUIType::NeedsFormatBroken;

  if (x6c_cardDriver->x14_error == CMemoryCardDriver::EError::CardWrongCharacterSet)
    return EUIType::NeedsFormatEncoding;

  if (x6c_cardDriver->x14_error == CMemoryCardDriver::EError::CardWrongDevice)
    return EUIType::WrongDevice;

  if (x6c_cardDriver->x14_error == CMemoryCardDriver::EError::CardFull) {
    if (x6c_cardDriver->x10_state == EState::CardCheckFailed)
      return EUIType::InsufficientSpaceBadCheck;
    return EUIType::InsufficientSpaceOKCheck;
  }

  if (x6c_cardDriver->x14_error == CMemoryCardDriver::EError::CardNon8KSectors)
    return EUIType::IncompatibleCard;

  if (x6c_cardDriver->x14_error == CMemoryCardDriver::EError::FileCorrupted)
    return EUIType::SaveCorrupt;

  if (x6c_cardDriver->x14_error == CMemoryCardDriver::EError::CardIOError)
    return EUIType::CardDamaged;

  return EUIType::Empty;
}

void CSaveGameScreen::SetUIText() {
  x91_uiTextDirty = false;

  s32 msgA = -1;
  s32 msgB = -1;
  s32 opt0 = -1;
  s32 opt1 = -1;
  s32 opt2 = -1;

  std::u16string msgAStr;
  std::u16string msgBStr;
  std::u16string opt0Str;
  std::u16string opt1Str;
  std::u16string opt2Str;
  std::u16string opt3Str;

  switch (x10_uiType) {
  case EUIType::BusyReading:
    msgB = 24; // Reading
    break;
  case EUIType::BusyWriting:
    msgB = 25; // Writing
    break;
  case EUIType::NoCardFound:
    if (g_Main->IsUSA() && !g_Main->IsTrilogy()) {
      msgB = 0;  // No card found
      opt0 = 17; // Continue without saving
      opt1 = 18; // Retry
      opt2 = -2;
      opt2Str = u"Create Dolphin Card";
    } else {
      msgAStr = u"This version of Metroid Prime\nhas a currently unsupported save format.\n";
      msgBStr = u"&push;&main-color=$ff0000ff;Saving has been disabled.&pop;\n";
      opt0 = -2;
      opt0Str = u"Press &image=SI,1.0,0.68,05AF9CAA; to proceed.\n";
    }
    break;
  case EUIType::NeedsFormatBroken:
    msgB = 1;  // Needs format (card broken)
    opt0 = 17; // Continue without saving
    opt1 = 18; // Retry
    opt2 = 20; // Format
    break;
  case EUIType::NeedsFormatEncoding:
    msgB = 2;  // Needs format (wrong char set)
    opt0 = 17; // Continue without saving
    opt1 = 18; // Retry
    opt2 = 20; // Format
    break;
  case EUIType::CardDamaged:
    msgB = 3;  // Damaged
    opt0 = 17; // Continue without saving
    opt1 = 18; // Retry
    break;
  case EUIType::WrongDevice:
    msgB = 5;  // Invalid device
    opt0 = 17; // Continue without saving
    opt1 = 18; // Retry
    break;
  case EUIType::InsufficientSpaceOKCheck:
    msgB = 6;  // Insufficient space (completely filled)
    opt0 = 17; // Continue without saving
    opt1 = 18; // Retry
    opt2 = 19; // Manage memory card
    break;
  case EUIType::InsufficientSpaceBadCheck:
    msgB = bool(x0_saveCtx) + 9; // Insufficient space A or B
    opt0 = 17;                   // Continue without saving
    opt1 = 18;                   // Retry
    opt2 = 19;                   // Manage memory card
    break;
  case EUIType::IncompatibleCard:
    msgB = 7;  // Incompatible card
    opt0 = 17; // Continue without saving
    opt1 = 18; // Retry
    break;
  case EUIType::SaveCorrupt:
    msgB = 4;  // Save corrupt
    opt0 = 22; // Delete corrupt file
    opt1 = 17; // Continue without saving
    opt2 = 18; // Retry
    break;
  case EUIType::StillInsufficientSpace:
    if (x0_saveCtx == ESaveContext::InGame) {
      msgB = 10; // Insufficient space B
      opt0 = 17; // Continue without saving
      opt1 = 18; // Retry
      opt2 = 19; // Manage memory card
    } else {
      msgB = 9;  // Insufficient space A
      opt0 = 17; // Continue without saving
      opt1 = 18; // Retry
      opt2 = 19; // Manage memory card
    }
    break;
  case EUIType::ProgressWillBeLost:
    msgA = 28; // Warning
    msgB = 11; // Progress will be lost
    opt0 = 21; // Cancel
    opt1 = 16; // Continue
    break;
  case EUIType::NotOriginalCard:
    msgA = 28;                                           // Warning
    msgB = 12;                                           // Not the original card
    opt0 = x0_saveCtx == ESaveContext::InGame ? 21 : 17; // Cancel : continue without saving
    opt1 = 16;                                           // Continue
    break;
  case EUIType::AllDataWillBeLost:
    msgA = 28; // Warning
    msgB = 13; // All card data will be erased
    opt0 = 16; // Continue
    opt1 = 21; // Cancel
    break;
  case EUIType::SaveReady:
    if (x0_saveCtx == ESaveContext::InGame) {
      msgB = 8;  // Save progress?
      opt0 = 14; // Yes
      opt1 = 15; // No
    }
    break;
  default:
    break;
  }

  if (msgA > -1)
    msgAStr = x38_strgMemoryCard->GetString(msgA);
  if (msgB > -1)
    msgBStr = x38_strgMemoryCard->GetString(msgB);
  x54_textpane_message->TextSupport().SetText(msgAStr + msgBStr);

  if (opt0 > -1)
    opt0Str = x38_strgMemoryCard->GetString(opt0);
  x5c_textpane_choice0->TextSupport().SetText(opt0Str);

  if (opt1 > -1)
    opt1Str = x38_strgMemoryCard->GetString(opt1);
  x60_textpane_choice1->TextSupport().SetText(opt1Str);

  if (opt2 > -1)
    opt2Str = x38_strgMemoryCard->GetString(opt2);
  x64_textpane_choice2->TextSupport().SetText(opt2Str);

  x68_textpane_choice3->TextSupport().SetText(opt3Str);

  m_touchBar->SetUIOpts(opt0Str, opt1Str, opt2Str);

  x5c_textpane_choice0->SetIsSelectable(opt0 != -1);
  x60_textpane_choice1->SetIsSelectable(opt1 != -1);
  x64_textpane_choice2->SetIsSelectable(opt2 != -1);
  x68_textpane_choice3->SetIsSelectable(false);

  x58_tablegroup_choices->SetIsActive(opt0 != -1 || opt1 != -1 || opt2 != -1);
  x58_tablegroup_choices->SetUserSelection(0);
  SetUIColors();
}

void CSaveGameScreen::SetUIColors() {
  x58_tablegroup_choices->SetColors(zeus::skWhite, zeus::CColor{0.627450f, 0.627450f, 0.627450f, 0.784313f});
}

void CSaveGameScreen::Draw() const {
  SCOPED_GRAPHICS_DEBUG_GROUP("CSaveGameScreen::Draw", zeus::skPurple);
  if (x50_loadedFrame)
    x50_loadedFrame->Draw(CGuiWidgetDrawParms::Default);
}

void CSaveGameScreen::ContinueWithoutSaving() {
  x80_iowRet = CIOWin::EMessageReturn::RemoveIOWin;
  g_GameState->SetCardSerial(0);
}

void CSaveGameScreen::DoAdvance(CGuiTableGroup* caller) {
  int userSel = x58_tablegroup_choices->GetUserSelection();
  int sfx = -1;

  switch (x10_uiType) {
  case EUIType::NoCardFound:
  case EUIType::CardDamaged:
  case EUIType::WrongDevice:
  case EUIType::IncompatibleCard:
    if (userSel == 0) {
      /* Continue without saving */
      if (x0_saveCtx == ESaveContext::InGame)
        x80_iowRet = CIOWin::EMessageReturn::RemoveIOWinAndExit;
      else
        ContinueWithoutSaving();
      sfx = x8c_navBackSfx;
    } else if (userSel == 1) {
      /* Retry */
      ResetCardDriver();
      sfx = x84_navConfirmSfx;
    } else if (userSel == 2 && x10_uiType == EUIType::NoCardFound) {
      /* Create Dolphin Card */
      CMemoryCardSys::CreateDolphinCard(kabufuda::ECardSlot::SlotA);
      ResetCardDriver();
      sfx = x84_navConfirmSfx;
    }
    break;

  case EUIType::NeedsFormatBroken:
  case EUIType::NeedsFormatEncoding:
    if (userSel == 0) {
      /* Continue without saving */
      if (x0_saveCtx == ESaveContext::InGame)
        x80_iowRet = CIOWin::EMessageReturn::RemoveIOWinAndExit;
      else
        ContinueWithoutSaving();
      sfx = x8c_navBackSfx;
    } else if (userSel == 1) {
      /* Retry */
      ResetCardDriver();
      sfx = x84_navConfirmSfx;
    } else if (userSel == 2) {
      /* Format */
      x10_uiType = EUIType::AllDataWillBeLost;
      x91_uiTextDirty = true;
      sfx = x84_navConfirmSfx;
    }
    break;

  case EUIType::InsufficientSpaceBadCheck:
  case EUIType::InsufficientSpaceOKCheck:
    if (userSel == 0) {
      /* Continue without saving */
      if (x0_saveCtx == ESaveContext::InGame)
        x80_iowRet = CIOWin::EMessageReturn::RemoveIOWinAndExit;
      else
        ContinueWithoutSaving();
      sfx = x8c_navBackSfx;
    } else if (userSel == 1) {
      /* Retry */
      ResetCardDriver();
      sfx = x84_navConfirmSfx;
    } else if (userSel == 2) {
      /* Manage memory card */
      if (x0_saveCtx == ESaveContext::InGame) {
        x10_uiType = EUIType::ProgressWillBeLost;
        x91_uiTextDirty = true;
        sfx = x84_navConfirmSfx;
      } else
        static_cast<MP1::CMain*>(g_Main)->SetManageCard(true);
    }
    break;

  case EUIType::SaveCorrupt:
    if (userSel == 0) {
      /* Delete corrupt file */
      x6c_cardDriver->StartFileDeleteBad();
      sfx = x84_navConfirmSfx;
    } else if (userSel == 1) {
      /* Continue without saving */
      if (x0_saveCtx == ESaveContext::InGame)
        x80_iowRet = CIOWin::EMessageReturn::RemoveIOWinAndExit;
      else
        ContinueWithoutSaving();
      sfx = x8c_navBackSfx;
    } else if (userSel == 2) {
      /* Retry */
      ResetCardDriver();
      sfx = x84_navConfirmSfx;
    }
    break;

  case EUIType::StillInsufficientSpace:
    if (x0_saveCtx == ESaveContext::InGame) {
      if (userSel == 0) {
        /* Continue without saving */
        x80_iowRet = CIOWin::EMessageReturn::RemoveIOWinAndExit;
        sfx = x8c_navBackSfx;
      } else if (userSel == 1) {
        /* Retry */
        ResetCardDriver();
        sfx = x84_navConfirmSfx;
      } else if (userSel == 2) {
        /* Manage memory card */
        x10_uiType = EUIType::ProgressWillBeLost;
        x91_uiTextDirty = true;
        sfx = x84_navConfirmSfx;
      }
    } else {
      if (userSel == 0) {
        /* Continue without saving */
        if (x93_inGame) {
          x80_iowRet = CIOWin::EMessageReturn::RemoveIOWinAndExit;
          sfx = x8c_navBackSfx;
        } else {
          x6c_cardDriver->ClearError();
          x92_savingDisabled = true;
          sfx = x84_navConfirmSfx;
        }
      } else if (userSel == 1) {
        /* Retry */
        ResetCardDriver();
        sfx = x84_navConfirmSfx;
      } else if (userSel == 2) {
        /* Manage memory card */
        static_cast<MP1::CMain*>(g_Main)->SetManageCard(true);
      }
    }
    break;

  case EUIType::ProgressWillBeLost:
    if (userSel == 1) {
      /* Continue */
      static_cast<MP1::CMain*>(g_Main)->SetManageCard(true);
    } else if (userSel == 0) {
      /* Cancel */
      x80_iowRet = CIOWin::EMessageReturn::RemoveIOWinAndExit;
      sfx = x8c_navBackSfx;
    }
    break;

  case EUIType::NotOriginalCard:
    if (userSel == 1) {
      /* Continue */
      x8_serial = x6c_cardDriver->x28_cardSerial;
      x10_uiType = EUIType::Empty;
      x6c_cardDriver->IndexFiles();
      sfx = x84_navConfirmSfx;
    } else if (userSel == 0) {
      /* Cancel */
      x80_iowRet = CIOWin::EMessageReturn::RemoveIOWinAndExit;
      sfx = x8c_navBackSfx;
    }
    break;

  case EUIType::AllDataWillBeLost:
    if (userSel == 0) {
      /* Continue */
      x6c_cardDriver->StartCardFormat();
      x10_uiType = EUIType::Empty;
      sfx = x84_navConfirmSfx;
    } else if (userSel == 1) {
      /* Cancel */
      ResetCardDriver();
      sfx = x8c_navBackSfx;
    }
    break;

  case EUIType::SaveReady:
    if (x0_saveCtx == ESaveContext::InGame) {
      if (userSel == 0) {
        /* Yes */
        x6c_cardDriver->BuildExistingFileSlot(g_GameState->GetFileIdx());
        x6c_cardDriver->StartFileCreateTransactional();
        sfx = x84_navConfirmSfx;
      } else if (userSel == 1) {
        /* No */
        x80_iowRet = CIOWin::EMessageReturn::RemoveIOWinAndExit;
        sfx = x8c_navBackSfx;
      }
    }
    break;

  default:
    break;
  }

  if (sfx >= 0)
    CSfxManager::SfxStart(sfx, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
}

void CSaveGameScreen::DoSelectionChange(CGuiTableGroup* caller, int oldSel) {
  SetUIColors();
  CSfxManager::SfxStart(x88_navMoveSfx, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
}

void CSaveGameScreen::ProcessUserInput(const CFinalInput& input) {
  if (x50_loadedFrame) {
    x50_loadedFrame->ProcessUserInput(input);

    int tbOpt = m_touchBar->PopOption();
    if (tbOpt != -1) {
      x58_tablegroup_choices->SetUserSelection(tbOpt);
      SetUIColors();
      DoAdvance(x58_tablegroup_choices);
    }
  }
}

void CSaveGameScreen::StartGame(int idx) {
  const CGameState::GameFileStateInfo* info = x6c_cardDriver->GetGameFileStateInfo(idx);
  x6c_cardDriver->ExportPersistentOptions();
  x6c_cardDriver->BuildNewFileSlot(idx);
  if (!info)
    x6c_cardDriver->StartFileCreateTransactional();
  else
    x80_iowRet = CIOWin::EMessageReturn::Exit;
}

void CSaveGameScreen::SaveNESState() {
  if (!x92_savingDisabled) {
    x90_needsDriverReset = true;
    x8_serial = x6c_cardDriver->x28_cardSerial;
    x6c_cardDriver->StartFileCreateTransactional();
  }
}

void CSaveGameScreen::EraseGame(int idx) {
  x6c_cardDriver->EraseFileSlot(idx);
  x90_needsDriverReset = true;
  x6c_cardDriver->StartFileCreateTransactional();
}

const CGameState::GameFileStateInfo* CSaveGameScreen::GetGameData(int idx) const {
  return x6c_cardDriver->GetGameFileStateInfo(idx);
}

CSaveGameScreen::CSaveGameScreen(ESaveContext saveCtx, u64 serial)
: x0_saveCtx(saveCtx), x8_serial(serial), m_touchBar(NewSaveUITouchBar()) {
  x14_txtrSaveBanner = g_SimplePool->GetObj("TXTR_SaveBanner");
  x20_txtrSaveIcon0 = g_SimplePool->GetObj("TXTR_SaveIcon0");
  x2c_txtrSaveIcon1 = g_SimplePool->GetObj("TXTR_SaveIcon1");
  x38_strgMemoryCard = g_SimplePool->GetObj("STRG_MemoryCard");
  x44_frmeGenericMenu = g_SimplePool->GetObj("FRME_GenericMenu");

  x6c_cardDriver = ConstructCardDriver(x0_saveCtx == ESaveContext::FrontEnd);

  if (saveCtx == ESaveContext::InGame) {
    x84_navConfirmSfx = SFXui_advance;
    x88_navMoveSfx = SFXui_table_selection_change;
    x8c_navBackSfx = SFXui_table_change_mode;
  }
  x93_inGame = bool(saveCtx);

  x70_saveWorlds.reserve(g_MemoryCardSys->GetMemoryWorlds().size());
  for (const std::pair<CAssetId, CSaveWorldMemory>& wld : g_MemoryCardSys->GetMemoryWorlds()) {
    x70_saveWorlds.push_back(g_SimplePool->GetObj(SObjectTag{FOURCC('SAVW'), wld.second.GetSaveWorldAssetId()}));
  }
}

std::unique_ptr<CMemoryCardDriver> CSaveGameScreen::ConstructCardDriver(bool importPersistent) {
  return std::make_unique<CMemoryCardDriver>(kabufuda::ECardSlot::SlotA,
                                             g_ResFactory->GetResourceIdByName("TXTR_SaveBanner")->id,
                                             g_ResFactory->GetResourceIdByName("TXTR_SaveIcon0")->id,
                                             g_ResFactory->GetResourceIdByName("TXTR_SaveIcon1")->id, importPersistent);
}

} // namespace urde::MP1
