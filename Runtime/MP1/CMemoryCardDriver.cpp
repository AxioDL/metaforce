#include "CMemoryCardDriver.hpp"
#include "MP1.hpp"
#include "CCRC32.hpp"

namespace urde::MP1 {

static const char* SaveFileNames[] = {"MetroidPrime A", "MetroidPrime B"};

using ECardResult = kabufuda::ECardResult;
using ECardSlot = kabufuda::ECardSlot;

ECardResult CMemoryCardDriver::SFileInfo::Open() {
  return CMemoryCardSys::OpenFile(GetFileCardPort(), x14_name.c_str(), x0_fileInfo);
}

ECardResult CMemoryCardDriver::SFileInfo::Close() { return CMemoryCardSys::CloseFile(x0_fileInfo); }

ECardResult CMemoryCardDriver::SFileInfo::StartRead() {
  CMemoryCardSys::CardStat stat = {};
  ECardResult result = CMemoryCardSys::GetStatus(x0_fileInfo.slot, x0_fileInfo.getFileNo(), stat);
  if (result != ECardResult::READY)
    return result;

  u32 length = stat.GetFileLength();
  x34_saveData.clear();
  x24_saveFileData.resize(length);
  return CMemoryCardSys::ReadFile(x0_fileInfo, x24_saveFileData.data(), length, 0);
}

ECardResult CMemoryCardDriver::SFileInfo::TryFileRead() {
  ECardResult res = CMemoryCardSys::GetResultCode(GetFileCardPort());
  if (res == ECardResult::READY)
    res = FileRead();
  return res;
}

ECardResult CMemoryCardDriver::SFileInfo::FileRead() {
  x34_saveData.clear();
  u32 existingCrc = hecl::SBig(*reinterpret_cast<u32*>(x24_saveFileData.data()));
  u32 newCrc = CCRC32::Calculate(x24_saveFileData.data() + 4, x24_saveFileData.size() - 4);
  if (existingCrc == newCrc) {
    u32 saveDataOff;
    ECardResult result = GetSaveDataOffset(saveDataOff);
    if (result != ECardResult::READY) {
      x24_saveFileData.clear();
      return result;
    }

    u32 saveSize = x24_saveFileData.size() - saveDataOff;
    x34_saveData.resize(saveSize);
    memmove(x34_saveData.data(), x24_saveFileData.data() + saveDataOff, saveSize);
    x24_saveFileData.clear();
    return ECardResult::READY;
  } else {
    x24_saveFileData.clear();
    return ECardResult::CRC_MISMATCH;
  }
}

ECardResult CMemoryCardDriver::SFileInfo::GetSaveDataOffset(u32& offOut) const {
  CMemoryCardSys::CardStat stat = {};
  ECardResult result = CMemoryCardSys::GetStatus(x0_fileInfo.slot, x0_fileInfo.getFileNo(), stat);
  if (result != ECardResult::READY) {
    offOut = -1;
    return result;
  }

  offOut = 4;
  offOut += 64;
  switch (stat.GetBannerFormat()) {
  case kabufuda::EImageFormat::C8:
    offOut += 3584;
    break;
  case kabufuda::EImageFormat::RGB5A3:
    offOut += 6144;
    break;
  default:
    break;
  }

  int idx = 0;
  bool paletteIcon = false;
  for (kabufuda::EImageFormat fmt = stat.GetIconFormat(idx); fmt != kabufuda::EImageFormat::None;
       fmt = stat.GetIconFormat(idx)) {
    if (fmt == kabufuda::EImageFormat::C8) {
      paletteIcon = true;
      offOut += 1024;
    } else
      offOut += 2048;
    ++idx;
  }

  if (paletteIcon)
    offOut += 512;

  return ECardResult::READY;
}

CMemoryCardDriver::SGameFileSlot::SGameFileSlot() { InitializeFromGameState(); }

CMemoryCardDriver::SGameFileSlot::SGameFileSlot(CMemoryInStream& in) {
  in.readBytesToBuf(x0_saveBuffer, 940);
  x944_fileInfo = CGameState::LoadGameFileState(x0_saveBuffer);
}

void CMemoryCardDriver::SGameFileSlot::InitializeFromGameState() {
  CBitStreamWriter w(x0_saveBuffer, 940);
  g_GameState->PutTo(w);
  w.Flush();
  x944_fileInfo = CGameState::LoadGameFileState(x0_saveBuffer);
}

void CMemoryCardDriver::SGameFileSlot::LoadGameState(u32 idx) {
  CBitStreamReader r(x0_saveBuffer, 940);
  static_cast<MP1::CMain*>(g_Main)->StreamNewGameState(r, idx);
}

CMemoryCardDriver::SFileInfo::SFileInfo(kabufuda::ECardSlot port, std::string_view name)
: x0_fileInfo(port), x14_name(name) {}

CMemoryCardDriver::CMemoryCardDriver(kabufuda::ECardSlot cardPort, CAssetId saveBanner, CAssetId saveIcon0,
                                     CAssetId saveIcon1, bool importPersistent)
: x0_cardPort(cardPort)
, x4_saveBanner(saveBanner)
, x8_saveIcon0(saveIcon0)
, xc_saveIcon1(saveIcon1)
, x19d_importPersistent(importPersistent) {
  x100_mcFileInfos.reserve(2);
  x100_mcFileInfos.emplace_back(EFileState::Unknown, SFileInfo(x0_cardPort, SaveFileNames[0]));
  x100_mcFileInfos.emplace_back(EFileState::Unknown, SFileInfo(x0_cardPort, SaveFileNames[1]));
}

void CMemoryCardDriver::NoCardFound() {
  x10_state = EState::NoCard;
  static_cast<CMain*>(g_Main)->SetCardBusy(false);
}

const CGameState::GameFileStateInfo* CMemoryCardDriver::GetGameFileStateInfo(int idx) const {
  SGameFileSlot* slot = xe4_fileSlots[idx].get();
  if (!slot)
    return nullptr;
  return &slot->x944_fileInfo;
}

CMemoryCardDriver::SSaveHeader CMemoryCardDriver::LoadSaveHeader(CMemoryInStream& in) {
  SSaveHeader ret;
  ret.x0_version = in.readUint32Big();
  for (int i = 0; i < 3; ++i)
    ret.x4_savePresent[i] = in.readBool();
  return ret;
}

std::unique_ptr<CMemoryCardDriver::SGameFileSlot> CMemoryCardDriver::LoadSaveFile(CMemoryInStream& in) {
  return std::make_unique<CMemoryCardDriver::SGameFileSlot>(in);
}

void CMemoryCardDriver::ReadFinished() {
  CMemoryCardSys::CardStat stat = {};
  SFileInfo& fileInfo = x100_mcFileInfos[x194_fileIdx].second;
  if (CMemoryCardSys::GetStatus(fileInfo.x0_fileInfo.slot, fileInfo.x0_fileInfo.getFileNo(), stat) !=
      ECardResult::READY) {
    NoCardFound();
    return;
  }

  x20_fileTime = stat.GetTime();
  CMemoryInStream r(fileInfo.x34_saveData.data(), 3004);
  SSaveHeader header = LoadSaveHeader(r);
  r.readBytesToBuf(x30_systemData, 174);

  for (int i = 0; i < 3; ++i)
    if (header.x4_savePresent[i])
      xe4_fileSlots[i] = LoadSaveFile(r);

  if (x19d_importPersistent)
    ImportPersistentOptions();
}

void CMemoryCardDriver::ImportPersistentOptions() {
  CBitStreamReader r(x30_systemData, 174);
  CPersistentOptions opts(r);
  g_GameState->ImportPersistentOptions(opts);
}

void CMemoryCardDriver::ExportPersistentOptions() {
  CBitStreamReader r(x30_systemData, 174);
  CPersistentOptions opts(r);
  g_GameState->ExportPersistentOptions(opts);
  CBitStreamWriter w(x30_systemData, 174);
  opts.PutTo(w);
}

void CMemoryCardDriver::CheckCardCapacity() {
  if (x18_cardFreeBytes < 0x2000 || !x1c_cardFreeFiles)
    x14_error = EError::CardStillFull;
}

void CMemoryCardDriver::IndexFiles() {
  x14_error = EError::OK;
  for (std::pair<EFileState, SFileInfo>& info : x100_mcFileInfos) {
    if (info.first == EFileState::Unknown) {
      ECardResult result = info.second.Open();
      if (result == ECardResult::NOFILE) {
        info.first = EFileState::NoFile;
        continue;
      } else if (result == ECardResult::READY) {
        CMemoryCardSys::CardStat stat = {};
        if (CMemoryCardSys::GetStatus(info.second.x0_fileInfo.slot, info.second.x0_fileInfo.getFileNo(), stat) ==
            ECardResult::READY) {
          u32 comment = stat.GetCommentAddr();
          if (comment == UINT32_MAX)
            info.first = EFileState::BadFile;
          else
            info.first = EFileState::File;
        } else {
          NoCardFound();
          return;
        }
        if (info.second.Close() == ECardResult::NOCARD) {
          NoCardFound();
          return;
        }
      } else {
        NoCardFound();
        return;
      }
    }
  }

  if (x100_mcFileInfos[0].first == EFileState::File) {
    if (x100_mcFileInfos[1].first == EFileState::File) {
      CMemoryCardSys::CardStat stat = {};
      if (CMemoryCardSys::GetStatus(x100_mcFileInfos[0].second.x0_fileInfo.slot,
                                    x100_mcFileInfos[0].second.x0_fileInfo.getFileNo(), stat) == ECardResult::READY) {
        u32 timeA = stat.GetTime();
        if (CMemoryCardSys::GetStatus(x100_mcFileInfos[1].second.x0_fileInfo.slot,
                                      x100_mcFileInfos[1].second.x0_fileInfo.getFileNo(), stat) == ECardResult::READY) {
          u32 timeB = stat.GetTime();
          if (timeA > timeB)
            x194_fileIdx = 0;
          else
            x194_fileIdx = 1;
          StartFileRead();
          return;
        }
        NoCardFound();
        return;
      }
      NoCardFound();
      return;
    }
    x194_fileIdx = 0;
    StartFileRead();
    return;
  }

  if (x100_mcFileInfos[1].first == EFileState::File) {
    x194_fileIdx = 1;
    StartFileRead();
    return;
  }

  if (x100_mcFileInfos[0].first == EFileState::BadFile || x100_mcFileInfos[1].first == EFileState::BadFile) {
    x10_state = EState::FileBad;
    x14_error = EError::FileCorrupted;
  } else {
    x10_state = EState::FileBad;
    x14_error = EError::FileMissing;
  }
}

void CMemoryCardDriver::StartCardProbe() {
  x10_state = EState::CardProbe;
  x14_error = EError::OK;
  UpdateCardProbe();
}

void CMemoryCardDriver::StartMountCard() {
  x10_state = EState::CardMount;
  x14_error = EError::OK;
  ECardResult result = CMemoryCardSys::MountCard(x0_cardPort);
  if (result != ECardResult::READY)
    UpdateMountCard(result);
}

void CMemoryCardDriver::StartCardCheck() {
  x14_error = EError::OK;
  x10_state = EState::CardCheck;
  ECardResult result = CMemoryCardSys::CheckCard(x0_cardPort);
  if (result != ECardResult::READY)
    UpdateCardCheck(result);
}

void CMemoryCardDriver::StartFileDeleteBad() {
  x14_error = EError::OK;
  x10_state = EState::FileDeleteBad;
  int idx = 0;
  for (std::pair<EFileState, SFileInfo>& info : x100_mcFileInfos) {
    if (info.first == EFileState::BadFile) {
      x194_fileIdx = idx;
      ECardResult result = CMemoryCardSys::FastDeleteFile(x0_cardPort, info.second.GetFileNo());
      if (result != ECardResult::READY) {
        UpdateFileDeleteBad(result);
        return;
      }
    }
    ++idx;
  }
}

void CMemoryCardDriver::StartFileRead() {
  x14_error = EError::OK;
  x10_state = EState::FileRead;
  ECardResult result = x100_mcFileInfos[x194_fileIdx].second.Open();
  if (result != ECardResult::READY) {
    UpdateFileRead(result);
    return;
  }

  result = x100_mcFileInfos[x194_fileIdx].second.StartRead();
  if (result != ECardResult::READY)
    UpdateFileRead(result);
}

void CMemoryCardDriver::StartFileDeleteAlt() {
  x14_error = EError::OK;
  x10_state = EState::FileDeleteAlt;
  SFileInfo& fileInfo = x100_mcFileInfos[!bool(x194_fileIdx)].second;
  ECardResult result = CMemoryCardSys::FastDeleteFile(x0_cardPort, fileInfo.GetFileNo());
  if (result != ECardResult::READY)
    UpdateFileDeleteAlt(result);
}

void CMemoryCardDriver::StartFileCreate() {
  x14_error = EError::OK;
  x10_state = EState::FileCreate;
  if (x18_cardFreeBytes < 8192 || x1c_cardFreeFiles < 2) {
    x10_state = EState::FileCreateFailed;
    x14_error = EError::CardFull;
    return;
  }

  x194_fileIdx = 0;
  x198_fileInfo = std::make_unique<CMemoryCardSys::CCardFileInfo>(x0_cardPort, SaveFileNames[x194_fileIdx]);
  InitializeFileInfo();
  ECardResult result = x198_fileInfo->CreateFile();
  if (result != ECardResult::READY)
    UpdateFileCreate(result);
}

void CMemoryCardDriver::StartFileWrite() {
  x14_error = EError::OK;
  x10_state = EState::FileWrite;
  ECardResult result = x198_fileInfo->WriteFile();
  if (result != ECardResult::READY)
    UpdateFileWrite(result);
}

void CMemoryCardDriver::StartFileCreateTransactional() {
  x14_error = EError::OK;
  x10_state = EState::FileCreateTransactional;
  ClearFileInfo();
  if (x18_cardFreeBytes < 8192 || !x1c_cardFreeFiles) {
    x10_state = EState::FileCreateTransactionalFailed;
    x14_error = EError::CardFull;
    return;
  }

  x194_fileIdx = !bool(x194_fileIdx);
  x198_fileInfo = std::make_unique<CMemoryCardSys::CCardFileInfo>(x0_cardPort, SaveFileNames[x194_fileIdx]);
  InitializeFileInfo();
  ECardResult result = x198_fileInfo->CreateFile();
  if (result != ECardResult::READY)
    UpdateFileCreateTransactional(result);
}

void CMemoryCardDriver::StartFileWriteTransactional() {
  x14_error = EError::OK;
  x10_state = EState::FileWriteTransactional;
  ECardResult result = x198_fileInfo->WriteFile();
  if (result != ECardResult::READY)
    UpdateFileWriteTransactional(result);
}

void CMemoryCardDriver::StartFileDeleteAltTransactional() {
  x14_error = EError::OK;
  x10_state = EState::FileDeleteAltTransactional;
  ECardResult result = CMemoryCardSys::DeleteFile(x0_cardPort, SaveFileNames[!bool(x194_fileIdx)]);
  if (result != ECardResult::READY)
    UpdateFileDeleteAltTransactional(result);
}

void CMemoryCardDriver::StartFileRenameBtoA() {
  if (x194_fileIdx == 1) {
    /* Rename B file to A file (ideally the card is always left with 'A' only) */
    x14_error = EError::OK;
    x10_state = EState::FileRenameBtoA;
    ECardResult result =
        CMemoryCardSys::Rename(x0_cardPort, SaveFileNames[x194_fileIdx], SaveFileNames[!bool(x194_fileIdx)]);
    if (result != ECardResult::READY)
      UpdateFileRenameBtoA(result);
  } else {
    x10_state = EState::DriverClosed;
    WriteBackupBuf();
  }
}

void CMemoryCardDriver::StartCardFormat() {
  x14_error = EError::OK;
  x10_state = EState::CardFormat;
  ECardResult result = CMemoryCardSys::FormatCard(x0_cardPort);
  if (result != ECardResult::READY)
    UpdateCardFormat(result);
}

void CMemoryCardDriver::UpdateMountCard(ECardResult result) {
  switch (result) {
  case ECardResult::READY:
    x10_state = EState::CardMountDone;
    StartCardCheck();
    break;
  case ECardResult::BROKEN:
    x10_state = EState::CardMountDone;
    x14_error = EError::CardBroken;
    // StartCardCheck();
    break;
  default:
    HandleCardError(result, EState::CardMountFailed);
    break;
  }
}

void CMemoryCardDriver::UpdateCardProbe() {
  auto result = CMemoryCardSys::CardProbe(x0_cardPort);
  switch (result.x0_error) {
  case ECardResult::READY:
    if (result.x8_sectorSize != 0x2000) {
      x10_state = EState::CardProbeFailed;
      x14_error = EError::CardNon8KSectors;
      return;
    }
    x10_state = EState::CardProbeDone;
    StartMountCard();
    break;
  case ECardResult::BUSY:
    break;
  case ECardResult::WRONGDEVICE:
    x10_state = EState::CardProbeFailed;
    x14_error = EError::CardWrongDevice;
    break;
  default:
    NoCardFound();
    break;
  }
}

void CMemoryCardDriver::UpdateCardCheck(ECardResult result) {
  switch (result) {
  case ECardResult::READY:
    x10_state = EState::CardCheckDone;
    if (!GetCardFreeBytes())
      return;
    if (CMemoryCardSys::GetSerialNo(x0_cardPort, x28_cardSerial) == ECardResult::READY)
      return;
    NoCardFound();
    break;
  case ECardResult::BROKEN:
    x10_state = EState::CardCheckFailed;
    x14_error = EError::CardBroken;
    break;
  default:
    HandleCardError(result, EState::CardCheckFailed);
  }
}

void CMemoryCardDriver::UpdateFileDeleteBad(ECardResult result) {
  if (result == ECardResult::READY) {
    x100_mcFileInfos[x194_fileIdx].first = EFileState::NoFile;
    if (x100_mcFileInfos[!bool(x194_fileIdx)].first == EFileState::BadFile) {
      x10_state = EState::FileBad;
      StartFileDeleteBad();
    } else {
      x10_state = EState::CardCheckDone;
      if (!GetCardFreeBytes())
        return;
      IndexFiles();
    }
  } else
    HandleCardError(result, EState::FileDeleteBadFailed);
}

void CMemoryCardDriver::UpdateFileRead(ECardResult result) {
  if (result == ECardResult::READY) {
    auto& fileInfo = x100_mcFileInfos[x194_fileIdx];
    ECardResult readRes = fileInfo.second.TryFileRead();
    if (fileInfo.second.Close() != ECardResult::READY) {
      NoCardFound();
      return;
    }

    u32 altFileIdx = !bool(x194_fileIdx);
    if (readRes == ECardResult::READY) {
      x10_state = EState::Ready;
      ReadFinished();
      EFileState fileSt = x100_mcFileInfos[altFileIdx].first;
      if (fileSt == EFileState::NoFile)
        CheckCardCapacity();
      else
        StartFileDeleteAlt();
      return;
    }

    if (readRes == ECardResult::CRC_MISMATCH) {
      x100_mcFileInfos[x194_fileIdx].first = EFileState::BadFile;
      if (x100_mcFileInfos[altFileIdx].first == EFileState::File) {
        x10_state = EState::CardCheckDone;
        IndexFiles();
      } else {
        x10_state = EState::FileBad;
        x14_error = EError::FileCorrupted;
      }
    }
  } else
    HandleCardError(result, EState::FileBad);
}

void CMemoryCardDriver::UpdateFileDeleteAlt(ECardResult result) {
  if (result == ECardResult::READY) {
    x10_state = EState::Ready;
    if (GetCardFreeBytes())
      CheckCardCapacity();
  } else
    HandleCardError(result, EState::FileDeleteAltFailed);
}

void CMemoryCardDriver::UpdateFileCreate(ECardResult result) {
  if (result == ECardResult::READY) {
    x10_state = EState::FileCreateDone;
    StartFileWrite();
  } else
    HandleCardError(result, EState::FileCreateFailed);
}

void CMemoryCardDriver::UpdateFileWrite(ECardResult result) {
  if (result == ECardResult::READY) {
    ECardResult xferResult = x198_fileInfo->PumpCardTransfer();
    if (xferResult == ECardResult::READY) {
      x10_state = EState::Ready;
      if (x198_fileInfo->CloseFile() == ECardResult::READY) {
        CMemoryCardSys::CommitToDisk(x0_cardPort);
        return;
      }
      NoCardFound();
      return;
    }
    if (xferResult == ECardResult::BUSY)
      return;
    if (xferResult == ECardResult::IOERROR) {
      x10_state = EState::FileWriteFailed;
      x14_error = EError::CardIOError;
      return;
    }
    NoCardFound();
  } else
    HandleCardError(result, EState::FileWriteFailed);
}

void CMemoryCardDriver::UpdateFileCreateTransactional(ECardResult result) {
  if (result == ECardResult::READY) {
    x10_state = EState::FileCreateTransactionalDone;
    StartFileWriteTransactional();
  } else
    HandleCardError(result, EState::FileCreateTransactionalFailed);
}

void CMemoryCardDriver::UpdateFileWriteTransactional(ECardResult result) {
  if (result == ECardResult::READY) {
    ECardResult xferResult = x198_fileInfo->PumpCardTransfer();
    if (xferResult == ECardResult::READY) {
      x10_state = EState::FileWriteTransactionalDone;
      if (x198_fileInfo->CloseFile() != ECardResult::READY) {
        NoCardFound();
        return;
      }
      StartFileDeleteAltTransactional();
      return;
    }
    if (xferResult == ECardResult::BUSY)
      return;
    if (xferResult == ECardResult::IOERROR) {
      x10_state = EState::FileWriteTransactionalFailed;
      x14_error = EError::CardIOError;
      return;
    }
    NoCardFound();
  } else
    HandleCardError(result, EState::FileWriteTransactionalFailed);
}

void CMemoryCardDriver::UpdateFileDeleteAltTransactional(ECardResult result) {
  if (result == ECardResult::READY) {
    x10_state = EState::FileDeleteAltTransactionalDone;
    if (GetCardFreeBytes())
      StartFileRenameBtoA();
  } else
    HandleCardError(result, EState::FileDeleteAltTransactionalFailed);
}

void CMemoryCardDriver::UpdateFileRenameBtoA(ECardResult result) {
  if (result == ECardResult::READY) {
    x10_state = EState::DriverClosed;
    CMemoryCardSys::CommitToDisk(x0_cardPort);
    WriteBackupBuf();
  } else
    HandleCardError(result, EState::FileRenameBtoAFailed);
}

void CMemoryCardDriver::UpdateCardFormat(ECardResult result) {
  if (result == ECardResult::READY)
    x10_state = EState::CardFormatted;
  else if (result == ECardResult::BROKEN) {
    x10_state = EState::CardFormatFailed;
    x14_error = EError::CardIOError;
  } else
    HandleCardError(result, EState::CardFormatFailed);
}

void CMemoryCardDriver::BuildNewFileSlot(u32 saveIdx) {
  g_GameState->SetFileIdx(saveIdx);
  bool fusionBackup = g_GameState->SystemOptions().GetPlayerFusionSuitActive();

  std::unique_ptr<SGameFileSlot>& slot = xe4_fileSlots[saveIdx];
  if (!slot)
    slot = std::make_unique<SGameFileSlot>();
  slot->LoadGameState(saveIdx);

  CBitStreamReader r(x30_systemData, 174);
  g_GameState->ReadPersistentOptions(r);
  ImportPersistentOptions();
  g_GameState->SetCardSerial(x28_cardSerial);
  g_GameState->SystemOptions().SetPlayerFusionSuitActive(fusionBackup);
}

void CMemoryCardDriver::EraseFileSlot(u32 saveIdx) { xe4_fileSlots[saveIdx].reset(); }

void CMemoryCardDriver::BuildExistingFileSlot(u32 saveIdx) {
  g_GameState->SetFileIdx(saveIdx);

  std::unique_ptr<SGameFileSlot>& slot = xe4_fileSlots[saveIdx];
  if (!slot)
    slot = std::make_unique<SGameFileSlot>();
  else
    slot->InitializeFromGameState();

  CBitStreamWriter w(x30_systemData, 174);
  g_GameState->PutTo(w);
}

void CMemoryCardDriver::InitializeFileInfo() {
  ExportPersistentOptions();

  OSCalendarTime time = CBasics::ToCalendarTime(std::chrono::system_clock::now());
  std::string timeString = fmt::format(fmt("{:02d}.{:02d}.{:02d}  {:02d}:{:02d}"),
                                       time.x10_mon + 1, time.xc_mday, time.x14_year % 100,
                                       time.x8_hour, time.x4_min);
  std::string comment("Metroid Prime                   ");
  comment += timeString;
  x198_fileInfo->SetComment(comment);

  x198_fileInfo->LockBannerToken(x4_saveBanner, *g_SimplePool);
  x198_fileInfo->LockIconToken(x8_saveIcon0, kabufuda::EAnimationSpeed::Middle, *g_SimplePool);

  CMemoryOutStream w = x198_fileInfo->BeginMemoryOut(3004);

  SSaveHeader header;
  for (int i = 0; i < 3; ++i)
    header.x4_savePresent[i] = xe4_fileSlots[i].operator bool();
  header.DoPut(w);

  w.writeBytes(x30_systemData, 174);

  for (int i = 0; i < 3; ++i)
    if (xe4_fileSlots[i])
      xe4_fileSlots[i]->DoPut(w);
}

void CMemoryCardDriver::WriteBackupBuf() {
  g_GameState->WriteBackupBuf();
  g_GameState->SetCardSerial(x28_cardSerial);
}

bool CMemoryCardDriver::GetCardFreeBytes() {
  ECardResult result = CMemoryCardSys::GetNumFreeBytes(x0_cardPort, x18_cardFreeBytes, x1c_cardFreeFiles);
  if (result == ECardResult::READY)
    return true;
  NoCardFound();
  return false;
}

void CMemoryCardDriver::HandleCardError(ECardResult result, EState state) {
  switch (result) {
  case ECardResult::WRONGDEVICE:
    x10_state = state;
    x14_error = EError::CardWrongDevice;
    break;
  case ECardResult::NOCARD:
    NoCardFound();
    break;
  case ECardResult::IOERROR:
    x10_state = state;
    x14_error = EError::CardIOError;
    break;
  case ECardResult::ENCODING:
    x10_state = state;
    x14_error = EError::CardWrongCharacterSet;
    break;
  default:
    break;
  }
}

void CMemoryCardDriver::Update() {
  kabufuda::ProbeResults result = CMemoryCardSys::CardProbe(x0_cardPort);

  if (result.x0_error == ECardResult::NOCARD) {
    if (x10_state != EState::NoCard)
      NoCardFound();
    static_cast<CMain*>(g_Main)->SetCardBusy(false);
    return;
  }

  if (x10_state == EState::CardProbe) {
    UpdateCardProbe();
    static_cast<CMain*>(g_Main)->SetCardBusy(false);
    return;
  }

  ECardResult resultCode = CMemoryCardSys::GetResultCode(x0_cardPort);
  bool cardBusy = false;

  if (IsCardBusy(x10_state)) {
    cardBusy = true;

    switch (x10_state) {
    case EState::CardMount:
      UpdateMountCard(resultCode);
      break;
    case EState::CardCheck:
      UpdateCardCheck(resultCode);
      break;
    case EState::FileDeleteBad:
      UpdateFileDeleteBad(resultCode);
      break;
    case EState::FileRead:
      UpdateFileRead(resultCode);
      break;
    case EState::FileDeleteAlt:
      UpdateFileDeleteAlt(resultCode);
      break;
    case EState::FileCreate:
      UpdateFileCreate(resultCode);
      break;
    case EState::FileWrite:
      UpdateFileWrite(resultCode);
      break;
    case EState::FileCreateTransactional:
      UpdateFileCreateTransactional(resultCode);
      break;
    case EState::FileWriteTransactional:
      UpdateFileWriteTransactional(resultCode);
      break;
    case EState::FileDeleteAltTransactional:
      UpdateFileDeleteAltTransactional(resultCode);
      break;
    case EState::FileRenameBtoA:
      UpdateFileRenameBtoA(resultCode);
      break;
    case EState::CardFormat:
      UpdateCardFormat(resultCode);
      break;
    default:
      break;
    }
  }

  static_cast<CMain*>(g_Main)->SetCardBusy(cardBusy);
}

} // namespace urde::MP1
