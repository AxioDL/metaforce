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
    virtual void activated(const boo::SWindowCoord& coord) {}
};

class Menu : public View
{
    IMenuNode* m_rootNode;
    IMenuNode* m_thisNode;
    std::unique_ptr<Menu> m_subMenu;
    std::unique_ptr<TextView> m_headText;

    int m_cWidth, m_cHeight, m_cTop;

    SolidShaderVert m_verts[8];
    VertexBufferBinding m_vertsBinding;
    void setVerts(int width, int height, float pf);

    struct ContentView : View
    {
        Menu& m_menu;
        ContentView(ViewResources& res, Menu& menu);

        boo::SWindowRect m_scissorRect;
        SolidShaderVert m_hlVerts[4];
        VertexBufferBinding m_hlVertsBinding;

        size_t m_highlightedItem = -1;
        void setHighlightedItem(size_t idx);
        void unsetHighlightedItem(size_t idx)
        {
            if (m_highlightedItem == idx)
                setHighlightedItem(-1);
        }

        void mouseDown(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
        void mouseUp(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
        void mouseMove(const boo::SWindowCoord&);
        void mouseLeave(const boo::SWindowCoord&);

        void resized(const boo::SWindowRect& root, const boo::SWindowRect& sub,
                     const boo::SWindowRect& scissor);
        void draw(boo::IGraphicsCommandQueue* gfxQ);

        int nominalWidth() const {return m_menu.m_cWidth;}
        int nominalHeight() const {return m_menu.m_cHeight;}
    };
    std::unique_ptr<ContentView> m_content;
    ViewChild<std::unique_ptr<ScrollView>> m_scroll;

    struct ItemView : View
    {
        Menu& m_menu;
        std::unique_ptr<TextView> m_textView;
        size_t m_idx;
        IMenuNode* m_node;
        ItemView(ViewResources& res, Menu& menu, const std::string& text, size_t idx, IMenuNode* node);

        void mouseDown(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
        void mouseUp(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
        void mouseEnter(const boo::SWindowCoord&);
        void mouseLeave(const boo::SWindowCoord&);

        void resized(const boo::SWindowRect& root, const boo::SWindowRect& sub);
        void draw(boo::IGraphicsCommandQueue* gfxQ);
    };
    std::vector<ViewChild<std::unique_ptr<ItemView>>> m_items;
    IMenuNode* m_deferredActivation = nullptr;

    Menu(ViewResources& res, View& parentView, IMenuNode* rootNode, IMenuNode* thisNode);

public:
    Menu(ViewResources& res, View& parentView, IMenuNode* rootNode);
    void reset(IMenuNode* rootNode);

    void mouseDown(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
    void mouseUp(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
    void mouseMove(const boo::SWindowCoord&);
    void mouseLeave(const boo::SWindowCoord&);
    void scroll(const boo::SWindowCoord&, const boo::SScrollDelta&);

    void think();
    void resized(const boo::SWindowRect& root, const boo::SWindowRect& sub);
    void draw(boo::IGraphicsCommandQueue* gfxQ);
};

}

#endif // SPECTER_MENU_HPP
