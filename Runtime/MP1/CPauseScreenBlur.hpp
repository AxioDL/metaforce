#ifndef __URDE_CPAUSESCREENBLUR_HPP__
#define __URDE_CPAUSESCREENBLUR_HPP__

#include "CInGameGuiManagerCommon.hpp"

namespace urde
{
class CStateManager;
namespace MP1
{

class CPauseScreenBlur
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
    CPauseScreenBlur();
    bool CheckLoadComplete();
    void OnNewInGameGuiState(EInGameGuiState state, CStateManager& stateMgr);
    bool GetX50_25() const { return x50_25_; }
};

}
}

#endif // __URDE_CPAUSESCREENBLUR_HPP__
