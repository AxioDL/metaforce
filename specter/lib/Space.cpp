#include "logvisor/logvisor.hpp"
#include "specter/Space.hpp"
#include "specter/ViewResources.hpp"
#include "specter/RootView.hpp"

namespace specter
{
static logvisor::Module Log("specter::Space");

#define TRIANGLE_DIM 12
#define TRIANGLE_DIM1 10
#define TRIANGLE_DIM2 8
#define TRIANGLE_DIM3 6
#define TRIANGLE_DIM4 4
#define TRIANGLE_DIM5 2

#define CORNER_DRAG_THRESHOLD 20

static const zeus::CColor TriColor = {0.75, 0.75, 0.75, 1.0};

Space::Space(ViewResources& res, View& parentView, ISpaceController& controller,
             Toolbar::Position tbPos, unsigned tbUnits)
: View(res, parentView), m_controller(controller), m_tbPos(tbPos)
{
    commitResources(res, [&](boo::IGraphicsDataFactory::Context& ctx) -> bool
    {
        buildResources(ctx, res);
        return true;
    });
    setBackground(res.themeData().spaceBackground());
    if (controller.spaceSplitAllowed())
        m_cornerView.m_view.reset(new CornerView(res, *this, TriColor));
    if (tbPos != Toolbar::Position::None)
        m_toolbar.m_view.reset(new Toolbar(res, *this, tbPos, tbUnits));
}

Space::CornerView::CornerView(ViewResources& res, Space& space, const zeus::CColor& triColor)
: View(res, space), m_space(space)
{
    commitResources(res, [&](boo::IGraphicsDataFactory::Context& ctx) -> bool
    {
        buildResources(ctx, res);
        m_vertexBinding.init(ctx, res, 34, m_viewVertBlockBuf);
        return true;
    });
    float pf = res.pixelFactor();

    zeus::CColor edgeColor1 = triColor * res.themeData().spaceTriangleShading1();
    zeus::CColor edgeColor2 = triColor * res.themeData().spaceTriangleShading2();
    View::SolidShaderVert verts[34];

    verts[0].m_pos.assign(0, TRIANGLE_DIM * pf, 0);
    verts[0].m_color = edgeColor1;
    verts[1].m_pos.assign(TRIANGLE_DIM * pf, 0, 0);
    verts[1].m_color = edgeColor1;
    verts[2].m_pos.assign(0, (TRIANGLE_DIM + 1) * pf, 0);
    verts[2].m_color = edgeColor1;
    verts[3].m_pos.assign((TRIANGLE_DIM + 1) * pf, 0, 0);
    verts[3].m_color = edgeColor1;
    verts[4] = verts[3];

    verts[5].m_pos.assign(0, TRIANGLE_DIM1 * pf, 0);
    verts[5].m_color = edgeColor2;
    verts[6] = verts[5];
    verts[7].m_pos.assign(TRIANGLE_DIM1 * pf, 0, 0);
    verts[7].m_color = edgeColor2;
    verts[8].m_pos.assign(0, (TRIANGLE_DIM1 + 1) * pf, 0);
    verts[8].m_color = edgeColor2;
    verts[9].m_pos.assign((TRIANGLE_DIM1 + 1) * pf, 0, 0);
    verts[9].m_color = edgeColor2;
    verts[10] = verts[9];

    verts[11].m_pos.assign(0, TRIANGLE_DIM2 * pf, 0);
    verts[11].m_color = edgeColor2;
    verts[12] = verts[11];
    verts[13].m_pos.assign(TRIANGLE_DIM2 * pf, 0, 0);
    verts[13].m_color = edgeColor2;
    verts[14].m_pos.assign(0, (TRIANGLE_DIM2 + 1) * pf, 0);
    verts[14].m_color = edgeColor2;
    verts[15].m_pos.assign((TRIANGLE_DIM2 + 1) * pf, 0, 0);
    verts[15].m_color = edgeColor2;
    verts[16] = verts[15];

    verts[17].m_pos.assign(0, TRIANGLE_DIM3 * pf, 0);
    verts[17].m_color = edgeColor2;
    verts[18] = verts[17];
    verts[19].m_pos.assign(TRIANGLE_DIM3 * pf, 0, 0);
    verts[19].m_color = edgeColor2;
    verts[20].m_pos.assign(0, (TRIANGLE_DIM3 + 1) * pf, 0);
    verts[20].m_color = edgeColor2;
    verts[21].m_pos.assign((TRIANGLE_DIM3 + 1) * pf, 0, 0);
    verts[21].m_color = edgeColor2;
    verts[22] = verts[21];

    verts[23].m_pos.assign(0, TRIANGLE_DIM4 * pf, 0);
    verts[23].m_color = edgeColor2;
    verts[24] = verts[23];
    verts[25].m_pos.assign(TRIANGLE_DIM4 * pf, 0, 0);
    verts[25].m_color = edgeColor2;
    verts[26].m_pos.assign(0, (TRIANGLE_DIM4 + 1) * pf, 0);
    verts[26].m_color = edgeColor2;
    verts[27].m_pos.assign((TRIANGLE_DIM4 + 1) * pf, 0, 0);
    verts[27].m_color = edgeColor2;
    verts[28] = verts[27];

    verts[29].m_pos.assign(0, TRIANGLE_DIM5 * pf, 0);
    verts[29].m_color = edgeColor2;
    verts[30] = verts[29];
    verts[31].m_pos.assign(TRIANGLE_DIM5 * pf, 0, 0);
    verts[31].m_color = edgeColor2;
    verts[32].m_pos.assign(0, (TRIANGLE_DIM5 + 1) * pf, 0);
    verts[32].m_color = edgeColor2;
    verts[33].m_pos.assign((TRIANGLE_DIM5 + 1) * pf, 0, 0);
    verts[33].m_color = edgeColor2;

    m_vertexBinding.load<decltype(verts)>(verts);
}

View* Space::setContentView(View* view)
{
    View* ret = m_contentView.m_view;
    m_contentView.m_view = view;
    updateSize();
    return ret;
}

void Space::mouseDown(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod)
{
    if (m_cornerView.mouseDown(coord, button, mod))
        return;
    m_contentView.mouseDown(coord, button, mod);
    m_toolbar.mouseDown(coord, button, mod);
}

void Space::CornerView::mouseDown(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod)
{
    if (button == boo::EMouseButton::Primary)
    {
        m_space.m_cornerDrag = true;
        m_space.m_cornerDragPoint[0] = coord.pixel[0];
        m_space.m_cornerDragPoint[1] = coord.pixel[1];
        rootView().setActiveDragView(&m_space);
    }
}

void Space::mouseUp(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod)
{
    m_cornerView.mouseUp(coord, button, mod);
    m_contentView.mouseUp(coord, button, mod);
    m_toolbar.mouseUp(coord, button, mod);
}

void Space::CornerView::mouseUp(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod)
{
    if (button == boo::EMouseButton::Primary)
    {
        m_space.m_cornerDrag = false;
        rootView().unsetActiveDragView(&m_space);
    }
}

void Space::mouseMove(const boo::SWindowCoord& coord)
{
    if (m_cornerDrag)
    {
        float pf = rootView().viewRes().pixelFactor();
        if (coord.pixel[0] < m_cornerDragPoint[0] - CORNER_DRAG_THRESHOLD * pf)
        {
            if (m_cornerView.m_view->m_flip)
            {
                rootView().mouseUp(coord, boo::EMouseButton::Primary, boo::EModifierKey::None);
                rootView().viewManager().deferSpaceSplit(&m_controller, SplitView::Axis::Vertical, 1, coord);
            }
            else
            {
                SplitView* sv = findSplitViewOnSide(SplitView::Axis::Vertical, 0);
                if (sv)
                {
                    rootView().mouseUp(coord, boo::EMouseButton::Primary, boo::EModifierKey::None);
                    rootView().beginInteractiveJoin(sv, coord);
                }
            }
        }
        else if (coord.pixel[1] < m_cornerDragPoint[1] - CORNER_DRAG_THRESHOLD * pf)
        {
            if (m_cornerView.m_view->m_flip)
            {
                rootView().mouseUp(coord, boo::EMouseButton::Primary, boo::EModifierKey::None);
                rootView().viewManager().deferSpaceSplit(&m_controller, SplitView::Axis::Horizontal, 1, coord);
            }
            else
            {
                SplitView* sv = findSplitViewOnSide(SplitView::Axis::Horizontal, 0);
                if (sv)
                {
                    rootView().mouseUp(coord, boo::EMouseButton::Primary, boo::EModifierKey::None);
                    rootView().beginInteractiveJoin(sv, coord);
                }
            }
        }
        else if (coord.pixel[0] > m_cornerDragPoint[0] + CORNER_DRAG_THRESHOLD * pf)
        {
            if (!m_cornerView.m_view->m_flip)
            {
                rootView().mouseUp(coord, boo::EMouseButton::Primary, boo::EModifierKey::None);
                rootView().viewManager().deferSpaceSplit(&m_controller, SplitView::Axis::Vertical, 0, coord);
            }
            else
            {
                SplitView* sv = findSplitViewOnSide(SplitView::Axis::Vertical, 1);
                if (sv)
                {
                    rootView().mouseUp(coord, boo::EMouseButton::Primary, boo::EModifierKey::None);
                    rootView().beginInteractiveJoin(sv, coord);
                }
            }
        }
        else if (coord.pixel[1] > m_cornerDragPoint[1] + CORNER_DRAG_THRESHOLD * pf)
        {
            if (!m_cornerView.m_view->m_flip)
            {
                rootView().mouseUp(coord, boo::EMouseButton::Primary, boo::EModifierKey::None);
                rootView().viewManager().deferSpaceSplit(&m_controller, SplitView::Axis::Horizontal, 0, coord);
            }
            else
            {
                SplitView* sv = findSplitViewOnSide(SplitView::Axis::Horizontal, 1);
                if (sv)
                {
                    rootView().mouseUp(coord, boo::EMouseButton::Primary, boo::EModifierKey::None);
                    rootView().beginInteractiveJoin(sv, coord);
                }
            }
        }
    }
    else
    {
        m_cornerView.mouseMove(coord);
        m_contentView.mouseMove(coord);
        m_toolbar.mouseMove(coord);
    }
}

void Space::mouseEnter(const boo::SWindowCoord& coord)
{
    m_cornerView.mouseEnter(coord);
    m_contentView.mouseEnter(coord);
    m_toolbar.mouseEnter(coord);
}

void Space::CornerView::mouseEnter(const boo::SWindowCoord& coord)
{
    rootView().setSpaceCornerHover(true);
}

void Space::mouseLeave(const boo::SWindowCoord& coord)
{
    m_cornerView.mouseLeave(coord);
    m_contentView.mouseLeave(coord);
    m_toolbar.mouseLeave(coord);
}

void Space::CornerView::mouseLeave(const boo::SWindowCoord& coord)
{
    rootView().setSpaceCornerHover(false);
}

SplitView* Space::findSplitViewOnSide(SplitView::Axis axis, int side)
{
    SplitView* ret = dynamic_cast<SplitView*>(&parentView());
    View* test = this;
    while (ret)
    {
        if (ret->axis() != axis)
            return nullptr;
        if (ret->m_views[side ^ 1].m_view == test)
            return ret;
        else if (ret->m_views[side].m_view == test)
            test = ret;
        ret = dynamic_cast<SplitView*>(&ret->parentView());
    }
    return nullptr;
}

void Space::resized(const boo::SWindowRect& root, const boo::SWindowRect& sub)
{
    View::resized(root, sub);

    float pf = rootView().viewRes().pixelFactor();
    if (m_cornerView.m_view)
    {
        boo::SWindowRect cornerRect = sub;
        int triDim = TRIANGLE_DIM * pf;
        cornerRect.size[0] = cornerRect.size[1] = triDim;
        if (cornerRect.location[0] < triDim && cornerRect.location[1] < triDim)
        {
            cornerRect.location[0] += sub.size[0] - triDim;
            cornerRect.location[1] += sub.size[1] - triDim;
            m_cornerView.m_view->resized(root, cornerRect, true);
        }
        else
            m_cornerView.m_view->resized(root, cornerRect, false);
    }

    boo::SWindowRect tbRect = sub;
    if (m_toolbar.m_view)
    {
        tbRect.size[1] = m_toolbar.m_view->nominalHeight();
        if (m_tbPos == Toolbar::Position::Top)
            tbRect.location[1] += sub.size[1] - tbRect.size[1];
        m_toolbar.m_view->resized(root, tbRect);
    }
    else
        tbRect.size[1] = 0;

    if (m_contentView.m_view)
    {
        boo::SWindowRect contentRect = sub;
        if (m_tbPos == Toolbar::Position::Bottom)
            contentRect.location[1] += tbRect.size[1];
        contentRect.size[1] = sub.size[1] - tbRect.size[1];
        contentRect.size[1] = std::max(contentRect.size[1], 0);
        m_contentView.m_view->resized(root, contentRect);
    }
}

void Space::CornerView::resized(const boo::SWindowRect& root, const boo::SWindowRect& sub, bool flip)
{
    m_flip = flip;
    if (flip)
    {
        m_viewVertBlock.m_mv[0][0] = -2.0f / root.size[0];
        m_viewVertBlock.m_mv[1][1] = -2.0f / root.size[1];
        m_viewVertBlock.m_mv[3][0] = (sub.location[0] + sub.size[0]) * -m_viewVertBlock.m_mv[0][0] - 1.0f;
        m_viewVertBlock.m_mv[3][1] = (sub.location[1] + sub.size[1]) * -m_viewVertBlock.m_mv[1][1] - 1.0f;
        View::resized(m_viewVertBlock, sub);
    }
    else
        View::resized(root, sub);
}

void Space::draw(boo::IGraphicsCommandQueue* gfxQ)
{
    View::draw(gfxQ);
    if (m_contentView.m_view)
        m_contentView.m_view->draw(gfxQ);
    if (m_toolbar.m_view)
        m_toolbar.m_view->draw(gfxQ);
    if (m_cornerView.m_view)
        m_cornerView.m_view->draw(gfxQ);
}

void Space::CornerView::draw(boo::IGraphicsCommandQueue* gfxQ)
{
    gfxQ->setShaderDataBinding(m_vertexBinding);
    gfxQ->draw(0, 34);
}

}

