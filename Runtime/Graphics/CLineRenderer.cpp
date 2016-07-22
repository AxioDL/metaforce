#include "CLineRenderer.hpp"
#include "Shaders/CLineRendererShaders.hpp"

namespace urde
{
logvisor::Module LineRendererLog("urde::CLineRenderer");

void CLineRenderer::Initialize()
{
    CLineRendererShaders::Initialize();
}

void CLineRenderer::Shutdown()
{
    CLineRendererShaders::Shutdown();
}

struct SDrawVertTex
{
    zeus::CVector4f pos;
    zeus::CColor color;
    zeus::CVector2f uv;
};

struct SDrawVertNoTex
{
    zeus::CVector4f pos;
    zeus::CColor color;
};

struct SDrawUniform
{
    zeus::CColor moduColor;
};

CLineRenderer::CLineRenderer(EPrimitiveMode mode, u32 maxVerts, boo::ITexture* texture, bool additive)
: m_mode(mode), m_maxVerts(maxVerts)
{
    if (maxVerts < 2)
    {
        LineRendererLog.report(logvisor::Fatal, _S("maxVerts < 2, maxVerts = %i"), maxVerts);
        return;
    }
    m_textured = texture != nullptr;

    u32 maxTriVerts;
    switch (mode)
    {
    case EPrimitiveMode::Lines:
        maxTriVerts = maxVerts * 3;
        break;
    case EPrimitiveMode::LineStrip:
        maxTriVerts = maxVerts * 2;
        break;
    case EPrimitiveMode::LineLoop:
        maxTriVerts = maxVerts * 2 + 2;
        break;
    }

    m_gfxToken = CGraphics::CommitResources([&](boo::IGraphicsDataFactory::Context& ctx) -> bool
    {
        m_vertBuf = ctx.newDynamicBuffer(boo::BufferUse::Vertex,
                                         texture ? sizeof(SDrawVertTex) : sizeof(SDrawVertNoTex),
                                         maxTriVerts);
        m_uniformBuf = ctx.newDynamicBuffer(boo::BufferUse::Uniform, sizeof(SDrawUniform), 1);
        CLineRendererShaders::BuildShaderDataBinding(ctx, *this, texture, additive);
        return true;
    });
}

static rstl::reserved_vector<SDrawVertTex, 256> g_StaticLineVertsTex;
static rstl::reserved_vector<SDrawVertNoTex, 256> g_StaticLineVertsNoTex;

static zeus::CVector2f IntersectLines(const zeus::CVector2f& pa1, const zeus::CVector2f& pa2,
                                      const zeus::CVector2f& pb1, const zeus::CVector2f& pb2)
{
    zeus::CVector2f pa1mpa2 = pa1 - pa2;
    zeus::CVector2f pb1mpb2 = pb1 - pb2;
    float denom = pa1mpa2.x * pb1mpb2.y - pa1mpa2.y * pb1mpb2.x;
    float numt1 = pa1.x * pa2.y - pa1.y * pa2.x;
    float numt2 = pb1.x * pb2.y - pb1.y * pb2.x;
    return {(numt1 * pb1mpb2.x - pa1mpa2.x * numt2) / denom,
            (numt1 * pb1mpb2.y - pa1mpa2.y * numt2) / denom};
}

void CLineRenderer::Reset()
{
    m_nextVert = 0;
    m_final = false;
    if (m_textured)
        g_StaticLineVertsTex.clear();
    else
        g_StaticLineVertsNoTex.clear();
}

void CLineRenderer::AddVertex(const zeus::CVector3f& position, const zeus::CColor& color, float width,
                              const zeus::CVector2f& uv)
{
    if (m_final || !m_shaderBind || m_nextVert >= m_maxVerts)
        return;

    float adjWidth = width / 480.f;
    zeus::CVector3f projPt = CGraphics::ProjectModelPointToViewportSpace(position);

    if (m_mode == EPrimitiveMode::LineLoop)
    {
        if (m_nextVert == 0)
        {
            m_firstPos = projPt;
            m_secondPos = projPt;
            m_firstUV = uv;
            m_firstColor = color;
            m_firstWidth = adjWidth;
        }
        else if (m_nextVert == 1)
        {
            m_secondPos = projPt;
        }
    }

    if (m_nextVert > 1)
    {
        zeus::CVector2f dva = (m_lastPos - m_lastPos2).toVec2f();
        if (!dva.canBeNormalized())
            dva = {0.f, 1.f};
        dva = dva.normalized().perpendicularVector() * m_lastWidth;
        dva.x /= CGraphics::g_ProjAspect;

        zeus::CVector2f dvb = (projPt - m_lastPos).toVec2f();
        if (!dvb.canBeNormalized())
            dvb = {0.f, 1.f};
        dvb = dvb.normalized().perpendicularVector() * m_lastWidth;
        dvb.x /= CGraphics::g_ProjAspect;

        if (m_textured)
        {
            if (m_mode == EPrimitiveMode::Lines)
            {
                if (m_nextVert & 1)
                {
                    g_StaticLineVertsTex.push_back(g_StaticLineVertsTex.back());
                    g_StaticLineVertsTex.push_back({m_lastPos + dvb, m_lastColor, m_lastUV});
                    g_StaticLineVertsTex.push_back(g_StaticLineVertsTex.back());
                    g_StaticLineVertsTex.push_back({m_lastPos - dvb, m_lastColor, m_lastUV});
                }
                else
                {
                    g_StaticLineVertsTex.push_back({m_lastPos + dva, m_lastColor, m_lastUV});
                    g_StaticLineVertsTex.push_back({m_lastPos - dva, m_lastColor, m_lastUV});
                }
            }
            else
            {
                zeus::CVector3f intersect1 = IntersectLines(m_lastPos2.toVec2f() + dva, m_lastPos.toVec2f() + dva,
                                                            m_lastPos.toVec2f() + dvb, projPt.toVec2f() + dvb);
                intersect1.z = m_lastPos.z;

                zeus::CVector3f intersect2 = IntersectLines(m_lastPos2.toVec2f() - dva, m_lastPos.toVec2f() - dva,
                                                            m_lastPos.toVec2f() - dvb, projPt.toVec2f() - dvb);
                intersect2.z = m_lastPos.z;

                g_StaticLineVertsTex.push_back({intersect1, m_lastColor, m_lastUV});
                g_StaticLineVertsTex.push_back({intersect2, m_lastColor, m_lastUV});
            }
        }
        else
        {
            if (m_mode == EPrimitiveMode::Lines)
            {
                if (m_nextVert & 1)
                {
                    g_StaticLineVertsNoTex.push_back(g_StaticLineVertsNoTex.back());
                    g_StaticLineVertsNoTex.push_back({m_lastPos + dvb, m_lastColor});
                    g_StaticLineVertsNoTex.push_back(g_StaticLineVertsNoTex.back());
                    g_StaticLineVertsNoTex.push_back({m_lastPos - dvb, m_lastColor});
                }
                else
                {
                    g_StaticLineVertsNoTex.push_back({m_lastPos + dva, m_lastColor});
                    g_StaticLineVertsNoTex.push_back({m_lastPos - dva, m_lastColor});
                }
            }
            else
            {
                zeus::CVector3f intersect1 = IntersectLines(m_lastPos2.toVec2f() + dva, m_lastPos.toVec2f() + dva,
                                                            m_lastPos.toVec2f() + dvb, projPt.toVec2f() + dvb);
                intersect1.z = m_lastPos.z;

                zeus::CVector3f intersect2 = IntersectLines(m_lastPos2.toVec2f() - dva, m_lastPos.toVec2f() - dva,
                                                            m_lastPos.toVec2f() - dvb, projPt.toVec2f() - dvb);
                intersect2.z = m_lastPos.z;

                g_StaticLineVertsNoTex.push_back({intersect1, m_lastColor});
                g_StaticLineVertsNoTex.push_back({intersect2, m_lastColor});
            }
        }
    }
    else if (m_nextVert == 1)
    {
        zeus::CVector2f dv = (projPt - m_lastPos).toVec2f();
        if (!dv.canBeNormalized())
            dv = {0.f, 1.f};
        dv = dv.normalized().perpendicularVector() * m_lastWidth;
        dv.x /= CGraphics::g_ProjAspect;
        if (m_textured)
        {
            g_StaticLineVertsTex.push_back({m_lastPos + dv, m_lastColor, m_lastUV});
            g_StaticLineVertsTex.push_back({m_lastPos - dv, m_lastColor, m_lastUV});
        }
        else
        {
            g_StaticLineVertsNoTex.push_back({m_lastPos + dv, m_lastColor});
            g_StaticLineVertsNoTex.push_back({m_lastPos - dv, m_lastColor});
        }
    }

    m_lastPos2 = m_lastPos;
    m_lastPos = projPt;
    m_lastUV = uv;
    m_lastColor = color;
    m_lastWidth = adjWidth;
    ++m_nextVert;
}

void CLineRenderer::Render(const zeus::CColor& moduColor)
{
    if (!m_final && m_nextVert > 1)
    {
        if (m_mode == EPrimitiveMode::LineLoop)
        {
            {
                zeus::CVector2f dva = (m_lastPos - m_lastPos2).toVec2f();
                if (!dva.canBeNormalized())
                    dva = {0.f, 1.f};
                dva = dva.normalized().perpendicularVector() * m_lastWidth;
                dva.x /= CGraphics::g_ProjAspect;

                zeus::CVector2f dvb = (m_firstPos - m_lastPos).toVec2f();
                if (!dvb.canBeNormalized())
                    dvb = {0.f, 1.f};
                dvb = dvb.normalized().perpendicularVector() * m_lastWidth;
                dvb.x /= CGraphics::g_ProjAspect;

                zeus::CVector3f intersect1 = IntersectLines(m_lastPos2.toVec2f() + dva, m_lastPos.toVec2f() + dva,
                                                            m_lastPos.toVec2f() + dvb, m_firstPos.toVec2f() + dvb);
                intersect1.z = m_lastPos.z;

                zeus::CVector3f intersect2 = IntersectLines(m_lastPos2.toVec2f() - dva, m_lastPos.toVec2f() - dva,
                                                            m_lastPos.toVec2f() - dvb, m_firstPos.toVec2f() - dvb);
                intersect2.z = m_lastPos.z;

                if (m_textured)
                {
                    g_StaticLineVertsTex.push_back({intersect1, m_lastColor, m_lastUV});
                    g_StaticLineVertsTex.push_back({intersect2, m_lastColor, m_lastUV});
                }
                else
                {
                    g_StaticLineVertsNoTex.push_back({intersect1, m_lastColor});
                    g_StaticLineVertsNoTex.push_back({intersect2, m_lastColor});
                }
            }
            {
                zeus::CVector2f dva = (m_firstPos - m_lastPos).toVec2f();
                if (!dva.canBeNormalized())
                    dva = {0.f, 1.f};
                dva = dva.normalized().perpendicularVector() * m_firstWidth;
                dva.x /= CGraphics::g_ProjAspect;

                zeus::CVector2f dvb = (m_secondPos - m_firstPos).toVec2f();
                if (!dvb.canBeNormalized())
                    dvb = {0.f, 1.f};
                dvb = dvb.normalized().perpendicularVector() * m_firstWidth;
                dvb.x /= CGraphics::g_ProjAspect;

                zeus::CVector3f intersect1 = IntersectLines(m_lastPos.toVec2f() + dva, m_firstPos.toVec2f() + dva,
                                                            m_firstPos.toVec2f() + dvb, m_secondPos.toVec2f() + dvb);
                intersect1.z = m_firstPos.z;

                zeus::CVector3f intersect2 = IntersectLines(m_lastPos.toVec2f() - dva, m_firstPos.toVec2f() - dva,
                                                            m_firstPos.toVec2f() - dvb, m_secondPos.toVec2f() - dvb);
                intersect2.z = m_firstPos.z;

                if (m_textured)
                {
                    g_StaticLineVertsTex.push_back({intersect1, m_firstColor, m_firstUV});
                    g_StaticLineVertsTex.push_back({intersect2, m_firstColor, m_firstUV});
                }
                else
                {
                    g_StaticLineVertsNoTex.push_back({intersect1, m_firstColor});
                    g_StaticLineVertsNoTex.push_back({intersect2, m_firstColor});
                }
            }
        }
        else
        {
            zeus::CVector2f dv = (m_lastPos - m_lastPos2).toVec2f();
            if (!dv.canBeNormalized())
                dv = {0.f, 1.f};
            dv = dv.normalized().perpendicularVector() * m_lastWidth;
            dv.x /= CGraphics::g_ProjAspect;
            if (m_textured)
            {
                if (m_mode == EPrimitiveMode::Lines && (m_nextVert & 1))
                {}
                else
                {
                    g_StaticLineVertsTex.push_back({m_lastPos + dv, m_lastColor, m_lastUV});
                    g_StaticLineVertsTex.push_back({m_lastPos - dv, m_lastColor, m_lastUV});
                }
            }
            else
            {
                if (m_mode == EPrimitiveMode::Lines && (m_nextVert & 1))
                {}
                else
                {
                    g_StaticLineVertsNoTex.push_back({m_lastPos + dv, m_lastColor});
                    g_StaticLineVertsNoTex.push_back({m_lastPos - dv, m_lastColor});
                }
            }
        }

        m_final = true;
    }

    SDrawUniform uniformData = {moduColor};
    m_uniformBuf->load(&uniformData, sizeof(SDrawUniform));
    if (m_textured)
    {
        m_vertBuf->load(g_StaticLineVertsTex.data(), sizeof(SDrawVertTex) * g_StaticLineVertsTex.size());
        CGraphics::SetShaderDataBinding(m_shaderBind);
        CGraphics::DrawArray(0, g_StaticLineVertsTex.size());
    }
    else
    {
        m_vertBuf->load(g_StaticLineVertsNoTex.data(), sizeof(SDrawVertNoTex) * g_StaticLineVertsNoTex.size());
        CGraphics::SetShaderDataBinding(m_shaderBind);
        CGraphics::DrawArray(0, g_StaticLineVertsNoTex.size());
    }
}

}
