#ifndef __URDE_CSAMUSHUD_HPP__
#define __URDE_CSAMUSHUD_HPP__

#include "CInGameGuiManager.hpp"

namespace urde
{
class CGuiFrame;
class CStateManager;

struct SHudMemoInfo
{
    float x0_;
    bool x4_;
    bool x5_;
    bool x6_;
};

namespace MP1
{

class CSamusHud
{
    friend class CInGameGuiManager;
    CGuiFrame* x274_loadedBaseHud = nullptr;
    static CSamusHud* g_SamusHud;
public:
    CSamusHud(CStateManager& stateMgr);
    bool CheckLoadComplete(CStateManager& stateMgr);
    void OnNewInGameGuiState(EInGameGuiState state, CStateManager& stateMgr);
    void Touch();
    static void DisplayHudMemo(const std::u16string& text, const SHudMemoInfo& info);
    static void DeferHintMemo(ResId strg, u32 timePeriods, const SHudMemoInfo& info)
    {
        if (g_SamusHud)
            g_SamusHud->_DeferHintMemo(strg, timePeriods, info);
    }
    void _DeferHintMemo(ResId strg, u32 timePeriods, const SHudMemoInfo& info);
};

}
}

#endif // __URDE_CSAMUSHUD_HPP__
