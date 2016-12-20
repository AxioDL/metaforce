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
        Zero,
        One = 1,
        Two = 2,
        Five = 5,
        Six = 6,
        Seven = 7,
        Twelve = 12,
        Thirteen = 13,
        Fourteen = 14,
        Fifteen = 15,
        Seventeen = 17,
        TwentyFive = 26,
        TwentySix = 26,
        TwentySeven = 27,
        TwentyEight = 28,
        TwentyNine = 29,
        Thirty = 30,
        ThirtyOne = 31,
        ThirtyTwo = 32,
        ThirtyThree = 33,
        ThirtyFour = 34,
        ThirtyFive = 35,
        ThirtySix = 36,
        ThirtySeven = 37
    };

    enum class EError
    {
        Zero,
        One,
        Two,
        Three,
        Four,
        Five,
        Six,
        Seven,
        Eight,
        Nine
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
        CMemoryCardSys::ECardResult Close();
        CMemoryCardSys::EMemoryCardPort GetFileCardPort() const { return x0_fileInfo.x0_cardPort; }
        int GetFileNo() const { return x0_fileInfo.x4_fileNo; }
        CMemoryCardSys::ECardResult TryFileRead();
        CMemoryCardSys::ECardResult FileRead();
        CMemoryCardSys::ECardResult GetSaveDataOffset(u32& offOut);
    };

    struct SSaveHeader
    {
        u32 x0_;
        u32 x4_[3];
    };

    struct SGameFileSlot
    {
        u8 x0_saveBuffer[940] = {};
        CGameState::GameFileStateInfo x944_fileInfo;
        SGameFileSlot();
        SGameFileSlot(CMemoryInStream& in);
        void InitializeFromGameState();
    };

    CMemoryCardSys::EMemoryCardPort x0_cardPort;
    ResId x4_saveBanner;
    ResId x8_saveIcon0;
    ResId xc_saveIcon1;
    EState x10_state = EState::Zero;
    EError x14_error = EError::Zero;
    s32 x18_cardFreeBytes = 0;
    s32 x1c_cardFreeFiles = 0;
    u32 x20_fileTime = 0;
    u32 x24_ = 0;
    u64 x28_cardSerial = 0;
    u8 x30_systemData[174] = {};
    std::unique_ptr<SGameFileSlot> xe4_fileSlots[3];
    std::vector<std::pair<u32, SFileInfo>> x100_mcFileInfos;
    u32 x194_fileIdx = -1;
    u32 x198_ = 0;
    bool x19c_ = false;
    bool x19d_doMergePersistent;

public:
    CMemoryCardDriver(CMemoryCardSys::EMemoryCardPort cardPort, ResId saveBanner,
                      ResId saveIcon0, ResId saveIcon1, bool flag);
    void FinishedLoading();
    void FinishedLoading2();
    void NoCardFound();
    void MountCard();
    void CheckCard();

    CGameState::GameFileStateInfo* GetGameFileStateInfo(int idx);
    static SSaveHeader LoadSaveHeader(CMemoryInStream& in);
    static std::unique_ptr<SGameFileSlot> LoadSaveFile(CMemoryInStream& in);
    void ReadFinished();
    void MergePersistentOptions();
    void DeleteFile();
    void CheckCardCapacity();

    void Case26(CMemoryCardSys::ECardResult result);
    void Case27(CMemoryCardSys::ECardResult result);
    void Case28(CMemoryCardSys::ECardResult result);
    void Case29(CMemoryCardSys::ECardResult result);
    void Case30(CMemoryCardSys::ECardResult result);
    void Case31(CMemoryCardSys::ECardResult result);
    void Case32(CMemoryCardSys::ECardResult result);
    void Case33(CMemoryCardSys::ECardResult result);
    void Case34(CMemoryCardSys::ECardResult result);
    void Case35(CMemoryCardSys::ECardResult result);
    void Case36(CMemoryCardSys::ECardResult result);
    void Case37(CMemoryCardSys::ECardResult result);

    void GoTo17();
    void GoTo28();

    bool GetCardFreeBytes();
    void HandleCardError(CMemoryCardSys::ECardResult result, EState state);
    void Update();

    static bool InCardInsertedRange(EState v)
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
};

}
}

#endif // __URDE_CMEMORYCARDDRIVER_HPP__
