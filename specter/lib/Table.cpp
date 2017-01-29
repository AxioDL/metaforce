#include "specter/Table.hpp"
#include "specter/ViewResources.hpp"
#include "specter/RootView.hpp"
#include "specter/ScrollView.hpp"

namespace specter
{
static logvisor::Module Log("specter::Table");
#define ROW_HEIGHT 18
#define CELL_MARGIN 1

Table::Table(ViewResources& res, View& parentView, ITableDataBinding* data,
             ITableStateBinding* state, size_t maxColumns)
: View(res, parentView), m_data(data), m_state(state), m_maxColumns(maxColumns),
  m_hVerts(new SolidShaderVert[maxColumns * 6]), m_rowsView(*this, res)
{
    if (!maxColumns)
        Log.report(logvisor::Fatal, "0-column tables not supported");

    m_scroll.m_view.reset(new ScrollView(res, *this, ScrollView::Style::ThinIndicator));

    commitResources(res, [&](boo::IGraphicsDataFactory::Context& ctx) -> bool
    {
        buildResources(ctx, res);
        m_vertsBinding.init(ctx, res, maxColumns * 6, m_viewVertBlockBuf);
        return true;
    });
    m_scroll.m_view->setContentView(&m_rowsView);

    updateData();
}

Table::RowsView::RowsView(Table& t, ViewResources& res)
: View(res, t), m_t(t), m_verts(new SolidShaderVert[SPECTER_TABLE_MAX_ROWS * t.m_maxColumns * 6])
{
    commitResources(res, [&](boo::IGraphicsDataFactory::Context& ctx) -> bool
    {
        buildResources(ctx, res);
        m_vertsBinding.init(ctx, res, SPECTER_TABLE_MAX_ROWS * t.m_maxColumns * 6, m_viewVertBlockBuf);
        return true;
    });
}

Table::CellView::CellView(Table& t, ViewResources& res)
: View(res, t), m_t(t), m_text(new TextView(res, *this, res.m_mainFont)), m_c(-1), m_r(-1)  {}

void Table::_setHeaderVerts(const boo::SWindowRect& sub)
{;
    if (m_headerViews.empty())
        return;
    SolidShaderVert* v = m_hVerts.get();
    const IThemeData& theme = rootView().themeData();

    float pf = rootView().viewRes().pixelFactor();
    int margin = CELL_MARGIN * pf;
    int rowHeight = ROW_HEIGHT * pf;
    int xOff = 0;
    int yOff = sub.size[1];
    size_t c;
    auto it = m_headerViews.cbegin();

    size_t sCol = -1;
    SortDirection sDir = SortDirection::None;
    if (m_state)
        sDir = m_state->getSort(sCol);

    boo::SWindowRect headRowRect = sub;
    headRowRect.size[1] = rowHeight;
    headRowRect.location[1] = sub.size[1] - rowHeight;
    m_hCellRects = getCellRects(headRowRect);
    auto cellRectsIt = m_hCellRects.begin();

    for (c=0 ; c<std::min(m_maxColumns, m_columns) ; ++c)
    {
        const ViewChild<std::unique_ptr<CellView>>& hv = *it;
        const zeus::CColor* c1 = &theme.button1Inactive();
        const zeus::CColor* c2 = &theme.button2Inactive();
        if (hv.m_mouseDown && hv.m_mouseIn)
        {
            c1 = &theme.button1Press();
            c2 = &theme.button2Press();
        }
        else if (hv.m_mouseIn)
        {
            c1 = &theme.button1Hover();
            c2 = &theme.button2Hover();
        }

        zeus::CColor cm1 = *c1;
        zeus::CColor cm2 = *c2;
        if (sCol == c)
        {
            if (sDir == SortDirection::Ascending)
            {
                cm1 *= zeus::CColor::skGreen;
                cm2 *= zeus::CColor::skGreen;
            }
            else if (sDir == SortDirection::Descending)
            {
                cm1 *= zeus::CColor::skRed;
                cm2 *= zeus::CColor::skRed;
            }
        }

        int div = cellRectsIt->size[0];
        v[0].m_pos.assign(xOff + margin, yOff - margin, 0);
        v[0].m_color = cm1;
        v[1] = v[0];
        v[2].m_pos.assign(xOff + margin, yOff - margin - rowHeight, 0);
        v[2].m_color = cm2;
        v[3].m_pos.assign(xOff + div - margin, yOff - margin, 0);
        v[3].m_color = cm1;
        v[4].m_pos.assign(xOff + div - margin, yOff - margin - rowHeight, 0);
        v[4].m_color = cm2;
        v[5] = v[4];
        v += 6;
        xOff += div;
        ++it;
        ++cellRectsIt;
    }

    if (c)
        m_vertsBinding.load(m_hVerts.get(), 6 * c);

    m_headerNeedsUpdate = false;
}

void Table::RowsView::_setRowVerts(const boo::SWindowRect& sub, const boo::SWindowRect& scissor)
{
    SolidShaderVert* v = m_verts.get();
    const IThemeData& theme = rootView().themeData();

    if (m_t.m_cellPools.empty())
        return;

    float pf = rootView().viewRes().pixelFactor();
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
    int startIdx = int(m_t.m_rows) - idx;

    std::vector<boo::SWindowRect> cellRects = m_t.getCellRects(sub);

    size_t r, c;
    for (r=0, c=0 ; r<SPECTER_TABLE_MAX_ROWS &&
         (sub.location[1] + yOff + spacing) >= scissor.location[1] ; ++r)
    {
        const zeus::CColor& color = (startIdx+r==m_t.m_selectedRow) ? theme.tableCellBgSelected() :
                                    ((r&1) ? theme.tableCellBg1() : theme.tableCellBg2());
        int xOff = 0;
        auto cellRectsIt = cellRects.begin();
        for (c=0 ; c<std::min(m_t.m_maxColumns, m_t.m_columns) ; ++c)
        {
            int div = cellRectsIt->size[0];
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
            ++cellRectsIt;
        }
        yOff -= spacing;
    }
    m_visibleStart = std::max(0, startIdx);
    m_visibleRows = r;
    if (r * c)
        m_vertsBinding.load(m_verts.get(), 6 * r * c);
}

void Table::cycleSortColumn(size_t c)
{
    if (c >= m_columns)
        Log.report(logvisor::Fatal, "cycleSortColumn out of bounds (%" PRISize ", %" PRISize ")",
                   c, m_columns);
    if (m_state)
    {
        size_t cIdx;
        SortDirection dir = m_state->getSort(cIdx);
        if (dir == SortDirection::None || cIdx != c)
            m_state->setSort(c, SortDirection::Ascending);
        else if (dir == SortDirection::Ascending)
            m_state->setSort(c, SortDirection::Descending);
        else if (dir == SortDirection::Descending)
            m_state->setSort(c, SortDirection::Ascending);
    }
}

void Table::selectRow(size_t r)
{
    if (m_inSelectRow)
        return;
    if (r >= m_rows && r != -1)
        Log.report(logvisor::Fatal, "selectRow out of bounds (%" PRISize ", %" PRISize ")",
                   r, m_rows);
    if (r == m_selectedRow)
    {
        if (m_state)
        {
            m_inSelectRow = true;
            m_state->setSelectedRow(r);
            m_inSelectRow = false;
        }
        return;
    }
    
    if (m_selectedRow != -1 && m_activePool != -1)
    {
        size_t poolIdx = m_selectedRow / SPECTER_TABLE_MAX_ROWS;
        int pool0 = (poolIdx & 1) != 0;
        int pool1 = (poolIdx & 1) == 0;
        if (m_activePool == poolIdx)
        {
            for (auto& col : m_cellPools)
            {
                ViewChild<std::unique_ptr<CellView>>& cv = col[pool0].at(m_selectedRow % SPECTER_TABLE_MAX_ROWS);
                if (cv.m_view)
                    cv.m_view->deselect();
            }
        }
        else if (m_activePool+1 == poolIdx)
        {
            for (auto& col : m_cellPools)
            {
                ViewChild<std::unique_ptr<CellView>>& cv = col[pool1].at(m_selectedRow % SPECTER_TABLE_MAX_ROWS);
                if (cv.m_view)
                    cv.m_view->deselect();
            }
        }
    }
    
    m_selectedRow = r;
    
    if (m_selectedRow != -1 && m_activePool != -1)
    {
        size_t poolIdx = m_selectedRow / SPECTER_TABLE_MAX_ROWS;
        int pool0 = (poolIdx & 1) != 0;
        int pool1 = (poolIdx & 1) == 0;
        if (m_activePool == poolIdx)
        {
            for (auto& col : m_cellPools)
            {
                ViewChild<std::unique_ptr<CellView>>& cv = col[pool0].at(m_selectedRow % SPECTER_TABLE_MAX_ROWS);
                if (cv.m_view)
                    cv.m_view->select();
            }
        }
        else if (m_activePool+1 == poolIdx)
        {
            for (auto& col : m_cellPools)
            {
                ViewChild<std::unique_ptr<CellView>>& cv = col[pool1].at(m_selectedRow % SPECTER_TABLE_MAX_ROWS);
                if (cv.m_view)
                    cv.m_view->select();
            }
        }
    }
    
    updateSize();
    
    if (m_state)
    {
        m_inSelectRow = true;
        m_state->setSelectedRow(r);
        m_inSelectRow = false;
    }
}

void Table::setMultiplyColor(const zeus::CColor& color)
{
    View::setMultiplyColor(color);
    if (m_scroll.m_view)
        m_scroll.m_view->setMultiplyColor(color);
    for (ViewChild<std::unique_ptr<CellView>>& hv : m_headerViews)
        if (hv.m_view)
            hv.m_view->m_text->setMultiplyColor(color);
    for (auto& col : m_cellPools)
    {
        for (ViewChild<std::unique_ptr<CellView>>& cv : col[0])
            if (cv.m_view)
                cv.m_view->m_text->setMultiplyColor(color);
        for (ViewChild<std::unique_ptr<CellView>>& cv : col[1])
            if (cv.m_view)
                cv.m_view->m_text->setMultiplyColor(color);
    }
}

void Table::CellView::select()
{
    m_selected = true;
    m_text->colorGlyphs(rootView().themeData().fieldText());
}

void Table::CellView::deselect()
{
    m_selected = false;
    m_text->colorGlyphs(rootView().themeData().uiText());
}

void Table::mouseDown(const boo::SWindowCoord& coord,
                      boo::EMouseButton button, boo::EModifierKey mod)
{
    if (m_state && m_state->columnSplitResizeAllowed())
    {
        size_t cIdx = 0;
        for (const boo::SWindowRect& rect : m_hCellRects)
        {
            if (cIdx == 0)
            {
                ++cIdx;
                continue;
            }
            if (abs(coord.pixel[0] - rect.location[0]) < 4 &&
                unsigned(coord.pixel[1] - subRect().location[1] -
                         subRect().size[1] + rect.size[1]) < unsigned(rect.size[1]))
            {
                m_hDraggingIdx = cIdx;
                rootView().setActiveDragView(this);
                return;
            }
            ++cIdx;
        }
    }

    m_scroll.mouseDown(coord, button, mod);
    if (m_headerNeedsUpdate)
        _setHeaderVerts(subRect());

    if (m_deferredActivation != -1 && m_state)
    {
        m_state->rowActivated(m_deferredActivation);
        m_deferredActivation = -1;
    }
}

void Table::RowsView::mouseDown(const boo::SWindowCoord& coord,
                                boo::EMouseButton button, boo::EModifierKey mod)
{
    for (ViewChild<std::unique_ptr<CellView>>& hv : m_t.m_headerViews)
        if (hv.mouseDown(coord, button, mod))
            return; /* Trap header event */
    for (auto& col : m_t.m_cellPools)
    {
        for (ViewChild<std::unique_ptr<CellView>>& cv : col[0])
            cv.mouseDown(coord, button, mod);
        for (ViewChild<std::unique_ptr<CellView>>& cv : col[1])
            cv.mouseDown(coord, button, mod);
    }
}

void Table::CellView::mouseDown(const boo::SWindowCoord& coord,
                                boo::EMouseButton button, boo::EModifierKey mod)
{
    if (m_r != -1)
    {
        m_t.selectRow(m_r);
        if (m_t.m_clickFrames < 15)
            m_t.m_deferredActivation = m_r;
        else
            m_t.m_clickFrames = 0;
    }
    else
        m_t.m_headerNeedsUpdate = true;
}

void Table::mouseUp(const boo::SWindowCoord& coord,
                    boo::EMouseButton button, boo::EModifierKey mod)
{
    m_scroll.mouseUp(coord, button, mod);
    if (m_headerNeedsUpdate)
        _setHeaderVerts(subRect());
    if (m_hDraggingIdx)
    {
        rootView().setActiveDragView(nullptr);
        m_hDraggingIdx = 0;
    }
}

void Table::RowsView::mouseUp(const boo::SWindowCoord& coord,
                              boo::EMouseButton button, boo::EModifierKey mod)
{
    size_t idx = 0;
    for (ViewChild<std::unique_ptr<CellView>>& hv : m_t.m_headerViews)
    {
        if (hv.m_mouseDown && hv.m_mouseIn)
            m_t.cycleSortColumn(idx);
        hv.mouseUp(coord, button, mod);
        ++idx;
    }
    for (auto& col : m_t.m_cellPools)
    {
        for (ViewChild<std::unique_ptr<CellView>>& cv : col[0])
            cv.mouseUp(coord, button, mod);
        for (ViewChild<std::unique_ptr<CellView>>& cv : col[1])
            cv.mouseUp(coord, button, mod);
    }
}

void Table::CellView::mouseUp(const boo::SWindowCoord& coord,
                              boo::EMouseButton button, boo::EModifierKey mod)
{
    if (m_r == -1)
        m_t.m_headerNeedsUpdate = true;
}

void Table::mouseMove(const boo::SWindowCoord& coord)
{
    if (m_state && m_state->columnSplitResizeAllowed())
    {
        if (m_hDraggingIdx)
        {
            float split = (coord.pixel[0] - subRect().location[0]) / float(subRect().size[0]);

            if (m_hDraggingIdx <= 1)
                split = std::max(0.03f, split);
            else
                split = std::max(m_state->getColumnSplit(m_hDraggingIdx-1)+0.03f, split);

            if (m_hDraggingIdx >= m_columns - 1)
                split = std::min(0.97f, split);
            else
                split = std::min(m_state->getColumnSplit(m_hDraggingIdx+1)-0.03f, split);

            m_state->setColumnSplit(m_hDraggingIdx, split);
            updateSize();
            return;
        }
        size_t cIdx = 0;
        bool hovering = false;
        for (const boo::SWindowRect& rect : m_hCellRects)
        {
            if (cIdx++ == 0)
                continue;
            if (abs(coord.pixel[0] - rect.location[0]) < 4 &&
                unsigned(coord.pixel[1] - subRect().location[1] -
                         subRect().size[1] + rect.size[1]) < unsigned(rect.size[1]))
            {
                hovering = true;
                break;
            }
        }
        rootView().setVerticalSplitHover(hovering);
    }

    m_scroll.mouseMove(coord);
    if (m_headerNeedsUpdate)
        _setHeaderVerts(subRect());
}

void Table::RowsView::mouseMove(const boo::SWindowCoord& coord)
{
    for (ViewChild<std::unique_ptr<CellView>>& hv : m_t.m_headerViews)
        hv.mouseMove(coord);
    for (auto& col : m_t.m_cellPools)
    {
        for (ViewChild<std::unique_ptr<CellView>>& cv : col[0])
            cv.mouseMove(coord);
        for (ViewChild<std::unique_ptr<CellView>>& cv : col[1])
            cv.mouseMove(coord);
    }
}

void Table::mouseEnter(const boo::SWindowCoord& coord)
{
    m_scroll.mouseEnter(coord);
    if (m_headerNeedsUpdate)
        _setHeaderVerts(subRect());
}

void Table::CellView::mouseEnter(const boo::SWindowCoord& coord)
{
    if (m_r == -1)
        m_t.m_headerNeedsUpdate = true;
}

void Table::mouseLeave(const boo::SWindowCoord& coord)
{
    m_scroll.mouseLeave(coord);
    if (m_headerNeedsUpdate)
        _setHeaderVerts(subRect());
}

void Table::RowsView::mouseLeave(const boo::SWindowCoord& coord)
{
    for (ViewChild<std::unique_ptr<CellView>>& hv : m_t.m_headerViews)
        hv.mouseLeave(coord);
    for (auto& col : m_t.m_cellPools)
    {
        for (ViewChild<std::unique_ptr<CellView>>& cv : col[0])
            cv.mouseLeave(coord);
        for (ViewChild<std::unique_ptr<CellView>>& cv : col[1])
            cv.mouseLeave(coord);
    }
}

void Table::CellView::mouseLeave(const boo::SWindowCoord& coord)
{
    if (m_r == -1)
        m_t.m_headerNeedsUpdate = true;
}

void Table::scroll(const boo::SWindowCoord& coord, const boo::SScrollDelta& scroll)
{
    m_scroll.scroll(coord, scroll);
}

void Table::think()
{
    if (m_scroll.m_view)
        m_scroll.m_view->think();
    ++m_clickFrames;
}
    
void Table::CellView::reset()
{
    m_c = -1;
    m_r = -1;
    if (m_textHash)
    {
        m_text->typesetGlyphs("");
        m_textHash = 0;
    }
}
    
bool Table::CellView::reset(size_t c)
{
    m_c = c;
    m_r = -1;
    const std::string* headerText = m_t.m_data->header(c);
    if (headerText)
    {
        uint64_t hash = XXH64(headerText->data(), headerText->size(), 0);
        if (hash != m_textHash)
        {
            m_text->typesetGlyphs(*headerText, rootView().themeData().uiText());
            m_textHash = hash;
        }
        return true;
    }
    else if (m_textHash)
    {
        m_text->typesetGlyphs("");
        m_textHash = 0;
    }
    return false;
}

bool Table::CellView::reset(size_t c, size_t r)
{
    m_c = c;
    m_r = r;
    const std::string* cellText = m_t.m_data->cell(c, r);
    if (cellText)
    {
        uint64_t hash = XXH64(cellText->data(), cellText->size(), 0);
        if (hash != m_textHash)
        {
            m_text->typesetGlyphs(*cellText, rootView().themeData().uiText());
            m_textHash = hash;
        }
        return true;
    }
    else if (m_textHash)
    {
        m_text->typesetGlyphs("");
        m_textHash = 0;
    }
    return false;
}
 
std::vector<Table::ColumnPool>& Table::ensureCellPools(size_t rows, size_t cols, ViewResources& res)
{
    if (m_cellPools.size() < cols)
    {
        m_cellPools.reserve(cols);
        for (size_t i=m_cellPools.size() ; i<cols ; ++i)
            m_cellPools.emplace_back();
        m_ensuredRows = 0;
    }
    if (m_ensuredRows < rows)
    {
        for (size_t i=0 ; i<cols ; ++i)
        {
            ColumnPool& cp = m_cellPools[i];
            if (rows > SPECTER_TABLE_MAX_ROWS)
            {
                for (int p=0 ; p<2 ; ++p)
                    for (ViewChild<std::unique_ptr<CellView>>& cv : cp[p])
                        if (!cv.m_view)
                            cv.m_view.reset(new CellView(*this, res));
            }
            else
            {
                size_t r = 0;
                for (ViewChild<std::unique_ptr<CellView>>& cv : cp[0])
                {
                    if (!cv.m_view)
                        cv.m_view.reset(new CellView(*this, res));
                    ++r;
                    if (r >= rows)
                        break;
                }
            }
        }
        m_ensuredRows = rows;
    }
    return m_cellPools;
}
    
void Table::_updateData()
{
    m_header = false;
    bool newViewChildren = false;
    if (m_columns != m_data->columnCount() || m_rows > m_data->rowCount() + SPECTER_TABLE_MAX_ROWS)
        newViewChildren = true;

    m_rows = m_data->rowCount();
    m_columns = m_data->columnCount();
    if (!m_columns)
        return;

    ViewResources& res = rootView().viewRes();
    if (newViewChildren)
    {
        m_headerViews.clear();
        m_cellPools.clear();
        m_headerViews.reserve(m_columns);
        for (size_t c=0 ; c<m_columns ; ++c)
            m_headerViews.emplace_back();
        m_ensuredRows = 0;
    }
    ensureCellPools(m_rows, m_columns, res);
    
    size_t poolIdx = m_rowsView.m_visibleStart / SPECTER_TABLE_MAX_ROWS;
    size_t startRow = poolIdx * SPECTER_TABLE_MAX_ROWS;
    int pool0 = (poolIdx & 1) != 0;
    int pool1 = (poolIdx & 1) == 0;
    
    for (size_t c=0 ; c<m_columns ; ++c)
    {
        std::unique_ptr<CellView>& cv = m_headerViews[c].m_view;
        if (!cv)
            cv.reset(new CellView(*this, res));
        if (cv->reset(c))
            m_header = true;

        ColumnPool& col = m_cellPools[c];

        if (poolIdx != m_activePool)
        {
            for (size_t r=startRow, i=0 ; i<SPECTER_TABLE_MAX_ROWS ; ++r, ++i)
            {
                ViewChild<std::unique_ptr<CellView>>& cv = col[pool0][i];
                if (cv.m_view)
                {
                    if (r < m_rows)
                        cv.m_view->reset(c, r);
                    else
                        cv.m_view->reset();
                }
            }
            for (size_t r=startRow+SPECTER_TABLE_MAX_ROWS, i=0 ; i<SPECTER_TABLE_MAX_ROWS ; ++r, ++i)
            {
                ViewChild<std::unique_ptr<CellView>>& cv = col[pool1][i];
                if (cv.m_view)
                {
                    if (r < m_rows)
                        cv.m_view->reset(c, r);
                    else
                        cv.m_view->reset();
                }
            }
        }
    }
    
    m_activePool = poolIdx;
}
    
void Table::updateData()
{
    m_activePool = -1;
    updateSize();
}

std::vector<boo::SWindowRect> Table::getCellRects(const boo::SWindowRect& sub) const
{
    if (!m_columns)
        return {};
    float pf = rootView().viewRes().pixelFactor();

    /* Validate column split values */
    bool valid = false;
    std::vector<float> splits;
    splits.reserve(m_columns);
    if (m_state)
    {
        float lastSplit = 0.0;
        size_t i;
        for (i=0 ; i<m_columns ; ++i)
        {
            float split = m_state->getColumnSplit(i);
            if (split < lastSplit || split < 0.0 || split > 1.0)
                break;
            splits.push_back(split);
            lastSplit = split;
        }
        if (i == m_columns)
            valid = true;
    }

    /* Uniform split otherwise */
    if (!valid)
    {
        float split = 0.0;
        for (size_t i=0 ; i<m_columns ; ++i)
        {
            splits.push_back(split);
            split += 1.0 / float(m_columns);
        }
    }

    std::vector<boo::SWindowRect> ret;
    ret.reserve(m_columns);

    int lastX = 0;
    for (size_t i=0 ; i<m_columns ; ++i)
    {
        float nextSplit = (i==m_columns-1) ? 1.0 : splits[i+1];
        int x = nextSplit * sub.size[0];
        ret.push_back({sub.location[0] + lastX, sub.location[1],
                       x - lastX, int(ROW_HEIGHT * pf)});
        lastX = x;
    }

    return ret;
}


void Table::resized(const boo::SWindowRect& root, const boo::SWindowRect& sub)
{
    View::resized(root, sub);
    if (m_scroll.m_view)
        m_scroll.m_view->resized(root, sub);

    boo::SWindowRect headRow = sub;
    float pf = rootView().viewRes().pixelFactor();
    headRow.location[1] += sub.size[1] - ROW_HEIGHT * pf;
    std::vector<boo::SWindowRect> cellRects = getCellRects(headRow);
    _setHeaderVerts(sub);
    size_t cIdx = 0;
    for (ViewChild<std::unique_ptr<CellView>>& hv : m_headerViews)
    {
        if (hv.m_view)
            hv.m_view->resized(root, cellRects[cIdx], sub);
        ++cIdx;
    }
}

int Table::RowsView::nominalHeight() const
{
    float pf = rootView().viewRes().pixelFactor();
    int rows = m_t.m_rows + 1;
    return rows * (ROW_HEIGHT + CELL_MARGIN * 2) * pf;
}

void Table::RowsView::resized(const boo::SWindowRect& root, const boo::SWindowRect& sub,
                              const boo::SWindowRect& scissor)
{
    View::resized(root, sub);
    _setRowVerts(sub, scissor);
    m_t._updateData();

    if (!m_t.m_columns)
        return;

    float pf = rootView().viewRes().pixelFactor();
    boo::SWindowRect cellScissor = scissor;
    cellScissor.size[1] -= 2 * pf;
    boo::SWindowRect rowRect = sub;
    rowRect.location[1] += sub.size[1] - ROW_HEIGHT * pf;
    int spacing = (ROW_HEIGHT + CELL_MARGIN * 2) * pf;
    std::vector<boo::SWindowRect> cellRects = m_t.getCellRects(rowRect);
    auto cellRectIt = cellRects.begin();
    int poolIdx = m_visibleStart / SPECTER_TABLE_MAX_ROWS;
    int pool0 = (poolIdx & 1) != 0;
    int pool1 = (poolIdx & 1) == 0;
    int locationStart = spacing * poolIdx * SPECTER_TABLE_MAX_ROWS;
    for (auto& col : m_t.m_cellPools)
    {
        cellRectIt->location[1] -= locationStart;
        for (ViewChild<std::unique_ptr<CellView>>& cv : col[pool0])
        {
            cellRectIt->location[1] -= spacing;
            if (cv.m_view)
                cv.m_view->resized(root, *cellRectIt, cellScissor);
        }
        for (ViewChild<std::unique_ptr<CellView>>& cv : col[pool1])
        {
            cellRectIt->location[1] -= spacing;
            if (cv.m_view)
                cv.m_view->resized(root, *cellRectIt, cellScissor);
        }
        ++cellRectIt;
    }

    m_scissorRect = scissor;
    m_scissorRect.size[1] -= spacing;
}

void Table::CellView::resized(const boo::SWindowRect& root, const boo::SWindowRect& sub,
                              const boo::SWindowRect& scissor)
{
    View::resized(root, sub);
    boo::SWindowRect textRect = sub;
    float pf = rootView().viewRes().pixelFactor();
    textRect.location[0] += 5 * pf;
    textRect.location[1] += 5 * pf;
    m_text->resized(root, textRect);
    m_scissorRect = sub.intersect(scissor);
}

void Table::draw(boo::IGraphicsCommandQueue* gfxQ)
{
    if (m_scroll.m_view)
        m_scroll.m_view->draw(gfxQ);
}

void Table::RowsView::draw(boo::IGraphicsCommandQueue* gfxQ)
{
    gfxQ->setShaderDataBinding(m_vertsBinding);

    gfxQ->setScissor(m_scissorRect);
    gfxQ->draw(1, m_visibleRows * m_t.m_columns * 6 - 2);
    
    int poolIdx = m_t.m_activePool;
    int pool0 = (poolIdx & 1) != 0;
    int pool1 = (poolIdx & 1) == 0;
    for (auto& col : m_t.m_cellPools)
    {
        size_t idx = poolIdx * SPECTER_TABLE_MAX_ROWS;
        for (ViewChild<std::unique_ptr<CellView>>& cv : col[pool0])
        {
            if (cv.m_view && idx >= m_visibleStart && idx < m_visibleStart + m_visibleRows)
                cv.m_view->draw(gfxQ);
            ++idx;
        }
        for (ViewChild<std::unique_ptr<CellView>>& cv : col[pool1])
        {
            if (cv.m_view && idx >= m_visibleStart && idx < m_visibleStart + m_visibleRows)
                cv.m_view->draw(gfxQ);
            ++idx;
        }
    }

    if (m_t.m_header)
    {
        gfxQ->setShaderDataBinding(m_t.m_vertsBinding);
        gfxQ->setScissor(rootView().subRect());
        gfxQ->draw(1, m_t.m_columns * 6 - 2);
        for (ViewChild<std::unique_ptr<CellView>>& hv : m_t.m_headerViews)
            if (hv.m_view)
                hv.m_view->draw(gfxQ);
    }

    gfxQ->setScissor(rootView().subRect());
}

void Table::CellView::draw(boo::IGraphicsCommandQueue* gfxQ)
{
    if (m_scissorRect.size[0] && m_scissorRect.size[1])
    {
        gfxQ->setScissor(m_scissorRect);
        m_text->draw(gfxQ);
    }
}

}
