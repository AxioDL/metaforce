#include "Specter/Table.hpp"
#include "Specter/ViewResources.hpp"
#include "Specter/RootView.hpp"

namespace Specter
{
#define ROW_HEIGHT 18
#define CELL_MARGIN 1

Table::Table(ViewResources& res, View& parentView, ITableDataBinding* data, ITableStateBinding* state, size_t maxColumns)
: View(res, parentView), m_data(data), m_state(state), m_maxColumns(maxColumns), m_rowsView(*this, res)
{
    commitResources(res);

    m_scroll.m_view.reset(new ScrollView(res, *this, ScrollView::Style::ThinIndicator));
    m_scroll.m_view->setContentView(&m_rowsView);
    updateData();
}

Table::RowsView::RowsView(Table& t, ViewResources& res)
: View(res, t), m_t(t), m_verts(new SolidShaderVert[SPECTER_TABLE_MAX_ROWS * t.m_maxColumns * 6])
{
    m_vertsBuf = res.m_factory->newDynamicBuffer(boo::BufferUse::Vertex, sizeof(SolidShaderVert),
                                                 SPECTER_TABLE_MAX_ROWS * t.m_maxColumns * 6);

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
}

Table::CellView::CellView(Table& t, ViewResources& res)
: View(res, t), m_t(t), m_text(new TextView(res, *this, res.m_mainFont))  {}

void Table::RowsView::_setRowVerts(const boo::SWindowRect& sub, const boo::SWindowRect& scissor)
{
    SolidShaderVert* v = m_verts.get();
    const ThemeData& theme = rootView().themeData();

    if (m_t.m_cellViews.empty())
        return;

    float pf = rootView().viewRes().pixelFactor();
    int div = sub.size[0] / m_t.m_cellViews.size();
    int spacing = (ROW_HEIGHT + CELL_MARGIN * 2) * pf;
    int margin = CELL_MARGIN * pf;
    int rowHeight = ROW_HEIGHT * pf;
    int yOff = 0;
    int idx = 0;
    while (sub.location[1] + yOff < scissor.location[1] + scissor.size[1] || (idx & 1) != 0)
    {
        yOff += spacing;
        ++idx;
    }
    int startIdx = std::max(0, int(m_t.m_rows) - idx);

    size_t r, c;
    for (r=0 ; r<SPECTER_TABLE_MAX_ROWS && (sub.location[1] + yOff + spacing) >= scissor.location[1] ; ++r)
    {
        const Zeus::CColor& color = (r&1) ? theme.tableCellBg1() : theme.tableCellBg2();
        int xOff = 0;
        for (c=0 ; c<std::min(m_t.m_maxColumns, m_t.m_columns) ; ++c)
        {
            v[0].m_pos.assign(xOff + margin, yOff - margin, 0);
            v[0].m_color = color;
            v[1] = v[0];
            v[2].m_pos.assign(xOff + margin, yOff - margin - rowHeight, 0);
            v[2].m_color = color;
            v[3].m_pos.assign(xOff + div - margin, yOff - margin, 0);
            v[3].m_color = color;
            v[4].m_pos.assign(xOff + div - margin, yOff - margin - rowHeight, 0);
            v[4].m_color = color;
            v[5] = v[4];
            v += 6;
            xOff += div;
        }
        yOff -= spacing;
    }
    m_visibleStart = startIdx;
    m_visibleRows = r;
    m_vertsBuf->load(m_verts.get(), sizeof(SolidShaderVert) * 6 * r * c);
}

void Table::setMultiplyColor(const Zeus::CColor& color)
{
    View::setMultiplyColor(color);
    if (m_scroll.m_view)
        m_scroll.m_view->setMultiplyColor(color);
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

void Table::think()
{
    if (m_scroll.m_view)
        m_scroll.m_view->think();
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
}

int Table::RowsView::nominalHeight() const
{
    float pf = rootView().viewRes().pixelFactor();
    int rows = m_t.m_rows;
    if (m_t.m_header)
        rows += 1;
    return rows * (ROW_HEIGHT + CELL_MARGIN * 2) * pf;
}

void Table::RowsView::resized(const boo::SWindowRect& root, const boo::SWindowRect& sub,
                              const boo::SWindowRect& scissor)
{
    View::resized(root, sub);
    _setRowVerts(sub, scissor);

    if (m_t.m_cellViews.empty())
        return;

    float pf = rootView().viewRes().pixelFactor();
    int div = sub.size[0] / m_t.m_cellViews.size();
    boo::SWindowRect cell = sub;
    cell.size[1] = ROW_HEIGHT * pf;
    cell.location[1] += sub.size[1] - cell.size[1];
    int spacing = (ROW_HEIGHT + CELL_MARGIN * 2) * pf;
    int hStart = cell.location[1];
    for (auto& col : m_t.m_cellViews)
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

    m_scissorRect = scissor;
    m_scissorRect.size[1] -= spacing;
}

void Table::CellView::resized(const boo::SWindowRect& root, const boo::SWindowRect& sub)
{
    View::resized(root, sub);
    boo::SWindowRect textRect = sub;
    float pf = rootView().viewRes().pixelFactor();
    textRect.location[0] += 5 * pf;
    textRect.location[1] += 5 * pf;
    m_text->resized(root, textRect);
}

void Table::draw(boo::IGraphicsCommandQueue* gfxQ)
{
    if (m_scroll.m_view)
        m_scroll.m_view->draw(gfxQ);
}

void Table::RowsView::draw(boo::IGraphicsCommandQueue* gfxQ)
{
    gfxQ->setShaderDataBinding(m_shaderBinding);
    gfxQ->setDrawPrimitive(boo::Primitive::TriStrips);

    gfxQ->setScissor(m_scissorRect);
    gfxQ->draw(1, m_visibleRows * m_t.m_columns * 6 - 2);
    for (auto& col : m_t.m_cellViews)
    {
        size_t idx = 0;
        for (std::unique_ptr<CellView>& cv : col)
        {
            if (cv && idx >= m_visibleStart && idx < m_visibleStart + m_visibleRows)
                cv->draw(gfxQ);
            ++idx;
        }
    }
    gfxQ->setScissor(rootView().subRect());

    if (m_t.m_header)
    {
        for (std::unique_ptr<CellView>& hv : m_t.m_headerViews)
            if (hv)
                hv->draw(gfxQ);
    }
}

void Table::CellView::draw(boo::IGraphicsCommandQueue* gfxQ)
{
    m_text->draw(gfxQ);
}

}
