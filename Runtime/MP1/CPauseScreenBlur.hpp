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
    u32 x10_ = 0;
    u32 x14_ = 0;
    float x18_blurAmt = 0.f;
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
    void Update(float dt, const CStateManager& stateMgr, bool);
    void Draw(const CStateManager& stateMgr) const;
    float GetBlurAmt() const { return std::fabs(x18_blurAmt); }
    bool IsNotTransitioning() const { return x10_ == x14_; }
};

}
}

#endif // __URDE_CPAUSESCREENBLUR_HPP__
