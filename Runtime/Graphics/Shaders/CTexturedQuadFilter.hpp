#pragma once

#include "Runtime/CToken.hpp"

#include "hsh/hsh.h"

#include "zeus/CColor.hpp"
#include "zeus/CMatrix4f.hpp"
#include "zeus/CRectangle.hpp"
#include "zeus/CVector2f.hpp"
#include "zeus/CVector3f.hpp"

namespace metaforce {
class CTexture;

enum class EFilterShape;
enum class EFilterType;

class CTexturedQuadFilter {
public:
  enum class ZTest { None, LEqual, GEqual, GEqualZWrite };
  struct Vert {
    hsh::float3 m_pos;
    hsh::float2 m_uv;
  };
  struct Uniform {
    hsh::float4x4 m_matrix;
    hsh::float4 m_color;
    float m_lod = 0.f;
  };

protected:
  TLockedToken<CTexture> m_tex;
  hsh::dynamic_owner<hsh::vertex_buffer<Vert>> m_vbo;
  hsh::dynamic_owner<hsh::uniform_buffer<Uniform>> m_uniBuf;
  hsh::binding m_dataBind;
  Uniform m_uniform{};
  ZTest m_zTest;

  CTexturedQuadFilter() = default;

public:
  static constexpr zeus::CRectangle DefaultRect{0.f, 0.f, 1.f, 1.f};
  explicit CTexturedQuadFilter(EFilterType type, TLockedToken<CTexture> tex, ZTest zTest = ZTest::None);
  explicit CTexturedQuadFilter(EFilterType type, hsh::texture2d tex, ZTest zTest = ZTest::None);
  explicit CTexturedQuadFilter(EFilterType type, hsh::render_texture2d tex, ZTest zTest = ZTest::None);
  CTexturedQuadFilter(const CTexturedQuadFilter&) = delete;
  CTexturedQuadFilter& operator=(const CTexturedQuadFilter&) = delete;
  CTexturedQuadFilter(CTexturedQuadFilter&&) = default;
  CTexturedQuadFilter& operator=(CTexturedQuadFilter&&) = default;
  void draw(const zeus::CColor& color, float uvScale, const zeus::CRectangle& rect = DefaultRect, float z = 0.f);
  void drawCropped(const zeus::CColor& color, float uvScale);
  void drawVerts(const zeus::CColor& color, const std::array<Vert, 4>& verts, float lod = 0.f);
  void DrawFilter(EFilterShape shape, const zeus::CColor& color, float t);
  const TLockedToken<CTexture>& GetTex() const { return m_tex; }
};

class CTexturedQuadFilterAlpha : public CTexturedQuadFilter {
public:
  explicit CTexturedQuadFilterAlpha(EFilterType type, TLockedToken<CTexture> tex);
  explicit CTexturedQuadFilterAlpha(EFilterType type, hsh::texture2d tex);
  explicit CTexturedQuadFilterAlpha(EFilterType type, hsh::render_texture2d tex);
};

} // namespace metaforce
