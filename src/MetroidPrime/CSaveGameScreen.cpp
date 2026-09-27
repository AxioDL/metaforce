#include "MetroidPrime/CSaveGameScreen.hpp"
#include "GuiSys/CGuiFrame.hpp"
#include "GuiSys/CGuiTableGroup.hpp"
#include "GuiSys/CGuiTextPane.hpp"
#include "GuiSys/CGuiWidgetDrawParms.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/Text/CStringTable.hpp"
#include "MetroidPrime/CMain.hpp"
#include "MetroidPrime/CMemoryCard.hpp"
#include "rstl/StringExtras.hpp"

static const char* const skSaveBanner = "TXTR_SaveBanner";
static const char* const skSaveIcon0 = "TXTR_SaveIcon0";
static const char* const skSaveIcon1 = "TXTR_SaveIcon1";
static const char* const skMemoryCardStrings = "STRG_MemoryCard";
static const char* const skGenericMenu = "FRME_GenericMenu";

bool CSaveGameScreen::IsHiddenFromFrontEnd(EUIType type) {
  switch (type) {
  case kUIT_SaveReady:
  case kUIT_Empty:
  case kUIT_BusyReading:
  case kUIT_BusyWriting:
    return false;
  default:
    return true;
  }
}

CSaveGameScreen::EUIType CSaveGameScreen::SelectUIType() const {
  const EState state = mCardDriver->GetState();
  const CMemoryCardDriver::EError error = mCardDriver->GetError();
  if (state == kS_NoCard) {
    return kUIT_NoCardFound;
  }

  if (mUiType == kUIT_ProgressWillBeLost || mUiType == kUIT_AllDataWillBeLost ||
      mUiType == kUIT_NotOriginalCard) {
    return mUiType;
  }

  if (CMemoryCardDriver::IsCardBusy(state)) {
    if (CMemoryCardDriver::IsCardReading(state)) {
      return kUIT_BusyReading;
    }
    return kUIT_BusyWriting;
  }

  if (state == kS_Ready) {
#if VERSION < VERSION_GM8P_00
    if (error == CMemoryCardDriver::kE_CardStillFull) {
      return kUIT_StillInsufficientSpace;
    }
#endif
    return kUIT_SaveReady;
  }

  if (error == CMemoryCardDriver::kE_CardBroken) {
    return kUIT_NeedsFormatBroken;
  }

  if (error == CMemoryCardDriver::kE_CardWrongCharacterSet) {
    return kUIT_NeedsFormatEncoding;
  }

  if (error == CMemoryCardDriver::kE_CardWrongDevice) {
    return kUIT_WrongDevice;
  }

  if (error == CMemoryCardDriver::kE_CardFull) {
#if VERSION < VERSION_GM8P_00
    if (state == kS_FileCreateTransactionalFailed) {
      return kUIT_InsufficientSpaceBadCheck;
    }
#endif
    return kUIT_InsufficientSpaceOKCheck;
  }

  if (error == CMemoryCardDriver::kE_CardNon8KSectors) {
    return kUIT_IncompatibleCard;
  }

  if (error == CMemoryCardDriver::kE_FileCorrupted) {
    return kUIT_SaveCorrupt;
  }

  if (error == CMemoryCardDriver::kE_CardIOError) {
    return kUIT_CardDamaged;
  }

  return kUIT_Empty;
}

void CSaveGameScreen::SetUIText() {
  mUiTextDirty = false;

  const CStringTable& strings = *mStrgMemoryCard.GetObject();
  int msgA = -1;
  int msgB = -1;
  int opt0 = -1;
  int opt1 = -1;
  int opt2 = -1;
  int opt3 = -1;

  switch (mUiType) {
#if VERSION < VERSION_GM8P_00
  case kUIT_BusyReading:
    msgB = 24;
    break;
#endif
  case kUIT_BusyWriting:
    msgB = 25;
    break;
  case kUIT_NoCardFound:
    msgB = 0;
    opt0 = 17;
    opt1 = 18;
    break;
  case kUIT_NeedsFormatBroken:
    msgB = 1;
    opt0 = 17;
    opt1 = 18;
    opt2 = 20;
    break;
  case kUIT_NeedsFormatEncoding:
    msgB = 2;
    opt0 = 17;
    opt1 = 18;
    opt2 = 20;
    break;
  case kUIT_CardDamaged:
    msgB = 3;
    opt0 = 17;
    opt1 = 18;
    break;
  case kUIT_WrongDevice:
    msgB = 5;
    opt0 = 17;
    opt1 = 18;
    break;
  case kUIT_InsufficientSpaceOKCheck:
    msgB = 6;
    opt0 = 17;
    opt1 = 18;
    opt2 = 19;
    break;
#if VERSION < VERSION_GM8P_00
  case kUIT_InsufficientSpaceBadCheck:
    msgB = mSaveCtx == kSC_InGame ? 10 : 9;
    opt0 = 17;
    opt1 = 18;
    opt2 = 19;
    break;
#endif
  case kUIT_IncompatibleCard:
    msgB = 7;
    opt0 = 17;
    opt1 = 18;
    break;
  case kUIT_SaveCorrupt:
    msgB = 4;
    opt0 = 22;
    opt1 = 17;
    opt2 = 18;
    break;
#if VERSION < VERSION_GM8P_00
  case kUIT_StillInsufficientSpace:
    if (mSaveCtx == kSC_InGame) {
      msgB = 10;
      opt0 = 17;
      opt1 = 18;
      opt2 = 19;
    } else {
      msgB = 9;
      opt0 = 17;
      opt1 = 18;
      opt2 = 19;
    }
    break;
#endif
  case kUIT_ProgressWillBeLost:
    msgA = 28;
    msgB = 11;
    opt0 = 21;
    opt1 = 16;
    break;
  case kUIT_NotOriginalCard:
    msgA = 28;
    msgB = 12;
    opt0 = mSaveCtx == kSC_InGame ? 21 : 17;
    opt1 = 16;
    break;
  case kUIT_AllDataWillBeLost:
    msgA = 28;
    msgB = 13;
    opt0 = 16;
    opt1 = 21;
    break;
  case kUIT_SaveReady:
    if (mSaveCtx == kSC_InGame) {
      msgB = 8;
      opt0 = 14;
      opt1 = 15;
    }
    break;
  default:
    break;
  }

  const rstl::wstring empty = rstl::wstring_l(L"");
  const rstl::wstring messageA = msgA == -1 ? empty : rstl::wstring_l(strings.GetString(msgA));
  const rstl::wstring message =
      messageA + (msgB == -1 ? empty : rstl::wstring_l(strings.GetString(msgB)));
  mTextpane_message->TextSupport().SetText(message);
  mTextpane_choice0->TextSupport().SetText(
      opt0 == -1 ? empty : rstl::wstring_l(strings.GetString(opt0)));
  mTextpane_choice1->TextSupport().SetText(
      opt1 == -1 ? empty : rstl::wstring_l(strings.GetString(opt1)));
  mTextpane_choice2->TextSupport().SetText(
      opt2 == -1 ? empty : rstl::wstring_l(strings.GetString(opt2)));
  mTextpane_choice3->TextSupport().SetText(
      opt3 == -1 ? empty : rstl::wstring_l(strings.GetString(opt3)));
  mTextpane_choice0->SetIsSelectable(opt0 != -1);
  mTextpane_choice1->SetIsSelectable(opt1 != -1);
  mTextpane_choice2->SetIsSelectable(opt2 != -1);
  mTextpane_choice3->SetIsSelectable(opt3 != -1);
  mTablegroup_choices->SetUserSelection(0);
  mTablegroup_choices->SetIsActive(opt0 != -1 || opt1 != -1 || opt2 != -1 || opt3 != -1);
  SetUIColors();
}

CMemoryCardDriver* CSaveGameScreen::ConstructCardDriver(bool importPersistent) {
  return rs_new CMemoryCardDriver(
      CMemoryCardSys::kCS_SlotA, gpResourceFactory->GetResourceIdByName(skSaveBanner)->GetId(),
      gpResourceFactory->GetResourceIdByName(skSaveIcon0)->GetId(),
      gpResourceFactory->GetResourceIdByName(skSaveIcon1)->GetId(), importPersistent);
}

CSaveGameScreen::CSaveGameScreen(ESaveContext saveCtx, u64 serial)
: mSaveCtx(saveCtx)
, mSerial(serial)
, mUiType(kUIT_Empty)
, mTxtrSaveBanner(gpSimplePool->GetObj(skSaveBanner))
, mTxtrSaveIcon0(gpSimplePool->GetObj(skSaveIcon0))
, mTxtrSaveIcon1(gpSimplePool->GetObj(skSaveIcon1))
, mStrgMemoryCard(gpSimplePool->GetObj(skMemoryCardStrings))
, mFrmeGenericMenu(gpSimplePool->GetObj(skGenericMenu))
, mLoadedFrame(nullptr)
, mCardDriver(ConstructCardDriver(mSaveCtx == kSC_FrontEnd))
, mIowRet(CIOWin::kMR_Normal)
, mNavConfirmSfx(mSaveCtx == kSC_InGame ? 0x598 : 0x5b4)
, mNavMoveSfx(mSaveCtx == kSC_InGame ? 0x59c : 0x5b5)
, mNavBackSfx(mSaveCtx == kSC_InGame ? 0x597 : 0x5b3)
, mNeedsDriverReset(false)
, mUiTextDirty(false)
, mSavingDisabled(false)
, mInGame(mSaveCtx == kSC_InGame) {
  mTxtrSaveBanner.Lock();
  mTxtrSaveIcon0.Lock();
  mTxtrSaveIcon1.Lock();
  mStrgMemoryCard.Lock();
  mFrmeGenericMenu.Lock();
  const rstl::vector< CMemoryCard::MemoryWorld >& worlds = gpMemoryCard->GetMemoryWorlds();
  mSaveWorlds.reserve(worlds.size());
  for (rstl::vector< CMemoryCard::MemoryWorld >::const_iterator it = worlds.begin();
       it != worlds.end(); ++it) {
    TToken< CWorldSaveGameInfo > token =
        gpSimplePool->GetObj(SObjectTag('SAVW', it->second.GetSaveWorldAssetId()));
    token.Lock();
    mSaveWorlds.push_back(token);
  }
}

CSaveGameScreen::~CSaveGameScreen() {}

void CSaveGameScreen::ResetCardDriver() {
  mSavingDisabled = false;
  mCardDriver = nullptr;
  bool importState = (mSaveCtx == kSC_FrontEnd && !mNeedsDriverReset);
  mCardDriver = ConstructCardDriver(importState);
  mCardDriver->StartCardProbe();
  mUiType = kUIT_Empty;
  SetUIText();
}

bool CSaveGameScreen::PumpLoad() {
  if (mLoadedFrame != nullptr) {
    return true;
  }
  if (!mTxtrSaveBanner.IsLoaded() || !mTxtrSaveIcon0.IsLoaded() ||
      !mTxtrSaveIcon1.IsLoaded() || !mStrgMemoryCard.TryCache()) {
    return false;
  }
  for (AUTO(it, mSaveWorlds.begin()); it != mSaveWorlds.end(); ++it) {
    if (!it->IsLoaded()) {
      return false;
    }
  }
  if (mFrmeGenericMenu.TryCache()) {
    mLoadedFrame = mFrmeGenericMenu.GetObject();
    mTextpane_message =
        static_cast< CGuiTextPane* >(mLoadedFrame->FindWidget("textpane_message"));
    mTablegroup_choices =
        static_cast< CGuiTableGroup* >(mLoadedFrame->FindWidget("tablegroup_choices"));
    mTextpane_choice0 =
        static_cast< CGuiTextPane* >(mLoadedFrame->FindWidget("textpane_choice0"));
    mTextpane_choice1 =
        static_cast< CGuiTextPane* >(mLoadedFrame->FindWidget("textpane_choice1"));
    mTextpane_choice2 =
        static_cast< CGuiTextPane* >(mLoadedFrame->FindWidget("textpane_choice2"));
    mTextpane_choice3 =
        static_cast< CGuiTextPane* >(mLoadedFrame->FindWidget("textpane_choice3"));

    mTablegroup_choices->SetMenuAdvanceCallback(
        TFunctor1FromMethod< CSaveGameScreen, CGuiTableGroup* const >::Make(
            *this, &CSaveGameScreen::DoAdvance));
    mTablegroup_choices->SetMenuSelectionChangeCallback(
        TFunctor2FromMethod< CSaveGameScreen, CGuiTableGroup* const, const int >::Make(
            *this, &CSaveGameScreen::DoSelectionChange));
  } else {
    return false;
  }

  if (mSaveCtx == kSC_InGame) {
    mCardDriver->StartCardProbe();
  }

  mUiType = SelectUIType();
  SetUIText();
  return true;
}

CIOWin::EMessageReturn CSaveGameScreen::Update(float dt) {
  if (!PumpLoad()) {
    return CIOWin::kMR_Normal;
  }

  mLoadedFrame->Update(dt);
  mCardDriver->Update();

  const EState state = mCardDriver->GetState();
  const CMemoryCardDriver::EError error = mCardDriver->GetError();
  if (state == kS_DriverClosed) {
    if (mNeedsDriverReset) {
      ResetCardDriver();
      mNeedsDriverReset = false;
    } else {
      mIowRet = CIOWin::kMR_Exit;
    }
  } else if (state == kS_CardCheckDone && mUiType != kUIT_NotOriginalCard) {
    const u64 cardSerial = mCardDriver->GetCardSerial();
    if (cardSerial != 0 && cardSerial != mSerial) {
      if (mInGame) {
        mUiType = kUIT_NotOriginalCard;
        mUiTextDirty = true;
      } else {
        mSerial = mCardDriver->GetCardSerial();
        mCardDriver->IndexFiles();
      }
    } else {
      mCardDriver->IndexFiles();
    }
  } else if (state == kS_Ready) {
    if (mNeedsDriverReset) {
      mCardDriver->StartFileCreateTransactional();
    }
  }

  if (mIowRet != CIOWin::kMR_Normal) {
    return mIowRet;
  }

  EUIType oldTp = mUiType;
  mUiType = SelectUIType();
  if (oldTp != mUiType || mUiTextDirty) {
    SetUIText();
  }

  if (state == kS_NoCard) {
    const ProbeResults res = CMemoryCardSys::IsMemoryCardInserted(CMemoryCardSys::kCS_SlotA);
    if (res.mError == kCR_READY || res.mError == kCR_WRONGDEVICE) {
      ResetCardDriver();
    }
  } else if (state == kS_CardFormatted) {
    ResetCardDriver();
  } else if (state == kS_FileBad && error == CMemoryCardDriver::kE_FileMissing) {
    mCardDriver->StartFileCreate();
  }

  return CIOWin::kMR_Normal;
}

void CSaveGameScreen::ProcessUserInput(const CFinalInput& input) {
  if (mLoadedFrame != nullptr) {
    mLoadedFrame->ProcessUserInput(input);
  }
}

void CSaveGameScreen::ContinueWithoutSaving() {
  mIowRet = CIOWin::kMR_RemoveIOWin;
  gpGameState->SetCardSerial(0);
}

void CSaveGameScreen::Draw()
#if VERSION != VERSION_GM8J_00
    const
#endif
{
  if (mLoadedFrame != nullptr) {
    mLoadedFrame->Draw(CGuiWidgetDrawParms::Default());
  }
}

const CGameState::GameFileStateInfo* CSaveGameScreen::GetGameData(int idx) const {
  return mCardDriver->GetGameFileStateInfo(idx);
}

void CSaveGameScreen::EraseGame(int idx) {
  mCardDriver->EraseFileSlot(idx);
  mNeedsDriverReset = true;
  mCardDriver->StartFileCreateTransactional();
}

void CSaveGameScreen::SaveNESState() {
  if (!mSavingDisabled) {
    mNeedsDriverReset = true;
    mSerial = mCardDriver->GetCardSerial();
    mCardDriver->StartFileCreateTransactional();
  }
}

void CSaveGameScreen::StartGame(int idx) {
  const bool newGame = mCardDriver->GetGameFileStateInfo(idx) == nullptr;
  mCardDriver->ExportPersistentOptions();
  mCardDriver->BuildNewFileSlot(idx);
  if (newGame) {
    mCardDriver->StartFileCreateTransactional();
  } else {
    mIowRet = CIOWin::kMR_Exit;
  }
}

void CSaveGameScreen::DoAdvance(CGuiTableGroup* caller) {
  int userSel = mTablegroup_choices->GetUserSelection();
  int sfx = -1;

  switch (mUiType) {
  case kUIT_Empty:
  case kUIT_BusyReading:
  case kUIT_BusyWriting:
    break;
  case kUIT_NoCardFound:
  case kUIT_CardDamaged:
  case kUIT_WrongDevice:
  case kUIT_IncompatibleCard:
    if (userSel == 0) {
      if (mSaveCtx == kSC_InGame) {
        mIowRet = CIOWin::kMR_RemoveIOWinAndExit;
      } else {
        ContinueWithoutSaving();
      }
      sfx = mNavBackSfx;
    } else if (userSel == 1) {
      ResetCardDriver();
      sfx = mNavConfirmSfx;
    }
    break;

  case kUIT_NeedsFormatBroken:
  case kUIT_NeedsFormatEncoding:
    if (userSel == 0) {
      if (mSaveCtx == kSC_InGame) {
        mIowRet = CIOWin::kMR_RemoveIOWinAndExit;
      } else {
        ContinueWithoutSaving();
      }
      sfx = mNavBackSfx;
    } else if (userSel == 1) {
      ResetCardDriver();
      sfx = mNavConfirmSfx;
    } else if (userSel == 2) {
      mUiType = kUIT_AllDataWillBeLost;
      mUiTextDirty = true;
      sfx = mNavConfirmSfx;
    }
    break;

#if VERSION < VERSION_GM8P_00
  case kUIT_InsufficientSpaceBadCheck:
#endif
  case kUIT_InsufficientSpaceOKCheck:
    if (userSel == 0) {
      if (mSaveCtx == kSC_InGame) {
        mIowRet = CIOWin::kMR_RemoveIOWinAndExit;
      } else {
        ContinueWithoutSaving();
      }
      sfx = mNavBackSfx;
    } else if (userSel == 1) {
      ResetCardDriver();
      sfx = mNavConfirmSfx;
    } else if (userSel == 2) {
      if (mSaveCtx == kSC_InGame) {
        mUiType = kUIT_ProgressWillBeLost;
        mUiTextDirty = true;
        sfx = mNavConfirmSfx;
      } else {
        gpMain->SetManageCard(true);
      }
    }
    break;

  case kUIT_SaveCorrupt:
    if (userSel == 0) {
      mCardDriver->StartFileDeleteBad();
      sfx = mNavConfirmSfx;
    } else if (userSel == 1) {
      if (mSaveCtx == kSC_InGame) {
        mIowRet = CIOWin::kMR_RemoveIOWinAndExit;
      } else {
        ContinueWithoutSaving();
      }
      sfx = mNavBackSfx;
    } else if (userSel == 2) {
      ResetCardDriver();
      sfx = mNavConfirmSfx;
    }
    break;

#if VERSION < VERSION_GM8P_00
  case kUIT_StillInsufficientSpace:
    if (mSaveCtx == kSC_InGame) {
      if (userSel == 0) {
        mIowRet = CIOWin::kMR_RemoveIOWinAndExit;
        sfx = mNavBackSfx;
      } else if (userSel == 1) {
        ResetCardDriver();
        sfx = mNavConfirmSfx;
      } else if (userSel == 2) {
        mUiType = kUIT_ProgressWillBeLost;
        mUiTextDirty = true;
        sfx = mNavConfirmSfx;
      }
    } else {
      if (userSel == 0) {
        if (mInGame) {
          mIowRet = CIOWin::kMR_RemoveIOWinAndExit;
          sfx = mNavBackSfx;
        } else {
          mCardDriver->ClearError();
          mSavingDisabled = true;
          sfx = mNavConfirmSfx;
        }
      } else if (userSel == 1) {
        ResetCardDriver();
        sfx = mNavConfirmSfx;
      } else if (userSel == 2) {
        gpMain->SetManageCard(true);
      }
    }
    break;

#endif
  case kUIT_ProgressWillBeLost:
    if (userSel == 1) {
      gpMain->SetManageCard(true);
    } else if (userSel == 0) {
      mIowRet = CIOWin::kMR_RemoveIOWinAndExit;
      sfx = mNavBackSfx;
    }
    break;

  case kUIT_NotOriginalCard:
    if (userSel == 1) {
      mSerial = mCardDriver->GetCardSerial();
      mUiType = kUIT_Empty;
      mCardDriver->IndexFiles();
      sfx = mNavConfirmSfx;
    } else if (userSel == 0) {
      mIowRet = CIOWin::kMR_RemoveIOWinAndExit;
      sfx = mNavBackSfx;
    }
    break;

  case kUIT_AllDataWillBeLost:
    if (userSel == 0) {
      mCardDriver->StartCardFormat();
      mUiType = kUIT_Empty;
      sfx = mNavConfirmSfx;
    } else if (userSel == 1) {
      ResetCardDriver();
      sfx = mNavBackSfx;
    }
    break;

  case kUIT_SaveReady:
    if (mSaveCtx != kSC_FrontEnd) {
      if (userSel == 0) {
        mCardDriver->BuildExistingFileSlot(gpGameState->GetFileIdx());
        mCardDriver->StartFileCreateTransactional();
        sfx = mNavConfirmSfx;
      } else if (userSel == 1) {
        mIowRet = CIOWin::kMR_RemoveIOWinAndExit;
        sfx = mNavBackSfx;
      }
    }
    break;
  default:
    break;
  }

  if (sfx >= 0) {
    CSfxManager::SfxStart(sfx, 0x7f, 0x40, false, CSfxManager::kMedPriority, false,
                          CSfxManager::kAllAreas);
  }
}

void CSaveGameScreen::DoSelectionChange(CGuiTableGroup* caller, int oldSelection) {
  SetUIColors();
  CSfxManager::SfxStart(mNavMoveSfx, 0x7f, 0x40, false, CSfxManager::kMedPriority, false,
                        CSfxManager::kAllAreas);
}

void CSaveGameScreen::SetUIColors() {
  const CColor selected(0xffffffff);
  const CColor unselected(uchar(160), uchar(160), uchar(160), uchar(200));
  mTablegroup_choices->SetColors(selected, unselected);
}

void CSaveGameScreen::SetInGame(bool inGame) { mInGame = inGame; }
