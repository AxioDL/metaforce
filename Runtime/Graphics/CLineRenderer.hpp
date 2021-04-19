#pragma once

#include <array>

#include "Runtime/RetroTypes.hpp"
#include "Runtime/rstl.hpp"
#include "Runtime/Graphics/CGraphics.hpp"

#include "zeus/CColor.hpp"
#include "zeus/CVector2f.hpp"
#include "zeus/CVector3f.hpp"
#include "zeus/CVector4f.hpp"

namespace metaforce {

class CLineRenderer {
public:
  enum class EPrimitiveMode { Lines, LineStrip, LineLoop };

  struct SDrawVertNoTex {
    hsh::float4 pos;
    hsh::float4 color;
    SDrawVertNoTex(hsh::float4 pos, hsh::float4 color) : pos(pos), color(color) {}
  };

  struct SDrawVertTex {
    hsh::float4 pos;
    hsh::float4 color;
    hsh::float2 uv;
    SDrawVertTex(hsh::float4 pos, hsh::float4 color, hsh::float2 uv) : pos(pos), color(color), uv(uv) {}
  };

  struct SDrawUniform {
    hsh::float4 moduColor;
    CGraphics::CFogState fog;
  };

private:
  EPrimitiveMode m_mode;
  u32 m_maxVerts;
  u32 m_nextVert = 0;
  bool m_final = false;
  bool m_textured;

  zeus::CVector3f m_firstPos;
  zeus::CVector3f m_secondPos;
  zeus::CVector2f m_firstUV;
  zeus::CColor m_firstColor;
  float m_firstWidth;
  float m_firstW;

  zeus::CVector3f m_lastPos;
  zeus::CVector3f m_lastPos2;
  zeus::CVector2f m_lastUV;
  zeus::CColor m_lastColor;
  float m_lastWidth;
  float m_lastW;

  static rstl::reserved_vector<SDrawVertTex, 1024> g_StaticLineVertsTex;
  static rstl::reserved_vector<SDrawVertNoTex, 1024> g_StaticLineVertsNoTex;

public:
  hsh::dynamic_owner<hsh::vertex_buffer_typeless> m_vertBuf;
  hsh::dynamic_owner<hsh::uniform_buffer<SDrawUniform>> m_uniformBuf;
  hsh::binding m_shaderBind[2];

  CLineRenderer(EPrimitiveMode mode, u32 maxVerts, hsh::texture2d texture, bool additive,
                hsh::Compare zComp = hsh::Always);
  CLineRenderer(CLineRenderer&&) = default;

  void Reset();
  void AddVertex(const zeus::CVector3f& position, const zeus::CColor& color, float width,
                 const zeus::CVector2f& uv = zeus::skZero2f);
  void Render(bool alphaWrite = false, const zeus::CColor& moduColor = zeus::skWhite);
};

} // namespace metaforce
