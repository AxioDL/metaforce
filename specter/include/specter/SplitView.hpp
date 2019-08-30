#pragma once

#include "specter/View.hpp"

#include <boo/BooObject.hpp>
#include <hecl/UniformBufferPool.hpp>

namespace specter {
struct ISplitSpaceController;

class SplitView : public View {
  friend class RootView;
  friend class Space;

public:
  class Resources {
    friend class SplitView;
    friend class ViewResources;
    boo::ObjToken<boo::ITextureS> m_shadingTex;

    void init(boo::IGraphicsDataFactory::Context& ctx, const IThemeData& theme);
    void destroy() { m_shadingTex.reset(); }
  };

  enum class ArrowDir { Up, Down, Left, Right };

  enum class Axis { Horizontal, Vertical };

private:
  ISplitSpaceController* m_controller;
  Axis m_axis;
  float m_slide = 0.5;
  void _setSplit(float slide);
  bool m_dragging = false;

  ViewChild<View*> m_views[2];
  ViewBlock m_splitBlock;
  hecl::UniformBufferPool<ViewBlock>::Token m_splitBlockBuf;
  TexShaderVert m_splitVerts[4];

  int m_clearanceA, m_clearanceB;

  void setHorizontalVerts(int width) {
    m_splitVerts[0].m_pos.assign(0, 2, 0);
    m_splitVerts[0].m_uv.assign(0, 0);
    m_splitVerts[1].m_pos.assign(0, -1, 0);
    m_splitVerts[1].m_uv.assign(1, 0);
    m_splitVerts[2].m_pos.assign(width, 2, 0);
    m_splitVerts[2].m_uv.assign(0, 0);
    m_splitVerts[3].m_pos.assign(width, -1, 0);
    m_splitVerts[3].m_uv.assign(1, 0);
  }

  void setVerticalVerts(int height) {
    m_splitVerts[0].m_pos.assign(-1, height, 0);
    m_splitVerts[0].m_uv.assign(0, 0);
    m_splitVerts[1].m_pos.assign(-1, 0, 0);
    m_splitVerts[1].m_uv.assign(0, 0);
    m_splitVerts[2].m_pos.assign(2, height, 0);
    m_splitVerts[2].m_uv.assign(1, 0);
    m_splitVerts[3].m_pos.assign(2, 0, 0);
    m_splitVerts[3].m_uv.assign(1, 0);
  }

  VertexBufferBindingTex m_splitVertsBinding;

public:
  SplitView(ViewResources& res, View& parentView, ISplitSpaceController* controller, Axis axis, float split,
            int clearanceA = -1, int clearanceB = -1);
  View* setContentView(int slot, View* view);
  void setSplit(float split);
  void setAxis(Axis axis);
  Axis axis() const { return m_axis; }
  float split() const { return m_slide; }
  bool testSplitHover(const boo::SWindowCoord& coord);
  bool testJoinArrowHover(const boo::SWindowCoord& coord, int& origSlotOut, SplitView*& splitOut, int& slotOut,
                          boo::SWindowRect& rectOut, ArrowDir& dirOut, int forceSlot = -1);
  void getJoinArrowHover(int slot, boo::SWindowRect& rectOut, ArrowDir& dirOut);
  bool testSplitLineHover(const boo::SWindowCoord& coord, int& slotOut, boo::SWindowRect& rectOut, float& splitOut,
                          Axis& axisOut);
  void getSplitLineHover(int slot, boo::SWindowRect& rectOut, Axis& axisOut);
  void startDragSplit(const boo::SWindowCoord& coord);
  void endDragSplit();
  void moveDragSplit(const boo::SWindowCoord& coord);
  void mouseDown(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey) override;
  void mouseUp(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey) override;
  void mouseMove(const boo::SWindowCoord&) override;
  void mouseEnter(const boo::SWindowCoord&) override;
  void mouseLeave(const boo::SWindowCoord&) override;
  void resized(const boo::SWindowRect& rootView, const boo::SWindowRect& sub) override;
  void draw(boo::IGraphicsCommandQueue* gfxQ) override;

  void setMultiplyColor(const zeus::CColor& color) override {
    View::setMultiplyColor(color);
    m_splitBlock.m_color = color;
    m_splitBlockBuf.access().finalAssign(m_splitBlock);

    if (m_views[0].m_view)
      m_views[0].m_view->setMultiplyColor(color);
    if (m_views[1].m_view)
      m_views[1].m_view->setMultiplyColor(color);
  }

  bool isSplitView() const override { return true; }
};
inline SplitView* View::castToSplitView() { return isSplitView() ? static_cast<SplitView*>(this) : nullptr; }

} // namespace specter
