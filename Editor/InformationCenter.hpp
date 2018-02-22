#ifndef INFORMATIONCENTER_HPP
#define INFORMATIONCENTER_HPP

#include "Space.hpp"
#include "ViewManager.hpp"

namespace urde
{
class InformationCenter : public ViewerSpace
{
    struct State : Space::State
    {
        AT_DECL_DNA_YAML
        AT_DECL_DNAV
        Value<bool> showLog;
    } m_state;

    const Space::State& spaceState() const { return m_state; }

    struct View : specter::View
    {
        InformationCenter& m_ic;
        std::vector<hecl::SystemString> m_log;

        View(InformationCenter& ic, specter::ViewResources& res)
        : specter::View(res, ic.m_vm.rootView()), m_ic(ic) {}
    };

    std::unique_ptr<View> m_view;

public:
    InformationCenter(ViewManager& vm, Space* parent)
        : ViewerSpace(vm, Class::InformationCenter, parent)
    {
        reloadState();
    }

    InformationCenter(ViewManager& vm, Space* parent, const InformationCenter& other)
        : InformationCenter(vm, parent)
    {
        m_state = other.m_state;
        reloadState();
    }

    InformationCenter(ViewManager& vm, Space* parent, ConfigReader& r)
        : InformationCenter(vm, parent)
    {
        m_state.read(r);
        reloadState();
    }

    void reloadState()
    {
    }

    virtual specter::View* buildContentView(specter::ViewResources& res)
    {
        m_view.reset(new View(*this, res));
        return m_view.get();
    }

    Space* copy(Space *parent) const
    {
        return new InformationCenter(m_vm, parent, *this);
    }

    bool usesToolbar() const { return true; }
};
}

#endif // INFORMATIONCENTER_HPP
