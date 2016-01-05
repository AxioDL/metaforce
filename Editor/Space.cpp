#include "Space.hpp"
#include "ViewManager.hpp"
#include "ResourceOutliner.hpp"

namespace RUDE
{
static LogVisor::LogModule Log("RUDE::Space");

Specter::View* Space::buildSpaceView(Specter::ViewResources& res)
{
    m_space.reset(new Specter::Space(res, m_vm.rootView(), Specter::Toolbar::Position::Bottom));
    Specter::View* sview = buildContentView(res);
    m_space->setContentView(sview);
    if (usesToolbar())
        buildToolbarView(res, m_space->toolbar());
    return m_space.get();
}

Specter::View* SplitSpace::buildContentView(Specter::ViewResources& res)
{
    m_splitView.reset(new Specter::SplitView(res, m_vm.rootView(), Specter::SplitView::Axis::Horizontal));
    if (m_slots[0])
        m_splitView->setContentView(0, m_slots[0]->buildSpaceView(res));
    if (m_slots[1])
        m_splitView->setContentView(1, m_slots[1]->buildSpaceView(res));
    return m_splitView.get();
}

void SplitSpace::setSpaceSlot(unsigned slot, std::unique_ptr<Space>&& space)
{
    if (slot > 1)
        Log.report(LogVisor::FatalError, "invalid slot %u for SplitView", slot);
    m_slots[slot] = std::move(space);
}

template <class Reader>
static Space* BuildNewSpace(ViewManager& vm, Space::Class cls, Reader& r)
{
    using Class = Space::Class;
    switch (cls)
    {
    case Class::SplitSpace:
        return new SplitSpace(vm, r);
    case Class::ResourceOutliner:
        return new ResourceOutliner(vm, r);
    default: break;
    }
    return nullptr;
}

Space* Space::NewSpaceFromConfigStream(ViewManager& vm, ConfigReader& r)
{
#ifdef RUDE_BINARY_CONFIGS
    Class cls = Class(r.readUint32Big());
    return BuildNewSpace(vm, cls, r);
#else
    Class cls = Class(r.readUint32("class"));
    return BuildNewSpace(vm, cls, r);
#endif
}

}
