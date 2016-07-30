#include "CSpaceWarpFilter.hpp"
#include "Graphics/CGraphics.hpp"
#include "Graphics/CBooRenderer.hpp"

#define WARP_RAMP_RES 32

namespace urde
{

void CSpaceWarpFilter::GenerateWarpRampTex(boo::IGraphicsDataFactory::Context& ctx)
{
    u8 data[WARP_RAMP_RES+1][WARP_RAMP_RES+1][4] = {};
    float halfRes = WARP_RAMP_RES / 2.f;
    for (int y=0 ; y<WARP_RAMP_RES+1 ; ++y)
    {
        for (int x=0 ; x<WARP_RAMP_RES+1 ; ++x)
        {
            zeus::CVector2f vec((x - halfRes) / halfRes, (y - halfRes) / halfRes);
            float mag = vec.magnitude();
            if (mag < 1.f && vec.canBeNormalized())
            {
                vec.normalize();
                vec *= std::sqrt(mag);
            }
            data[y][x][0] = zeus::clamp(0, int((((vec.x / 2.f + 0.5f) - x / float(WARP_RAMP_RES)) + 0.5f) * 255), 255);
            data[y][x][1] = zeus::clamp(0, int((((vec.y / 2.f + 0.5f) - y / float(WARP_RAMP_RES)) + 0.5f) * 255), 255);
        }
    }
    m_warpTex = ctx.newStaticTexture(WARP_RAMP_RES+1, WARP_RAMP_RES+1, 1,
                                     boo::TextureFormat::RGBA8, data[0],
                                     (WARP_RAMP_RES+1) * (WARP_RAMP_RES+1) * 4);
}

CSpaceWarpFilter::CSpaceWarpFilter()
{
    m_token = CGraphics::g_BooFactory->commitTransaction([&](boo::IGraphicsDataFactory::Context& ctx) -> bool
    {
        GenerateWarpRampTex(ctx);
        struct Vert
        {
            zeus::CVector2f m_pos;
            zeus::CVector2f m_uv;
        } verts[4] =
        {
        {{-1.0, -1.0}, {0.0, 0.0}},
        {{-1.0,  1.0}, {0.0, 1.0}},
        {{ 1.0, -1.0}, {1.0, 0.0}},
        {{ 1.0,  1.0}, {1.0, 1.0}},
        };
        m_vbo = ctx.newStaticBuffer(boo::BufferUse::Vertex, verts, 32, 4);
        m_uniBuf = ctx.newDynamicBuffer(boo::BufferUse::Uniform, sizeof(Uniform), 1);
        m_dataBind = TShader<CSpaceWarpFilter>::BuildShaderDataBinding(ctx, *this);
        return true;
    });
}

void CSpaceWarpFilter::draw(const zeus::CVector2f& pt)
{
    /* Indirect coords are full-texture sampling when warp is completely in viewport */
    m_uniform.m_indXf[1][1] = 1.f;
    m_uniform.m_indXf[0][0] = 1.f;
    m_uniform.m_indXf[2][0] = 0.f;
    m_uniform.m_indXf[2][1] = 0.f;

    /* Warp effect is fixed at 192x192 rectangle in original (1/2.5 viewport height) */
    m_uniform.m_matrix[1][1] = 1.f / 2.5f;
    m_uniform.m_matrix[0][0] = m_uniform.m_matrix[1][1] / CGraphics::g_ProjAspect;

    SClipScreenRect clipRect = {};
    clipRect.x4_left = ((pt[0] - m_uniform.m_matrix[0][0]) / 2.f + 0.5f) * CGraphics::g_ViewportResolution.x;
    if (clipRect.x4_left >= CGraphics::g_ViewportResolution.x)
        return;
    clipRect.x8_top = ((pt[1] - m_uniform.m_matrix[1][1]) / 2.f + 0.5f) * CGraphics::g_ViewportResolution.y;
    if (clipRect.x8_top >= CGraphics::g_ViewportResolution.y)
        return;
    clipRect.xc_width = CGraphics::g_ViewportResolution.x * m_uniform.m_matrix[0][0];
    if (clipRect.x4_left + clipRect.xc_width <= 0)
        return;
    clipRect.x10_height = CGraphics::g_ViewportResolution.y * m_uniform.m_matrix[1][1];
    if (clipRect.x8_top + clipRect.x10_height <= 0)
        return;

    float oldW = clipRect.xc_width;
    if (clipRect.x4_left < 0)
    {
        clipRect.xc_width += clipRect.x4_left;
        m_uniform.m_indXf[0][0] = clipRect.xc_width / oldW;
        m_uniform.m_indXf[2][0] = -clipRect.x4_left / oldW;
        clipRect.x4_left = 0;
    }
    
    float oldH = clipRect.x10_height;
    if (clipRect.x8_top < 0)
    {
        clipRect.x10_height += clipRect.x8_top;
        m_uniform.m_indXf[1][1] = clipRect.x10_height / oldH;
        m_uniform.m_indXf[2][1] = -clipRect.x8_top / oldH;
        clipRect.x8_top = 0;
    }

    float tmp = clipRect.x4_left + clipRect.xc_width;
    if (tmp >= CGraphics::g_ViewportResolution.x)
    {
        clipRect.xc_width = CGraphics::g_ViewportResolution.x - clipRect.x4_left;
        m_uniform.m_indXf[0][0] = clipRect.xc_width / oldW;
    }
    
    tmp = clipRect.x8_top + clipRect.x10_height;
    if (tmp >= CGraphics::g_ViewportResolution.y)
    {
        clipRect.x10_height = CGraphics::g_ViewportResolution.y - clipRect.x8_top;
        m_uniform.m_indXf[1][1] = clipRect.x10_height / oldH;
    }

    CGraphics::ResolveSpareTexture(clipRect);

    /* Transform UV coordinates of rectangle within viewport and sampled scene texels (clamped to viewport bounds) */
    zeus::CVector2f vp{CGraphics::g_ViewportResolution.x, CGraphics::g_ViewportResolution.y};
    m_uniform.m_matrix[0][0] = clipRect.xc_width / vp.x;
    m_uniform.m_matrix[1][1] = clipRect.x10_height / vp.y;
    m_uniform.m_matrix[2][0] = pt.x + (1.f / vp.x);
    m_uniform.m_matrix[2][1] = pt.y + (1.f / vp.y);
    
    m_uniform.m_strength.x = m_uniform.m_matrix[0][0] * m_strength * 0.5f;
    m_uniform.m_strength.y = m_uniform.m_matrix[1][1] * m_strength * 0.5f;
    m_uniBuf->load(&m_uniform, sizeof(m_uniform));

    CGraphics::g_BooMainCommandQueue->setShaderDataBinding(m_dataBind);
    CGraphics::g_BooMainCommandQueue->draw(0, 4);
}

URDE_SPECIALIZE_SHADER(CSpaceWarpFilter)

}
