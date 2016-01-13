#include "Specter/Menu.hpp"
#include "Specter/RootView.hpp"
#include "Specter/ViewResources.hpp"

namespace Specter
{

Menu::Menu(ViewResources& res, View& parentView, IMenuNode* rootNode)
: View(res, parentView)
{
    m_vertsBinding.initSolid(res, 16, m_viewVertBlockBuf);
    commitResources(res);
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

    for (int i=0 ; i<16 ; ++i)
        m_verts[i].m_color = res.themeData().tooltipBackground();
    m_vertsBinding.load(m_verts, sizeof(m_verts));
    setBackground(Zeus::CColor::skBlue);

    m_subMenu.reset();

    const std::string* headText = rootNode->text();
    m_headText->typesetGlyphs(headText?*headText:"", rootView().themeData().uiText());

    float pf = rootView().viewRes().pixelFactor();
    m_cWidth = m_headText->nominalWidth() + 10*pf;
    m_cHeight = 22*pf;

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
                item.m_view.reset(new ItemView(res, *this, *nodeText));
                m_cWidth = std::max(m_cWidth, int(item.m_view->m_textView->nominalWidth() + 10*pf));
            }

            m_cHeight += 22*pf;
        }
    }
}

Menu::Menu(ViewResources& res, View& parentView, IMenuNode* rootNode, IMenuNode* thisNode)
: View(res, parentView), m_rootNode(rootNode), m_thisNode(thisNode)
{
    m_vertsBinding.initSolid(res, 16, m_viewVertBlockBuf);
    commitResources(res);
    m_headText.reset(new TextView(res, *this, res.m_mainFont));
    m_scroll.m_view.reset(new ScrollView(res, *this, ScrollView::Style::ThinIndicator));
    m_content.reset(new ContentView(res, *this));
    m_scroll.m_view->setContentView(m_content.get());
}

Menu::ItemView::ItemView(ViewResources& res, Menu& menu, const std::string& text)
: View(res, menu), m_menu(menu)
{
    commitResources(res);
    m_textView.reset(new Specter::TextView(res, *this, res.m_mainFont));
    m_textView->typesetGlyphs(text, res.themeData().uiText());
}

void Menu::mouseDown(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod)
{

}

void Menu::ContentView::mouseDown(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod)
{

}

void Menu::ItemView::mouseDown(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod)
{

}

void Menu::mouseUp(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod)
{

}

void Menu::ContentView::mouseUp(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod)
{

}

void Menu::ItemView::mouseUp(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod)
{

}

void Menu::mouseMove(const boo::SWindowCoord& coord)
{

}

void Menu::ContentView::mouseMove(const boo::SWindowCoord& coord)
{

}

void Menu::ItemView::mouseEnter(const boo::SWindowCoord& coord)
{

}

void Menu::mouseLeave(const boo::SWindowCoord& coord)
{

}

void Menu::ContentView::mouseLeave(const boo::SWindowCoord& coord)
{

}

void Menu::ItemView::mouseLeave(const boo::SWindowCoord& coord)
{

}

void Menu::scroll(const boo::SWindowCoord& coord, const boo::SScrollDelta& scroll)
{

}

void Menu::resized(const boo::SWindowRect& root, const boo::SWindowRect& sub)
{
    boo::SWindowRect rect = sub;
    View::resized(root, rect);
}

void Menu::ContentView::resized(const boo::SWindowRect& root, const boo::SWindowRect& sub)
{
}

void Menu::draw(boo::IGraphicsCommandQueue* gfxQ)
{
    View::draw(gfxQ);
}

void Menu::ContentView::draw(boo::IGraphicsCommandQueue* gfxQ)
{
    View::draw(gfxQ);
}

}
