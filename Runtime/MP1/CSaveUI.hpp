#ifndef __URDE_CSAVEUI_HPP__
#define __URDE_CSAVEUI_HPP__

#include "RetroTypes.hpp"
#include "CToken.hpp"
#include "CIOWin.hpp"
#include "CMemoryCardDriver.hpp"

namespace urde
{
class CTexture;
class CStringTable;
class CGuiFrame;
class CSaveWorld;
class CFinalInput;
class CGuiTextPane;
class CGuiTableGroup;

namespace MP1
{

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
    std::unique_ptr<CMemoryCardDriver> x6c_cardDriver;
    std::vector<TLockedToken<CSaveWorld>> x70_saveWorlds;
    u32 x80_ = 0;
    u32 x84_navConfirmSfx = 1460;
    u32 x88_navMoveSfx = 1461;
    u32 x8c_navBackSfx = 1459;
    bool x90_ = false;
    bool x91_ = false;
    bool x92_ = false;
    bool x93_secondaryInst;

    static std::unique_ptr<CMemoryCardDriver> ConstructCardDriver(bool flag);
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
}

#endif // __URDE_CSAVEUI_HPP__
