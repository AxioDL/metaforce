#pragma once

namespace specter {
class Outliner {
  class Node : public View {
    struct INodeController {
      virtual boo::ITexture* icon() const { return nullptr; }
      virtual const std::string* text() const { return nullptr; }
      virtual size_t subNodeCount() const { return 0; }
      virtual INodeController* subNode(size_t idx) { return nullptr; }
      virtual void activated(const boo::SWindowCoord& coord) {}
    };

    std::string m_description;
    std::vector<std::unique_ptr<Node>> m_children;
    bool m_collapsible;
    bool m_collapsed;

  public:
    class Resources {
      friend class ViewResources;

      void init(boo::IGraphicsDataFactory* factory, const IThemeData& theme);
    };

    Node(ViewResources& res, View& parentView, std::string_view description);
    void mouseDown(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
    void mouseUp(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
    void mouseEnter(const boo::SWindowCoord&);
    void mouseLeave(const boo::SWindowCoord&);
    void resized(const boo::SWindowRect& root, const boo::SWindowRect& sub);
    void draw(boo::IGraphicsCommandQueue* gfxQ);
    void think();
  };
};
} // namespace specter
