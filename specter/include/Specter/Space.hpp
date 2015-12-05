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
    std::unique_ptr<View> m_contentView;
    bool m_contentMouseIn = false;
    bool m_contentMouseDown = false;
public:
    Space(ViewResources& res, View& parentView, Toolbar::Position toolbarPos);
    std::unique_ptr<View> setContentView(std::unique_ptr<View>&& view);
    Toolbar& toolbar() {return *m_toolbar;}
    void mouseDown(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
    void mouseUp(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
    void mouseMove(const boo::SWindowCoord&);
    void mouseEnter(const boo::SWindowCoord&);
    void mouseLeave(const boo::SWindowCoord&);
    void resized(const boo::SWindowRect& rootView, const boo::SWindowRect& sub);
    void resetResources(ViewResources& res);
    void draw(boo::IGraphicsCommandQueue* gfxQ);
};

}

#endif // SPECTER_SPACE_HPP
