#include "specter/RootView.hpp"
#include "specter/ViewResources.hpp"
#include "specter/Space.hpp"
#include "specter/Menu.hpp"

namespace specter
{
static logvisor::Module Log("specter::RootView");

RootView::RootView(IViewManager& viewMan, ViewResources& res, boo::IWindow* window)
: View(res), m_window(window), m_viewMan(viewMan), m_viewRes(&res), m_events(*this)
{
    window->setCallback(&m_events);
    boo::SWindowRect rect = window->getWindowFrame();
    commitResources(res, [&](boo::IGraphicsDataFactory::Context& ctx) -> bool
    {
        buildResources(ctx, res);
        m_splitMenuSystem.emplace(*this, ctx);
        m_renderTex = ctx.newRenderTexture(rect.size[0], rect.size[1], boo::TextureClampMode::Repeat, 1, 3);
        return true;
    });
    resized(rect, rect);
}

RootView::SplitMenuSystem::SplitMenuSystem(RootView& rv, boo::IGraphicsDataFactory::Context& ctx)
: m_rv(rv), m_text(rv.m_viewMan.translateOr("boundary_action", "Boundary Action")),
  m_splitActionNode(*this), m_joinActionNode(*this)
{
    ViewResources& res = *rv.m_viewRes;
    m_viewVertBlockBuf = res.m_viewRes.m_bufPool.allocateBlock(res.m_factory);
    m_vertsBinding.init(ctx, res, 32, m_viewVertBlockBuf);

    zeus::CColor col = {0.0,0.0,0.0,0.5};
    for (int i=0 ; i<32 ; ++i)
        m_verts[i].m_color = col;

    m_verts[0].m_pos.assign(0.0, 0.25, 0);
    m_verts[1].m_pos.assign(0.0, 0.0, 0);
    m_verts[2].m_pos.assign(0.375, 0.25, 0);
    m_verts[3].m_pos.assign(0.375, 0.0, 0);
    m_verts[4].m_pos = m_verts[3].m_pos;

    m_verts[5].m_pos.assign(0.0, 1.0, 0);
    m_verts[6].m_pos = m_verts[5].m_pos;
    m_verts[7].m_pos = m_verts[0].m_pos;
    m_verts[8].m_pos.assign(0.5, 1.0, 0);
    m_verts[9].m_pos.assign(0.25, 0.25, 0);
    m_verts[10].m_pos.assign(0.5, 0.5, 0);
    m_verts[11].m_pos = m_verts[10].m_pos;

    m_verts[12].m_pos = m_verts[8].m_pos;
    m_verts[13].m_pos = m_verts[12].m_pos;
    m_verts[14].m_pos = m_verts[11].m_pos;
    m_verts[15].m_pos.assign(1.0, 1.0, 0);
    m_verts[16].m_pos.assign(1.0, 0.25, 0);
    m_verts[17].m_pos = m_verts[16].m_pos;

    m_verts[18].m_pos = m_verts[14].m_pos;
    m_verts[19].m_pos = m_verts[18].m_pos;
    m_verts[20].m_pos.assign(0.75, 0.25, 0);
    m_verts[21].m_pos = m_verts[17].m_pos;
    m_verts[22].m_pos = m_verts[21].m_pos;

    m_verts[23].m_pos.assign(0.625, 0.25, 0);
    m_verts[24].m_pos = m_verts[23].m_pos;
    m_verts[25].m_pos.assign(0.625, 0.0, 0);
    m_verts[26].m_pos = m_verts[22].m_pos;
    m_verts[27].m_pos.assign(1.0, 0.0, 0);

    m_verts[28].m_pos.assign(-1.0, 1.0, 0);
    m_verts[29].m_pos.assign(-1.0, -1.0, 0);
    m_verts[30].m_pos.assign(1.0, 1.0, 0);
    m_verts[31].m_pos.assign(1.0, -1.0, 0);

    m_vertsBinding.load<decltype(m_verts)>(m_verts);
}

RootView::SplitMenuSystem::SplitActionNode::SplitActionNode(SplitMenuSystem& smn)
: m_smn(smn), m_text(smn.m_rv.m_viewMan.translateOr("split", "Split")) {}

RootView::SplitMenuSystem::JoinActionNode::JoinActionNode(SplitMenuSystem& smn)
: m_smn(smn), m_text(smn.m_rv.m_viewMan.translateOr("join", "Join")) {}

void RootView::SplitMenuSystem::setArrowVerts(const boo::SWindowRect& rect, SplitView::ArrowDir dir)
{
    const boo::SWindowRect& root = m_rv.subRect();
    if (dir == SplitView::ArrowDir::Left || dir == SplitView::ArrowDir::Right)
    {
        m_viewBlock.m_mv[0][1] = 2.0f * rect.size[1] / float(root.size[1]);
        m_viewBlock.m_mv[0][0] = 0.0f;
        m_viewBlock.m_mv[1][0] = 2.0f * (dir == SplitView::ArrowDir::Left ? -rect.size[0] : rect.size[0]) /
                                 float(root.size[0]);
        m_viewBlock.m_mv[1][1] = 0.0f;
        m_viewBlock.m_mv[3][0] = 2.0f * (rect.location[0] + (dir == SplitView::ArrowDir::Left ? rect.size[0] : 0)) /
                                 float(root.size[0]) - 1.0f;
        m_viewBlock.m_mv[3][1] = 2.0f * rect.location[1] / float(root.size[1]) - 1.0f;
    }
    else
    {
        m_viewBlock.m_mv[0][0] = 2.0f * rect.size[0] / float(root.size[0]);
        m_viewBlock.m_mv[0][1] = 0.0f;
        m_viewBlock.m_mv[1][1] = 2.0f * (dir == SplitView::ArrowDir::Down ? -rect.size[1] : rect.size[1]) /
                                 float(root.size[1]);
        m_viewBlock.m_mv[1][0] = 0.0f;
        m_viewBlock.m_mv[3][0] = 2.0f * rect.location[0] / float(root.size[0]) - 1.0f;
        m_viewBlock.m_mv[3][1] = 2.0f * (rect.location[1] + (dir == SplitView::ArrowDir::Down ? rect.size[1] : 0)) /
                                 float(root.size[1]) - 1.0f;
    }
    m_viewVertBlockBuf.access() = m_viewBlock;
}

void RootView::SplitMenuSystem::setLineVerts(const boo::SWindowRect& rect, float split, SplitView::Axis axis)
{
    const boo::SWindowRect& root = m_rv.subRect();
    if (axis == SplitView::Axis::Horizontal)
    {
        m_viewBlock.m_mv[0][0] = rect.size[0] / float(root.size[0]);
        m_viewBlock.m_mv[0][1] = 0.0f;
        m_viewBlock.m_mv[1][1] = 2.0f / float(root.size[1]);
        m_viewBlock.m_mv[1][0] = 0.0f;
        m_viewBlock.m_mv[3][0] = 2.0f * (rect.location[0] + rect.size[0] / 2.0f) / float(root.size[0]) - 1.0f;
        m_viewBlock.m_mv[3][1] = (rect.location[1] + split * rect.size[1]) * m_viewBlock.m_mv[1][1] - 1.0f;
    }
    else
    {
        m_viewBlock.m_mv[0][0] = 2.0f / float(root.size[0]);
        m_viewBlock.m_mv[0][1] = 0.0f;
        m_viewBlock.m_mv[1][1] = rect.size[1] / float(root.size[1]);
        m_viewBlock.m_mv[1][0] = 0.0f;
        m_viewBlock.m_mv[3][0] = (rect.location[0] + split * rect.size[0]) * m_viewBlock.m_mv[0][0] - 1.0f;
        m_viewBlock.m_mv[3][1] = 2.0f * (rect.location[1] + rect.size[1] / 2.0f) / float(root.size[1]) - 1.0f;
    }
    m_viewVertBlockBuf.access() = m_viewBlock;
}

void RootView::destroyed()
{
    m_destroyed = true;
}

void RootView::resized(const boo::SWindowRect& root, const boo::SWindowRect&)
{
    m_rootRect = root;
    m_rootRect.location[0] = 0;
    m_rootRect.location[1] = 0;
    View::resized(m_rootRect, m_rootRect);
    for (View* v : m_views)
        v->resized(m_rootRect, m_rootRect);
    if (m_tooltip)
        m_tooltip->resized(m_rootRect, m_rootRect);
    if (m_rightClickMenu.m_view)
    {
         float wr = root.size[0] / float(m_rightClickMenuRootAndLoc.size[0]);
         float hr = root.size[1] / float(m_rightClickMenuRootAndLoc.size[1]);
         m_rightClickMenuRootAndLoc.size[0] = root.size[0];
         m_rightClickMenuRootAndLoc.size[1] = root.size[1];
         m_rightClickMenuRootAndLoc.location[0] *= wr;
         m_rightClickMenuRootAndLoc.location[1] *= hr;
         m_rightClickMenu.m_view->resized(root, m_rightClickMenuRootAndLoc);
    }
    m_splitMenuSystem->resized();
    m_resizeRTDirty = true;
}

void RootView::SplitMenuSystem::resized()
{
    if (m_phase == Phase::InteractiveJoin)
    {
        boo::SWindowRect rect;
        SplitView::ArrowDir arrow;
        m_splitView->getJoinArrowHover(m_interactiveSlot, rect, arrow);
        setArrowVerts(rect, arrow);
    }
    else if (m_phase == Phase::InteractiveSplit)
    {
        boo::SWindowRect rect;
        SplitView::Axis axis;
        m_splitView->getSplitLineHover(m_interactiveSlot, rect, axis);
        setLineVerts(rect, m_interactiveSplit, axis);
    }
}

void RootView::mouseDown(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mods)
{
    if (m_splitMenuSystem->m_phase != SplitMenuSystem::Phase::Inactive)
    {
        m_splitMenuSystem->mouseDown(coord, button, mods);
        return;
    }

    if (m_rightClickMenu.m_view)
    {
        if (!m_rightClickMenu.mouseDown(coord, button, mods))
            m_rightClickMenu.m_view.reset();
        return;
    }

    if (m_activeMenuButton)
    {
        ViewChild<std::unique_ptr<View>>& mv = m_activeMenuButton->getMenu();
        if (!mv.mouseDown(coord, button, mods))
            m_activeMenuButton->closeMenu(coord);
        return;
    }

    if (m_hoverSplitDragView)
    {
        if (button == boo::EMouseButton::Primary)
        {
            m_activeSplitDragView = true;
            m_hoverSplitDragView->startDragSplit(coord);
        }
        else if (button == boo::EMouseButton::Secondary)
        {
            m_splitMenuSystem->m_splitView = m_hoverSplitDragView;
            adoptRightClickMenu(std::make_unique<specter::Menu>(*m_viewRes, *this, &*m_splitMenuSystem), coord);
        }
        return;
    }

    if (m_activeTextView && !m_activeTextView->subRect().coordInRect(coord))
        setActiveTextView(nullptr);
    for (View* v : m_views)
        v->mouseDown(coord, button, mods);
}

void RootView::SplitMenuSystem::mouseDown(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mods)
{
    if (m_phase == Phase::InteractiveJoin)
    {
        int origDummy;
        SplitView* selSplit;
        boo::SWindowRect rect;
        SplitView::ArrowDir arrow;
        if (m_splitView->testJoinArrowHover(coord, origDummy, selSplit, m_interactiveSlot, rect, arrow))
        {
            setArrowVerts(rect, arrow);
            m_interactiveDown = true;
        }
    }
    else if (m_phase == Phase::InteractiveSplit)
    {
        boo::SWindowRect rect;
        SplitView::Axis axis;
        if (m_splitView->testSplitLineHover(coord, m_interactiveSlot, rect, m_interactiveSplit, axis))
        {
            setLineVerts(rect, m_interactiveSplit, axis);
            m_interactiveDown = true;
        }
    }
}

void RootView::mouseUp(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mods)
{
    if (m_splitMenuSystem->m_phase != SplitMenuSystem::Phase::Inactive)
    {
        m_splitMenuSystem->mouseUp(coord, button, mods);
        return;
    }

    if (m_rightClickMenu.m_view)
    {
        m_rightClickMenu.mouseUp(coord, button, mods);
        return;
    }

    if (m_activeMenuButton)
    {
        ViewChild<std::unique_ptr<View>>& mv = m_activeMenuButton->getMenu();
        mv.mouseUp(coord, button, mods);
        return;
    }

    if (m_activeSplitDragView && button == boo::EMouseButton::Primary)
    {
        m_activeSplitDragView = false;
        m_hoverSplitDragView->endDragSplit();
        m_spaceCornerHover = false;
        m_hSplitHover = false;
        m_vSplitHover = false;
        _updateCursor();
    }

    for (View* v : m_views)
        v->mouseUp(coord, button, mods);
}

void RootView::SplitMenuSystem::mouseUp(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mods)
{
    if (m_phase == Phase::InteractiveJoin)
    {
        int origSlot;
        SplitView* selSplit;
        boo::SWindowRect rect;
        SplitView::ArrowDir arrow;
        if (m_splitView->testJoinArrowHover(coord, origSlot, selSplit, m_interactiveSlot, rect, arrow))
        {
            setArrowVerts(rect, arrow);
            if (m_interactiveDown)
            {
                m_interactiveDown = false;
                m_phase = Phase::Inactive;
                m_splitView->m_controller->joinViews(m_splitView, origSlot, selSplit, m_interactiveSlot);
            }
        }
    }
    else if (m_phase == Phase::InteractiveSplit)
    {
        boo::SWindowRect rect;
        SplitView::Axis axis;
        if (m_splitView->testSplitLineHover(coord, m_interactiveSlot, rect, m_interactiveSplit, axis))
        {
            setLineVerts(rect, m_interactiveSplit, axis);
            if (m_interactiveDown)
            {
                m_interactiveDown = false;
                m_phase = Phase::Inactive;
                Space* space = dynamic_cast<Space*>(m_splitView->m_views[m_interactiveSlot].m_view);
                if (space && space->m_controller.spaceSplitAllowed())
                {
                    ISplitSpaceController* ss = space->m_controller.spaceSplit(axis, 0);
                    ss->splitView()->setSplit(m_interactiveSplit);
                }
            }
        }
    }
}

SplitView* RootView::recursiveTestSplitHover(SplitView* sv, const boo::SWindowCoord& coord) const
{
    if (sv->testSplitHover(coord))
        return sv;
    for (int i=0 ; i<2 ; ++i)
    {
        SplitView* child = dynamic_cast<SplitView*>(sv->m_views[i].m_view);
        if (child)
        {
            SplitView* res = recursiveTestSplitHover(child, coord);
            if (res)
                return res;
        }
    }
    return nullptr;
}

void RootView::mouseMove(const boo::SWindowCoord& coord)
{
    if (m_splitMenuSystem->m_phase != SplitMenuSystem::Phase::Inactive)
    {
        m_splitMenuSystem->mouseMove(coord);
        return;
    }

    if (m_rightClickMenu.m_view)
    {
        m_hSplitHover = false;
        m_vSplitHover = false;
        _updateCursor();
        m_rightClickMenu.mouseMove(coord);
        return;
    }

    if (m_activeMenuButton)
    {
        m_hSplitHover = false;
        m_vSplitHover = false;
        _updateCursor();
        ViewChild<std::unique_ptr<View>>& mv = m_activeMenuButton->getMenu();
        mv.mouseMove(coord);
        return;
    }

    if (m_activeSplitDragView)
    {
        m_hoverSplitDragView->moveDragSplit(coord);
        m_spaceCornerHover = false;
        if (m_hoverSplitDragView->axis() == SplitView::Axis::Horizontal)
            setHorizontalSplitHover(true);
        else
            setVerticalSplitHover(true);
        return;
    }

    m_hoverSplitDragView = nullptr;
    if (!m_spaceCornerHover)
    {
        for (View* v : m_views)
        {
            SplitView* sv = dynamic_cast<SplitView*>(v);
            if (sv)
                sv = recursiveTestSplitHover(sv, coord);
            if (sv)
            {
                m_hoverSplitDragView = sv;
                break;
            }
            else
            {
                m_hSplitHover = false;
                m_vSplitHover = false;
                _updateCursor();
            }
        }
    }

    if (m_activeDragView)
        m_activeDragView->mouseMove(coord);
    else
    {
        for (View* v : m_views)
            v->mouseMove(coord);
    }

    if (m_hoverSplitDragView)
    {
        if (m_hoverSplitDragView->axis() == SplitView::Axis::Horizontal)
            setHorizontalSplitHover(true);
        else
            setVerticalSplitHover(true);
    }

    boo::SWindowRect ttrect = m_rootRect;
    ttrect.location[0] = coord.pixel[0];
    ttrect.location[1] = coord.pixel[1];
    if (m_tooltip)
    {
        if (coord.pixel[0] + m_tooltip->nominalWidth() > m_rootRect.size[0])
            ttrect.location[0] -= m_tooltip->nominalWidth();
        if (coord.pixel[1] + m_tooltip->nominalHeight() > m_rootRect.size[1])
            ttrect.location[1] -= m_tooltip->nominalHeight();
        m_tooltip->resized(m_rootRect, ttrect);
    }
}

void RootView::SplitMenuSystem::mouseMove(const boo::SWindowCoord& coord)
{
    if (m_phase == Phase::InteractiveJoin)
    {
        int origDummy;
        SplitView* selSplit;
        boo::SWindowRect rect;
        SplitView::ArrowDir arrow;
        if (m_splitView->testJoinArrowHover(coord, origDummy, selSplit, m_interactiveSlot, rect, arrow))
            setArrowVerts(rect, arrow);
    }
    else if (m_phase == Phase::InteractiveSplit)
    {
        boo::SWindowRect rect;
        SplitView::Axis axis;
        if (m_splitView->testSplitLineHover(coord, m_interactiveSlot, rect, m_interactiveSplit, axis))
            setLineVerts(rect, m_interactiveSplit, axis);
    }
}

void RootView::mouseEnter(const boo::SWindowCoord& coord)
{
    for (View* v : m_views)
        v->mouseEnter(coord);
}

void RootView::mouseLeave(const boo::SWindowCoord& coord)
{
    if (m_splitMenuSystem->m_phase != SplitMenuSystem::Phase::Inactive)
    {
        m_splitMenuSystem->mouseLeave(coord);
        return;
    }

    if (m_rightClickMenu.m_view)
    {
        m_rightClickMenu.mouseLeave(coord);
        return;
    }

    if (m_activeMenuButton)
    {
        ViewChild<std::unique_ptr<View>>& mv = m_activeMenuButton->getMenu();
        mv.mouseLeave(coord);
        return;
    }

    for (View* v : m_views)
        v->mouseLeave(coord);
}

void RootView::SplitMenuSystem::mouseLeave(const boo::SWindowCoord& coord)
{

}

void RootView::scroll(const boo::SWindowCoord& coord, const boo::SScrollDelta& scroll)
{
    if (m_activeMenuButton)
    {
        ViewChild<std::unique_ptr<View>>& mv = m_activeMenuButton->getMenu();
        mv.scroll(coord, scroll);
        return;
    }

    for (View* v : m_views)
        v->scroll(coord, scroll);
}

void RootView::touchDown(const boo::STouchCoord& coord, uintptr_t tid)
{
    for (View* v : m_views)
        v->touchDown(coord, tid);
}

void RootView::touchUp(const boo::STouchCoord& coord, uintptr_t tid)
{
    for (View* v : m_views)
        v->touchUp(coord, tid);
}

void RootView::touchMove(const boo::STouchCoord& coord, uintptr_t tid)
{
    for (View* v : m_views)
        v->touchMove(coord, tid);
}

void RootView::charKeyDown(unsigned long charCode, boo::EModifierKey mods, bool isRepeat)
{
    for (View* v : m_views)
        v->charKeyDown(charCode, mods, isRepeat);
    if (m_activeTextView &&
        (mods & (boo::EModifierKey::Ctrl|boo::EModifierKey::Command)) != boo::EModifierKey::None)
    {
        if (charCode == 'c' || charCode == 'C')
            m_activeTextView->clipboardCopy();
        else if (charCode == 'x' || charCode == 'X')
            m_activeTextView->clipboardCut();
        else if (charCode == 'v' || charCode == 'V')
            m_activeTextView->clipboardPaste();
    }
}

void RootView::charKeyUp(unsigned long charCode, boo::EModifierKey mods)
{
    for (View* v : m_views)
        v->charKeyUp(charCode, mods);
}

void RootView::specialKeyDown(boo::ESpecialKey key, boo::EModifierKey mods, bool isRepeat)
{
    if (key == boo::ESpecialKey::Enter && (mods & boo::EModifierKey::Alt) != boo::EModifierKey::None)
    {
        m_window->setFullscreen(!m_window->isFullscreen());
        return;
    }
    if (key == boo::ESpecialKey::Esc && m_splitMenuSystem->m_phase != SplitMenuSystem::Phase::Inactive)
    {
        m_splitMenuSystem->m_phase = SplitMenuSystem::Phase::Inactive;
        return;
    }
    for (View* v : m_views)
        v->specialKeyDown(key, mods, isRepeat);
    if (m_activeTextView)
        m_activeTextView->specialKeyDown(key, mods, isRepeat);
}

void RootView::specialKeyUp(boo::ESpecialKey key, boo::EModifierKey mods)
{
    for (View* v : m_views)
        v->specialKeyUp(key, mods);
    if (m_activeTextView)
        m_activeTextView->specialKeyUp(key, mods);
}

void RootView::modKeyDown(boo::EModifierKey mod, bool isRepeat)
{
    for (View* v : m_views)
        v->modKeyDown(mod, isRepeat);
    if (m_activeTextView)
        m_activeTextView->modKeyDown(mod, isRepeat);
}

void RootView::modKeyUp(boo::EModifierKey mod)
{
    for (View* v : m_views)
        v->modKeyUp(mod);
    if (m_activeTextView)
        m_activeTextView->modKeyUp(mod);
}

void RootView::resetTooltip(ViewResources& res)
{
    m_tooltip.reset(new Tooltip(res, *this, "Test", "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Morbi hendrerit nisl quis lobortis mattis. Mauris efficitur, est a vestibulum iaculis, leo orci pellentesque nunc, non rutrum ipsum lectus eget nisl. Aliquam accumsan vestibulum turpis. Duis id lacus ac lectus sollicitudin posuere vel sit amet metus. Aenean nec tortor id enim efficitur accumsan vitae eu ante. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Fusce magna eros, lacinia a leo eget, volutpat rhoncus urna."));
}

void RootView::displayTooltip(const std::string& name, const std::string& help)
{
}

void RootView::internalThink()
{
    if (m_splitMenuSystem->m_deferredSplit)
    {
        m_splitMenuSystem->m_deferredSplit = false;
        m_rightClickMenu.m_view.reset();
        m_splitMenuSystem->m_phase = SplitMenuSystem::Phase::InteractiveSplit;
        m_splitMenuSystem->mouseMove(m_splitMenuSystem->m_deferredCoord);
    }

    if (m_splitMenuSystem->m_deferredJoin)
    {
        m_splitMenuSystem->m_deferredJoin = false;
        m_rightClickMenu.m_view.reset();
        m_splitMenuSystem->m_phase = SplitMenuSystem::Phase::InteractiveJoin;
        m_splitMenuSystem->mouseMove(m_splitMenuSystem->m_deferredCoord);
    }

    if (m_rightClickMenu.m_view)
        m_rightClickMenu.m_view->think();
}

void RootView::draw(boo::IGraphicsCommandQueue* gfxQ)
{
    if (m_resizeRTDirty)
    {
        gfxQ->resizeRenderTexture(m_renderTex, m_rootRect.size[0], m_rootRect.size[1]);
        m_resizeRTDirty = false;
        gfxQ->schedulePostFrameHandler([&](){m_events.m_resizeCv.notify_one();});
    }
    m_viewRes->updateBuffers();
    gfxQ->setRenderTarget(m_renderTex);
    gfxQ->setViewport(m_rootRect);
    gfxQ->setScissor(m_rootRect);
    View::draw(gfxQ);
    for (View* v : m_views)
        v->draw(gfxQ);
    if (m_tooltip)
        m_tooltip->draw(gfxQ);
    m_splitMenuSystem->draw(gfxQ);
    if (m_rightClickMenu.m_view)
        m_rightClickMenu.m_view->draw(gfxQ);
    gfxQ->resolveDisplay(m_renderTex);
}

void RootView::SplitMenuSystem::draw(boo::IGraphicsCommandQueue* gfxQ)
{
    if (m_phase == Phase::Inactive)
        return;
    gfxQ->setShaderDataBinding(m_vertsBinding);
    if (m_phase == Phase::InteractiveJoin)
        gfxQ->draw(0, 28);
    else
        gfxQ->draw(28, 4);
}

}
