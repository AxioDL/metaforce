#ifndef SPECTER_TABLE_HPP
#define SPECTER_TABLE_HPP

#include "View.hpp"
#include "ScrollView.hpp"

namespace Specter
{
#define SPECTER_TABLE_MAX_ROWS 128

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

    SolidShaderVert m_verts[SPECTER_TABLE_MAX_ROWS * 6];
    boo::IGraphicsBufferD* m_vertsBuf = nullptr;
    boo::IVertexFormat* m_vtxFmt = nullptr; /* OpenGL only */
    boo::IShaderDataBinding* m_shaderBinding = nullptr;

    ViewChild<std::unique_ptr<ScrollView>> m_scroll;

    struct RowsView : public View
    {
        Table& m_t;
        RowsView(Table& t, ViewResources& res) : View(res, t), m_t(t) {}
        void resized(const boo::SWindowRect& root, const boo::SWindowRect& sub);
        void draw(boo::IGraphicsCommandQueue* gfxQ);
    } m_rowsView;

public:
    Table(ViewResources& res, View& parentView, ITableDataBinding* data, ITableStateBinding* state=nullptr);

    void mouseDown(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
    void mouseUp(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
    void mouseEnter(const boo::SWindowCoord&);
    void mouseLeave(const boo::SWindowCoord&);
    void scroll(const boo::SWindowCoord&, const boo::SScrollDelta&);

    void resized(const boo::SWindowRect& root, const boo::SWindowRect& sub);
    void draw(boo::IGraphicsCommandQueue* gfxQ);
};

}

#endif // SPECTER_TABLE_HPP
