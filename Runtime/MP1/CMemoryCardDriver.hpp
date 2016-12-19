#ifndef __URDE_CMEMORYCARDDRIVER_HPP__
#define __URDE_CMEMORYCARDDRIVER_HPP__

#include "CMemoryCardSys.hpp"

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
    struct SMemoryCardSlotInfo
    {
        CMemoryCardSys::EMemoryCardPort x0_cardPort;
        u32 x4_ = -1;
        std::string x14_name;
        std::vector<u8> x24_;
        std::vector<u8> x34_;
        SMemoryCardSlotInfo(CMemoryCardSys::EMemoryCardPort cardPort, const std::string& name);
    };

    CMemoryCardSys::EMemoryCardPort x0_cardPort;
    ResId x4_saveBanner;
    ResId x8_saveIcon0;
    ResId xc_saveIcon1;
    EState x10_state = EState::Zero;
    EError x14_error = EError::Zero;
    s32 x18_cardFreeBytes = 0;
    s32 x1c_cardFreeFiles = 0;
    u32 x20_ = 0;
    u32 x24_ = 0;
    u64 x28_cardSerial = 0;
    u8 x30_[174];
    std::unique_ptr<u8> xe4_[3];
    std::vector<std::pair<u32, SMemoryCardSlotInfo>> x100_mcSlotInfos;
    u32 x194_ = -1;
    u32 x198_ = 0;
    bool x19c_ = false;
    bool x19d_flag;

public:
    CMemoryCardDriver(CMemoryCardSys::EMemoryCardPort cardPort, ResId saveBanner,
                      ResId saveIcon0, ResId saveIcon1, bool flag);
    void FinishedLoading();
    void FinishedLoading2();
    void NoCardFound();
    void MountCard();
    void MountCardFailed(CMemoryCardSys::ECardResult result);
    void CheckCard();
    void CheckCardFailed(CMemoryCardSys::ECardResult result);
    bool GetCardFreeBytes();
    void HandleCardError(CMemoryCardSys::ECardResult result, EState state);
    void Update();
};

}
}

#endif // __URDE_CMEMORYCARDDRIVER_HPP__
