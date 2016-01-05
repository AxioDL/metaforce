#ifndef RUDE_RESOURCE_OUTLINER_HPP
#define RUDE_RESOURCE_OUTLINER_HPP

#include "Space.hpp"
#include "ViewManager.hpp"

namespace RUDE
{

class ResourceOutliner : public Space
{
    struct State : Space::State
    {
        DECL_YAML
    } m_state;
    const Space::State& spaceState() const {return m_state;}

    struct View : public Specter::View
    {
        ResourceOutliner& m_ro;
        View(ResourceOutliner& ro, Specter::ViewResources& res)
        : Specter::View(res, ro.m_vm.rootView()), m_ro(ro)
        {
            commitResources(res);
            setBackground(Zeus::CColor::skBlue);
        }
    };
    std::unique_ptr<View> m_view;

public:
    ResourceOutliner(ViewManager& vm) : Space(vm, Class::ResourceOutliner) {}
    ResourceOutliner(ViewManager& vm, ConfigReader& r)
    : ResourceOutliner(vm)
    {
        m_state.read(r);
    }

    Specter::View* buildContentView(Specter::ViewResources& res)
    {
        m_view.reset(new View(*this, res));
        return m_view.get();
    }
};

}

#endif // RUDE_RESOURCE_OUTLINER_HPP
