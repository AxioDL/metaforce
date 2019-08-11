#pragma once

#include "Space.hpp"

namespace urde {

class EffectEditor : public EditorSpace {
  struct State : Space::State {
    AT_DECL_DNA_YAMLV
    String<-1> path;
  } m_state;
  const Space::State& spaceState() const { return m_state; }

  struct View : specter::View {
    View(specter::ViewResources& res, specter::View& parent) : specter::View(res, parent) {}
  };

  specter::View* buildContentView(specter::ViewResources& res) { return nullptr; }

public:
  EffectEditor(ViewManager& vm, Space* parent) : EditorSpace(vm, Class::EffectEditor, parent) {}
  EffectEditor(ViewManager& vm, Space* parent, ConfigReader& r) : EffectEditor(vm, parent) { m_state.read(r); }
  EffectEditor(ViewManager& vm, Space* parent, const EffectEditor& other) : EffectEditor(vm, parent) {
    m_state = other.m_state;
  }

  Space* copy(Space* parent) const { return new EffectEditor(m_vm, parent, *this); }

  bool usesToolbar() const { return true; }
};

} // namespace urde
