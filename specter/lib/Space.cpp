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
    setBackground(res.themeData().spaceBackground());
    if (tbPos != Toolbar::Position::None)
        m_toolbar.reset(new Toolbar(res, *this, tbPos));
}

View* Space::setContentView(View* view)
{
    View* ret = m_contentView;
    m_contentView = view;
    updateSize();
    return ret;
}

void Space::mouseDown(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod)
{
    if (m_contentView && !m_contentMouseDown &&
        m_contentView->subRect().coordInRect(coord))
    {
        m_contentView->mouseDown(coord, button, mod);
        m_contentMouseDown = true;
    }
    if (m_toolbar && !m_toolbarMouseDown &&
        m_toolbar->subRect().coordInRect(coord))
    {
        m_toolbar->mouseDown(coord, button, mod);
        m_toolbarMouseDown = true;
    }
}

void Space::mouseUp(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod)
{
    if (m_contentView && m_contentMouseDown)
    {
        m_contentView->mouseUp(coord, button, mod);
        m_contentMouseDown = false;
    }
    if (m_toolbar && m_toolbarMouseDown)
    {
        m_toolbar->mouseUp(coord, button, mod);
        m_toolbarMouseDown = false;
    }
}

void Space::mouseMove(const boo::SWindowCoord& coord)
{
    if (m_contentView)
    {
        if (m_contentView->subRect().coordInRect(coord))
        {
            if (!m_contentMouseIn)
            {
                m_contentView->mouseEnter(coord);
                m_contentMouseIn = true;
            }
            m_contentView->mouseMove(coord);
        }
        else
        {
            if (m_contentMouseIn)
            {
                m_contentView->mouseLeave(coord);
                m_contentMouseIn = false;
            }
        }
    }
    if (m_toolbar)
    {
        if (m_toolbar->subRect().coordInRect(coord))
        {
            if (!m_toolbarMouseIn)
            {
                m_toolbar->mouseEnter(coord);
                m_toolbarMouseIn = true;
            }
            m_toolbar->mouseMove(coord);
        }
        else
        {
            if (m_toolbarMouseIn)
            {
                m_toolbar->mouseLeave(coord);
                m_toolbarMouseIn = false;
            }
        }
    }
}

void Space::mouseEnter(const boo::SWindowCoord& coord)
{
}

void Space::mouseLeave(const boo::SWindowCoord& coord)
{
    if (m_contentView && m_contentMouseIn)
    {
        m_contentView->mouseLeave(coord);
        m_contentMouseIn = false;
    }
    if (m_toolbar && m_toolbarMouseIn)
    {
        m_toolbar->mouseLeave(coord);
        m_toolbarMouseIn = false;
    }
}

void Space::resized(const boo::SWindowRect& root, const boo::SWindowRect& sub)
{
    View::resized(root, sub);

    boo::SWindowRect tbRect = sub;
    if (m_toolbar)
    {
        tbRect.size[1] = m_toolbar->nominalHeight();
        if (m_tbPos == Toolbar::Position::Top)
            tbRect.location[1] += sub.size[1] - tbRect.size[1];
        m_toolbar->resized(root, tbRect);
    }
    else
        tbRect.size[1] = 0;

    if (m_contentView)
    {
        boo::SWindowRect contentRect = sub;
        if (m_tbPos == Toolbar::Position::Bottom)
            contentRect.location[1] += tbRect.size[1];
        contentRect.size[1] = sub.size[1] - tbRect.size[1];
        contentRect.size[1] = std::max(contentRect.size[1], 0);
        m_contentView->resized(root, contentRect);
    }
}

void Space::draw(boo::IGraphicsCommandQueue* gfxQ)
{
    View::draw(gfxQ);
    if (m_contentView)
        m_contentView->draw(gfxQ);
    if (m_toolbar)
        m_toolbar->draw(gfxQ);
}

}

