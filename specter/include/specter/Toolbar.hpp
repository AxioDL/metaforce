#pragma once

#include "specter/View.hpp"

namespace specter {
#define SPECTER_TOOLBAR_GAUGE 28

class Toolbar : public View {
public:
  class Resources {
    friend class ViewResources;
    friend class Toolbar;
    boo::ObjToken<boo::ITextureS> m_shadingTex;

    void init(boo::IGraphicsDataFactory::Context& ctx, const IThemeData& theme);
    void destroy() { m_shadingTex.reset(); }
  };

  enum class Position { None, Bottom, Top };

private:
  unsigned m_units;
  std::vector<std::vector<ViewChild<View*>>> m_children;

  ViewBlock m_tbBlock;
  hecl::UniformBufferPool<ViewBlock>::Token m_tbBlockBuf;
  TexShaderVert m_tbVerts[10];
  int m_nomGauge = 25;
  int m_padding = 10;

  void setHorizontalVerts(int width);
  void setVerticalVerts(int height);

  VertexBufferBindingTex m_vertsBinding;

public:
  Toolbar(ViewResources& res, View& parentView, Position toolbarPos, unsigned units);
  void mouseDown(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey) override;
  void mouseUp(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey) override;
  void mouseMove(const boo::SWindowCoord&) override;
  void mouseEnter(const boo::SWindowCoord&) override;
  void mouseLeave(const boo::SWindowCoord& coord) override;
  void resized(const boo::SWindowRect& rootView, const boo::SWindowRect& sub) override;
  void draw(boo::IGraphicsCommandQueue* gfxQ) override;

  int nominalHeight() const override { return m_nomGauge; }

  void clear() {
    for (std::vector<ViewChild<View*>>& u : m_children)
      u.clear();
  }
  void push_back(View* v, unsigned unit);

  void setMultiplyColor(const zeus::CColor& color) override {
    View::setMultiplyColor(color);
    for (std::vector<ViewChild<View*>>& u : m_children)
      for (ViewChild<View*>& c : u)
        if (c.m_view)
          c.m_view->setMultiplyColor(color);
  }
};

} // namespace specter
