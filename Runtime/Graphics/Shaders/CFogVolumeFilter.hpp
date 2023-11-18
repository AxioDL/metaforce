#pragma once

namespace zeus {
class CColor;
} // namespace zeus

namespace metaforce {

class CFogVolumeFilter {
//  boo::ObjToken<boo::IGraphicsBufferS> m_vbo;
//  boo::ObjToken<boo::IGraphicsBufferD> m_uniBuf;
//  boo::ObjToken<boo::IShaderDataBinding> m_dataBind1Way;
//  boo::ObjToken<boo::IShaderDataBinding> m_dataBind2Way;

public:
  static void Initialize();
  static void Shutdown();
  CFogVolumeFilter();
  void draw2WayPass(const zeus::CColor& color);
  void draw1WayPass(const zeus::CColor& color);
};

} // namespace metaforce
