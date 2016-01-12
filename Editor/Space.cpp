#include "Space.hpp"
#include "ViewManager.hpp"
#include "ResourceBrowser.hpp"

namespace URDE
{
static LogVisor::LogModule Log("URDE::Space");

Specter::View* Space::buildSpaceView(Specter::ViewResources& res)
{
    if (usesToolbar())
    {
        m_spaceView.reset(new Specter::Space(res, m_vm.rootView(), *this, Specter::Toolbar::Position::Bottom));
        Specter::View* sview = buildContentView(res);
        m_spaceView->setContentView(sview);
        buildToolbarView(res, *m_spaceView->toolbar());
        return m_spaceView.get();
    }
    else
    {
        m_spaceView.reset(new Specter::Space(res, m_vm.rootView(), *this, Specter::Toolbar::Position::None));
        Specter::View* sview = buildContentView(res);
        m_spaceView->setContentView(sview);
        return m_spaceView.get();
    }
}

Specter::View* RootSpace::buildSpaceView(Specter::ViewResources& res)
{
    Specter::View* newRoot = buildContentView(res);
    m_vm.RootSpaceViewBuilt(newRoot);
    return newRoot;
}

Specter::View* SplitSpace::buildContentView(Specter::ViewResources& res)
{
    int clearance = res.pixelFactor() * SPECTER_TOOLBAR_GAUGE;
    m_splitView.reset(new Specter::SplitView(res, m_vm.rootView(), this, m_state.axis,
                                             m_state.split, clearance, clearance));
    if (m_slots[0])
        m_splitView->setContentView(0, m_slots[0]->buildSpaceView(res));
    if (m_slots[1])
        m_splitView->setContentView(1, m_slots[1]->buildSpaceView(res));
    return m_splitView.get();
}

void SplitSpace::setChildSlot(unsigned slot, std::unique_ptr<Space>&& space)
{
    if (slot > 1)
        Log.report(LogVisor::FatalError, "invalid slot %u for SplitView", slot);
    m_slots[slot] = std::move(space);
    m_slots[slot]->m_parent = this;
}

Specter::ISplitSpaceController* Space::spaceSplit(Specter::SplitView::Axis axis, int thisSlot)
{
    if (m_parent)
    {
        SplitSpace* ss = new SplitSpace(m_vm, m_parent, axis);
        ss->setChildSlot(thisSlot, std::move(m_parent->exchangeSpaceSplitJoin(this, std::unique_ptr<Space>(ss))));
        ss->setChildSlot(thisSlot ^ 1, std::unique_ptr<Space>(copy(ss)));
        m_vm.BuildSpaceViews();
        return ss;
    }
    return nullptr;
}

std::unique_ptr<Space> RootSpace::exchangeSpaceSplitJoin(Space* removeSpace, std::unique_ptr<Space>&& keepSpace)
{
    std::unique_ptr<Space> ret = std::move(keepSpace);

    if (removeSpace == m_spaceTree.get())
    {
        m_spaceTree.swap(ret);
        m_spaceTree->m_parent = this;
    }
    else
        Log.report(LogVisor::FatalError, "RootSpace::exchangeSpaceSplitJoin() failure");

    return ret;
}

std::unique_ptr<Space> SplitSpace::exchangeSpaceSplitJoin(Space* removeSpace, std::unique_ptr<Space>&& keepSpace)
{
    std::unique_ptr<Space> ret = std::move(keepSpace);

    if (removeSpace == m_slots[0].get())
    {
        m_slots[0].swap(ret);
        m_slots[0]->m_parent = this;
    }
    else if (removeSpace == m_slots[1].get())
    {
        m_slots[1].swap(ret);
        m_slots[1]->m_parent = this;
    }
    else
        Log.report(LogVisor::FatalError, "SplitSpace::exchangeSpaceSplitJoin() failure");

    return ret;
}

template <class Reader>
static Space* BuildNewSpace(ViewManager& vm, Space::Class cls, Space* parent, Reader& r)
{
    using Class = Space::Class;
    switch (cls)
    {
    case Class::SplitSpace:
        return new SplitSpace(vm, parent, r);
    case Class::ResourceBrowser:
        return new ResourceBrowser(vm, parent, r);
    default: break;
    }
    return nullptr;
}

void Space::saveState(Athena::io::IStreamWriter& w) const
{
    w.writeUint32Big(atUint32(m_class));
    spaceState().write(w);
}

void Space::saveState(Athena::io::YAMLDocWriter& w) const
{
    w.writeUint32("class", atUint32(m_class));
    spaceState().write(w);
}

Space* Space::NewSpaceFromConfigStream(ViewManager& vm, Space* parent, ConfigReader& r)
{
#ifdef URDE_BINARY_CONFIGS
    Class cls = Class(r.readUint32Big());
    return BuildNewSpace(vm, cls, parent, r);
#else
    Class cls = Class(r.readUint32("class"));
    return BuildNewSpace(vm, cls, parent, r);
#endif
}

RootSpace* Space::NewRootSpaceFromConfigStream(ViewManager& vm, ConfigReader& r)
{
#ifdef URDE_BINARY_CONFIGS
    Class cls = Class(r.readUint32Big());
    if (cls != Class::RootSpace)
        return nullptr;
    return BuildNewSpace(vm, cls, r);
#else
    Class cls = Class(r.readUint32("class"));
    if (cls != Class::RootSpace)
        return nullptr;
    return new RootSpace(vm, r);
#endif
}

}
