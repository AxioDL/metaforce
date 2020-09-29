#pragma once

#include "Runtime/CToken.hpp"

#include "zeus/CColor.hpp"
#include "zeus/CMatrix4f.hpp"
#include "zeus/CRectangle.hpp"

namespace urde {
class CTexture;

enum class EFilterShape;
enum class EFilterType;

class CColoredQuadFilter {
  template <EFilterType Type>
  friend struct CColoredQuadFilterPipeline;
  struct Uniform {
    hsh::float4x4 m_matrix;
    hsh::float4 m_color;
  };
  struct Vert {
    hsh::float3 m_pos;
  };
  hsh::owner<hsh::vertex_buffer<Vert>> m_vbo;
  hsh::dynamic_owner<hsh::uniform_buffer<Uniform>> m_uniBuf;
  hsh::binding m_dataBind;
  Uniform m_uniform;

public:
  static constexpr zeus::CRectangle DefaultRect{0.f, 0.f, 1.f, 1.f};
  explicit CColoredQuadFilter(EFilterType type);
  explicit CColoredQuadFilter(EFilterType type, const TLockedToken<CTexture>&) : CColoredQuadFilter(type) {}
  void draw(const zeus::CColor& color, const zeus::CRectangle& rect = DefaultRect);
  void DrawFilter(EFilterShape shape, const zeus::CColor& color, float t) { draw(color); }
};

class CWideScreenFilter {
  CColoredQuadFilter m_top;
  CColoredQuadFilter m_bottom;

public:
  explicit CWideScreenFilter(EFilterType type) : m_top(type), m_bottom(type) {}
  explicit CWideScreenFilter(EFilterType type, const TLockedToken<CTexture>&) : CWideScreenFilter(type) {}
  void draw(const zeus::CColor& color, float t);
  void DrawFilter(EFilterShape shape, const zeus::CColor& color, float t) { draw(color, t); }

  static float SetViewportToMatch(float t);
  static void SetViewportToFull();
};

} // namespace urde
