#include <LogVisor/LogVisor.hpp>
#include "Specter/SplitView.hpp"
#include "Specter/RootView.hpp"
#include "Specter/ViewResources.hpp"

namespace Specter
{
static LogVisor::LogModule Log("Specter::SplitView");

void SplitView::Resources::init(boo::IGraphicsDataFactory* factory, const ThemeData& theme)
{
    static const Zeus::RGBA32 tex[3] =
    {
        {0,0,0,64},
        {0,0,0,255},
        {255,255,255,64}
    };
    m_shadingTex = factory->newStaticTexture(3, 1, 1, boo::TextureFormat::RGBA8, tex, 12);
}

SplitView::SplitView(ViewResources& res, View& parentView, Axis axis, int clearanceA, int clearanceB)
: View(res, parentView), m_axis(axis), m_clearanceA(clearanceA), m_clearanceB(clearanceB)
{
    m_splitBlockBuf = res.m_factory->newDynamicBuffer(boo::BufferUse::Uniform, sizeof(ViewBlock), 1);
    m_splitVertsBinding.initTex(res, 4, m_splitBlockBuf, res.m_splitRes.m_shadingTex);
    commitResources(res);
}

View* SplitView::setContentView(int slot, View* view)
{
    if (slot < 0 || slot > 1)
        Log.report(LogVisor::FatalError, "out-of-range slot to RootView::SplitView::setContentView");
    View* ret = m_views[slot].m_view;
    m_views[slot].m_view = view;
    m_views[slot].m_mouseDown = 0;
    m_views[slot].m_mouseIn = false;
    updateSize();
    return ret;
}

void SplitView::_setSlide(float slide)
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
}

void SplitView::setSlide(float slide)
{
    _setSlide(slide);
    updateSize();
}

void SplitView::mouseDown(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod)
{
    if (m_axis == Axis::Horizontal)
    {
        int slidePx = subRect().size[1] * m_slide;
        if (abs(int(coord.pixel[1] - subRect().location[1]) - slidePx) < 4)
        {
            if (button == boo::EMouseButton::Primary)
            {
                m_dragging = true;
                setSlide((coord.pixel[1] - subRect().location[1]) / float(subRect().size[1]));
            }
            else if (button == boo::EMouseButton::Secondary)
            {
                // TODO: Split menu
            }
            return;
        }
    }
    else if (m_axis == Axis::Vertical)
    {
        int slidePx = subRect().size[0] * m_slide;
        if (abs(int(coord.pixel[0] - subRect().location[0]) - slidePx) < 4)
        {
            if (button == boo::EMouseButton::Primary)
            {
                m_dragging = true;
                setSlide((coord.pixel[0] - subRect().location[0]) / float(subRect().size[0]));
            }
            else if (button == boo::EMouseButton::Secondary)
            {
                // TODO: Split menu
            }
            return;
        }
    }
    m_views[0].mouseDown(coord, button, mod);
    m_views[1].mouseDown(coord, button, mod);
}

void SplitView::mouseUp(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod)
{
    if (button == boo::EMouseButton::Primary)
        m_dragging = false;
    m_views[0].mouseUp(coord, button, mod);
    m_views[1].mouseUp(coord, button, mod);
}

void SplitView::mouseMove(const boo::SWindowCoord& coord)
{
    if (m_axis == Axis::Horizontal)
    {
        if (m_dragging)
            setSlide((coord.pixel[1] - subRect().location[1]) / float(subRect().size[1]));
        int slidePx = subRect().size[1] * m_slide;
        rootView().setHorizontalSplitHover(abs(int(coord.pixel[1] - subRect().location[1]) - slidePx) < 4);
    }
    else if (m_axis == Axis::Vertical)
    {
        if (m_dragging)
            setSlide((coord.pixel[0] - subRect().location[0]) / float(subRect().size[0]));
        int slidePx = subRect().size[0] * m_slide;
        rootView().setVerticalSplitHover(abs(int(coord.pixel[0] - subRect().location[0]) - slidePx) < 4);
    }

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
    _setSlide(m_slide);
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

