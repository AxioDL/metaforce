#include "Space.hpp"
#include "ViewManager.hpp"
#include "ResourceOutliner.hpp"

namespace RUDE
{

Specter::Space* Space::buildSpaceView(Specter::ViewResources& res)
{
    m_space.reset(new Specter::Space(res, m_vm.rootView(), Specter::Toolbar::Position::Bottom));
    return m_space.get();
}

Specter::View* SplitSpace::buildContentView(Specter::ViewResources& res)
{
    m_splitView.reset(new Specter::SplitView(res, m_vm.rootView(), Specter::SplitView::Axis::Horizontal));
    return m_splitView.get();
}

Space* Space::NewSpaceFromYAMLStream(ViewManager& vm, Athena::io::YAMLDocReader& r)
{
    StateHead head;
    head.read(r);
    switch (head.cls)
    {
    case Class::SplitSpace:
        return new SplitSpace(vm, r);
    case Class::ResourceOutliner:
        return new ResourceOutliner(vm, r);
    default: break;
    }
    return nullptr;
}

}
