#ifndef RUDE_RESOURCE_OUTLINER_HPP
#define RUDE_RESOURCE_OUTLINER_HPP

#include "Space.hpp"

namespace RUDE
{

class ResourceOutliner : public Space
{
    struct State : Space::State
    {
        DECL_YAML
    } m_state;
    Space::State& spaceState() {return m_state;}

public:
    ResourceOutliner(ViewManager& vm) : Space(vm, Class::ResourceOutliner) {}
    ResourceOutliner(ViewManager& vm, Athena::io::YAMLDocReader& r)
    : ResourceOutliner(vm)
    {
        m_state.read(r);
    }

    Specter::View* buildContentView(Specter::ViewResources& res) {return nullptr;}
};

}

#endif // RUDE_RESOURCE_OUTLINER_HPP
