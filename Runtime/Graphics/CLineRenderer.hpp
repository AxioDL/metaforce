#pragma once

#include <array>

#include "Runtime/RetroTypes.hpp"
#include "Runtime/rstl.hpp"
#include "Runtime/Graphics/CGraphics.hpp"

#include <zeus/CColor.hpp>
#include <zeus/CVector2f.hpp>
#include <zeus/CVector3f.hpp>
#include <zeus/CVector4f.hpp>

namespace metaforce {

class CLineRenderer {
public:
  enum class EPrimitiveMode { Lines, LineStrip, LineLoop };

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

//  static rstl::reserved_vector<SDrawVertTex, 1024> g_StaticLineVertsTex;
//  static rstl::reserved_vector<SDrawVertNoTex, 1024> g_StaticLineVertsNoTex;

//  static hecl::VertexBufferPool<SDrawVertTex> s_vertPoolTex;
//  static hecl::VertexBufferPool<SDrawVertNoTex> s_vertPoolNoTex;
//  static hecl::UniformBufferPool<SDrawUniform> s_uniformPool;

public:
//  hecl::VertexBufferPool<SDrawVertTex>::Token m_vertBufTex;
//  hecl::VertexBufferPool<SDrawVertNoTex>::Token m_vertBufNoTex;
//  hecl::UniformBufferPool<SDrawUniform>::Token m_uniformBuf;
//  std::array<boo::ObjToken<boo::IShaderDataBinding>, 2> m_shaderBind;

  CLineRenderer(EPrimitiveMode mode, u32 maxVerts, const aurora::gfx::TextureHandle& texture, bool additive,
                bool zTest = false, bool zGEqual = false);
  CLineRenderer(CLineRenderer&&) = default;

  void Reset();
  void AddVertex(const zeus::CVector3f& position, const zeus::CColor& color, float width,
                 const zeus::CVector2f& uv = zeus::skZero2f);
  void Render(bool alphaWrite = false, const zeus::CColor& moduColor = zeus::skWhite);

//  static void UpdateBuffers() {
//    s_vertPoolTex.updateBuffers();
//    s_vertPoolNoTex.updateBuffers();
//    s_uniformPool.updateBuffers();
//  }

  static void Initialize();
  static void Shutdown();
};

} // namespace metaforce
