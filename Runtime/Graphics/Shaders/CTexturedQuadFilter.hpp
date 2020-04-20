#pragma once

#include "Runtime/CToken.hpp"

#include <boo/graphicsdev/IGraphicsDataFactory.hpp>

#include <zeus/CColor.hpp>
#include <zeus/CMatrix4f.hpp>
#include <zeus/CRectangle.hpp>
#include <zeus/CVector2f.hpp>
#include <zeus/CVector3f.hpp>

namespace urde {
class CTexture;

enum class EFilterShape;
enum class EFilterType;

class CTexturedQuadFilter {
public:
  enum class ZTest { None, LEqual, GEqual, GEqualZWrite };

protected:
  struct Uniform {
    zeus::CMatrix4f m_matrix;
    zeus::CColor m_color;
    float m_lod = 0.f;
  };
  TLockedToken<CTexture> m_tex;
  boo::ObjToken<boo::ITexture> m_booTex;
  boo::ObjToken<boo::IGraphicsBufferD> m_vbo;
  boo::ObjToken<boo::IGraphicsBufferD> m_uniBuf;
  boo::ObjToken<boo::IShaderDataBinding> m_dataBind;
  Uniform m_uniform;
  ZTest m_zTest;
  bool m_flipRect = false;

  explicit CTexturedQuadFilter(const boo::ObjToken<boo::ITexture>& tex);

public:
  struct Vert {
    zeus::CVector3f m_pos;
    zeus::CVector2f m_uv;
  };
  static void Initialize();
  static void Shutdown();
  static const zeus::CRectangle DefaultRect;
  explicit CTexturedQuadFilter(EFilterType type, TLockedToken<CTexture> tex, ZTest zTest = ZTest::None);
  explicit CTexturedQuadFilter(EFilterType type, const boo::ObjToken<boo::ITexture>& tex, ZTest zTest = ZTest::None);
  CTexturedQuadFilter(const CTexturedQuadFilter&) = delete;
  CTexturedQuadFilter& operator=(const CTexturedQuadFilter&) = delete;
  CTexturedQuadFilter(CTexturedQuadFilter&&) = default;
  CTexturedQuadFilter& operator=(CTexturedQuadFilter&&) = default;
  void draw(const zeus::CColor& color, float uvScale, const zeus::CRectangle& rect = DefaultRect, float z = 0.f);
  void drawCropped(const zeus::CColor& color, float uvScale);
  void drawVerts(const zeus::CColor& color, std::array<Vert, 4> verts, float lod = 0.f);
  void DrawFilter(EFilterShape shape, const zeus::CColor& color, float t);
  const TLockedToken<CTexture>& GetTex() const { return m_tex; }
  const boo::ObjToken<boo::ITexture>& GetBooTex() const { return m_booTex; }
};

class CTexturedQuadFilterAlpha : public CTexturedQuadFilter {
public:
  static void Initialize();
  static void Shutdown();
  explicit CTexturedQuadFilterAlpha(EFilterType type, TLockedToken<CTexture> tex);
  explicit CTexturedQuadFilterAlpha(EFilterType type, const boo::ObjToken<boo::ITexture>& tex);
};

} // namespace urde
