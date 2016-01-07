#ifndef SPECTER_SPACE_HPP
#define SPECTER_SPACE_HPP

#include "Specter/View.hpp"
#include "Specter/Toolbar.hpp"

namespace Specter
{

class Space : public View
{
    Toolbar::Position m_tbPos;
    std::unique_ptr<Toolbar> m_toolbar;
    bool m_toolbarMouseIn = false;
    bool m_toolbarMouseDown = false;
    View* m_contentView = nullptr;
    bool m_contentMouseIn = false;
    bool m_contentMouseDown = false;
public:
    Space(ViewResources& res, View& parentView, Toolbar::Position toolbarPos);
    View* setContentView(View* view);
    Toolbar* toolbar() {return m_toolbar.get();}
    void mouseDown(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
    void mouseUp(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
    void mouseMove(const boo::SWindowCoord&);
    void mouseEnter(const boo::SWindowCoord&);
    void mouseLeave(const boo::SWindowCoord&);
    void resized(const boo::SWindowRect& rootView, const boo::SWindowRect& sub);
    void draw(boo::IGraphicsCommandQueue* gfxQ);

    void setMultiplyColor(const Zeus::CColor& color)
    {
        View::setMultiplyColor(color);
        if (m_contentView)
            m_contentView->setMultiplyColor(color);
        if (m_toolbar)
            m_toolbar->setMultiplyColor(color);
    }
};

}

#endif // SPECTER_SPACE_HPP
