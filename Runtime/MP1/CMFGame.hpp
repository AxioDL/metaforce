#ifndef __URDE_CMFGAME_HPP__
#define __URDE_CMFGAME_HPP__

#include "CMFGameBase.hpp"

namespace urde
{
class CStateManager;
class CInGameGuiManager;
class CToken;

namespace MP1
{

class CMFGame : public CMFGameBase
{
    std::shared_ptr<CStateManager> x14_stateManager;
    std::shared_ptr<CInGameGuiManager> x18_guiManager;
    u32 x1c_ = 0;
    u32 x24_ = 0;
    TUniqueId x28_ = kInvalidUniqueId;
    union
    {
        struct
        {
            bool x2a_24_ : 1;
            bool x2a_25_ : 1;
        };
        u8 _dummy = 0;
    };
public:
    CMFGame(const std::weak_ptr<CStateManager>& stateMgr, const std::weak_ptr<CInGameGuiManager>& guiMgr,
            const CArchitectureQueue&);
    CIOWin::EMessageReturn OnMessage(const CArchitectureMessage& msg, CArchitectureQueue& queue);
    void Draw() const;
};

class CMFGameLoader : public CMFGameLoaderBase
{
    std::shared_ptr<CStateManager> x14_stateMgr;
    std::shared_ptr<CInGameGuiManager> x18_guiMgr;
    std::vector<CToken> x1c_;

    union
    {
        struct
        {
            bool x2c_24_ : 1;
            bool x2c_25_ : 1;
        };
        u8 _dummy = 0;
    };

public:
    CMFGameLoader();
    EMessageReturn OnMessage(const CArchitectureMessage& msg, CArchitectureQueue& queue);
    void Draw() const;
};

}
}

#endif // __URDE_CMFGAME_HPP__
