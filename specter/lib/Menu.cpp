#include "specter/Menu.hpp"
#include "specter/RootView.hpp"
#include "specter/ViewResources.hpp"

#define ROW_HEIGHT 18
#define ITEM_MARGIN 1

namespace specter
{

Menu::Menu(ViewResources& res, View& parentView, IMenuNode* rootNode)
: View(res, parentView)
{
    commitResources(res, [&](boo::IGraphicsDataFactory::Context& ctx) -> bool
    {
        buildResources(ctx, res);
        m_vertsBinding.init(ctx, res, 8, *m_viewVertBlockBuf);
        return true;
    });
    m_headText.reset(new TextView(res, *this, res.m_mainFont));
    m_scroll.m_view.reset(new ScrollView(res, *this, ScrollView::Style::ThinIndicator));
    m_content.reset(new ContentView(res, *this));
    m_scroll.m_view->setContentView(m_content.get());
    reset(rootNode);
}

void Menu::reset(IMenuNode* rootNode)
{
    m_rootNode = rootNode;
    m_thisNode = rootNode;
    ViewResources& res = rootView().viewRes();

    for (int i=0 ; i<8 ; ++i)
        m_verts[i].m_color = res.themeData().tooltipBackground();
    m_vertsBinding.load<decltype(m_verts)>(m_verts);

    m_subMenu.reset();

    const std::string* headText = rootNode->text();
    m_headText->typesetGlyphs(headText?*headText:"", rootView().themeData().uiAltText());

    float pf = rootView().viewRes().pixelFactor();
    int itemAdv = (ROW_HEIGHT + ITEM_MARGIN*2) * pf;
    m_cWidth = m_headText->nominalWidth() + 10*pf;
    m_cHeight = headText ? itemAdv : 0;
    m_cTop = m_cHeight;

    size_t subCount = rootNode->subNodeCount();
    m_items.clear();
    if (subCount)
    {
        m_items.reserve(subCount);
        for (size_t i=0 ; i<subCount ; ++i)
        {
            IMenuNode* node = rootNode->subNode(i);
            const std::string* nodeText = node->text();

            m_items.emplace_back();
            ViewChild<std::unique_ptr<ItemView>>& item = m_items.back();

            if (nodeText)
            {
                item.m_view.reset(new ItemView(res, *this, *nodeText, i, node));
                m_cWidth = std::max(m_cWidth, int(item.m_view->m_textView->nominalWidth() + 10*pf));
            }

            m_cHeight += itemAdv;
        }
    }
}

Menu::Menu(ViewResources& res, View& parentView, IMenuNode* rootNode, IMenuNode* thisNode)
: View(res, parentView), m_rootNode(rootNode), m_thisNode(thisNode)
{
    commitResources(res, [&](boo::IGraphicsDataFactory::Context& ctx) -> bool
    {
        buildResources(ctx, res);
        m_vertsBinding.init(ctx, res, 8, *m_viewVertBlockBuf);
        return true;
    });
    m_headText.reset(new TextView(res, *this, res.m_mainFont));
    m_scroll.m_view.reset(new ScrollView(res, *this, ScrollView::Style::ThinIndicator));
    m_content.reset(new ContentView(res, *this));
    m_scroll.m_view->setContentView(m_content.get());
}

Menu::ContentView::ContentView(ViewResources& res, Menu& menu)
: View(res, menu), m_menu(menu)
{
    commitResources(res, [&](boo::IGraphicsDataFactory::Context& ctx) -> bool
    {
        buildResources(ctx, res);
        m_hlVertsBinding.init(ctx, res, 4, *m_viewVertBlockBuf);
        return true;
    });

    m_hlVerts[0].m_color = res.themeData().button1Hover();
    m_hlVerts[1].m_color = res.themeData().button2Hover();
    m_hlVerts[2].m_color = res.themeData().button1Hover();
    m_hlVerts[3].m_color = res.themeData().button2Hover();
}

Menu::ItemView::ItemView(ViewResources& res, Menu& menu, const std::string& text, size_t idx, IMenuNode* node)
: View(res, menu), m_menu(menu), m_idx(idx), m_node(node)
{
    commitResources(res, [&](boo::IGraphicsDataFactory::Context& ctx) -> bool
    {
        buildResources(ctx, res);
        return true;
    });
    m_textView.reset(new specter::TextView(res, *this, res.m_mainFont));
    m_textView->typesetGlyphs(text, res.themeData().uiText());
}

void Menu::setVerts(int width, int height, float pf)
{
    m_verts[0].m_pos.assign(0, height-m_cTop-pf, 0);
    m_verts[1].m_pos.assign(0, 0, 0);
    m_verts[2].m_pos.assign(width, height-m_cTop-pf, 0);
    m_verts[3].m_pos.assign(width, 0, 0);

    m_verts[4].m_pos.assign(0, height, 0);
    m_verts[5].m_pos.assign(0, height-m_cTop+pf, 0);
    m_verts[6].m_pos.assign(width, height, 0);
    m_verts[7].m_pos.assign(width, height-m_cTop+pf, 0);

    m_vertsBinding.load<decltype(m_verts)>(m_verts);
}

void Menu::ContentView::setHighlightedItem(size_t idx)
{
    if (idx == -1)
    {
        m_highlightedItem = -1;
        return;
    }
    ViewChild<std::unique_ptr<ItemView>>& vc = m_menu.m_items[idx];

    if (!vc.m_view)
    {
        m_highlightedItem = -1;
        return;
    }

    m_highlightedItem = idx;
    const boo::SWindowRect& bgRect = subRect();
    const boo::SWindowRect& itemRect = vc.m_view->subRect();
    int y = itemRect.location[1] - bgRect.location[1];

    m_hlVerts[0].m_pos.assign(0, y+itemRect.size[1], 0);
    m_hlVerts[1].m_pos.assign(0, y, 0);
    m_hlVerts[2].m_pos.assign(itemRect.size[0], y+itemRect.size[1], 0);
    m_hlVerts[3].m_pos.assign(itemRect.size[0], y, 0);

    m_hlVertsBinding.load<decltype(m_hlVerts)>(m_hlVerts);
}

void Menu::mouseDown(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod)
{
    m_scroll.mouseDown(coord, button, mod);
}

void Menu::ContentView::mouseDown(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod)
{
    for (ViewChild<std::unique_ptr<ItemView>>& v : m_menu.m_items)
        v.mouseDown(coord, button, mod);
}

void Menu::ItemView::mouseDown(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod)
{
}

void Menu::mouseUp(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod)
{
    m_scroll.mouseUp(coord, button, mod);
    if (m_deferredActivation)
    {
        IMenuNode* node = m_deferredActivation;
        m_deferredActivation = nullptr;
        node->activated(coord);
    }
}

void Menu::ContentView::mouseUp(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod)
{
    for (ViewChild<std::unique_ptr<ItemView>>& v : m_menu.m_items)
        v.mouseUp(coord, button, mod);
}

void Menu::ItemView::mouseUp(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod)
{
    if (m_menu.m_content->m_highlightedItem == m_idx)
        m_menu.m_deferredActivation = m_node;
}

void Menu::mouseMove(const boo::SWindowCoord& coord)
{
    m_scroll.mouseMove(coord);
}

void Menu::ContentView::mouseMove(const boo::SWindowCoord& coord)
{
    for (ViewChild<std::unique_ptr<ItemView>>& v : m_menu.m_items)
        v.mouseMove(coord);
}

void Menu::ItemView::mouseEnter(const boo::SWindowCoord& coord)
{
    m_menu.m_content->setHighlightedItem(m_idx);
}

void Menu::mouseLeave(const boo::SWindowCoord& coord)
{
    m_scroll.mouseLeave(coord);
}

void Menu::ContentView::mouseLeave(const boo::SWindowCoord& coord)
{
    for (ViewChild<std::unique_ptr<ItemView>>& v : m_menu.m_items)
        v.mouseLeave(coord);
}

void Menu::ItemView::mouseLeave(const boo::SWindowCoord& coord)
{
    m_menu.m_content->unsetHighlightedItem(m_idx);
}

void Menu::scroll(const boo::SWindowCoord& coord, const boo::SScrollDelta& scroll)
{
    m_scroll.scroll(coord, scroll);
}

void Menu::think()
{
    m_scroll.m_view->think();
}

void Menu::resized(const boo::SWindowRect& root, const boo::SWindowRect& sub)
{
    float pf = rootView().viewRes().pixelFactor();
    boo::SWindowRect rect = sub;
    rect.size[0] = m_cWidth;
    if (rect.location[1] - m_cHeight < 0)
    {
        rect.location[1] += ROW_HEIGHT*pf;
        rect.size[1] = std::min(root.size[1] - rect.location[1], m_cHeight);
    }
    else
    {
        rect.location[1] -= m_cHeight;
        rect.size[1] = m_cHeight;
    }

    View::resized(root, rect);
    m_scroll.m_view->resized(root, rect);
    setVerts(rect.size[0], rect.size[1], pf);

    rect.location[0] += 5*pf;
    rect.location[1] += rect.size[1] - (ROW_HEIGHT + ITEM_MARGIN - 5)*pf;
    rect.size[0] = m_headText->nominalWidth();
    rect.size[1] = m_headText->nominalHeight();
    m_headText->resized(root, rect);
}

void Menu::ContentView::resized(const boo::SWindowRect& root, const boo::SWindowRect& sub,
                                const boo::SWindowRect& scissor)
{
    View::resized(root, sub);
    float pf = rootView().viewRes().pixelFactor();
    m_scissorRect = scissor;
    boo::SWindowRect itemRect = sub;
    itemRect.size[0] = m_menu.m_cWidth;
    itemRect.size[1] = ROW_HEIGHT*pf;
    itemRect.location[1] += sub.size[1] - m_menu.m_cTop + ITEM_MARGIN*pf;
    int itemAdv = (ROW_HEIGHT + ITEM_MARGIN*2) * pf;
    for (ViewChild<std::unique_ptr<ItemView>>& c : m_menu.m_items)
    {
        itemRect.location[1] -= itemAdv;
        if (c.m_view)
            c.m_view->resized(root, itemRect);
    }
}

void Menu::ItemView::resized(const boo::SWindowRect& root, const boo::SWindowRect& sub)
{
    View::resized(root, sub);
    float pf = rootView().viewRes().pixelFactor();
    boo::SWindowRect textRect = sub;
    textRect.location[0] += 5*pf;
    textRect.location[1] += 5*pf;
    m_textView->resized(root, textRect);
}

void Menu::draw(boo::IGraphicsCommandQueue* gfxQ)
{
    View::draw(gfxQ);
    gfxQ->setShaderDataBinding(m_vertsBinding);
    gfxQ->draw(0, 4);
    m_scroll.m_view->draw(gfxQ);
    gfxQ->setShaderDataBinding(m_vertsBinding);
    gfxQ->draw(4, 4);
    m_headText->draw(gfxQ);
}

void Menu::ContentView::draw(boo::IGraphicsCommandQueue* gfxQ)
{
    View::draw(gfxQ);
    gfxQ->setScissor(m_scissorRect);
    if (m_highlightedItem != -1)
    {
        gfxQ->setShaderDataBinding(m_hlVertsBinding);
        gfxQ->draw(0, 4);
    }
    for (ViewChild<std::unique_ptr<ItemView>>& c : m_menu.m_items)
        if (c.m_view)
            c.m_view->draw(gfxQ);
    gfxQ->setScissor(rootView().subRect());
}

void Menu::ItemView::draw(boo::IGraphicsCommandQueue* gfxQ)
{
    View::draw(gfxQ);
    m_textView->draw(gfxQ);
}

}
