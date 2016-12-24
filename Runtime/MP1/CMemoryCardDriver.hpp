#ifndef __URDE_CMEMORYCARDDRIVER_HPP__
#define __URDE_CMEMORYCARDDRIVER_HPP__

#include "CMemoryCardSys.hpp"
#include "CGameState.hpp"

namespace urde
{
namespace MP1
{

class CMemoryCardDriver
{
    friend class CSaveUI;
public:
    enum class EState
    {
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

    enum class EError
    {
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
    struct CARDFileInfo
    {
        CMemoryCardSys::EMemoryCardPort x0_cardPort;
        int x4_fileNo = -1;
        int x8_offset;
        int xc_length;
        u16 iBlock;
    };

    struct SFileInfo
    {
        CARDFileInfo x0_fileInfo;
        std::string x14_name;
        std::vector<u8> x24_saveFileData;
        std::vector<u8> x34_saveData;
        SFileInfo(CMemoryCardSys::EMemoryCardPort cardPort, const std::string& name);
        CMemoryCardSys::ECardResult Open();
        CMemoryCardSys::ECardResult Close();
        CMemoryCardSys::EMemoryCardPort GetFileCardPort() const { return x0_fileInfo.x0_cardPort; }
        int GetFileNo() const { return x0_fileInfo.x4_fileNo; }
        CMemoryCardSys::ECardResult StartRead();
        CMemoryCardSys::ECardResult TryFileRead();
        CMemoryCardSys::ECardResult FileRead();
        CMemoryCardSys::ECardResult GetSaveDataOffset(u32& offOut);
    };

    struct SSaveHeader
    {
        u32 x0_version = 0;
        bool x4_savePresent[3];
        void DoPut(CMemoryOutStream& out) const
        {
            out.writeUint32Big(x0_version);
            for (int i=0 ; i<3 ; ++i)
                out.writeBool(x4_savePresent[i]);
        }
    };

    struct SGameFileSlot
    {
        u8 x0_saveBuffer[940] = {};
        CGameState::GameFileStateInfo x944_fileInfo;
        SGameFileSlot();
        SGameFileSlot(CMemoryInStream& in);
        void InitializeFromGameState();
        void DoPut(CMemoryOutStream& w) const
        {
            w.writeBytes(x0_saveBuffer, 940);
        }
    };

    enum class EFileState
    {
        Unknown,
        NoFile,
        File,
        BadFile
    };

    CMemoryCardSys::EMemoryCardPort x0_cardPort;
    ResId x4_saveBanner;
    ResId x8_saveIcon0;
    ResId xc_saveIcon1;
    EState x10_state = EState::Initial;
    EError x14_error = EError::OK;
    s32 x18_cardFreeBytes = 0;
    s32 x1c_cardFreeFiles = 0;
    u32 x20_fileTime = 0;
    u64 x28_cardSerial = 0;
    u8 x30_systemData[174] = {};
    std::unique_ptr<SGameFileSlot> xe4_fileSlots[3];
    std::vector<std::pair<EFileState, SFileInfo>> x100_mcFileInfos;
    u32 x194_fileIdx = -1;
    std::unique_ptr<CMemoryCardSys::CCardFileInfo> x198_fileInfo;
    bool x19c_ = false;
    bool x19d_doImportPersistent;

public:
    CMemoryCardDriver(CMemoryCardSys::EMemoryCardPort cardPort, ResId saveBanner,
                      ResId saveIcon0, ResId saveIcon1, bool importPersistent);

    void NoCardFound();
    CGameState::GameFileStateInfo* GetGameFileStateInfo(int idx);
    static SSaveHeader LoadSaveHeader(CMemoryInStream& in);
    static std::unique_ptr<SGameFileSlot> LoadSaveFile(CMemoryInStream& in);
    void ReadFinished();
    void ImportPersistentOptions();
    void ExportPersistentOptions();
    void CheckCardCapacity();
    void IndexFiles();

    void StartCardProbe(); // 25
    void StartMountCard(); // 26
    void StartCardCheck(); // 27
    void StartFileDeleteBad(); // 28
    void StartFileRead(); // 29
    void StartFileDeleteAlt(); // 30
    void StartFileCreate(); // 31
    void StartFileWrite(); // 32
    void StartFileCreateTransactional(); // 33
    void StartFileWriteTransactional(); // 34
    void StartFileDeleteAltTransactional(); // 35
    void StartFileRenameBtoA(); // 36
    void StartCardFormat(); // 37

    void UpdateCardProbe(); // 25
    void UpdateMountCard(CMemoryCardSys::ECardResult result); // 26
    void UpdateCardCheck(CMemoryCardSys::ECardResult result); // 27
    void UpdateFileDeleteBad(CMemoryCardSys::ECardResult result); // 28
    void UpdateFileRead(CMemoryCardSys::ECardResult result); // 29
    void UpdateFileDeleteAlt(CMemoryCardSys::ECardResult result); // 30
    void UpdateFileCreate(CMemoryCardSys::ECardResult result); // 31
    void UpdateFileWrite(CMemoryCardSys::ECardResult result); // 32
    void UpdateFileCreateTransactional(CMemoryCardSys::ECardResult result); // 33
    void UpdateFileWriteTransactional(CMemoryCardSys::ECardResult result); // 34
    void UpdateFileDeleteAltTransactional(CMemoryCardSys::ECardResult result); // 35
    void UpdateFileRenameBtoA(CMemoryCardSys::ECardResult result); // 36
    void UpdateCardFormat(CMemoryCardSys::ECardResult result); // 37

    void ClearFileInfo() { x198_fileInfo.reset(); }
    void InitializeFileInfo();
    void WriteBackupBuf();
    bool GetCardFreeBytes();
    void HandleCardError(CMemoryCardSys::ECardResult result, EState state);
    void Update();

    static bool IsCardBusy(EState v)
    {
        return v >= EState::CardMount && v <= EState::CardFormat;
    }

    static bool IsOperationDestructive(EState v)
    {
        if (v < EState::CardProbe)
            return false;
        if (v == EState::CardCheck)
            return false;
        if (v == EState::FileRead)
            return false;
        return true;
    }
};

}
}

#endif // __URDE_CMEMORYCARDDRIVER_HPP__
