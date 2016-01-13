#ifndef SPECTER_MENU_HPP
#define SPECTER_MENU_HPP

#include "View.hpp"
#include "TextView.hpp"
#include "ScrollView.hpp"

namespace Specter
{

struct IMenuNode
{
    virtual boo::ITexture* icon() const {return nullptr;}
    virtual const std::string* text() const {return nullptr;}
    virtual size_t subNodeCount() const {return 0;}
    virtual IMenuNode* subNode(size_t idx) {return nullptr;}
    virtual void activated() {}
};

class Menu : public View
{
    IMenuNode* m_rootNode;
    IMenuNode* m_thisNode;
    std::unique_ptr<Menu> m_subMenu;
    std::unique_ptr<TextView> m_headText;

    int m_cWidth, m_cHeight;

    SolidShaderVert m_verts[16];
    VertexBufferBinding m_vertsBinding;

    struct ContentView : View
    {
        ContentView(ViewResources& res, Menu& menu) : View(res, menu) {}

        void mouseDown(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
        void mouseUp(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
        void mouseMove(const boo::SWindowCoord&);
        void mouseLeave(const boo::SWindowCoord&);

        void resized(const boo::SWindowRect& root, const boo::SWindowRect& sub);
        void draw(boo::IGraphicsCommandQueue* gfxQ);
    };
    std::unique_ptr<ContentView> m_content;
    ViewChild<std::unique_ptr<ScrollView>> m_scroll;

    struct ItemView : View
    {
        Menu& m_menu;
        std::unique_ptr<TextView> m_textView;
        ItemView(ViewResources& res, Menu& menu, const std::string& text);

        void mouseDown(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
        void mouseUp(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
        void mouseEnter(const boo::SWindowCoord&);
        void mouseLeave(const boo::SWindowCoord&);
    };
    std::vector<ViewChild<std::unique_ptr<ItemView>>> m_items;

    Menu(ViewResources& res, View& parentView, IMenuNode* rootNode, IMenuNode* thisNode);

public:
    Menu(ViewResources& res, View& parentView, IMenuNode* rootNode);
    void reset(IMenuNode* rootNode);

    void mouseDown(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
    void mouseUp(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
    void mouseMove(const boo::SWindowCoord&);
    void mouseLeave(const boo::SWindowCoord&);
    void scroll(const boo::SWindowCoord&, const boo::SScrollDelta&);

    void resized(const boo::SWindowRect& root, const boo::SWindowRect& sub);
    void draw(boo::IGraphicsCommandQueue* gfxQ);
};

}

#endif // SPECTER_MENU_HPP
