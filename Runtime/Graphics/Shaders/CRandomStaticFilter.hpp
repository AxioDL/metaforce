#pragma once

#include "hsh/hsh.h"

#include "Runtime/CToken.hpp"

namespace metaforce {
class CTexture;

enum class EFilterShape;
enum class EFilterType;

class CRandomStaticFilter {
public:
  struct Uniform {
    hsh::float4 color;
    float randOff;
    float discardThres;
  };
  struct Vert {
    hsh::float2 m_pos;
    hsh::float2 m_uv;
  };

private:
  hsh::owner<hsh::vertex_buffer<Vert>> m_vbo;
  hsh::dynamic_owner<hsh::uniform_buffer<Uniform>> m_uniBuf;
  hsh::binding m_dataBind;
  Uniform m_uniform;
  bool m_cookieCutter;

public:
  explicit CRandomStaticFilter(EFilterType type, bool cookieCutter = false);
  explicit CRandomStaticFilter(EFilterType type, const TLockedToken<CTexture>&) : CRandomStaticFilter(type) {}
  void draw(const zeus::CColor& color, float t);
  void DrawFilter(EFilterShape, const zeus::CColor& color, float t) { draw(color, t); }
};

class CCookieCutterDepthRandomStaticFilter : public CRandomStaticFilter {
public:
  explicit CCookieCutterDepthRandomStaticFilter(EFilterType type) : CRandomStaticFilter(type, true) {}
  explicit CCookieCutterDepthRandomStaticFilter(EFilterType type, const TLockedToken<CTexture>&)
  : CCookieCutterDepthRandomStaticFilter(type) {}
};

} // namespace metaforce
