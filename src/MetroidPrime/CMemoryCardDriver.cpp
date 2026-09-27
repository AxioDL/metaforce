#include "MetroidPrime/CMemoryCardDriver.hpp"

#include "MetroidPrime/CMain.hpp"

#include "Kyoto/Streams/CMemoryInStream.hpp"

#include "dolphin/os.h"
#include "stdio.h"

static bool lbl_805A9118;
static const char* const skSaveFileNames[2] = {"MetroidPrime A", "MetroidPrime B"};

// Diagnostic names remain in the retail string pool after their users were removed.
static const char* const skStateNames[] = {
    "NotLoaded",
    "Loaded",
    "NoCard",
    "Saved",
    "Formatted",
    "Probed",
    "Mounted",
    "CheckedCard",
    "CreatedInitial",
    "CreatedCopy",
    "WroteCopy",
    "DeletedOriginal",
    "FailedProbe",
    "FailedMount",
    "FailedCheck",
    "FailedDeleteCorruptedFile",
    "FailedDeleteDuplicateFile",
    "FailedLoad",
    "FailedCreateInitial",
    "FailedWriteInitial",
    "FailedCreateCopy",
    "FailedWriteCopy",
    "FailedDeleteOriginal",
    "FailedRenameCopy",
    "FailedFormat",
    "Probing",
    "Mounting",
    "CheckingCard",
    "DeletingCorruptedFile",
    "Reading",
    "DeletingDuplicateFile",
    "CreatingInitial",
    "WritingInitial",
    "CreatingCopy",
    "WritingCopy",
    "DeletingOriginal",
    "RenamingCopy",
    "Formatting",
};

static const char* const skErrorNames[] = {
    "NoError",
    "CorruptedFile",
    "EncodingMismatch",
    "Damaged",
    "WrongDevice",
    "InsufficientSpace",
    "InsufficientBackupSpace",
    "BadSectorSize",
    "NoFile",
    "CorruptedFile",
};

bool CMemoryCardDriver::IsCardBusy(EState v) { return v >= kS_CardMount && v <= kS_CardFormat; }

bool CMemoryCardDriver::IsCardReading(EState v) {
  return (v == kS_CardProbe || v == kS_CardMount) || v == kS_CardCheck || v == kS_FileRead;
}

CMemoryCardDriver::CMemoryCardDriver(CMemoryCardSys::EMemoryCardPort cardPort, CAssetId saveBanner,
                                   CAssetId saveIcon0, CAssetId saveIcon1, const bool importPersistent)
: mCardPort(cardPort)
, mSaveBanner(saveBanner)
, mSaveIcon0(saveIcon0)
, mSaveIcon1(saveIcon1)
, mState(kS_Initial)
, mError(kE_OK)
, mCardFreeBytes(0)
, mCardFreeFiles(0)
, mFileTime(0)
, mCardSerial(0)
, mSystemData(0)
, mFileSlots(nullptr)
, mMcFileInfos()
, mFileIdx(-1)
, mFileInfo(nullptr)
, x19c_(false)
, mImportPersistent(importPersistent) {
  lbl_805A9118 = true;

  mMcFileInfos.push_back(
      SFileInfo(kFS_Unknown, mCardPort, rstl::string_l(skSaveFileNames[0])));

  mMcFileInfos.push_back(
      SFileInfo(kFS_Unknown, mCardPort, rstl::string_l(skSaveFileNames[1])));
}

void CMemoryCardDriver::ClearFileInfo() { mFileInfo = nullptr; }

CMemoryCardDriver::~CMemoryCardDriver() {
  CMemoryCardSys::UnmountCard(mCardPort);
  lbl_805A9118 = false;
  gpMain->SetCardBusy(false);
}

void CMemoryCardDriver::Update() {
  ProbeResults result = CMemoryCardSys::IsMemoryCardInserted(mCardPort);

  if (result.mError == kCR_NOCARD) {
    if (mState != kS_NoCard)
      NoCardFound();
    gpMain->SetCardBusy(false);
    return;
  }

  if (mState == kS_CardProbe) {
    UpdateCardProbe();
    gpMain->SetCardBusy(false);
    return;
  }

  ECardResult resultCode = CMemoryCardSys::GetResultCode(mCardPort);
  bool cardBusy = false;

  if (IsCardBusy(mState)) {
    cardBusy = true;

    switch (mState) {
    case kS_CardProbe:
      break;
    case kS_CardMount:
      UpdateMountCard(resultCode);
      break;
    case kS_CardCheck:
      UpdateCardCheck(resultCode);
      break;
    case kS_FileDeleteBad:
      UpdateFileDeleteBad(resultCode);
      break;
    case kS_FileRead:
      UpdateFileRead(resultCode);
      break;
    case kS_FileDeleteAlt:
      UpdateFileDeleteAlt(resultCode);
      break;
    case kS_FileCreate:
      UpdateFileCreate(resultCode);
      break;
    case kS_FileWrite:
      UpdateFileWrite(resultCode);
      break;
    case kS_FileCreateTransactional:
      UpdateFileCreateTransactional(resultCode);
      break;
    case kS_FileWriteTransactional:
      UpdateFileWriteTransactional(resultCode);
      break;
    case kS_FileAltDeleteTransactional:
      UpdateFileAltDeleteTransactional(resultCode);
      break;
    case kS_FileRenameBtoA:
      UpdateFileRenameBtoA(resultCode);
      break;
    case kS_CardFormat:
      UpdateCardFormat(resultCode);
      break;
    default:
      break;
    }
  }

  gpMain->SetCardBusy(cardBusy);
}

void CMemoryCardDriver::HandleCardError(ECardResult result, EState state) {
  switch (result) {
  case kCR_BUSY:
    break;
  case kCR_WRONGDEVICE:
    mState = state;
    mError = kE_CardWrongDevice;
    break;
  case kCR_NOCARD:
    NoCardFound();
    break;
  case kCR_IOERROR:
    mState = state;
    mError = kE_CardIOError;
    break;
  case kCR_ENCODING:
    mState = state;
    mError = kE_CardWrongCharacterSet;
    break;
  }
}

void CMemoryCardDriver::UpdateMountCard(ECardResult result) {
  if (result == kCR_READY) {
    mState = kS_CardMountDone;
    StartCardCheck();
  } else if (result == kCR_BROKEN) {
    mState = kS_CardMountDone;
    mError = kE_CardBroken;
    StartCardCheck();
  } else {
    HandleCardError(result, kS_CardMountFailed);
  }
}

void CMemoryCardDriver::UpdateCardCheck(ECardResult result) {
  if (result == kCR_READY) {
    mState = kS_CardCheckDone;
    if (!GetCardFreeBytes())
      return;
    if (CMemoryCardSys::GetSerialNo(mCardPort, mCardSerial) == kCR_READY)
      return;
    NoCardFound();

  } else if (result == kCR_BROKEN) {
    mState = kS_CardCheckFailed;
    mError = kE_CardBroken;

  } else {
    HandleCardError(result, kS_CardCheckFailed);
  }
}

void CMemoryCardDriver::UpdateFileRead(ECardResult result) {
  if (result == kCR_READY) {
    ECardResult readRes = mMcFileInfos[mFileIdx].second.TryFileRead();
    if (mMcFileInfos[mFileIdx].second.Close() != kCR_READY) {
      NoCardFound();
      return;
    }

    int altFileIdx;
    if (mFileIdx == 0) {
      altFileIdx = 1;
    } else {
      altFileIdx = 0;
    }
    if (readRes == kCR_READY) {
      mState = kS_Ready;
      ReadFinished();
      EFileState fileSt = mMcFileInfos[altFileIdx].first;
      if (fileSt != kFS_NoFile) {
        StartFileDeleteAlt();
      } else {
        CheckCardCapacity();
      }
      return;
    }

    if (readRes == kCR_CRC_MISMATCH) {
      mMcFileInfos[mFileIdx].first = kFS_BadFile;
      if (mMcFileInfos[altFileIdx].first == kFS_File) {
        mState = kS_CardCheckDone;
        IndexFiles();
      } else {
        mState = kS_FileBad;
        mError = kE_FileCorrupted;
      }
    }
  } else {
    HandleCardError(result, kS_FileBad);
  }
}

void CMemoryCardDriver::UpdateFileDeleteAlt(ECardResult result) {
  if (result == kCR_READY) {
    mState = kS_Ready;
    if (GetCardFreeBytes()) {
      CheckCardCapacity();
    }
  } else {
    HandleCardError(result, kS_FileDeleteAltFailed);
  }
}

void CMemoryCardDriver::UpdateFileDeleteBad(ECardResult result) {
  if (result == kCR_READY) {
    mMcFileInfos[mFileIdx].first = kFS_NoFile;
    if (mMcFileInfos[mFileIdx ? 0 : 1].first == kFS_BadFile) {
      mState = kS_FileBad;
      StartFileDeleteBad();
    } else {
      mState = kS_CardCheckDone;
      if (!GetCardFreeBytes()) {
        return;
      }
      IndexFiles();
    }
  } else {
    HandleCardError(result, kS_FileDeleteBadFailed);
  }
}

void CMemoryCardDriver::UpdateFileCreate(ECardResult result) {
  if (result == kCR_READY) {
    mState = kS_FileCreateDone;
    StartFileWrite();
  } else {
    HandleCardError(result, kS_FileCreateFailed);
  }
}

void CMemoryCardDriver::UpdateFileWrite(ECardResult result) {
  if (result == kCR_READY) {
    ECardResult xferResult = mFileInfo->PumpCardTransfer();
    if (xferResult == kCR_READY) {
      mState = kS_Ready;
      if (mFileInfo->CloseFile() != kCR_READY) {
        NoCardFound();
      }
      return;
    }
    if (xferResult == kCR_BUSY) {
      return;
    }
    if (xferResult == kCR_IOERROR) {
      mState = kS_FileWriteFailed;
      mError = kE_CardIOError;
      return;
    }
    NoCardFound();
  } else {
    HandleCardError(result, kS_FileWriteFailed);
  }
}

void CMemoryCardDriver::UpdateFileCreateTransactional(ECardResult result) {
  if (result == kCR_READY) {
    mState = kS_FileCreateTransactionalDone;
    StartFileWriteTransactional();
  } else {
    HandleCardError(result, kS_FileCreateTransactionalFailed);
  }
}

void CMemoryCardDriver::UpdateFileWriteTransactional(ECardResult result) {
  if (result == kCR_READY) {
    ECardResult xferResult = mFileInfo->PumpCardTransfer();
    if (xferResult == kCR_READY) {
      mState = kS_FileWriteTransactionalDone;
      if (mFileInfo->CloseFile() != kCR_READY) {
        NoCardFound();
      } else {
        StartFileDeleteAltTransactional();
      }
      return;
    }
    if (xferResult == kCR_BUSY) {
      return;
    }
    if (xferResult == kCR_IOERROR) {
      mState = kS_FileWriteTransactionalFailed;
      mError = kE_CardIOError;
      return;
    }
    NoCardFound();
  } else {
    HandleCardError(result, kS_FileWriteTransactionalFailed);
  }
}

void CMemoryCardDriver::UpdateFileRenameBtoA(ECardResult result) {
  if (result == kCR_READY) {
    mState = kS_DriverClosed;
    WriteBackupBuf();
  } else {
    HandleCardError(result, kS_FileRenameBtoAFailed);
  }
}

void CMemoryCardDriver::StartFileRenameBtoA() {
  if (mFileIdx == 1) {
    /* Rename B file to A file (ideally the card is always left with 'A' only) */
    mError = kE_OK;
    mState = kS_FileRenameBtoA;
    int bidx = mFileIdx == 0 ? 1 : 0;
    ECardResult result =
        CMemoryCardSys::Rename(mCardPort, rstl::string_l(skSaveFileNames[mFileIdx]),
                               rstl::string_l(skSaveFileNames[bidx]));
    if (result != kCR_READY) {
      UpdateFileRenameBtoA(result);
    }
  } else {
    mState = kS_DriverClosed;
    WriteBackupBuf();
  }
}

void CMemoryCardDriver::WriteBackupBuf() {
  gpGameState->WriteBackupBuf();
  gpGameState->SetCardSerial(mCardSerial);
}

void CMemoryCardDriver::UpdateFileAltDeleteTransactional(ECardResult result) {
  if (result == kCR_READY) {
    mState = kS_FileAltDeleteTransactionalDone;
    if (GetCardFreeBytes())
      StartFileRenameBtoA();
  } else {
    HandleCardError(result, kS_FileAltDeleteTransactionalFailed);
  }
}

void CMemoryCardDriver::UpdateCardFormat(ECardResult result) {
  if (result == kCR_READY)
    mState = kS_CardFormatted;
  else if (result == kCR_BROKEN) {
    mState = kS_CardFormatFailed;
    mError = kE_CardIOError;
  } else {
    HandleCardError(result, kS_CardFormatFailed);
  }
}

void CMemoryCardDriver::StartCardProbe() {
  mState = kS_CardProbe;
  mError = kE_OK;
  UpdateCardProbe();
}

void CMemoryCardDriver::UpdateCardProbe() {
  ProbeResults result = CMemoryCardSys::IsMemoryCardInserted(mCardPort);
  ECardResult error = result.mError;

  if (error == kCR_READY) {
    if (result.mSectorSize != 0x2000) {
      mState = kS_CardProbeFailed;
      mError = kE_CardNon8KSectors;
      return;
    }
  } else if (error == kCR_BUSY) {
    return;
  } else if (error == kCR_WRONGDEVICE) {
    mState = kS_CardProbeFailed;
    mError = kE_CardWrongDevice;
    return;
  } else {
    NoCardFound();
    return;
  }

  mState = kS_CardProbeDone;
  StartMountCard();
}

void CMemoryCardDriver::StartMountCard() {
  mState = kS_CardMount;
  mError = kE_OK;
  ECardResult result = CMemoryCardSys::MountCard(mCardPort);
  if (result != kCR_READY)
    UpdateMountCard(result);
}

void CMemoryCardDriver::StartCardCheck() {
  mError = kE_OK;
  mState = kS_CardCheck;
  ECardResult result = CMemoryCardSys::CheckCard(mCardPort);
  if (result != kCR_READY)
    UpdateCardCheck(result);
}

void CMemoryCardDriver::ClearError() { mError = kE_OK; }

void CMemoryCardDriver::CheckCardCapacity() {
  if (mCardFreeBytes >= 0x2000 && mCardFreeFiles >= 1) {
    return;
  }
  mError = kE_CardStillFull;
}

void CMemoryCardDriver::NoCardFound() {
  mState = kS_NoCard;
  gpMain->SetCardBusy(false);
}

void CMemoryCardDriver::IndexFiles() {
  mError = kE_OK;
  for (int i = 0; i < mMcFileInfos.capacity(); ++i) {
    SFileInfo& info = mMcFileInfos[i];
    if (info.first == kFS_Unknown) {
      ECardResult result = info.second.Open();
      if (result == kCR_NOFILE) {
        info.first = kFS_NoFile;
        continue;
      } else if (result == kCR_READY) {
        CardStat stat;
        if (CMemoryCardSys::GetStatus(mCardPort, info.second.GetFileNo(), stat) == kCR_READY) {
          int comment = stat.GetCommentAddr();
          if (comment == -1)
            info.first = kFS_BadFile;
          else
            info.first = kFS_File;
        } else {
          NoCardFound();
          return;
        }
        if (info.second.Close() == kCR_NOCARD) {
          NoCardFound();
          return;
        }
      } else {
        NoCardFound();
        return;
      }
    }
  }

  if (mMcFileInfos[0].first == kFS_File) {
    if (mMcFileInfos[1].first == kFS_File) {
      CardStat stat;
      if (CMemoryCardSys::GetStatus(mCardPort, mMcFileInfos[0].second.GetFileNo(), stat) ==
          kCR_READY) {
        u32 timeA = stat.GetTime();
        if (CMemoryCardSys::GetStatus(mCardPort, mMcFileInfos[1].second.GetFileNo(), stat) ==
            kCR_READY) {
          u32 timeB = stat.GetTime();
          if (timeA > timeB)
            mFileIdx = 0;
          else
            mFileIdx = 1;
          StartFileRead();
          return;
        }
        NoCardFound();
        return;
      }
      NoCardFound();
      return;
    }
    mFileIdx = 0;
    StartFileRead();
    return;
  }

  if (mMcFileInfos[1].first == kFS_File) {
    mFileIdx = 1;
    StartFileRead();
    return;
  }

  if (mMcFileInfos[0].first == kFS_BadFile || mMcFileInfos[1].first == kFS_BadFile) {
    mError = kE_FileCorrupted;
    mState = kS_FileBad;
  } else {
    mError = kE_FileMissing;
    mState = kS_FileBad;
  }
}

void CMemoryCardDriver::StartFileDeleteBad() {
  mError = kE_OK;
  mState = kS_FileDeleteBad;

  for (int idx = 0; idx < mMcFileInfos.capacity(); ++idx) {
    SFileInfo& info = mMcFileInfos[idx];
    if (info.first == kFS_BadFile) {
      mFileIdx = idx;
      ECardResult result = CMemoryCardSys::FastDeleteFile(mCardPort, info.second.GetFileNo());
      if (result != kCR_READY) {
        UpdateFileDeleteBad(result);
      }
      return;
    }
  }
}

void CMemoryCardDriver::StartFileDeleteAlt() {
  mError = kE_OK;
  mState = kS_FileDeleteAlt;

  int altFileIdx;
  if (mFileIdx == 0) {
    altFileIdx = 1;
  } else {
    altFileIdx = 0;
  }

  SMemoryCardFileInfo& fileInfo = mMcFileInfos[altFileIdx].second;
  ECardResult result = CMemoryCardSys::FastDeleteFile(mCardPort, fileInfo.GetFileNo());
  if (result != kCR_READY)
    UpdateFileDeleteAlt(result);
}

void CMemoryCardDriver::StartFileRead() {
  mError = kE_OK;
  mState = kS_FileRead;
  ECardResult result = mMcFileInfos[mFileIdx].second.Open();
  if (result != kCR_READY) {
    UpdateFileRead(result);
    return;
  }

  result = mMcFileInfos[mFileIdx].second.StartRead();
  if (result != kCR_READY)
    UpdateFileRead(result);
}

void CMemoryCardDriver::StartFileCreate() {
  mError = kE_OK;
  mState = kS_FileCreate;
  if (mCardFreeBytes < 0x4000 || mCardFreeFiles < 2) {
    mState = kS_FileCreateFailed;
    mError = kE_CardFull;
    return;
  }

  mFileIdx = 0;
  mFileInfo = rs_new CMemoryCardSys::CCardFileInfo(
      mCardPort, rstl::string_l(skSaveFileNames[mFileIdx]));
  InitializeFileInfo();
  ECardResult result = mFileInfo->CreateFile();
  if (result != kCR_READY)
    UpdateFileCreate(result);
}

void CMemoryCardDriver::StartFileWrite() {
  mError = kE_OK;
  mState = kS_FileWrite;
  ECardResult result = mFileInfo->WriteFile();
  if (result != kCR_READY)
    UpdateFileWrite(result);
}

void CMemoryCardDriver::StartFileCreateTransactional() {
  mError = kE_OK;
  mState = kS_FileCreateTransactional;
  ClearFileInfo();
  if (mCardFreeBytes < 8192 || mCardFreeFiles < 1) {
    mState = kS_FileCreateTransactionalFailed;
    mError = kE_CardFull;
    return;
  }

  int altFileIdx;
  if (mFileIdx == 0) {
    altFileIdx = 1;
  } else {
    altFileIdx = 0;
  }

  mFileIdx = altFileIdx;
  mFileInfo = rs_new CMemoryCardSys::CCardFileInfo(
      mCardPort, rstl::string_l(skSaveFileNames[mFileIdx]));
  InitializeFileInfo();
  ECardResult result = mFileInfo->CreateFile();
  if (result != kCR_READY)
    UpdateFileCreateTransactional(result);
}

void CMemoryCardDriver::StartFileWriteTransactional() {
  mError = kE_OK;
  mState = kS_FileWriteTransactional;
  ECardResult result = mFileInfo->WriteFile();
  if (result != kCR_READY)
    UpdateFileWriteTransactional(result);
}

void CMemoryCardDriver::StartFileDeleteAltTransactional() {
  mError = kE_OK;
  mState = kS_FileAltDeleteTransactional;
  int bidx = mFileIdx == 0 ? 1 : 0;
  ECardResult result =
      CMemoryCardSys::DeleteFile(mCardPort, rstl::string_l(skSaveFileNames[bidx]));
  if (result != kCR_READY)
    UpdateFileAltDeleteTransactional(result);
}

void CMemoryCardDriver::StartCardFormat() {
  mError = kE_OK;
  mState = kS_CardFormat;
  ECardResult result = CMemoryCardSys::FormatCard(mCardPort);
  if (result != kCR_READY)
    UpdateCardFormat(result);
}

void CMemoryCardDriver::InitializeFileInfo() {
  CMemoryCardSys::CCardFileInfo& fileInfo = *mFileInfo;
  ExportPersistentOptions();

  const char nameConstant[33] = "Metroid Prime                   ";

  OSCalendarTime time;
  OSTicksToCalendarTime(OSGetTime(), &time);

  char nameBuffer[36];

#if NONMATCHING
  snprintf(nameBuffer, sizeof(nameBuffer), "%02d.%02d.%02d  %02d:%02d", time.mon + 1, time.mday, time.year % 100,
          time.hour, time.min);
#else
  sprintf(nameBuffer, "%02d.%02d.%02d  %02d:%02d", time.mon + 1, time.mday, time.year % 100,
          time.hour, time.min);
#endif

  fileInfo.SetComment(rstl::string_l(nameConstant) + nameBuffer);
  fileInfo.LockBannerToken(mSaveBanner, *gpSimplePool);
  fileInfo.LockIconToken(mSaveIcon0, 2, *gpSimplePool);

  rstl::vector< u8 >& saveBuffer = mFileInfo->SaveBuffer();
  saveBuffer.assign(3004);
  CMemoryStreamOut w(saveBuffer.data(), 3004);

  SSaveHeader header(0);
  for (int i = 0; i < mFileSlots.capacity(); ++i) {
    header.SetSavePresent(i, !mFileSlots[i].null());
  }
  w.Put(header);

  w.Put(mSystemData.data(), mSystemData.capacity());

  for (rstl::reserved_vector< rstl::auto_ptr< SGameFileSlot >, 3 >::iterator it =
           mFileSlots.begin();
       it != mFileSlots.end(); ++it) {
    if (!it->null()) {
      w.Put(**it);
    }
  }
}

void CMemoryCardDriver::ReadFinished() {
  SMemoryCardFileInfo& fileInfo = mMcFileInfos[mFileIdx].second;
  CardStat stat;
  if (CMemoryCardSys::GetStatus(mCardPort, fileInfo.GetFileNo(), stat) != kCR_READY) {
    NoCardFound();
    return;
  }

  mFileTime = stat.GetTime();

  CMemoryInStream r(fileInfo.mSaveData.data(), 3004);
  SSaveHeader header(r);
  r.Get(mSystemData.data(), mSystemData.capacity());

  for (int i = 0; i < mFileSlots.capacity(); ++i) {
    rstl::auto_ptr< SGameFileSlot >& slot = mFileSlots[i];
    if (header.mSavePresent[i]) {
      slot = rs_new SGameFileSlot(r);
    } else {
      slot = nullptr;
    }
  }

  if (mImportPersistent) {
    ImportPersistentOptions();
  }
}

void CMemoryCardDriver::EraseFileSlot(int saveIdx) { mFileSlots[saveIdx] = nullptr; }

void CMemoryCardDriver::BuildNewFileSlot(int saveIdx) {
  bool fusionBackup = gpGameState->SystemState().GetHasFusion();
  gpGameState->SetFileIdx(saveIdx);

  rstl::auto_ptr< SGameFileSlot >& slot = mFileSlots[saveIdx];
  if (slot.null())
    slot = rs_new SGameFileSlot();

  slot->LoadGameState(saveIdx);

  {
    CMemoryInStream r(mSystemData.data(), mSystemData.capacity());
    gpGameState->ReadSystemOptions(r);
  }

  ImportPersistentOptions();
  gpGameState->SetCardSerial(mCardSerial);
  gpGameState->SystemState().SetHasFusion(fusionBackup);
}

void CMemoryCardDriver::BuildExistingFileSlot(int saveIdx) {
  gpGameState->SetFileIdx(saveIdx);

  rstl::auto_ptr< SGameFileSlot >& slot = mFileSlots[saveIdx];
  if (slot.null())
    slot = rs_new SGameFileSlot();
  else
    slot->InitializeFromGameState();

  CMemoryStreamOut w(mSystemData.data(), mSystemData.capacity());
  gpGameState->WriteSystemOptions(w);
}

void CMemoryCardDriver::ImportPersistentOptions() {
  CMemoryInStream r(mSystemData.data(), mSystemData.capacity());
  CSystemState state(r);
  gpGameState->ImportPersistentOptions(state);
}

void CMemoryCardDriver::ExportPersistentOptions() {
  u8* data = mSystemData.data();
  CMemoryInStream r(data, mSystemData.capacity());
  CSystemState state(r);
  gpGameState->ExportPersistentOptions(state);

  CMemoryStreamOut w(data, mSystemData.capacity());
  state.PutTo(w);
}

SSaveHeader::SSaveHeader(int i) : mVersion(i) {}

SSaveHeader::SSaveHeader(CMemoryInStream& in) {
  mVersion = in.ReadLong();
  for (int i = 0; i < 3; ++i) {
    mSavePresent[i] = in.ReadBool();
  }
}

void SSaveHeader::PutTo(COutputStream& out) const {
  out.WriteLong(mVersion);
  for (int i = 0; i < 3; ++i) {
    out.WriteBool(mSavePresent[i]);
  }
}

SGameFileSlot::SGameFileSlot() : mSaveBuffer('\x00') { InitializeFromGameState(); }

SGameFileSlot::SGameFileSlot(CMemoryInStream& in) : mSaveBuffer('\x00') {
  in.Get(mSaveBuffer.data(), mSaveBuffer.capacity());
  mFileInfo = gpGameState->LoadGameFileState(mSaveBuffer.data());
}

void SGameFileSlot::PutTo(COutputStream& w) const {
  w.Put(mSaveBuffer.data(), mSaveBuffer.capacity());
}

void SGameFileSlot::InitializeFromGameState() {
  {
    CMemoryStreamOut w(mSaveBuffer.data(), mSaveBuffer.capacity());
    gpGameState->PutTo(w);
  }
  mFileInfo = CGameState::LoadGameFileState(mSaveBuffer.data());
}

void SGameFileSlot::LoadGameState(int idx) {
  CMemoryInStream r(mSaveBuffer.data(), mSaveBuffer.capacity());
  gpMain->StreamNewGameState(r, idx);
}

const CGameState::GameFileStateInfo* CMemoryCardDriver::GetGameFileStateInfo(int saveIdx) {
  if (mFileSlots[saveIdx].null()) {
    return nullptr;
  }
  return &mFileSlots[saveIdx]->mFileInfo;
};

bool CMemoryCardDriver::GetCardFreeBytes() {
  if (CMemoryCardSys::GetNumFreeBytes(mCardPort, mCardFreeBytes, mCardFreeFiles) !=
      kCR_READY) {
    NoCardFound();
    return false;
  }

  return true;
}
