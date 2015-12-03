#include <LogVisor/LogVisor.hpp>
#include "Specter/Space.hpp"
#include "Specter/ViewResources.hpp"

namespace Specter
{
static LogVisor::LogModule Log("Specter::Space");

Space::Space(ViewResources& res, View& parentView, Toolbar::Position tbPos)
: View(res, parentView), m_tbPos(tbPos)
{
    commitResources(res);
    m_toolbar.reset(new Toolbar(res, *this, tbPos));
}

std::unique_ptr<View> Space::setContentView(std::unique_ptr<View>&& view)
{
    std::unique_ptr<View> ret;
    m_contentView.swap(ret);
    m_contentView = std::move(view);
    updateSize();
    return ret;
}

void Space::mouseDown(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod)
{
}

void Space::mouseUp(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod)
{
}

void Space::mouseMove(const boo::SWindowCoord& coord)
{
}

void Space::resetResources(ViewResources& res)
{
    if (m_contentView)
        m_contentView->resetResources(res);
}

void Space::resized(const boo::SWindowRect& root, const boo::SWindowRect& sub)
{
    View::resized(root, sub);
}

void Space::draw(boo::IGraphicsCommandQueue* gfxQ)
{
    View::draw(gfxQ);
}

}

