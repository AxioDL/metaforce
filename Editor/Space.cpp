#include "Space.hpp"
#include "ViewManager.hpp"

namespace RUDE
{

Specter::Space* Space::buildSpace(Specter::ViewResources& res)
{
    m_space.reset(new Specter::Space(res, m_vm.rootView(), Specter::Toolbar::Position::Bottom));
    return m_space.get();
}

Specter::View* SplitSpace::buildContent(Specter::ViewResources& res)
{
    m_splitView.reset(new Specter::SplitView(res, m_vm.rootView(), Specter::SplitView::Axis::Horizontal));
    return m_splitView.get();
}

}
