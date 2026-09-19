#ifndef _CMEMORYCARDDRIVER
#define _CMEMORYCARDDRIVER

#include "MetroidPrime/Player/CGameState.hpp"

#include "Kyoto/CMemoryCardSys.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/pair.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"
#include "rstl/vector.hpp"

class CMemoryInStream;

struct SSaveHeader {
  uint x0_version;
  bool x4_savePresent[3];

  explicit SSaveHeader(int);
  explicit SSaveHeader(CMemoryInStream& in);

  void SetSavePresent(int idx, const bool present) { x4_savePresent[idx] = present; }
  void PutTo(COutputStream& out) const;
};

struct SGameFileSlot {
  rstl::reserved_vector< u8, 940 > x0_saveBuffer;
  CGameState::GameFileStateInfo x944_fileInfo;

  SGameFileSlot();
  explicit SGameFileSlot(CMemoryInStream& in);

  void InitializeFromGameState();
  void LoadGameState(int idx);
  void PutTo(COutputStream& w) const;
};
CHECK_SIZEOF(SGameFileSlot, 0x3d8)

enum EState {
  kS_Initial = 0,
  kS_Ready = 1,
  kS_NoCard = 2,
  kS_DriverClosed = 3,
  kS_CardFormatted = 4,
  kS_CardProbeDone = 5,
  kS_CardMountDone = 6,
  kS_CardCheckDone = 7,
  kS_FileCreateDone = 8,
  kS_FileCreateTransactionalDone = 9,
  kS_FileWriteTransactionalDone = 10,
  kS_FileAltDeleteTransactionalDone = 11,
  kS_CardProbeFailed = 12,
  kS_CardMountFailed = 13,
  kS_CardCheckFailed = 14,
  kS_FileDeleteBadFailed = 15,
  kS_FileDeleteAltFailed = 16,
  kS_FileBad = 17,
  kS_FileCreateFailed = 18,
  kS_FileWriteFailed = 19,
  kS_FileCreateTransactionalFailed = 20,
  kS_FileWriteTransactionalFailed = 21,
  kS_FileAltDeleteTransactionalFailed = 22,
  kS_FileRenameBtoAFailed = 23,
  kS_CardFormatFailed = 24,
  kS_CardProbe = 25,
  kS_CardMount = 26,
  kS_CardCheck = 27,
  kS_FileDeleteBad = 28,
  kS_FileRead = 29,
  kS_FileDeleteAlt = 30,
  kS_FileCreate = 31,
  kS_FileWrite = 32,
  kS_FileCreateTransactional = 33,
  kS_FileWriteTransactional = 34,
  kS_FileAltDeleteTransactional = 35,
  kS_FileRenameBtoA = 36,
  kS_CardFormat = 37
};

class CMemoryCardDriver {
public:
  static void LoadLanguageFromCard(int port);

  enum EError {
    kE_OK,
    kE_CardBroken,
    kE_CardWrongCharacterSet,
    kE_CardIOError,
    kE_CardWrongDevice,
    kE_CardFull,
    kE_CardStillFull, /* After attempting alt-delete (if needed) */
    kE_CardNon8KSectors,
    kE_FileMissing,
    kE_FileCorrupted
  };
  enum EFileState { kFS_Unknown, kFS_NoFile, kFS_File, kFS_BadFile };

  struct SFileInfo {
    EFileState first;
    SMemoryCardFileInfo second;

    SFileInfo(EFileState state, int cardPort, const rstl::string& name)
    : first(state)
    , second(cardPort, name) {}
    ~SFileInfo() {}
  };

private:
  CMemoryCardSys::EMemoryCardPort x0_cardPort;
  CAssetId x4_saveBanner;
  CAssetId x8_saveIcon0;
  CAssetId xc_saveIcon1;
  EState x10_state;
  EError x14_error;
  uint x18_cardFreeBytes;
  uint x1c_cardFreeFiles;
  uint x20_fileTime;
  long long x28_cardSerial;
  rstl::reserved_vector< u8, 174 > x30_systemData;
  rstl::reserved_vector< rstl::auto_ptr< SGameFileSlot >, 3 > xe4_fileSlots;
  rstl::reserved_vector< SFileInfo, 2 > x100_mcFileInfos;
  int x194_fileIdx;
  rstl::single_ptr< CMemoryCardSys::CCardFileInfo > x198_fileInfo;
  bool x19c_;
  bool x19d_importPersistent;

public:
  static bool IsCardBusy(EState);
  static bool IsCardReading(EState);
  EState GetState() const { return x10_state; }
  EError GetError() const { return x14_error; }
  u64 GetCardSerial() const { return x28_cardSerial; }
  CMemoryCardDriver(CMemoryCardSys::EMemoryCardPort cardPort, CAssetId saveBanner,
                    CAssetId saveIcon0, CAssetId saveIcon1, bool importPersistent);
  void ClearFileInfo();
  ~CMemoryCardDriver();
  void Update();
  void HandleCardError(ECardResult result, EState state);
  void UpdateMountCard(ECardResult);
  void UpdateCardCheck(ECardResult);
  void UpdateFileRead(ECardResult);
  void UpdateFileDeleteAlt(ECardResult);
  void UpdateFileDeleteBad(ECardResult);
  void UpdateFileCreate(ECardResult);
  void UpdateFileWrite(ECardResult);
  void UpdateFileCreateTransactional(ECardResult);
  void UpdateFileWriteTransactional(ECardResult);
  void UpdateFileRenameBtoA(ECardResult);
  void StartFileRenameBtoA();
  void WriteBackupBuf();
  void UpdateFileAltDeleteTransactional(ECardResult);
  void UpdateCardFormat(ECardResult);
  void StartCardProbe();
  void UpdateCardProbe();
  void StartMountCard();
  void StartCardCheck();
  void ClearError();
  void CheckCardCapacity();
  void NoCardFound();
  void IndexFiles();
  void StartFileDeleteBad();
  void StartFileDeleteAlt();
  void StartFileRead();
  void StartFileCreate();
  void StartFileWrite();
  void StartFileCreateTransactional();
  void StartFileWriteTransactional();
  void StartFileDeleteAltTransactional();
  void StartCardFormat();
  void InitializeFileInfo();
  void ReadFinished();
  void EraseFileSlot(int);
  void BuildNewFileSlot(int);
  void BuildExistingFileSlot(int);
  void ImportPersistentOptions();
  void ExportPersistentOptions();
  const CGameState::GameFileStateInfo* GetGameFileStateInfo(int);
  bool GetCardFreeBytes();
};

CHECK_SIZEOF(CMemoryCardDriver, 0x1a0)

#endif // _CMEMORYCARDDRIVER
