#ifndef SPECTER_TABLE_HPP
#define SPECTER_TABLE_HPP

#include "View.hpp"
#include "ScrollView.hpp"
#include "TextView.hpp"
#include <array>

namespace specter
{
#define SPECTER_TABLE_MAX_ROWS 128ul

enum class SortDirection
{
    None,
    Ascending,
    Descending
};

struct ITableDataBinding
{
    virtual size_t columnCount() const=0;
    virtual size_t rowCount() const=0;
    virtual std::string_view header(size_t cIdx) const {return nullptr;}
    virtual std::string_view cell(size_t cIdx, size_t rIdx) const {return nullptr;}
};

struct ITableStateBinding
{
    virtual float getColumnSplit(size_t cIdx) const {return -1.0;}
    virtual bool columnSplitResizeAllowed() const {return false;}
    virtual void setColumnSplit(size_t cIdx, float split) {}
    virtual SortDirection getSort(size_t& cIdx) const {cIdx = 0; return SortDirection::None;}
    virtual void setSort(size_t cIdx, SortDirection dir) {}
    virtual void setSelectedRow(size_t rIdx) {}
    virtual void rowActivated(size_t rIdx) {}
};

class Table : public View
{
    ITableDataBinding* m_data;
    ITableStateBinding* m_state;

    size_t m_maxColumns;
    size_t m_rows = 0;
    size_t m_columns = 0;
    size_t m_selectedRow = -1;
    size_t m_deferredActivation = -1;
    size_t m_clickFrames = 15;

    struct CellView : public View
    {
        Table& m_t;
        std::unique_ptr<TextView> m_text;
        size_t m_c, m_r;
        boo::SWindowRect m_scissorRect;
        uint64_t m_textHash = 0;
        CellView(Table& t, ViewResources& res);

        bool m_selected = false;
        void select();
        void deselect();
        void reset();
        bool reset(size_t c);
        bool reset(size_t c, size_t r);

        void mouseDown(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
        void mouseUp(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
        void mouseEnter(const boo::SWindowCoord&);
        void mouseLeave(const boo::SWindowCoord&);
        void resized(const boo::SWindowRect& root, const boo::SWindowRect& sub,
                     const boo::SWindowRect& scissor);
        void draw(boo::IGraphicsCommandQueue* gfxQ);
    };
    std::vector<ViewChild<std::unique_ptr<CellView>>> m_headerViews;
    using ColumnPool = std::array<std::array<ViewChild<std::unique_ptr<CellView>>, SPECTER_TABLE_MAX_ROWS>, 2>;
    std::vector<ColumnPool> m_cellPools;
    size_t m_ensuredRows = 0;
    std::vector<ColumnPool>& ensureCellPools(size_t rows, size_t cols, ViewResources& res);
    size_t m_activePool = -1;
    bool m_header = false;

    std::vector<boo::SWindowRect> m_hCellRects;
    size_t m_hDraggingIdx = 0;

    std::unique_ptr<SolidShaderVert[]> m_hVerts;
    VertexBufferBindingSolid m_vertsBinding;
    void _setHeaderVerts(const boo::SWindowRect& rect);

    std::vector<boo::SWindowRect> getCellRects(const boo::SWindowRect& tableRect) const;

    ViewChild<std::unique_ptr<ScrollView>> m_scroll;

    struct RowsView : public View
    {
        Table& m_t;

        std::unique_ptr<SolidShaderVert[]> m_verts;
        VertexBufferBindingSolid m_vertsBinding;
        size_t m_visibleStart = 0;
        size_t m_visibleRows = 0;
        boo::SWindowRect m_scissorRect;
        void _setRowVerts(const boo::SWindowRect& rowsRect, const boo::SWindowRect& scissor);

        RowsView(Table& t, ViewResources& res);
        int nominalHeight() const;
        int nominalWidth() const {return m_t.m_scroll.m_view->nominalWidth();}
        void mouseDown(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
        void mouseUp(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
        void mouseMove(const boo::SWindowCoord&);
        void mouseLeave(const boo::SWindowCoord&);
        void resized(const boo::SWindowRect& root, const boo::SWindowRect& sub,
                     const boo::SWindowRect& scissor);
        void draw(boo::IGraphicsCommandQueue* gfxQ);
    } m_rowsView;

    bool m_headerNeedsUpdate = false;
    bool m_inSelectRow = false;
    
    void _updateData();

public:
    Table(ViewResources& res, View& parentView, ITableDataBinding* data,
          ITableStateBinding* state=nullptr, size_t maxColumns=8);

    void cycleSortColumn(size_t c);
    void selectRow(size_t r);
    void setMultiplyColor(const zeus::CColor& color);

    void mouseDown(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
    void mouseUp(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
    void mouseMove(const boo::SWindowCoord&);
    void mouseEnter(const boo::SWindowCoord&);
    void mouseLeave(const boo::SWindowCoord&);
    void scroll(const boo::SWindowCoord&, const boo::SScrollDelta&);

    void think();
    void updateData();
    void resized(const boo::SWindowRect& root, const boo::SWindowRect& sub);
    void draw(boo::IGraphicsCommandQueue* gfxQ);
};

}

#endif // SPECTER_TABLE_HPP
