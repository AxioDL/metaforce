#include "Specter/RootView.hpp"

namespace Specter
{

RootView::RootView(ViewSystem& system, boo::IWindow* window)
: View(system), m_window(window)
{

}

void RootView::resized(const boo::SWindowRect& rect)
{
}

void RootView::mouseDown(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mods)
{
}

void RootView::mouseUp(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mods)
{
}

void RootView::mouseMove(const boo::SWindowCoord& coord)
{
}

void RootView::mouseEnter(const boo::SWindowCoord& coord)
{
}

void RootView::mouseLeave(const boo::SWindowCoord& coord)
{
}

void RootView::scroll(const boo::SWindowCoord& coord, const boo::SScrollDelta& scroll)
{
}

void RootView::touchDown(const boo::STouchCoord& coord, uintptr_t tid)
{
}

void RootView::touchUp(const boo::STouchCoord& coord, uintptr_t tid)
{
}

void RootView::touchMove(const boo::STouchCoord& coord, uintptr_t tid)
{
}

void RootView::charKeyDown(unsigned long charCode, boo::EModifierKey mods, bool isRepeat)
{
}

void RootView::charKeyUp(unsigned long charCode, boo::EModifierKey mods)
{
}

void RootView::specialKeyDown(boo::ESpecialKey key, boo::EModifierKey mods, bool isRepeat)
{
}

void RootView::specialKeyUp(boo::ESpecialKey key, boo::EModifierKey mods)
{
}

void RootView::modKeyDown(boo::EModifierKey mod, bool isRepeat)
{
}

void RootView::modKeyUp(boo::EModifierKey mod)
{
}

void RootView::draw(boo::IGraphicsCommandQueue* gfxQ)
{
    View::draw(gfxQ);
}

}
