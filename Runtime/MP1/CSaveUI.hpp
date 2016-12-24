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

class CSaveUI
{
public:
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

private:
    u32 x0_instIdx;
    u64 x8_serial;
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
    CIOWin::EMessageReturn x80_iowRet = CIOWin::EMessageReturn::Normal;
    u32 x84_navConfirmSfx = 1460;
    u32 x88_navMoveSfx = 1461;
    u32 x8c_navBackSfx = 1459;
    bool x90_needsDriverReset = false;
    bool x91_ = false;
    bool x92_ = false;
    bool x93_secondaryInst;

    void ResetCardDriver();

public:
    static std::unique_ptr<CMemoryCardDriver> ConstructCardDriver(bool importState);
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
    UIType GetUIType() const { return x10_uiType; }
    CSaveUI(u32 inst, u64 serial);
};

}
}

#endif // __URDE_CSAVEUI_HPP__
