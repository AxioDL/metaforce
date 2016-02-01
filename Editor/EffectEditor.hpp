#ifndef URDE_EFFECT_EDITOR_HPP
#define URDE_EFFECT_EDITOR_HPP

#include "Space.hpp"

namespace URDE
{

class EffectEditor : public EditorSpace
{
    struct State : Space::State
    {
        DECL_YAML
        String<-1> path;
    } m_state;
    const Space::State& spaceState() const {return m_state;}

    Specter::View* buildContentView(Specter::ViewResources& res)
    {
        return nullptr;
    }

public:
    EffectEditor(ViewManager& vm, Space* parent)
    : EditorSpace(vm, Class::EffectEditor, parent) {}
    EffectEditor(ViewManager& vm, Space* parent, ConfigReader& r)
    : EffectEditor(vm, parent) {m_state.read(r);}
    EffectEditor(ViewManager& vm, Space* parent, const EffectEditor& other)
    : EffectEditor(vm, parent) {m_state = other.m_state;}

    Space* copy(Space* parent) const
    {
        return new EffectEditor(m_vm, parent, *this);
    }
};

}

#endif // URDE_EFFECT_EDITOR_HPP
