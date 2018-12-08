#pragma once

#include "Space.hpp"
#include "ViewManager.hpp"
#include "Runtime/IMain.hpp"

namespace urde {
class GameMode : public ViewerSpace {
  std::shared_ptr<IMain> m_main;

  struct State : Space::State {
    AT_DECL_DNA_YAML
    AT_DECL_DNAV
    Value<bool> showToolbar = true;
  } m_state;

  const Space::State& spaceState() const { return m_state; }

  struct View : specter::View {
    GameMode& m_gMode;

    View(GameMode& gMode, specter::ViewResources& res) : specter::View(res, gMode.m_vm.rootView()), m_gMode(gMode) {}

    void draw(boo::IGraphicsCommandQueue* gfxQ);
  };

  std::unique_ptr<View> m_view;

public:
  GameMode(ViewManager& vm, Space* parent) : ViewerSpace(vm, Class::GameMode, parent) { reloadState(); }

  GameMode(ViewManager& vm, Space* parent, const GameMode& other) : GameMode(vm, parent) {
    m_state = other.m_state;
    reloadState();
  }

  GameMode(ViewManager& vm, Space* parent, ConfigReader& r) : GameMode(vm, parent) {
    m_state.read(r);
    reloadState();
  }

  void reloadState() {}

  virtual specter::View* buildContentView(specter::ViewResources& res) {
    m_view.reset(new View(*this, res));
    return m_view.get();
  }

  void think();

  Space* copy(Space* parent) const { return new GameMode(m_vm, parent, *this); }

  bool usesToolbar() const { return m_state.showToolbar; }
};
} // namespace urde
