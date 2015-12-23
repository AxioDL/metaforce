#include "Specter/Table.hpp"
#include "Specter/ViewResources.hpp"

namespace Specter
{

Table::Table(ViewResources& res, View& parentView, ITableDataBinding* data, ITableStateBinding* state)
: View(res, parentView), m_data(data), m_state(state), m_rowsView(*this, res)
{
    m_vertsBuf = res.m_factory->newDynamicBuffer(boo::BufferUse::Vertex, sizeof(SolidShaderVert),
                                                 SPECTER_TABLE_MAX_ROWS * 6);

    if (!res.m_viewRes.m_texVtxFmt)
    {
        boo::VertexElementDescriptor vdescs[] =
        {
            {m_vertsBuf, nullptr, boo::VertexSemantic::Position4},
            {m_vertsBuf, nullptr, boo::VertexSemantic::Color}
        };
        m_vtxFmt = res.m_factory->newVertexFormat(2, vdescs);
        boo::IGraphicsBuffer* bufs[] = {m_viewVertBlockBuf};
        m_shaderBinding = res.m_factory->newShaderDataBinding(res.m_viewRes.m_solidShader,
                                                              m_vtxFmt, m_vertsBuf, nullptr,
                                                              nullptr, 1, bufs, 0, nullptr);
    }
    else
    {
        boo::IGraphicsBuffer* bufs[] = {m_viewVertBlockBuf};
        m_shaderBinding = res.m_factory->newShaderDataBinding(res.m_viewRes.m_solidShader,
                                                              res.m_viewRes.m_texVtxFmt,
                                                              m_vertsBuf, nullptr,
                                                              nullptr, 1, bufs, 0, nullptr);
    }
    commitResources(res);

    m_scroll.m_view.reset(new ScrollView(res, *this));
    m_scroll.m_view->setContentView(&m_rowsView);
}

void Table::mouseDown(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod)
{
    m_scroll.mouseDown(coord, button, mod);
}

void Table::mouseUp(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod)
{
    m_scroll.mouseUp(coord, button, mod);
}

void Table::mouseEnter(const boo::SWindowCoord& coord)
{
    m_scroll.mouseEnter(coord);
}

void Table::mouseLeave(const boo::SWindowCoord& coord)
{
    m_scroll.mouseLeave(coord);
}

void Table::scroll(const boo::SWindowCoord& coord, const boo::SScrollDelta& scroll)
{
    m_scroll.scroll(coord, scroll);
}

void Table::resized(const boo::SWindowRect& root, const boo::SWindowRect& sub)
{
    boo::SWindowRect rowsRect = sub;
    m_scroll.m_view->resized(root, rowsRect);
}

void Table::RowsView::resized(const boo::SWindowRect& root, const boo::SWindowRect& sub)
{
}

void Table::draw(boo::IGraphicsCommandQueue* gfxQ)
{
    m_scroll.m_view->draw(gfxQ);
}

void Table::RowsView::draw(boo::IGraphicsCommandQueue* gfxQ)
{
}

}
