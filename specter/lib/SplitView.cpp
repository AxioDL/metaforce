#include "logvisor/logvisor.hpp"
#include "specter/SplitView.hpp"
#include "specter/RootView.hpp"
#include "specter/ViewResources.hpp"
#include "specter/Space.hpp"

namespace specter
{
static logvisor::Module Log("specter::SplitView");

void SplitView::Resources::init(boo::IGraphicsDataFactory::Context& ctx, const IThemeData& theme)
{
    static const zeus::RGBA32 tex[3] =
    {
        {0,0,0,64},
        {0,0,0,255},
        {255,255,255,64}
    };
    m_shadingTex = ctx.newStaticTexture(3, 1, 1, boo::TextureFormat::RGBA8, tex, 12);
}

SplitView::SplitView(ViewResources& res, View& parentView, ISplitSpaceController* controller,
                     Axis axis, float split, int clearanceA, int clearanceB)
: View(res, parentView), m_controller(controller), m_axis(axis), m_slide(split),
  m_clearanceA(clearanceA), m_clearanceB(clearanceB)
{
    commitResources(res, [&](boo::IGraphicsDataFactory::Context& ctx) -> bool
    {
        buildResources(ctx, res);
        m_splitBlockBuf = ctx.newDynamicBuffer(boo::BufferUse::Uniform, sizeof(ViewBlock), 1);
        m_splitVertsBinding.initTex(ctx, res, 4, m_splitBlockBuf, res.m_splitRes.m_shadingTex);
        return true;
    });
}

View* SplitView::setContentView(int slot, View* view)
{
    if (slot < 0 || slot > 1)
        Log.report(logvisor::Fatal, "out-of-range slot to RootView::SplitView::setContentView");
    View* ret = m_views[slot].m_view;
    m_views[slot].m_view = view;
    m_views[slot].m_mouseDown = 0;
    m_views[slot].m_mouseIn = false;
    updateSize();
    return ret;
}

void SplitView::_setSplit(float slide)
{
    m_slide = std::min(std::max(slide, 0.0f), 1.0f);
    const boo::SWindowRect& rect = subRect();
    if (rect.size[0] && rect.size[1] &&
        (m_clearanceA >= 0 || m_clearanceB >= 0))
    {
        if (m_axis == Axis::Horizontal)
        {
            int slidePx = rect.size[1] * m_slide;
            if (m_clearanceA >= 0 && slidePx < m_clearanceA)
                m_slide = m_clearanceA / float(rect.size[1]);
            if (m_clearanceB >= 0 && (rect.size[1] - slidePx) < m_clearanceB)
                m_slide = 1.0 - m_clearanceB / float(rect.size[1]);
        }
        else if (m_axis == Axis::Vertical)
        {
            int slidePx = rect.size[0] * m_slide;
            if (m_clearanceA >= 0 && slidePx < m_clearanceA)
                m_slide = m_clearanceA / float(rect.size[0]);
            if (m_clearanceB >= 0 && (rect.size[0] - slidePx) < m_clearanceB)
                m_slide = 1.0 - m_clearanceB / float(rect.size[0]);
        }
        m_slide = std::min(std::max(m_slide, 0.0f), 1.0f);
    }
    if (m_controller)
        m_controller->updateSplit(m_slide);
}

void SplitView::setSplit(float slide)
{
    _setSplit(slide);
    updateSize();
}

void SplitView::setAxis(Axis axis)
{
    m_axis = axis;
    setSplit(m_slide);
}

bool SplitView::testSplitHover(const boo::SWindowCoord& coord)
{
    if (m_axis == Axis::Horizontal)
    {
        int slidePx = subRect().size[1] * m_slide;
        if (abs(int(coord.pixel[1] - subRect().location[1]) - slidePx) < 4)
            return true;
    }
    else if (m_axis == Axis::Vertical)
    {
        int slidePx = subRect().size[0] * m_slide;
        if (abs(int(coord.pixel[0] - subRect().location[0]) - slidePx) < 4)
            return true;
    }
    return false;
}

bool SplitView::testJoinArrowHover(const boo::SWindowCoord& coord, int& origSlotOut,
                                   SplitView*& splitOut, int& slotOut,
                                   boo::SWindowRect& rectOut, ArrowDir& dirOut, int forceSlot)
{
    if (!subRect().coordInRect(coord))
        return false;

    int origDummy;
    ArrowDir dirDummy;

    if (m_axis == Axis::Horizontal)
    {
        int slidePx = subRect().size[1] * m_slide;
        if ((forceSlot == -1 && coord.pixel[1] - subRect().location[1] - slidePx >= 0) || forceSlot == 1)
        {
            origSlotOut = 0;
            dirOut = ArrowDir::Up;
            if (m_views[1].m_view)
            {
                SplitView* chSplit = dynamic_cast<SplitView*>(m_views[1].m_view);
                if (chSplit && chSplit->m_axis == Axis::Horizontal)
                    return chSplit->testJoinArrowHover(coord, origDummy, splitOut, slotOut, rectOut, dirDummy, 0);
            }
            splitOut = this;
            slotOut = 1;
            rectOut = subRect();
            rectOut.location[1] += slidePx;
            rectOut.size[1] -= slidePx;
            return true;
        }
        else
        {
            origSlotOut = 1;
            dirOut = ArrowDir::Down;
            if (m_views[0].m_view)
            {
                SplitView* chSplit = dynamic_cast<SplitView*>(m_views[0].m_view);
                if (chSplit && chSplit->m_axis == Axis::Horizontal)
                    return chSplit->testJoinArrowHover(coord, origDummy, splitOut, slotOut, rectOut, dirDummy, 1);
            }
            splitOut = this;
            slotOut = 0;
            rectOut = subRect();
            rectOut.size[1] = slidePx;
            return true;
        }
    }
    else
    {
        int slidePx = subRect().size[0] * m_slide;
        if ((forceSlot == -1 && coord.pixel[0] - subRect().location[0] - slidePx >= 0) || forceSlot == 1)
        {
            origSlotOut = 0;
            dirOut = ArrowDir::Right;
            if (m_views[1].m_view)
            {
                SplitView* chSplit = dynamic_cast<SplitView*>(m_views[1].m_view);
                if (chSplit && chSplit->m_axis == Axis::Vertical)
                    return chSplit->testJoinArrowHover(coord, origDummy, splitOut, slotOut, rectOut, dirDummy, 0);
            }
            splitOut = this;
            slotOut = 1;
            rectOut = subRect();
            rectOut.location[0] += slidePx;
            rectOut.size[0] -= slidePx;
            return true;
        }
        else
        {
            origSlotOut = 1;
            dirOut = ArrowDir::Left;
            if (m_views[0].m_view)
            {
                SplitView* chSplit = dynamic_cast<SplitView*>(m_views[0].m_view);
                if (chSplit && chSplit->m_axis == Axis::Vertical)
                    return chSplit->testJoinArrowHover(coord, origDummy, splitOut, slotOut, rectOut, dirDummy, 1);
            }
            splitOut = this;
            slotOut = 0;
            rectOut = subRect();
            rectOut.size[0] = slidePx;
            return true;
        }
    }
}

void SplitView::getJoinArrowHover(int slot, boo::SWindowRect& rectOut, ArrowDir& dirOut)
{
    if (m_axis == Axis::Horizontal)
    {
        int slidePx = subRect().size[1] * m_slide;
        if (slot == 1)
        {
            if (m_views[1].m_view)
            {
                SplitView* chSplit = dynamic_cast<SplitView*>(m_views[1].m_view);
                if (chSplit && chSplit->m_axis == Axis::Horizontal)
                    return chSplit->getJoinArrowHover(0, rectOut, dirOut);
            }
            rectOut = subRect();
            rectOut.location[1] += slidePx;
            rectOut.size[1] -= slidePx;
            dirOut = ArrowDir::Up;
        }
        else
        {
            if (m_views[0].m_view)
            {
                SplitView* chSplit = dynamic_cast<SplitView*>(m_views[0].m_view);
                if (chSplit && chSplit->m_axis == Axis::Horizontal)
                    return chSplit->getJoinArrowHover(1, rectOut, dirOut);
            }
            rectOut = subRect();
            rectOut.size[1] = slidePx;
            dirOut = ArrowDir::Down;
        }
    }
    else
    {
        int slidePx = subRect().size[0] * m_slide;
        if (slot == 1)
        {
            if (m_views[1].m_view)
            {
                SplitView* chSplit = dynamic_cast<SplitView*>(m_views[1].m_view);
                if (chSplit && chSplit->m_axis == Axis::Vertical)
                    return chSplit->getJoinArrowHover(0, rectOut, dirOut);
            }
            rectOut = subRect();
            rectOut.location[0] += slidePx;
            rectOut.size[0] -= slidePx;
            dirOut = ArrowDir::Right;
        }
        else
        {
            if (m_views[0].m_view)
            {
                SplitView* chSplit = dynamic_cast<SplitView*>(m_views[0].m_view);
                if (chSplit && chSplit->m_axis == Axis::Vertical)
                    return chSplit->getJoinArrowHover(1, rectOut, dirOut);
            }
            rectOut = subRect();
            rectOut.size[0] = slidePx;
            dirOut = ArrowDir::Left;
        }
    }
}

bool SplitView::testSplitLineHover(const boo::SWindowCoord& coord, int& slotOut,
                                   boo::SWindowRect& rectOut, float& splitOut, Axis& axisOut)
{
    if (!subRect().coordInRect(coord))
        return false;

    if (m_axis == Axis::Horizontal)
    {
        int slidePx = subRect().size[1] * m_slide;
        if (coord.pixel[1] - subRect().location[1] - slidePx >= 0)
        {
            if (m_views[1].m_view)
            {
                SplitView* chSplit = dynamic_cast<SplitView*>(m_views[1].m_view);
                if (chSplit && chSplit->m_axis == Axis::Horizontal)
                    return chSplit->testSplitLineHover(coord, slotOut, rectOut, splitOut, axisOut);
            }
            slotOut = 1;
            rectOut = subRect();
            rectOut.location[1] += slidePx;
            rectOut.size[1] -= slidePx;
            splitOut = (coord.pixel[0] - rectOut.location[0]) / float(rectOut.size[0]);
            axisOut = Axis::Vertical;
            return true;
        }
        else
        {
            if (m_views[0].m_view)
            {
                SplitView* chSplit = dynamic_cast<SplitView*>(m_views[0].m_view);
                if (chSplit && chSplit->m_axis == Axis::Horizontal)
                    return chSplit->testSplitLineHover(coord, slotOut, rectOut, splitOut, axisOut);
            }
            slotOut = 0;
            rectOut = subRect();
            rectOut.size[1] = slidePx;
            splitOut = (coord.pixel[0] - rectOut.location[0]) / float(rectOut.size[0]);
            axisOut = Axis::Vertical;
            return true;
        }
    }
    else
    {
        int slidePx = subRect().size[0] * m_slide;
        if (coord.pixel[0] - subRect().location[0] - slidePx >= 0)
        {
            if (m_views[1].m_view)
            {
                SplitView* chSplit = dynamic_cast<SplitView*>(m_views[1].m_view);
                if (chSplit && chSplit->m_axis == Axis::Vertical)
                    return chSplit->testSplitLineHover(coord, slotOut, rectOut, splitOut, axisOut);
            }
            slotOut = 1;
            rectOut = subRect();
            rectOut.location[0] += slidePx;
            rectOut.size[0] -= slidePx;
            splitOut = (coord.pixel[1] - rectOut.location[1]) / float(rectOut.size[1]);
            axisOut = Axis::Horizontal;
            return true;
        }
        else
        {
            if (m_views[0].m_view)
            {
                SplitView* chSplit = dynamic_cast<SplitView*>(m_views[0].m_view);
                if (chSplit && chSplit->m_axis == Axis::Vertical)
                    return chSplit->testSplitLineHover(coord, slotOut, rectOut, splitOut, axisOut);
            }
            slotOut = 0;
            rectOut = subRect();
            rectOut.size[0] = slidePx;
            splitOut = (coord.pixel[1] - rectOut.location[1]) / float(rectOut.size[1]);
            axisOut = Axis::Horizontal;
            return true;
        }
    }
}

void SplitView::getSplitLineHover(int slot, boo::SWindowRect& rectOut, Axis& axisOut)
{
    if (m_axis == Axis::Horizontal)
    {
        int slidePx = subRect().size[1] * m_slide;
        if (slot == 1)
        {
            if (m_views[1].m_view)
            {
                SplitView* chSplit = dynamic_cast<SplitView*>(m_views[1].m_view);
                if (chSplit && chSplit->m_axis == Axis::Horizontal)
                    return chSplit->getSplitLineHover(0, rectOut, axisOut);
            }
            rectOut = subRect();
            rectOut.location[1] += slidePx;
            rectOut.size[1] -= slidePx;
            axisOut = Axis::Vertical;
        }
        else
        {
            if (m_views[0].m_view)
            {
                SplitView* chSplit = dynamic_cast<SplitView*>(m_views[0].m_view);
                if (chSplit && chSplit->m_axis == Axis::Horizontal)
                    return chSplit->getSplitLineHover(1, rectOut, axisOut);
            }
            rectOut = subRect();
            rectOut.size[1] = slidePx;
            axisOut = Axis::Vertical;
        }
    }
    else
    {
        int slidePx = subRect().size[0] * m_slide;
        if (slot == 1)
        {
            if (m_views[1].m_view)
            {
                SplitView* chSplit = dynamic_cast<SplitView*>(m_views[1].m_view);
                if (chSplit && chSplit->m_axis == Axis::Vertical)
                    return chSplit->getSplitLineHover(0, rectOut, axisOut);
            }
            rectOut = subRect();
            rectOut.location[0] += slidePx;
            rectOut.size[0] -= slidePx;
            axisOut = Axis::Horizontal;
        }
        else
        {
            if (m_views[0].m_view)
            {
                SplitView* chSplit = dynamic_cast<SplitView*>(m_views[0].m_view);
                if (chSplit && chSplit->m_axis == Axis::Vertical)
                    return chSplit->getSplitLineHover(1, rectOut, axisOut);
            }
            rectOut = subRect();
            rectOut.size[0] = slidePx;
            axisOut = Axis::Horizontal;
        }
    }
}

void SplitView::startDragSplit(const boo::SWindowCoord& coord)
{
    m_dragging = true;
    if (m_axis == Axis::Horizontal)
        setSplit((coord.pixel[1] - subRect().location[1]) / float(subRect().size[1]));
    else if (m_axis == Axis::Vertical)
        setSplit((coord.pixel[0] - subRect().location[0]) / float(subRect().size[0]));
}

void SplitView::endDragSplit()
{
    m_dragging = false;
}

void SplitView::moveDragSplit(const boo::SWindowCoord& coord)
{
    if (m_axis == Axis::Horizontal)
        setSplit((coord.pixel[1] - subRect().location[1]) / float(subRect().size[1]));
    else if (m_axis == Axis::Vertical)
        setSplit((coord.pixel[0] - subRect().location[0]) / float(subRect().size[0]));
}

void SplitView::mouseDown(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod)
{
    m_views[0].mouseDown(coord, button, mod);
    m_views[1].mouseDown(coord, button, mod);
}

void SplitView::mouseUp(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod)
{
    m_views[0].mouseUp(coord, button, mod);
    m_views[1].mouseUp(coord, button, mod);
}

void SplitView::mouseMove(const boo::SWindowCoord& coord)
{
    m_views[0].mouseMove(coord);
    m_views[1].mouseMove(coord);
}

void SplitView::mouseEnter(const boo::SWindowCoord& coord)
{
    m_views[0].mouseEnter(coord);
    m_views[1].mouseEnter(coord);
}

void SplitView::mouseLeave(const boo::SWindowCoord& coord)
{
    m_views[0].mouseLeave(coord);
    m_views[1].mouseLeave(coord);
}

void SplitView::resized(const boo::SWindowRect& root, const boo::SWindowRect& sub)
{
    View::resized(root, sub);
    _setSplit(m_slide);
    if (m_axis == Axis::Horizontal)
    {
        boo::SWindowRect ssub = sub;
        ssub.size[1] *= m_slide;
        if (m_views[0].m_view)
            m_views[0].m_view->resized(root, ssub);
        ssub.location[1] += ssub.size[1];
        ssub.size[1] = sub.size[1] - ssub.size[1];
        if (m_views[1].m_view)
            m_views[1].m_view->resized(root, ssub);
        ssub.location[1] -= 1;
        m_splitBlock.setViewRect(root, ssub);
        setHorizontalVerts(ssub.size[0]);
    }
    else if (m_axis == Axis::Vertical)
    {
        boo::SWindowRect ssub = sub;
        ssub.size[0] *= m_slide;
        if (m_views[0].m_view)
            m_views[0].m_view->resized(root, ssub);
        ssub.location[0] += ssub.size[0];
        ssub.size[0] = sub.size[0] - ssub.size[0];
        if (m_views[1].m_view)
            m_views[1].m_view->resized(root, ssub);
        ssub.location[0] -= 1;
        m_splitBlock.setViewRect(root, ssub);
        setVerticalVerts(ssub.size[1]);
    }
    m_splitBlockBuf->load(&m_splitBlock, sizeof(ViewBlock));
    m_splitVertsBinding.load(m_splitVerts, sizeof(m_splitVerts));
}

void SplitView::draw(boo::IGraphicsCommandQueue* gfxQ)
{
    View::draw(gfxQ);
    if (m_views[0].m_view)
        m_views[0].m_view->draw(gfxQ);
    if (m_views[1].m_view)
        m_views[1].m_view->draw(gfxQ);
    gfxQ->setShaderDataBinding(m_splitVertsBinding);
    gfxQ->draw(0, 4);

}

}

