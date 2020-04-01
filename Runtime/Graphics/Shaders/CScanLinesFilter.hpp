#pragma once

#include "Runtime/CToken.hpp"

#include <boo/graphicsdev/IGraphicsDataFactory.hpp>
#include <zeus/CColor.hpp>

namespace urde {
class CTexture;

enum class EFilterShape;
enum class EFilterType;

class CScanLinesFilter {
  struct Uniform {
    zeus::CColor color;
  };
  boo::ObjToken<boo::IGraphicsBufferD> m_uniBuf;
  boo::ObjToken<boo::IShaderDataBinding> m_dataBind;
  Uniform m_uniform;
  bool m_even;

public:
  static void Initialize();
  static void Shutdown();
  explicit CScanLinesFilter(EFilterType type, bool even);
  void draw(const zeus::CColor& color);
  void DrawFilter(EFilterShape, const zeus::CColor& color, float) { draw(color); }
};

class CScanLinesFilterEven : public CScanLinesFilter {
public:
  explicit CScanLinesFilterEven(EFilterType type) : CScanLinesFilter(type, true) {}
  explicit CScanLinesFilterEven(EFilterType type, const TLockedToken<CTexture>&) : CScanLinesFilterEven(type) {}
};

class CScanLinesFilterOdd : public CScanLinesFilter {
public:
  explicit CScanLinesFilterOdd(EFilterType type) : CScanLinesFilter(type, false) {}
  explicit CScanLinesFilterOdd(EFilterType type, const TLockedToken<CTexture>&) : CScanLinesFilterOdd(type) {}
};

} // namespace urde
