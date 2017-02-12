#ifndef __URDE_CSAMUSHUD_HPP__
#define __URDE_CSAMUSHUD_HPP__

namespace urde
{
class CGuiFrame;
class CStateManager;

namespace MP1
{

class CSamusHud
{
    friend class CInGameGuiManager;
    CGuiFrame* x274_loadedBaseHud = nullptr;
public:
    CSamusHud(CStateManager& stateMgr);
    bool CheckLoadComplete(CStateManager& stateMgr);
    void Touch();
};

}
}

#endif // __URDE_CSAMUSHUD_HPP__
