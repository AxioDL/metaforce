#include "CLineRenderer.hpp"
#include "Shaders/CLineRendererShaders.hpp"

namespace urde {
logvisor::Module LineRendererLog("urde::CLineRenderer");

void CLineRenderer::Initialize() { CLineRendererShaders::Initialize(); }

void CLineRenderer::Shutdown() {
  CLineRendererShaders::Shutdown();
  s_vertPoolTex.doDestroy();
  s_vertPoolNoTex.doDestroy();
  s_uniformPool.doDestroy();
}

hecl::VertexBufferPool<CLineRenderer::SDrawVertTex> CLineRenderer::s_vertPoolTex = {};
hecl::VertexBufferPool<CLineRenderer::SDrawVertNoTex> CLineRenderer::s_vertPoolNoTex = {};
hecl::UniformBufferPool<CLineRenderer::SDrawUniform> CLineRenderer::s_uniformPool = {};

CLineRenderer::CLineRenderer(boo::IGraphicsDataFactory::Context& ctx, EPrimitiveMode mode, u32 maxVerts,
                             const boo::ObjToken<boo::ITexture>& texture, bool additive, bool zTest, bool zGEqual)
: m_mode(mode), m_maxVerts(maxVerts) {
  if (maxVerts < 2) {
    LineRendererLog.report(logvisor::Fatal, fmt(_SYS_STR("maxVerts < 2, maxVerts = {}")), maxVerts);
    return;
  }
  m_textured = bool(texture);

  u32 maxTriVerts;
  switch (mode) {
  case EPrimitiveMode::Lines:
  case EPrimitiveMode::LineStrip:
    maxTriVerts = maxVerts * 4;
    break;
  case EPrimitiveMode::LineLoop:
    maxTriVerts = maxVerts * 4 + 4;
    break;
  }

  if (bool(texture))
    m_vertBufTex = s_vertPoolTex.allocateBlock(CGraphics::g_BooFactory, maxTriVerts);
  else
    m_vertBufNoTex = s_vertPoolNoTex.allocateBlock(CGraphics::g_BooFactory, maxTriVerts);

  m_uniformBuf = s_uniformPool.allocateBlock(CGraphics::g_BooFactory);

  CLineRendererShaders::BuildShaderDataBinding(ctx, *this, texture, additive, zTest, zGEqual);
}

CLineRenderer::CLineRenderer(EPrimitiveMode mode, u32 maxVerts, const boo::ObjToken<boo::ITexture>& texture,
                             bool additive, bool zTest, bool zGEqual)
: m_mode(mode), m_maxVerts(maxVerts) {
  if (maxVerts < 2) {
    LineRendererLog.report(logvisor::Fatal, fmt(_SYS_STR("maxVerts < 2, maxVerts = {}")), maxVerts);
    return;
  }
  m_textured = bool(texture);

  u32 maxTriVerts;
  switch (mode) {
  case EPrimitiveMode::Lines:
  case EPrimitiveMode::LineStrip:
    maxTriVerts = maxVerts * 4;
    break;
  case EPrimitiveMode::LineLoop:
    maxTriVerts = maxVerts * 4 + 4;
    break;
  }

  if (bool(texture))
    m_vertBufTex = s_vertPoolTex.allocateBlock(CGraphics::g_BooFactory, maxTriVerts);
  else
    m_vertBufNoTex = s_vertPoolNoTex.allocateBlock(CGraphics::g_BooFactory, maxTriVerts);

  m_uniformBuf = s_uniformPool.allocateBlock(CGraphics::g_BooFactory);

  CGraphics::CommitResources([&](boo::IGraphicsDataFactory::Context& ctx) {
    CLineRendererShaders::BuildShaderDataBinding(ctx, *this, texture, additive, zTest, zGEqual);
    return true;
  } BooTrace);
}

rstl::reserved_vector<CLineRenderer::SDrawVertTex, 1024> CLineRenderer::g_StaticLineVertsTex = {};
rstl::reserved_vector<CLineRenderer::SDrawVertNoTex, 1024> CLineRenderer::g_StaticLineVertsNoTex = {};

static bool IntersectLines(const zeus::CVector2f& pa1, const zeus::CVector2f& pa2, const zeus::CVector2f& pb1,
                           const zeus::CVector2f& pb2, zeus::CVector3f& intersect) {
  zeus::CVector2f deltaA = pa1 - pa2;
  zeus::CVector2f deltaB = pb1 - pb2;
  float det = deltaA.cross(deltaB);
  if (std::fabs(det) < 0.000001f)
    return false;
  float c0 = pa1.cross(pa2);
  float c1 = pb1.cross(pb2);
  intersect = (c0 * (pb1 - pb2) - c1 * (pa1 - pa2)) / det;
  return true;
}

void CLineRenderer::Reset() {
  m_nextVert = 0;
  m_final = false;
  if (m_textured)
    g_StaticLineVertsTex.clear();
  else
    g_StaticLineVertsNoTex.clear();
}

void CLineRenderer::AddVertex(const zeus::CVector3f& position, const zeus::CColor& color, float width,
                              const zeus::CVector2f& uv) {
  if (m_final || !m_shaderBind[0] || m_nextVert >= m_maxVerts)
    return;

  float adjWidth = width / 480.f;
  float w;
  zeus::CVector3f projPt = CGraphics::ProjectModelPointToViewportSpace(position, w);

  if (m_mode == EPrimitiveMode::LineLoop) {
    if (m_nextVert == 0) {
      m_firstPos = projPt;
      m_firstW = w;
      m_secondPos = projPt;
      m_firstUV = uv;
      m_firstColor = color;
      m_firstWidth = adjWidth;
    } else if (m_nextVert == 1) {
      m_secondPos = projPt;
    }
  }

  if (m_nextVert > 1) {
    zeus::CVector2f dva = (m_lastPos - m_lastPos2).toVec2f();
    if (!dva.canBeNormalized())
      dva = {0.f, 1.f};
    dva = dva.normalized().perpendicularVector() * m_lastWidth;
    dva.x() /= CGraphics::g_ProjAspect;

    zeus::CVector2f dvb = (projPt - m_lastPos).toVec2f();
    if (!dvb.canBeNormalized())
      dvb = {0.f, 1.f};
    dvb = dvb.normalized().perpendicularVector() * m_lastWidth;
    dvb.x() /= CGraphics::g_ProjAspect;

    if (m_textured) {
      if (m_mode == EPrimitiveMode::Lines) {
        if (m_nextVert & 1) {
          g_StaticLineVertsTex.push_back(g_StaticLineVertsTex.back());
          g_StaticLineVertsTex.push_back({zeus::CVector4f::ToClip(m_lastPos + dvb, m_lastW), m_lastColor, m_lastUV});
          g_StaticLineVertsTex.push_back(g_StaticLineVertsTex.back());
          g_StaticLineVertsTex.push_back({zeus::CVector4f::ToClip(m_lastPos - dvb, m_lastW), m_lastColor, m_lastUV});
        } else {
          g_StaticLineVertsTex.push_back({zeus::CVector4f::ToClip(m_lastPos + dva, m_lastW), m_lastColor, m_lastUV});
          g_StaticLineVertsTex.push_back({zeus::CVector4f::ToClip(m_lastPos - dva, m_lastW), m_lastColor, m_lastUV});
        }
      } else {
        zeus::CVector3f intersect1;
        bool good1 = IntersectLines(m_lastPos2.toVec2f() + dva, m_lastPos.toVec2f() + dva, m_lastPos.toVec2f() + dvb,
                                    projPt.toVec2f() + dvb, intersect1);
        if ((intersect1.toVec2f() - m_lastPos.toVec2f()).magnitude() > m_lastWidth * 2.f)
          good1 = false;

        zeus::CVector3f intersect2;
        bool good2 = IntersectLines(m_lastPos2.toVec2f() - dva, m_lastPos.toVec2f() - dva, m_lastPos.toVec2f() - dvb,
                                    projPt.toVec2f() - dvb, intersect2);
        if ((intersect2.toVec2f() - m_lastPos.toVec2f()).magnitude() > m_lastWidth * 2.f)
          good2 = false;

        if (good1 && good2) {
          intersect1.z() = float(m_lastPos.z());
          intersect2.z() = float(m_lastPos.z());
          g_StaticLineVertsTex.push_back({zeus::CVector4f::ToClip(intersect1, m_lastW), m_lastColor, m_lastUV});
          g_StaticLineVertsTex.push_back({zeus::CVector4f::ToClip(intersect2, m_lastW), m_lastColor, m_lastUV});
        } else {
          g_StaticLineVertsTex.push_back({zeus::CVector4f::ToClip(m_lastPos + dva, m_lastW), m_lastColor, m_lastUV});
          g_StaticLineVertsTex.push_back({zeus::CVector4f::ToClip(m_lastPos - dva, m_lastW), m_lastColor, m_lastUV});
          g_StaticLineVertsTex.push_back({zeus::CVector4f::ToClip(m_lastPos + dvb, m_lastW), m_lastColor, m_lastUV});
          g_StaticLineVertsTex.push_back({zeus::CVector4f::ToClip(m_lastPos - dvb, m_lastW), m_lastColor, m_lastUV});
        }
      }
    } else {
      if (m_mode == EPrimitiveMode::Lines) {
        if (m_nextVert & 1) {
          g_StaticLineVertsNoTex.push_back(g_StaticLineVertsNoTex.back());
          g_StaticLineVertsNoTex.push_back({zeus::CVector4f::ToClip(m_lastPos + dvb, m_lastW), m_lastColor});
          g_StaticLineVertsNoTex.push_back(g_StaticLineVertsNoTex.back());
          g_StaticLineVertsNoTex.push_back({zeus::CVector4f::ToClip(m_lastPos - dvb, m_lastW), m_lastColor});
        } else {
          g_StaticLineVertsNoTex.push_back({zeus::CVector4f::ToClip(m_lastPos + dva, m_lastW), m_lastColor});
          g_StaticLineVertsNoTex.push_back({zeus::CVector4f::ToClip(m_lastPos - dva, m_lastW), m_lastColor});
        }
      } else {
        zeus::CVector3f intersect1;
        bool good1 = IntersectLines(m_lastPos2.toVec2f() + dva, m_lastPos.toVec2f() + dva, m_lastPos.toVec2f() + dvb,
                                    projPt.toVec2f() + dvb, intersect1);
        if ((intersect1.toVec2f() - m_lastPos.toVec2f()).magnitude() > m_lastWidth * 2.f)
          good1 = false;

        zeus::CVector3f intersect2;
        bool good2 = IntersectLines(m_lastPos2.toVec2f() - dva, m_lastPos.toVec2f() - dva, m_lastPos.toVec2f() - dvb,
                                    projPt.toVec2f() - dvb, intersect2);
        if ((intersect2.toVec2f() - m_lastPos.toVec2f()).magnitude() > m_lastWidth * 2.f)
          good2 = false;

        if (good1 && good2) {
          intersect1.z() = float(m_lastPos.z());
          intersect2.z() = float(m_lastPos.z());
          g_StaticLineVertsNoTex.push_back({zeus::CVector4f::ToClip(intersect1, m_lastW), m_lastColor});
          g_StaticLineVertsNoTex.push_back({zeus::CVector4f::ToClip(intersect2, m_lastW), m_lastColor});
        } else {
          g_StaticLineVertsNoTex.push_back({zeus::CVector4f::ToClip(m_lastPos + dva, m_lastW), m_lastColor});
          g_StaticLineVertsNoTex.push_back({zeus::CVector4f::ToClip(m_lastPos - dva, m_lastW), m_lastColor});
          g_StaticLineVertsNoTex.push_back({zeus::CVector4f::ToClip(m_lastPos + dvb, m_lastW), m_lastColor});
          g_StaticLineVertsNoTex.push_back({zeus::CVector4f::ToClip(m_lastPos - dvb, m_lastW), m_lastColor});
        }
      }
    }
  } else if (m_nextVert == 1) {
    zeus::CVector2f dv = (projPt - m_lastPos).toVec2f();
    if (!dv.canBeNormalized())
      dv = {0.f, 1.f};
    dv = dv.normalized().perpendicularVector() * m_lastWidth;
    dv.x() /= CGraphics::g_ProjAspect;
    if (m_textured) {
      g_StaticLineVertsTex.push_back({zeus::CVector4f::ToClip(m_lastPos + dv, m_lastW), m_lastColor, m_lastUV});
      g_StaticLineVertsTex.push_back({zeus::CVector4f::ToClip(m_lastPos - dv, m_lastW), m_lastColor, m_lastUV});
    } else {
      g_StaticLineVertsNoTex.push_back({zeus::CVector4f::ToClip(m_lastPos + dv, m_lastW), m_lastColor});
      g_StaticLineVertsNoTex.push_back({zeus::CVector4f::ToClip(m_lastPos - dv, m_lastW), m_lastColor});
    }
  }

  m_lastPos2 = m_lastPos;
  m_lastPos = projPt;
  m_lastW = w;
  m_lastUV = uv;
  m_lastColor = color;
  m_lastWidth = adjWidth;
  ++m_nextVert;
}

void CLineRenderer::Render(bool alphaWrite, const zeus::CColor& moduColor) {
  SCOPED_GRAPHICS_DEBUG_GROUP("CLineRenderer::Render", zeus::skGrey);

  if (!m_final && m_nextVert > 1) {
    if (m_mode == EPrimitiveMode::LineLoop) {
      {
        zeus::CVector2f dva = (m_lastPos - m_lastPos2).toVec2f();
        if (!dva.canBeNormalized())
          dva = {0.f, 1.f};
        dva = dva.normalized().perpendicularVector() * m_lastWidth;
        dva.x() /= CGraphics::g_ProjAspect;

        zeus::CVector2f dvb = (m_firstPos - m_lastPos).toVec2f();
        if (!dvb.canBeNormalized())
          dvb = {0.f, 1.f};
        dvb = dvb.normalized().perpendicularVector() * m_lastWidth;
        dvb.x() /= CGraphics::g_ProjAspect;

        zeus::CVector3f intersect1;
        bool good1 = IntersectLines(m_lastPos2.toVec2f() + dva, m_lastPos.toVec2f() + dva, m_lastPos.toVec2f() + dvb,
                                    m_firstPos.toVec2f() + dvb, intersect1);
        if ((intersect1.toVec2f() - m_lastPos.toVec2f()).magnitude() > m_lastWidth * 2.f)
          good1 = false;

        zeus::CVector3f intersect2;
        bool good2 = IntersectLines(m_lastPos2.toVec2f() - dva, m_lastPos.toVec2f() - dva, m_lastPos.toVec2f() - dvb,
                                    m_firstPos.toVec2f() - dvb, intersect2);
        if ((intersect2.toVec2f() - m_lastPos.toVec2f()).magnitude() > m_lastWidth * 2.f)
          good2 = false;

        if (m_textured) {
          if (good1 && good2) {
            intersect1.z() = float(m_lastPos.z());
            intersect2.z() = float(m_lastPos.z());
            g_StaticLineVertsTex.push_back({zeus::CVector4f::ToClip(intersect1, m_lastW), m_lastColor, m_lastUV});
            g_StaticLineVertsTex.push_back({zeus::CVector4f::ToClip(intersect2, m_lastW), m_lastColor, m_lastUV});
          } else {
            g_StaticLineVertsTex.push_back({zeus::CVector4f::ToClip(m_lastPos + dva, m_lastW), m_lastColor, m_lastUV});
            g_StaticLineVertsTex.push_back({zeus::CVector4f::ToClip(m_lastPos - dva, m_lastW), m_lastColor, m_lastUV});
            g_StaticLineVertsTex.push_back({zeus::CVector4f::ToClip(m_lastPos + dvb, m_lastW), m_lastColor, m_lastUV});
            g_StaticLineVertsTex.push_back({zeus::CVector4f::ToClip(m_lastPos - dvb, m_lastW), m_lastColor, m_lastUV});
          }
        } else {
          if (good1 && good2) {
            intersect1.z() = float(m_lastPos.z());
            intersect2.z() = float(m_lastPos.z());
            g_StaticLineVertsNoTex.push_back({zeus::CVector4f::ToClip(intersect1, m_lastW), m_lastColor});
            g_StaticLineVertsNoTex.push_back({zeus::CVector4f::ToClip(intersect2, m_lastW), m_lastColor});
          } else {
            g_StaticLineVertsNoTex.push_back({zeus::CVector4f::ToClip(m_lastPos + dva, m_lastW), m_lastColor});
            g_StaticLineVertsNoTex.push_back({zeus::CVector4f::ToClip(m_lastPos - dva, m_lastW), m_lastColor});
            g_StaticLineVertsNoTex.push_back({zeus::CVector4f::ToClip(m_lastPos + dvb, m_lastW), m_lastColor});
            g_StaticLineVertsNoTex.push_back({zeus::CVector4f::ToClip(m_lastPos - dvb, m_lastW), m_lastColor});
          }
        }
      }
      {
        zeus::CVector2f dva = (m_firstPos - m_lastPos).toVec2f();
        if (!dva.canBeNormalized())
          dva = {0.f, 1.f};
        dva = dva.normalized().perpendicularVector() * m_firstWidth;
        dva.x() /= CGraphics::g_ProjAspect;

        zeus::CVector2f dvb = (m_secondPos - m_firstPos).toVec2f();
        if (!dvb.canBeNormalized())
          dvb = {0.f, 1.f};
        dvb = dvb.normalized().perpendicularVector() * m_firstWidth;
        dvb.x() /= CGraphics::g_ProjAspect;

        zeus::CVector3f intersect1;
        bool good1 = IntersectLines(m_lastPos.toVec2f() + dva, m_firstPos.toVec2f() + dva, m_firstPos.toVec2f() + dvb,
                                    m_secondPos.toVec2f() + dvb, intersect1);
        if ((intersect1.toVec2f() - m_firstPos.toVec2f()).magnitude() > m_firstWidth * 2.f)
          good1 = false;

        zeus::CVector3f intersect2;
        bool good2 = IntersectLines(m_lastPos.toVec2f() - dva, m_firstPos.toVec2f() - dva, m_firstPos.toVec2f() - dvb,
                                    m_secondPos.toVec2f() - dvb, intersect2);
        if ((intersect2.toVec2f() - m_firstPos.toVec2f()).magnitude() > m_firstWidth * 2.f)
          good2 = false;

        if (m_textured) {
          if (good1 && good2) {
            intersect1.z() = float(m_firstPos.z());
            intersect2.z() = float(m_firstPos.z());
            g_StaticLineVertsTex.push_back({zeus::CVector4f::ToClip(intersect1, m_lastW), m_lastColor, m_lastUV});
            g_StaticLineVertsTex.push_back({zeus::CVector4f::ToClip(intersect2, m_lastW), m_lastColor, m_lastUV});
          } else {
            g_StaticLineVertsTex.push_back({zeus::CVector4f::ToClip(m_firstPos + dva, m_lastW), m_lastColor, m_lastUV});
            g_StaticLineVertsTex.push_back({zeus::CVector4f::ToClip(m_firstPos - dva, m_lastW), m_lastColor, m_lastUV});
            g_StaticLineVertsTex.push_back({zeus::CVector4f::ToClip(m_firstPos + dvb, m_lastW), m_lastColor, m_lastUV});
            g_StaticLineVertsTex.push_back({zeus::CVector4f::ToClip(m_firstPos - dvb, m_lastW), m_lastColor, m_lastUV});
          }
        } else {
          if (good1 && good2) {
            intersect1.z() = float(m_firstPos.z());
            intersect2.z() = float(m_firstPos.z());
            g_StaticLineVertsNoTex.push_back({zeus::CVector4f::ToClip(intersect1, m_lastW), m_lastColor});
            g_StaticLineVertsNoTex.push_back({zeus::CVector4f::ToClip(intersect2, m_lastW), m_lastColor});
          } else {
            g_StaticLineVertsNoTex.push_back({zeus::CVector4f::ToClip(m_firstPos + dva, m_lastW), m_lastColor});
            g_StaticLineVertsNoTex.push_back({zeus::CVector4f::ToClip(m_firstPos - dva, m_lastW), m_lastColor});
            g_StaticLineVertsNoTex.push_back({zeus::CVector4f::ToClip(m_firstPos + dvb, m_lastW), m_lastColor});
            g_StaticLineVertsNoTex.push_back({zeus::CVector4f::ToClip(m_firstPos - dvb, m_lastW), m_lastColor});
          }
        }
      }
    } else {
      zeus::CVector2f dv = (m_lastPos - m_lastPos2).toVec2f();
      if (!dv.canBeNormalized())
        dv = {0.f, 1.f};
      dv = dv.normalized().perpendicularVector() * m_lastWidth;
      dv.x() /= CGraphics::g_ProjAspect;
      if (m_textured) {
        if (m_mode == EPrimitiveMode::Lines && (m_nextVert & 1)) {
        } else {
          g_StaticLineVertsTex.push_back({zeus::CVector4f::ToClip(m_lastPos + dv, m_lastW), m_lastColor, m_lastUV});
          g_StaticLineVertsTex.push_back({zeus::CVector4f::ToClip(m_lastPos - dv, m_lastW), m_lastColor, m_lastUV});
        }
      } else {
        if (m_mode == EPrimitiveMode::Lines && (m_nextVert & 1)) {
        } else {
          g_StaticLineVertsNoTex.push_back({zeus::CVector4f::ToClip(m_lastPos + dv, m_lastW), m_lastColor});
          g_StaticLineVertsNoTex.push_back({zeus::CVector4f::ToClip(m_lastPos - dv, m_lastW), m_lastColor});
        }
      }
    }

    m_final = true;
  }

  m_uniformBuf.access() = SDrawUniform{moduColor, CGraphics::g_Fog};
  if (m_textured) {
    if (!g_StaticLineVertsTex.empty()) {
      memmove(m_vertBufTex.access(), g_StaticLineVertsTex.data(), sizeof(SDrawVertTex) * g_StaticLineVertsTex.size());
      CGraphics::SetShaderDataBinding(m_shaderBind[alphaWrite]);
      CGraphics::DrawArray(0, g_StaticLineVertsTex.size());
    }
  } else {
    if (!g_StaticLineVertsNoTex.empty()) {
      memmove(m_vertBufNoTex.access(), g_StaticLineVertsNoTex.data(),
              sizeof(SDrawVertNoTex) * g_StaticLineVertsNoTex.size());
      CGraphics::SetShaderDataBinding(m_shaderBind[alphaWrite]);
      CGraphics::DrawArray(0, g_StaticLineVertsNoTex.size());
    }
  }
}

} // namespace urde
