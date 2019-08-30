#pragma once

#include <cstddef>
#include <memory>
#include <string_view>
#include <vector>

#include "specter/View.hpp"

#include <boo/IWindow.hpp>

namespace specter {
class ScrollView;
class TextView;

struct IMenuNode;

class Menu : public View {
  IMenuNode* m_rootNode;
  IMenuNode* m_thisNode;
  std::unique_ptr<Menu> m_subMenu;
  std::unique_ptr<TextView> m_headText;

  int m_cWidth, m_cHeight, m_cTop;

  SolidShaderVert m_verts[8];
  VertexBufferBindingSolid m_vertsBinding;
  void setVerts(int width, int height, float pf);

  struct ContentView : View {
    Menu& m_menu;
    ContentView(ViewResources& res, Menu& menu);

    boo::SWindowRect m_scissorRect;
    SolidShaderVert m_hlVerts[4];
    VertexBufferBindingSolid m_hlVertsBinding;

    size_t m_highlightedItem = SIZE_MAX;
    void setHighlightedItem(size_t idx);
    void unsetHighlightedItem(size_t idx) {
      if (m_highlightedItem == idx)
        setHighlightedItem(-1);
    }

    void mouseDown(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey) override;
    void mouseUp(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey) override;
    void mouseMove(const boo::SWindowCoord&) override;
    void mouseLeave(const boo::SWindowCoord&) override;

    void resized(const boo::SWindowRect& root, const boo::SWindowRect& sub, const boo::SWindowRect& scissor) override;
    void draw(boo::IGraphicsCommandQueue* gfxQ) override;

    int nominalWidth() const override { return m_menu.m_cWidth; }
    int nominalHeight() const override { return m_menu.m_cHeight; }
  };
  std::unique_ptr<ContentView> m_content;
  ViewChild<std::unique_ptr<ScrollView>> m_scroll;

  struct ItemView : View {
    Menu& m_menu;
    std::unique_ptr<TextView> m_textView;
    size_t m_idx;
    IMenuNode* m_node;
    ItemView(ViewResources& res, Menu& menu, std::string_view text, size_t idx, IMenuNode* node);

    void mouseDown(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey) override;
    void mouseUp(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey) override;
    void mouseEnter(const boo::SWindowCoord&) override;
    void mouseLeave(const boo::SWindowCoord&) override;

    void resized(const boo::SWindowRect& root, const boo::SWindowRect& sub) override;
    void draw(boo::IGraphicsCommandQueue* gfxQ) override;
  };
  std::vector<ViewChild<std::unique_ptr<ItemView>>> m_items;
  IMenuNode* m_deferredActivation = nullptr;

  Menu(ViewResources& res, View& parentView, IMenuNode* rootNode, IMenuNode* thisNode);

public:
  Menu(ViewResources& res, View& parentView, IMenuNode* rootNode);
  ~Menu() override;

  void reset(IMenuNode* rootNode);

  void mouseDown(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey) override;
  void mouseUp(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey) override;
  void mouseMove(const boo::SWindowCoord&) override;
  void mouseLeave(const boo::SWindowCoord&) override;
  void scroll(const boo::SWindowCoord&, const boo::SScrollDelta&) override;

  void think() override;
  void resized(const boo::SWindowRect& root, const boo::SWindowRect& sub) override;
  void draw(boo::IGraphicsCommandQueue* gfxQ) override;
};

} // namespace specter
