#pragma once

#include <array>
#include <vector>

#include <boo/graphicsdev/IGraphicsDataFactory.hpp>

#include <zeus/CColor.hpp>
#include <zeus/CVector2f.hpp>
#include <zeus/CVector3f.hpp>

namespace urde {
class CTexture;

class CRadarPaintShader {
public:
  struct Instance {
    std::array<zeus::CVector3f, 4> pos;
    std::array<zeus::CVector2f, 4> uv;
    zeus::CColor color;
  };

private:
  boo::ObjToken<boo::IGraphicsBufferD> m_vbo;
  boo::ObjToken<boo::IGraphicsBufferD> m_uniBuf;
  boo::ObjToken<boo::IShaderDataBinding> m_dataBind;
  const CTexture* m_tex = nullptr;
  size_t m_maxInsts = 0;

public:
  static void Initialize();
  static void Shutdown();
  void draw(const std::vector<Instance>& instances, const CTexture* tex);
};

} // namespace urde
