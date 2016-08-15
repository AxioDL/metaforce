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
public:
    CMFGame() : CMFGameBase("CMFGame") {}
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
