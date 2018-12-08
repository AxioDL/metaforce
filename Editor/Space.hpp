#pragma once

#include "athena/DNAYaml.hpp"
#include "specter/Space.hpp"
#include "specter/IMenuNode.hpp"
#include "specter/Button.hpp"
#include "specter/MultiLineTextView.hpp"
#include "specter/ViewResources.hpp"
#include "specter/RootView.hpp"
#include "ProjectManager.hpp"

namespace specter {
class View;
class SplitView;
class ViewResources;
class Toolbar;
struct Icon;
} // namespace specter
namespace urde {
class ViewManager;
class RootSpace;
class SplitSpace;

class Space : public specter::ISpaceController {
  friend class SplitSpace;

public:
  virtual ~Space() = default;
  Space(const Space& other) = delete;
  Space& operator=(const Space& other) = delete;

  /** Common encoded-enumeration for all space classes */
  enum class Class {
    None,
    RootSpace,
    SplitSpace,
    TestSpace,
    ResourceBrowser,
    ModelViewer,
    EffectEditor,
    InformationCenter,
    GameMode
  };

  struct State : athena::io::DNAVYaml<athena::Big> {
    Delete _d;
  };
  static Space* NewSpaceFromConfigStream(ViewManager& vm, Space* parent, ConfigReader& r);
  static RootSpace* NewRootSpaceFromConfigStream(ViewManager& vm, ConfigReader& r);

  struct SpaceMenuNode final : specter::IMenuNode {
    struct SubNodeData final : specter::IMenuNode {
      Class m_cls;
      std::string m_key;
      std::string m_text;
      specter::Icon& m_icon;
      zeus::CColor m_color;
      const std::string* text() const { return &m_text; }
      void activated(const boo::SWindowCoord& coord) {}

      SubNodeData(Class cls, const char* key, const char* text, specter::Icon& icon, const zeus::CColor& color)
      : m_cls(cls), m_key(key), m_text(text), m_icon(icon), m_color(color) {}
    };
    static std::vector<SubNodeData> s_subNodeDats;

    struct SubNode final : specter::IMenuNode {
      Space& m_space;
      const SubNodeData& m_data;
      const std::string* text() const { return &m_data.m_text; }
      void activated(const boo::SWindowCoord& coord);

      SubNode(Space& space, const SubNodeData& data) : m_space(space), m_data(data) {}
    };
    std::vector<SubNode> m_subNodes;

    SpaceMenuNode(Space& space) {
      m_subNodes.reserve(s_subNodeDats.size());
      for (const SubNodeData& sn : s_subNodeDats)
        m_subNodes.emplace_back(space, sn);
    }

    static std::string s_text;
    const std::string* text() const { return &s_text; }

    size_t subNodeCount() const { return m_subNodes.size(); }
    IMenuNode* subNode(size_t idx) { return &m_subNodes[idx]; }

    static void InitializeStrings(ViewManager& vm);
    static const std::string* LookupClassString(Class cls) {
      for (const SubNodeData& sn : s_subNodeDats)
        if (sn.m_cls == cls)
          return &sn.m_text;
      return nullptr;
    }
    static specter::Icon* LookupClassIcon(Class cls) {
      for (SubNodeData& sn : s_subNodeDats)
        if (sn.m_cls == cls)
          return &sn.m_icon;
      return nullptr;
    }
    static const zeus::CColor* LookupClassColor(Class cls) {
      for (SubNodeData& sn : s_subNodeDats)
        if (sn.m_cls == cls)
          return &sn.m_color;
      return nullptr;
    }
  } m_spaceMenuNode;

  struct SpaceSelectBind : specter::IButtonBinding {
    Space& m_space;
    std::string_view name(const specter::Control* control) const { return SpaceMenuNode::s_text; }

    MenuStyle menuStyle(const specter::Button* button) const { return MenuStyle::Primary; }
    std::unique_ptr<specter::View> buildMenu(const specter::Button* button);

    SpaceSelectBind(Space& space) : m_space(space) {}
  } m_spaceSelectBind;
  std::unique_ptr<specter::Button> m_spaceSelectButton;

protected:
  friend class ViewManager;
  friend class RootSpace;
  ViewManager& m_vm;
  Class m_class = Class::None;
  Space* m_parent;
  std::unique_ptr<specter::Space> m_spaceView;
  Space(ViewManager& vm, Class cls, Space* parent);

  /* Allows common Space code to access DNA-encoded state */
  virtual const Space::State& spaceState() const = 0;

  /* Structural control */
  virtual bool usesToolbar() const { return false; }
  virtual unsigned toolbarUnits() const { return 1; }
  virtual void buildToolbarView(specter::ViewResources& res, specter::Toolbar& tb) {}
  virtual specter::View* buildContentView(specter::ViewResources& res) = 0;
  virtual specter::View* buildSpaceView(specter::ViewResources& res);

public:
  virtual void saveState(athena::io::IStreamWriter& w) const;
  virtual void saveState(athena::io::YAMLDocWriter& w) const;
  virtual void reloadState() {}

  virtual void think() {}

  virtual Space* copy(Space* parent) const = 0;
  bool spaceSplitAllowed() const { return true; }

  specter::ISplitSpaceController* spaceSplit(specter::SplitView::Axis axis, int thisSlot);
  virtual std::unique_ptr<Space> exchangeSpaceSplitJoin(Space* removeSpace, std::unique_ptr<Space>&& keepSpace) {
    return std::unique_ptr<Space>();
  }

  virtual specter::View* basisView() { return m_spaceView.get(); }
  Class cls() const { return m_class; }
  SplitSpace* castToSplitSpace();
};

class RootSpace : public Space {
  friend class ViewManager;
  std::unique_ptr<specter::RootView> m_rootView;
  std::unique_ptr<Space> m_spaceTree;
  struct State : Space::State{AT_DECL_DNA_YAML AT_DECL_DNAV} m_state;
  const Space::State& spaceState() const { return m_state; }

public:
  RootSpace(ViewManager& vm) : Space(vm, Class::RootSpace, nullptr) {}
  RootSpace(ViewManager& vm, ConfigReader& r) : RootSpace(vm) {
    m_state.read(r);
    if (auto rec = r.enterSubRecord("spaceTree"))
      m_spaceTree.reset(NewSpaceFromConfigStream(vm, this, r));
  }

  void think() {
    if (m_spaceTree)
      m_spaceTree->think();
  }

  void saveState(athena::io::IStreamWriter& w) const {
    w.writeUint32Big(atUint32(m_class));
    m_state.write(w);

    if (m_spaceTree)
      m_spaceTree->saveState(w);
    else
      w.writeUint32Big(0);
  }

  void saveState(athena::io::YAMLDocWriter& w) const {
    w.writeUint32("class", atUint32(m_class));
    m_state.write(w);

    if (auto rec = w.enterSubRecord("spaceTree")) {
      if (m_spaceTree)
        m_spaceTree->saveState(w);
      else
        w.writeUint32("class", 0);
    }
  }

  void setChild(std::unique_ptr<Space>&& space) {
    m_spaceTree = std::move(space);
    m_spaceTree->m_parent = this;
  }

  Space* copy(Space* parent) const { return nullptr; }
  bool spaceSplitAllowed() const { return false; }

  specter::View* buildSpaceView(specter::ViewResources& res);
  specter::View* buildContentView(specter::ViewResources& res) { return m_spaceTree->buildSpaceView(res); }

  std::unique_ptr<Space> exchangeSpaceSplitJoin(Space* removeSpace, std::unique_ptr<Space>&& keepSpace);

  specter::View* basisView();
};

class SplitSpace : public Space, public specter::ISplitSpaceController {
  friend class ViewManager;
  std::unique_ptr<Space> m_slots[2];
  std::unique_ptr<specter::SplitView> m_splitView;
  struct State : Space::State {
    AT_DECL_DNA_YAML
    AT_DECL_DNAV
    Value<specter::SplitView::Axis> axis = specter::SplitView::Axis::Horizontal;
    Value<float> split = 0.5;
  } m_state;
  const Space::State& spaceState() const { return m_state; }

public:
  SplitSpace(ViewManager& vm, Space* parent, specter::SplitView::Axis axis) : Space(vm, Class::SplitSpace, parent) {
    m_state.axis = axis;
    reloadState();
  }
  SplitSpace(ViewManager& vm, Space* parent, ConfigReader& r)
  : SplitSpace(vm, parent, specter::SplitView::Axis::Horizontal) {
    m_state.read(r);
    if (auto rec = r.enterSubRecord("slot0"))
      m_slots[0].reset(NewSpaceFromConfigStream(vm, this, r));
    if (auto rec = r.enterSubRecord("slot1"))
      m_slots[1].reset(NewSpaceFromConfigStream(vm, this, r));
    reloadState();
  }

  void reloadState() {
    m_state.split = std::min(1.f, std::max(0.f, m_state.split));
    if (m_state.axis != specter::SplitView::Axis::Horizontal && m_state.axis != specter::SplitView::Axis::Vertical)
      m_state.axis = specter::SplitView::Axis::Horizontal;
    if (m_splitView) {
      m_splitView->setSplit(m_state.split);
      m_splitView->setAxis(m_state.axis);
    }
  }

  void think() {
    if (m_slots[0])
      m_slots[0]->think();
    if (m_slots[1])
      m_slots[1]->think();
  }

  void saveState(athena::io::IStreamWriter& w) const {
    w.writeUint32Big(atUint32(m_class));
    m_state.write(w);

    if (m_slots[0])
      m_slots[0]->saveState(w);
    else
      w.writeUint32Big(0);

    if (m_slots[1])
      m_slots[1]->saveState(w);
    else
      w.writeUint32Big(0);
  }

  void saveState(athena::io::YAMLDocWriter& w) const {
    w.writeUint32("class", atUint32(m_class));
    m_state.write(w);

    if (auto rec = w.enterSubRecord("slot0")) {
      if (m_slots[0])
        m_slots[0]->saveState(w);
      else
        w.writeUint32("class", 0);
    }

    if (auto rec = w.enterSubRecord("slot1")) {
      if (m_slots[1])
        m_slots[1]->saveState(w);
      else
        w.writeUint32("class", 0);
    }
  }

  void setChildSlot(unsigned slot, std::unique_ptr<Space>&& space);

  specter::View* buildSpaceView(specter::ViewResources& res) { return buildContentView(res); }
  specter::View* buildContentView(specter::ViewResources& res);

  Space* copy(Space* parent) const { return nullptr; }
  bool spaceSplitAllowed() const { return false; }

  ISpaceController* spaceJoin(int keepSlot) {
    if (m_parent) {
      ISpaceController* ret = m_slots[keepSlot].get();
      m_parent->exchangeSpaceSplitJoin(this, std::move(m_slots[keepSlot]));
      return ret;
    }
    return nullptr;
  }

  std::unique_ptr<Space> exchangeSpaceSplitJoin(Space* removeSpace, std::unique_ptr<Space>&& keepSpace);

  specter::SplitView* splitView() { return m_splitView.get(); }
  void updateSplit(float split) { m_state.split = split; }
  void joinViews(specter::SplitView* thisSplit, int thisSlot, specter::SplitView* otherSplit, int otherSlot);

  void setAxis(specter::SplitView::Axis axis) {
    m_state.axis = axis;
    reloadState();
  }

  specter::SplitView::Axis axis() const { return m_state.axis; }
  float split() const { return m_state.split; }

  specter::View* basisView() { return m_splitView.get(); }
};
inline SplitSpace* Space::castToSplitSpace() {
  return cls() == Class::SplitSpace ? static_cast<SplitSpace*>(this) : nullptr;
}

class ViewerSpace : public Space {
public:
  ViewerSpace(ViewManager& vm, Class cls, Space* parent) : Space(vm, cls, parent) {}
};

class EditorSpace : public Space {
public:
  EditorSpace(ViewManager& vm, Class cls, Space* parent) : Space(vm, cls, parent) {}
};

class GameSpace : public Space {
public:
  GameSpace(ViewManager& vm, Class cls, Space* parent) : Space(vm, cls, parent) {}
};

class TestSpace : public Space {
  std::unique_ptr<specter::Button> m_button;
  std::unique_ptr<specter::MultiLineTextView> m_textView;

  std::string m_contentStr;
  std::string m_buttonStr;

  specter::IButtonBinding* m_binding;

public:
  TestSpace(ViewManager& vm, Space* parent, std::string_view content, std::string_view button,
            specter::IButtonBinding* binding)
  : Space(vm, Class::TestSpace, parent), m_contentStr(content), m_buttonStr(button), m_binding(binding) {}

  struct State : Space::State{AT_DECL_DNA_YAML AT_DECL_DNAV} m_state;
  const Space::State& spaceState() const { return m_state; }

  bool usesToolbar() const { return true; }
  void buildToolbarView(specter::ViewResources& res, specter::Toolbar& tb) {
    m_button.reset(new specter::Button(res, tb, m_binding, m_buttonStr));
    tb.push_back(m_button.get(), 0);
  }

  specter::View* buildContentView(specter::ViewResources& res) {
    m_textView.reset(new specter::MultiLineTextView(res, *m_spaceView, res.m_heading14));
    m_textView->setBackground(res.themeData().viewportBackground());
    m_textView->typesetGlyphs(m_contentStr, res.themeData().uiText());
    return m_textView.get();
  }
};

} // namespace urde
