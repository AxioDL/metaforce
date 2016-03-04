#include "specter/Icon.hpp"
#include "specter/RootView.hpp"

namespace specter
{

IconView::IconView(ViewResources& res, View& parentView, Icon& icon)
: View(res, parentView)
{
    m_vertexBinding.initTex(res, 4, m_viewVertBlockBuf, icon.m_tex);
    commitResources(res);
    TexShaderVert verts[] =
    {
        {{0, 1, 0}, icon.m_uvCoords[0]},
        {{0, 0, 0}, icon.m_uvCoords[1]},
        {{1, 1, 0}, icon.m_uvCoords[2]},
        {{1, 0, 0}, icon.m_uvCoords[3]},
    };
    m_vertexBinding.load(verts, sizeof(verts));
    setBackground(zeus::CColor::skBlue);
}

void IconView::resized(const boo::SWindowRect& root, const boo::SWindowRect& sub)
{
    m_viewVertBlock.setViewRect(root, sub);
    float pf = rootView().viewRes().pixelFactor();
    m_viewVertBlock.m_mv[0][0] *= sub.size[0];
    m_viewVertBlock.m_mv[1][1] *= sub.size[1];
    View::resized(m_viewVertBlock, sub);
}

void IconView::draw(boo::IGraphicsCommandQueue* gfxQ)
{
    gfxQ->setShaderDataBinding(m_vertexBinding);
    gfxQ->draw(0, 4);
}

}
