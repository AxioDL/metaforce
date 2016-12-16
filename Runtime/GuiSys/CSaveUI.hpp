#ifndef __URDE_CSAVEUI_HPP__
#define __URDE_CSAVEUI_HPP__

#include "RetroTypes.hpp"
#include "CToken.hpp"
#include "CIOWin.hpp"

namespace urde
{
class CTexture;
class CStringTable;
class CGuiFrame;
class CSaveWorld;
class CFinalInput;

struct CSaveUI
{
    u32 x0_instIdx;
    u32 x8_a;
    u32 xc_b;
    u32 x10_ = 0;
    TLockedToken<CTexture> x14_txtrSaveBanner;
    TLockedToken<CTexture> x20_txtrSaveIcon0;
    TLockedToken<CTexture> x2c_txtrSaveIcon1;
    TLockedToken<CStringTable> x38_strgMemoryCard;
    TLockedToken<CGuiFrame> x44_frmeGenericMenu;
    u32 x50_ = 0;
    std::unique_ptr<u32> x6c_bannerInfo;
    std::vector<TLockedToken<CSaveWorld>> x70_saveWorlds;
    u32 x80_ = 0;
    u32 x84_navConfirmSfx = 1460;
    u32 x88_navMoveSfx = 1461;
    u32 x8c_navBackSfx = 1459;
    bool x90_ = false;
    bool x91_ = false;
    bool x92_ = false;
    bool x93_secondaryInst;

    CIOWin::EMessageReturn Update(float dt);
    bool PumpLoad();
    void ProcessUserInput(const CFinalInput& input);
    void StartGame(int idx);
    void EraseGame(int idx);
    void* GetGameData(int idx) const;
    CSaveUI(u32 inst, u32 a, u32 b);
};

}

#endif // __URDE_CSAVEUI_HPP__
