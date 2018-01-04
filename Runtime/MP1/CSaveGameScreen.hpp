#ifndef __URDE_CSAVEUI_HPP__
#define __URDE_CSAVEUI_HPP__

#include "RetroTypes.hpp"
#include "CToken.hpp"
#include "CIOWin.hpp"
#include "CMemoryCardDriver.hpp"
#include "CSaveGameScreenTouchBar.hpp"

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

enum class ESaveContext
{
    FrontEnd,
    InGame
};

class CSaveGameScreen
{
public:
    enum class EUIType
    {
        Empty = 0,
        BusyReading = 1,
        BusyWriting = 2,
        NoCardFound = 3,
        NeedsFormatBroken = 4,
        NeedsFormatEncoding = 5,
        CardDamaged = 6,
        WrongDevice = 7,
        InsufficientSpaceBadCheck = 8,
        InsufficientSpaceOKCheck = 9,
        IncompatibleCard = 10,
        SaveCorrupt = 11,
        StillInsufficientSpace = 12,
        ProgressWillBeLost = 13,
        NotOriginalCard = 14,
        AllDataWillBeLost = 15,
        SaveReady = 16
    };

    bool IsHiddenFromFrontEnd()
    {
        switch (x10_uiType)
        {
        case EUIType::SaveReady:
        case EUIType::Empty:
        case EUIType::BusyReading:
        case EUIType::BusyWriting:
            return false;
        default:
            return true;
        }
    }

private:
    ESaveContext x0_saveCtx;
    u64 x8_serial;
    EUIType x10_uiType = EUIType::Empty;
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
    bool x91_uiTextDirty = false;
    bool x92_savingDisabled = false;
    bool x93_inGame;

    std::unique_ptr<CSaveGameScreenTouchBar> m_touchBar;

    void ContinueWithoutSaving();

public:
    static std::unique_ptr<CMemoryCardDriver> ConstructCardDriver(bool inGame);
    void ResetCardDriver();
    CIOWin::EMessageReturn Update(float dt);
    void SetInGame(bool v) { x93_inGame = v; }
    bool PumpLoad();
    EUIType SelectUIType() const;
    void SetUIText();
    void SetUIColors();
    void Draw() const;

    void DoAdvance(CGuiTableGroup* caller);
    void DoSelectionChange(CGuiTableGroup* caller, int oldSel);

    void ProcessUserInput(const CFinalInput& input);
    void StartGame(int idx);
    void SaveNESState();
    void EraseGame(int idx);
    const CGameState::GameFileStateInfo* GetGameData(int idx) const;
    EUIType GetUIType() const { return x10_uiType; }
    bool IsSavingDisabled() const { return x92_savingDisabled; }
    CSaveGameScreen(ESaveContext saveCtx, u64 serial);
};

}
}

#endif // __URDE_CSAVEUI_HPP__
