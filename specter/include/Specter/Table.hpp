#ifndef SPECTER_TABLE_HPP
#define SPECTER_TABLE_HPP

#include "View.hpp"
#include "ScrollView.hpp"
#include "TextView.hpp"

namespace Specter
{
#define SPECTER_TABLE_MAX_ROWS 128ul
#define SPECTER_TABLE_MAX_COLUMNS 32ul

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
    virtual const std::string* header(size_t cIdx) const {return nullptr;}
    virtual const std::string* cell(size_t cIdx, size_t rIdx) const {return nullptr;}
};

struct ITableStateBinding
{
    virtual float columnSplit(size_t cIdx) {return -1.0;}
    virtual void setColumnSplit(size_t cIdx, float split) {}
    virtual SortDirection sort(size_t cIdx) {return SortDirection::None;}
    virtual void setSort(size_t cIdx, SortDirection dir) {}
};

class Table : public View
{
    ITableDataBinding* m_data;
    ITableStateBinding* m_state;

    size_t m_rows = 0;
    size_t m_columns = 0;
    struct CellView : public View
    {
        Table& m_t;
        std::unique_ptr<TextView> m_text;
        CellView(Table& t, ViewResources& res);
        void resized(const boo::SWindowRect& root, const boo::SWindowRect& sub);
        void draw(boo::IGraphicsCommandQueue* gfxQ);
    };
    std::vector<std::unique_ptr<CellView>> m_headerViews;
    std::vector<std::vector<std::unique_ptr<CellView>>> m_cellViews;
    bool m_header = false;

    ViewChild<std::unique_ptr<ScrollView>> m_scroll;

    struct RowsView : public View
    {
        Table& m_t;

        SolidShaderVert m_verts[SPECTER_TABLE_MAX_ROWS * SPECTER_TABLE_MAX_COLUMNS * 6];
        boo::IGraphicsBufferD* m_vertsBuf = nullptr;
        boo::IVertexFormat* m_vtxFmt = nullptr; /* OpenGL only */
        boo::IShaderDataBinding* m_shaderBinding = nullptr;
        size_t m_visibleStart = 0;
        size_t m_visibleRows = 0;
        boo::SWindowRect m_scissorRect;
        void _setRowVerts(const boo::SWindowRect& rowsRect, const boo::SWindowRect& scissor);

        RowsView(Table& t, ViewResources& res);
        int nominalHeight() const;
        int nominalWidth() const {return m_t.m_scroll.m_view->nominalWidth();}
        void resized(const boo::SWindowRect& root, const boo::SWindowRect& sub,
                     const boo::SWindowRect& scissor);
        void draw(boo::IGraphicsCommandQueue* gfxQ);
    } m_rowsView;

public:
    Table(ViewResources& res, View& parentView, ITableDataBinding* data, ITableStateBinding* state=nullptr);

    void setMultiplyColor(const Zeus::CColor& color);

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
