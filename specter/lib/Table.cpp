#include "Specter/Table.hpp"
#include "Specter/ViewResources.hpp"
#include "Specter/RootView.hpp"

namespace Specter
{
#define ROW_HEIGHT 18
#define ROW_SPACING 2

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
    updateData();
}

Table::CellView::CellView(Table& t, ViewResources& res)
: View(res, t), m_t(t), m_text(new TextView(res, *this, res.m_mainFont))  {}

void Table::_setRowVerts(const boo::SWindowRect& rowsRect)
{
    SolidShaderVert* v = m_verts;
    const ThemeData& theme = rootView().themeData();
    float pf = rootView().viewRes().pixelFactor();
    int rowHeight = ROW_HEIGHT * pf;
    int rowSpace = ROW_SPACING * pf;
    int hAdv = rowHeight + rowSpace;
    int yOff = rowsRect.size[1] - hAdv;
    size_t i;
    for (i=0 ; i<SPECTER_TABLE_MAX_ROWS && (yOff + rowHeight + rowSpace) >= 0 ; ++i)
    {
        v[0].m_pos.assign(0, yOff, 0);
        v[0].m_color = (i&1) ? theme.tableCellBg1() : theme.tableCellBg2();
        v[1] = v[0];
        v[2].m_pos.assign(0, yOff - rowHeight, 0);
        v[2].m_color = v[0].m_color;
        v[3].m_pos.assign(rowsRect.size[0], yOff, 0);
        v[3].m_color = v[0].m_color;
        v[4].m_pos.assign(rowsRect.size[0], yOff - rowHeight, 0);
        v[4].m_color = v[0].m_color;
        v[5] = v[4];
        yOff -= hAdv;
        v += 6;
    }
    m_visibleRows = i;
    m_vertsBuf->load(m_verts, sizeof(SolidShaderVert) * 6 * i);
}

void Table::setMultiplyColor(const Zeus::CColor& color)
{
    View::setMultiplyColor(color);
    for (std::unique_ptr<CellView>& hv : m_headerViews)
        if (hv)
            hv->m_text->setMultiplyColor(color);
    for (auto& col : m_cellViews)
    {
        for (std::unique_ptr<CellView>& cv : col)
            if (cv)
                cv->m_text->setMultiplyColor(color);
    }
}

void Table::mouseDown(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod)
{
    m_scroll.mouseDown(coord, button, mod);
}

void Table::mouseUp(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod)
{
    m_scroll.mouseUp(coord, button, mod);
}

void Table::mouseMove(const boo::SWindowCoord& coord)
{
    m_scroll.mouseMove(coord);
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

void Table::updateData()
{
    m_header = false;
    m_headerViews.clear();
    m_cellViews.clear();
    m_rows = m_data->rowCount();
    m_columns = m_data->columnCount();
    if (!m_columns)
        return;

    m_headerViews.reserve(m_columns);
    m_cellViews.reserve(m_columns);

    ViewResources& res = rootView().viewRes();
    const Zeus::CColor& textColor = rootView().themeData().uiText();
    for (size_t c=0 ; c<m_columns ; ++c)
    {
        const std::string* headerText = m_data->header(c);
        if (headerText)
        {
            m_header = true;
            CellView* cv = new CellView(*this, res);
            m_headerViews.emplace_back(cv);
            cv->m_text->typesetGlyphs(*headerText, textColor);
        }
        else
            m_headerViews.emplace_back();

        m_cellViews.emplace_back();
        std::vector<std::unique_ptr<CellView>>& col = m_cellViews.back();
        col.reserve(m_rows);
        for (size_t r=0 ; r<m_rows ; ++r)
        {
            const std::string* cellText = m_data->cell(c, r);
            if (cellText)
            {
                CellView* cv = new CellView(*this, res);
                col.emplace_back(cv);
                cv->m_text->typesetGlyphs(*cellText, textColor);
            }
            else
                col.emplace_back();
        }
    }

    updateSize();
}

void Table::resized(const boo::SWindowRect& root, const boo::SWindowRect& sub)
{
    View::resized(root, sub);
    if (m_scroll.m_view)
        m_scroll.m_view->resized(root, sub);

    float pf = rootView().viewRes().pixelFactor();
    boo::SWindowRect cell = sub;
    cell.size[1] = ROW_HEIGHT * pf;
    cell.location[1] += sub.size[1] - cell.size[1];
    int div = sub.size[0] / m_cellViews.size();

    for (std::unique_ptr<CellView>& hv : m_headerViews)
    {
        if (hv)
            hv->resized(root, cell);
        cell.location[0] += div;
    }

    int spacing = ROW_HEIGHT + ROW_SPACING * pf;
    cell.location[0] = sub.location[0];
    int hStart = cell.location[1];
    for (auto& col : m_cellViews)
    {
        cell.location[1] = hStart;
        for (std::unique_ptr<CellView>& cv : col)
        {
            cell.location[1] -= spacing;
            if (cv)
                cv->resized(root, cell);
        }
        cell.location[0] += div;
    }
}

int Table::RowsView::nominalHeight() const
{
    float pf = rootView().viewRes().pixelFactor();
    return m_t.m_rows * (ROW_HEIGHT + ROW_SPACING) * pf;
}

void Table::RowsView::resized(const boo::SWindowRect& root, const boo::SWindowRect& sub,
                              const boo::SWindowRect& scissor)
{
    m_t._setRowVerts(sub);
}

void Table::CellView::resized(const boo::SWindowRect& root, const boo::SWindowRect& sub)
{
    View::resized(root, sub);
    boo::SWindowRect textRect = sub;
    float pf = rootView().viewRes().pixelFactor();
    textRect.location[0] += 5 * pf;
    textRect.location[1] += 6 * pf;
    m_text->resized(root, textRect);
}

void Table::draw(boo::IGraphicsCommandQueue* gfxQ)
{
    if (m_scroll.m_view)
        m_scroll.m_view->draw(gfxQ);
}

void Table::RowsView::draw(boo::IGraphicsCommandQueue* gfxQ)
{
    gfxQ->setShaderDataBinding(m_t.m_shaderBinding);
    gfxQ->setDrawPrimitive(boo::Primitive::TriStrips);
    gfxQ->setScissor(subRect());
    size_t rows = std::min(m_t.m_visibleRows, m_t.m_rows);
    gfxQ->draw(1, rows * 6 - 2);
    for (std::unique_ptr<CellView>& hv : m_t.m_headerViews)
        if (hv)
            hv->draw(gfxQ);
    for (auto& col : m_t.m_cellViews)
    {
        for (std::unique_ptr<CellView>& cv : col)
            if (cv)
                cv->draw(gfxQ);
    }
    gfxQ->setScissor(rootView().subRect());
}

void Table::CellView::draw(boo::IGraphicsCommandQueue* gfxQ)
{
    m_text->draw(gfxQ);
}

}
