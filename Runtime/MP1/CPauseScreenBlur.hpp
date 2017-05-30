#ifndef __URDE_CPAUSESCREENBLUR_HPP__
#define __URDE_CPAUSESCREENBLUR_HPP__

#include "CInGameGuiManagerCommon.hpp"
#include "CToken.hpp"
#include "Camera/CCameraFilter.hpp"
#include "Graphics/CTexture.hpp"

namespace urde
{
class CStateManager;

namespace MP1
{

class CPauseScreenBlur
{
    enum class EState
    {
        InGame,
        MapScreen,
        SaveGame,
        HUDMessage,
        Pause
    };

    TLockedToken<CTexture> x4_mapLightQuarter;
    EState x10_prevState = EState::InGame;
    EState x14_nextState = EState::InGame;
    float x18_blurAmt = 0.f;
    CCameraBlurPass x1c_camBlur;

    union
    {
        struct
        {
            bool x50_24_blurring : 1;
            bool x50_25_gameDraw : 1;
        };
        u32 _dummy = 0;
    };
    void OnBlurComplete(bool);
    void SetState(EState state);

public:
    CPauseScreenBlur();
    void OnNewInGameGuiState(EInGameGuiState state, CStateManager& stateMgr);
    bool IsGameDraw() const { return x50_25_gameDraw; }
    void Update(float dt, const CStateManager& stateMgr, bool);
    void Draw(const CStateManager& stateMgr) const;
    float GetBlurAmt() const { return std::fabs(x18_blurAmt); }
    bool IsNotTransitioning() const { return x10_prevState == x14_nextState; }
};

}
}

#endif // __URDE_CPAUSESCREENBLUR_HPP__
