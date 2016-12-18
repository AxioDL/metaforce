#ifndef __URDE_CSAVEUI_HPP__
#define __URDE_CSAVEUI_HPP__

#include "RetroTypes.hpp"
#include "CToken.hpp"
#include "CIOWin.hpp"
#include "CMemoryCardSys.hpp"

namespace urde
{
class CTexture;
class CStringTable;
class CGuiFrame;
class CSaveWorld;
class CFinalInput;
class CGuiTextPane;
class CGuiTableGroup;

struct CSaveUI
{
    enum class UIType
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
        Nine,
        Ten,
        Eleven,
        Twelve,
        Thirteen,
        Fourteen,
        Fifteen,
        Sixteen
    };

    bool IsDrawConditional()
    {
        switch (x10_uiType)
        {
        case UIType::Sixteen:
        case UIType::Zero:
        case UIType::One:
        case UIType::Two:
            return false;
        default:
            return true;
        }
    }

    struct SBannerInfo
    {
        struct SMemoryCardSlotInfo
        {
            CMemoryCardSys::EMemoryCardPort x0_cardPort;
            u32 x4_ = -1;
            std::string x14_name;
            std::vector<u8> x24_;
            std::vector<u8> x34_;
            SMemoryCardSlotInfo(CMemoryCardSys::EMemoryCardPort cardPort, const std::string& name);
        };

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

        SBannerInfo(CMemoryCardSys::EMemoryCardPort cardPort, ResId saveBanner,
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

    u32 x0_instIdx;
    u32 x8_a;
    u32 xc_b;
    UIType x10_uiType = UIType::Zero;
    TLockedToken<CTexture> x14_txtrSaveBanner;
    TLockedToken<CTexture> x20_txtrSaveIcon0;
    TLockedToken<CTexture> x2c_txtrSaveIcon1;
    TLockedToken<CStringTable> x38_strgMemoryCard;
    TLockedToken<CGuiFrame> x44_frmeGenericMenu;
    CGuiFrame* x50_loadedFrame = nullptr;
    CGuiTextPane* x54_textpane_message;
    CGuiTableGroup* x58_tablegroup_choices;
    CGuiTextPane* x5c_textpane_choice0;
    CGuiTextPane* x60_textpane_choice1;
    CGuiTextPane* x64_textpane_choice2;
    CGuiTextPane* x68_textpane_choice3;
    std::unique_ptr<SBannerInfo> x6c_bannerInfo;
    std::vector<TLockedToken<CSaveWorld>> x70_saveWorlds;
    u32 x80_ = 0;
    u32 x84_navConfirmSfx = 1460;
    u32 x88_navMoveSfx = 1461;
    u32 x8c_navBackSfx = 1459;
    bool x90_ = false;
    bool x91_ = false;
    bool x92_ = false;
    bool x93_secondaryInst;

    static std::unique_ptr<SBannerInfo> ConstructBannerInfo(bool flag);
    CIOWin::EMessageReturn Update(float dt);
    bool PumpLoad();
    UIType SelectUIType() const;
    void FinishedLoading();
    void Draw() const;

    void DoAdvance(CGuiTableGroup* caller);
    void DoSelectionChange(CGuiTableGroup* caller);

    void ProcessUserInput(const CFinalInput& input);
    void StartGame(int idx);
    void EraseGame(int idx);
    void* GetGameData(int idx) const;
    CSaveUI(u32 inst, u32 a, u32 b);
};

}

#endif // __URDE_CSAVEUI_HPP__
