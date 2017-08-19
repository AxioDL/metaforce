#include "CMorphBall.hpp"
#include "CPlayer.hpp"
#include "CMorphBallShadow.hpp"
#include "Particle/CGenDescription.hpp"

namespace urde
{

CMorphBall::CMorphBall(CPlayer& player, float)
: x0_player(player)
{
}

void CMorphBall::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr)
{

}

void CMorphBall::DrawBallShadow(const CStateManager& mgr)
{
    if (!x1e50_shadow)
        return;

    float alpha = 1.f;
    switch (x0_player.x2f8_morphBallState)
    {
    case CPlayer::EPlayerMorphBallState::Unmorphed:
        return;
    case CPlayer::EPlayerMorphBallState::Unmorphing:
        alpha = 0.f;
        if (x0_player.x578_morphDuration != 0.f)
            alpha = zeus::clamp(0.f, x0_player.x574_morphTime / x0_player.x578_morphDuration, 1.f);
        alpha = 1.f - alpha;
    case CPlayer::EPlayerMorphBallState::Morphing:
        alpha = 0.f;
        if (x0_player.x578_morphDuration != 0.f)
            alpha = zeus::clamp(0.f, x0_player.x574_morphTime / x0_player.x578_morphDuration, 1.f);
    default: break;
    }
    x1e50_shadow->Render(mgr, alpha);
}

void CMorphBall::DeleteBallShadow()
{

}

void CMorphBall::CreateBallShadow()
{

}

void CMorphBall::RenderToShadowTex(CStateManager& mgr)
{

}

}
