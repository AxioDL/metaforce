#pragma once

#include "Runtime/CToken.hpp"

//#include <boo/graphicsdev/IGraphicsDataFactory.hpp>
#include <zeus/CColor.hpp>

namespace metaforce {
class CTexture;

enum class EFilterShape;
enum class EFilterType;

class CRandomStaticFilter {
  struct Uniform {
    zeus::CColor color;
    float randOff;
    float discardThres;
  };
//  boo::ObjToken<boo::IGraphicsBufferS> m_vbo;
//  boo::ObjToken<boo::IGraphicsBufferD> m_uniBuf;
//  boo::ObjToken<boo::IShaderDataBinding> m_dataBind;
  Uniform m_uniform;
  bool m_cookieCutter;

public:
  static void Initialize();
  static void Shutdown();
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
