#include "specter/Icon.hpp"
#include "specter/RootView.hpp"

namespace specter {

IconView::IconView(ViewResources& res, View& parentView, Icon& icon) : View(res, parentView) {
  commitResources(res, [&](boo::IGraphicsDataFactory::Context& ctx) -> bool {
    buildResources(ctx, res);
    m_vertexBinding.init(ctx, res, 4, m_viewVertBlockBuf, icon.m_tex);
    return true;
  });
  TexShaderVert verts[] = {
      {{0, 1, 0}, icon.m_uvCoords[0]},
      {{0, 0, 0}, icon.m_uvCoords[1]},
      {{1, 1, 0}, icon.m_uvCoords[2]},
      {{1, 0, 0}, icon.m_uvCoords[3]},
  };
  m_vertexBinding.load<decltype(verts)>(verts);
  setBackground(zeus::skBlue);
}

void IconView::resized(const boo::SWindowRect& root, const boo::SWindowRect& sub) {
  m_viewVertBlock.setViewRect(root, sub);
  m_viewVertBlock.m_mv[0][0] *= sub.size[0];
  m_viewVertBlock.m_mv[1][1] *= sub.size[1];
  View::resized(m_viewVertBlock, sub);
}

void IconView::draw(boo::IGraphicsCommandQueue* gfxQ) {
  gfxQ->setShaderDataBinding(m_vertexBinding);
  gfxQ->draw(0, 4);
}

} // namespace specter
