#ifndef __URDE_CMORPHBALLTRAIL_HPP__
#define __URDE_CMORPHBALLTRAIL_HPP__

#include "CToken.hpp"
#include "Graphics/CTexture.hpp"
#include "zeus/CAABox.hpp"

namespace urde
{
class CStateManager;
class CPlayer;
class CGameArea;

class CMorphBallTrail
{
    std::list<u32> x0_;
    std::list<CGameArea*> x18_areas;
    std::vector<u32> x30_;
    //CTexture x40_;
    boo::GraphicsDataToken m_gfxToken;
    boo::ITextureR* m_renderTex;
    TToken<CTexture> xa8_ballFade;
    int xb0_w;
    int xb4_h;
    zeus::CAABox xb8_;
    bool xd0_ = false;
    void GatherAreas(CStateManager& mgr);
public:
    CMorphBallTrail(int w, int h, const TToken<CTexture>& fadeTex);
    void RenderToTex(const zeus::CAABox& aabb, CStateManager& mgr, CPlayer& player);
    void Render(CStateManager& mgr);
};

}

#endif // __URDE_CMORPHBALLTRAIL_HPP__
