#include "CMorphBallTrail.hpp"
#include "CStateManager.hpp"
#include "World/CWorld.hpp"

namespace urde
{

static union
{
    struct
    {
        bool x_24_ : 1;
        bool x_25_ : 1;
        bool x_26_ : 1;
        bool x_27_ : 1;
    };
    u16 _dummy = 0;
} s_flags;

CMorphBallTrail::CMorphBallTrail(int w, int h, const TToken<CTexture>& fadeTex)
: xb0_w(w), xb4_h(h), xa8_ballFade(fadeTex)
{
    m_gfxToken = CGraphics::CommitResources([this, w, h](boo::IGraphicsDataFactory::Context& ctx)
    {
        m_renderTex = ctx.newRenderTexture(w, h, true, false);
        return true;
    });
}

void CMorphBallTrail::GatherAreas(CStateManager& mgr)
{
    x18_areas.clear();
    for (CGameArea* area = mgr.WorldNC()->GetChainHead(EChain::Alive);
         area != CWorld::GetAliveAreasEnd();
         area = area->GetNext())
    {
        CGameArea::EOcclusionState occState = CGameArea::EOcclusionState::NotOccluded;
        if (area->IsPostConstructed())
            occState = area->GetPostConstructed()->x10dc_occlusionState;
        if (occState == CGameArea::EOcclusionState::Occluded)
            x18_areas.push_back(area);
    }
}

void CMorphBallTrail::RenderToTex(const zeus::CAABox& aabb, CStateManager& mgr, CPlayer& player)
{
    xb8_ = aabb;
    x0_.clear();
    x18_areas.clear();
    x30_.clear();
    s_flags.x_26_ = true;

    if (!s_flags.x_27_)
    {
        xd0_ = false;
        return;
    }

    GatherAreas(mgr);

    // TODO: finish
}

void CMorphBallTrail::Render(CStateManager& mgr)
{

}

}
