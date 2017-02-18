#ifndef __URDE_CPAUSESCREEN_HPP__
#define __URDE_CPAUSESCREEN_HPP__

#include "CInGameGuiManager.hpp"

namespace urde
{
namespace MP1
{

class CPauseScreen
{
    union
    {
        struct
        {
            bool x50_24_ : 1;
            bool x50_25_ : 1;
        };
        u32 _dummy = 0;
    };
public:
    CPauseScreen();
    bool CheckLoadComplete();
    void OnNewInGameGuiState(EInGameGuiState state, CStateManager& stateMgr);
    bool GetX50_25() const { return x50_25_; }
};

}
}

#endif // __URDE_CPAUSESCREEN_HPP__
