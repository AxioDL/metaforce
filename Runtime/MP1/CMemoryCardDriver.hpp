#pragma once

#include <array>
#include <memory>
#include <string>
#include <vector>

#include "Runtime/CGameState.hpp"
#include "Runtime/CMemoryCardSys.hpp"

namespace metaforce::MP1 {

class CMemoryCardDriver {
  friend class CSaveGameScreen;

public:
  enum class EState {
    Initial = 0,
    Ready = 1,
    NoCard = 2,
    DriverClosed = 3,
    CardFormatted = 4,
    CardProbeDone = 5,
    CardMountDone = 6,
    CardCheckDone = 7,
    FileCreateDone = 8,
    FileCreateTransactionalDone = 9,
    FileWriteTransactionalDone = 10,
    FileDeleteAltTransactionalDone = 11,
    CardProbeFailed = 12,
    CardMountFailed = 13,
    CardCheckFailed = 14,
    FileDeleteBadFailed = 15,
    FileDeleteAltFailed = 16,
    FileBad = 17,
    FileCreateFailed = 18,
    FileWriteFailed = 19,
    FileCreateTransactionalFailed = 20,
    FileWriteTransactionalFailed = 21,
    FileDeleteAltTransactionalFailed = 22,
    FileRenameBtoAFailed = 23,
    CardFormatFailed = 24,
    CardProbe = 25,
    CardMount = 26,
    CardCheck = 27,
    FileDeleteBad = 28,
    FileRead = 29,
    FileDeleteAlt = 30,
    FileCreate = 31,
    FileWrite = 32,
    FileCreateTransactional = 33,
    FileWriteTransactional = 34,
    FileDeleteAltTransactional = 35,
    FileRenameBtoA = 36,
    CardFormat = 37
  };

  enum class EError {
    OK,
    CardBroken,
    CardWrongCharacterSet,
    CardIOError,
    CardWrongDevice,
    CardFull,
    CardStillFull, /* After attempting alt-delete (if needed) */
    CardNon8KSectors,
    FileMissing,
    FileCorrupted
  };

private:
  struct SFileInfo {
    CMemoryCardSys::CardFileHandle x0_fileInfo;

    std::string x14_name;
    std::vector<u8> x24_saveFileData;
    std::vector<u8> x34_saveData;
    SFileInfo(kabufuda::ECardSlot cardPort, std::string_view name);
    kabufuda::ECardResult Open();
    kabufuda::ECardResult Close();
    kabufuda::ECardSlot GetFileCardPort() const { return x0_fileInfo.slot; }
    int GetFileNo() const { return x0_fileInfo.getFileNo(); }
    kabufuda::ECardResult StartRead();
    kabufuda::ECardResult TryFileRead();
    kabufuda::ECardResult FileRead();
    kabufuda::ECardResult GetSaveDataOffset(u32& offOut) const;
  };

  struct SSaveHeader {
    u32 x0_version = 0;
    std::array<bool, 3> x4_savePresent{};

    void DoPut(CMemoryStreamOut& out) const {
      out.WriteLong(x0_version);
      for (const bool savePresent : x4_savePresent) {
        out.Put(savePresent);
      }
    }
  };

  struct SGameFileSlot {
    std::array<u8, 940> x0_saveBuffer{};
    CGameState::GameFileStateInfo x944_fileInfo;

    SGameFileSlot();
    explicit SGameFileSlot(CMemoryInStream& in);
    void InitializeFromGameState();
    void LoadGameState(u32 idx);
    void DoPut(CMemoryStreamOut& w) const { w.Put(x0_saveBuffer.data(), x0_saveBuffer.size()); }
  };

  enum class EFileState { Unknown, NoFile, File, BadFile };

  kabufuda::ECardSlot x0_cardPort;
  CAssetId x4_saveBanner;
  CAssetId x8_saveIcon0;
  CAssetId xc_saveIcon1;
  EState x10_state = EState::Initial;
  EError x14_error = EError::OK;
  s32 x18_cardFreeBytes = 0;
  s32 x1c_cardFreeFiles = 0;
  u32 x20_fileTime = 0;
  u64 x28_cardSerial = 0;
  std::array<u8, 174> x30_systemData{};
  std::array<std::unique_ptr<SGameFileSlot>, 3> xe4_fileSlots;
  std::vector<std::pair<EFileState, SFileInfo>> x100_mcFileInfos;
  u32 x194_fileIdx = -1;
  std::unique_ptr<CMemoryCardSys::CCardFileInfo> x198_fileInfo;
  bool x19c_ = false;
  bool x19d_importPersistent;

public:
  CMemoryCardDriver(kabufuda::ECardSlot cardPort, CAssetId saveBanner, CAssetId saveIcon0, CAssetId saveIcon1,
                    bool importPersistent);

  void NoCardFound();
  const CGameState::GameFileStateInfo* GetGameFileStateInfo(int idx) const;
  static SSaveHeader LoadSaveHeader(CMemoryInStream& in);
  static std::unique_ptr<SGameFileSlot> LoadSaveFile(CMemoryInStream& in);
  void ReadFinished();
  void ImportPersistentOptions();
  void ExportPersistentOptions();
  void CheckCardCapacity();
  void IndexFiles();

  void StartCardProbe();                  // 25
  void StartMountCard();                  // 26
  void StartCardCheck();                  // 27
  void StartFileDeleteBad();              // 28
  void StartFileRead();                   // 29
  void StartFileDeleteAlt();              // 30
  void StartFileCreate();                 // 31
  void StartFileWrite();                  // 32
  void StartFileCreateTransactional();    // 33
  void StartFileWriteTransactional();     // 34
  void StartFileDeleteAltTransactional(); // 35
  void StartFileRenameBtoA();             // 36
  void StartCardFormat();                 // 37

  void UpdateCardProbe();                                              // 25
  void UpdateMountCard(kabufuda::ECardResult result);                  // 26
  void UpdateCardCheck(kabufuda::ECardResult result);                  // 27
  void UpdateFileDeleteBad(kabufuda::ECardResult result);              // 28
  void UpdateFileRead(kabufuda::ECardResult result);                   // 29
  void UpdateFileDeleteAlt(kabufuda::ECardResult result);              // 30
  void UpdateFileCreate(kabufuda::ECardResult result);                 // 31
  void UpdateFileWrite(kabufuda::ECardResult result);                  // 32
  void UpdateFileCreateTransactional(kabufuda::ECardResult result);    // 33
  void UpdateFileWriteTransactional(kabufuda::ECardResult result);     // 34
  void UpdateFileDeleteAltTransactional(kabufuda::ECardResult result); // 35
  void UpdateFileRenameBtoA(kabufuda::ECardResult result);             // 36
  void UpdateCardFormat(kabufuda::ECardResult result);                 // 37

  void ClearFileInfo() { x198_fileInfo.reset(); }
  void BuildNewFileSlot(u32 saveIdx);
  void EraseFileSlot(u32 saveIdx);
  void BuildExistingFileSlot(u32 saveIdx);
  void InitializeFileInfo();
  void WriteBackupBuf();
  bool GetCardFreeBytes();
  void HandleCardError(kabufuda::ECardResult result, EState state);
  void Update();
  void ClearError() { x14_error = EError::OK; }

  static bool IsCardBusy(EState v) { return v >= EState::CardMount && v <= EState::CardFormat; }

  static bool IsCardWriting(EState v) {
    if (v < EState::CardProbe)
      return false;
    if (v == EState::CardCheck)
      return false;
    if (v == EState::FileRead)
      return false;
    return true;
  }
};

} // namespace metaforce::MP1
