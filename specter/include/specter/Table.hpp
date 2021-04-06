#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "specter/View.hpp"

namespace specter {
#define SPECTER_TABLE_MAX_ROWS 128ul

class ScrollView;

enum class SortDirection { None, Ascending, Descending };

struct ITableDataBinding {
  virtual size_t columnCount() const = 0;
  virtual size_t rowCount() const = 0;
  virtual std::string_view header(size_t cIdx) const { return {}; }
  virtual std::string_view cell(size_t cIdx, size_t rIdx) const { return {}; }
};

struct ITableStateBinding {
  virtual float getColumnSplit(size_t cIdx) const { return -1.0; }
  virtual bool columnSplitResizeAllowed() const { return false; }
  virtual void setColumnSplit(size_t cIdx, float split) {}
  virtual SortDirection getSort(size_t& cIdx) const {
    cIdx = 0;
    return SortDirection::None;
  }
  virtual void setSort(size_t cIdx, SortDirection dir) {}
  virtual void setSelectedRow(size_t rIdx) {}
  virtual void rowActivated(size_t rIdx) {}
};

class Table : public View {
  struct CellView;

  ITableDataBinding* m_data;
  ITableStateBinding* m_state;

  size_t m_maxColumns;
  size_t m_rows = 0;
  size_t m_columns = 0;
  size_t m_selectedRow = SIZE_MAX;
  size_t m_deferredActivation = SIZE_MAX;
  size_t m_clickFrames = 15;

  std::vector<ViewChild<std::unique_ptr<CellView>>> m_headerViews;
  using ColumnPool = std::array<std::array<ViewChild<std::unique_ptr<CellView>>, SPECTER_TABLE_MAX_ROWS>, 2>;
  std::vector<ColumnPool> m_cellPools;
  size_t m_ensuredRows = 0;
  std::vector<ColumnPool>& ensureCellPools(size_t rows, size_t cols, ViewResources& res);
  size_t m_activePool = SIZE_MAX;
  bool m_header = false;

  std::vector<boo::SWindowRect> m_hCellRects;
  size_t m_hDraggingIdx = 0;

  std::unique_ptr<SolidShaderVert[]> m_hVerts;
  VertexBufferBindingSolid m_vertsBinding;
  void _setHeaderVerts(const boo::SWindowRect& rect);

  std::vector<boo::SWindowRect> getCellRects(const boo::SWindowRect& tableRect) const;

  ViewChild<std::unique_ptr<ScrollView>> m_scroll;

  struct RowsView : public View {
    Table& m_t;

    std::unique_ptr<SolidShaderVert[]> m_verts;
    VertexBufferBindingSolid m_vertsBinding;
    size_t m_visibleStart = 0;
    size_t m_visibleRows = 0;
    boo::SWindowRect m_scissorRect;
    void _setRowVerts(const boo::SWindowRect& rowsRect, const boo::SWindowRect& scissor);

    RowsView(Table& t, ViewResources& res);
    int nominalHeight() const override;
    int nominalWidth() const override;
    void mouseDown(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey) override;
    void mouseUp(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey) override;
    void mouseMove(const boo::SWindowCoord&) override;
    void mouseLeave(const boo::SWindowCoord&) override;
    void resized(const boo::SWindowRect& root, const boo::SWindowRect& sub, const boo::SWindowRect& scissor) override;
    void draw(boo::IGraphicsCommandQueue* gfxQ) override;
  } m_rowsView;

  bool m_headerNeedsUpdate = false;
  bool m_inSelectRow = false;

  void _updateData();

public:
  Table(ViewResources& res, View& parentView, ITableDataBinding* data, ITableStateBinding* state = nullptr,
        size_t maxColumns = 8);
  ~Table() override;

  void cycleSortColumn(size_t c);
  void selectRow(size_t r);
  void setMultiplyColor(const zeus::CColor& color) override;

  void mouseDown(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey) override;
  void mouseUp(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey) override;
  void mouseMove(const boo::SWindowCoord&) override;
  void mouseEnter(const boo::SWindowCoord&) override;
  void mouseLeave(const boo::SWindowCoord&) override;
  void scroll(const boo::SWindowCoord&, const boo::SScrollDelta&) override;

  void think() override;
  void updateData();
  void resized(const boo::SWindowRect& root, const boo::SWindowRect& sub) override;
  void draw(boo::IGraphicsCommandQueue* gfxQ) override;
};

} // namespace specter
