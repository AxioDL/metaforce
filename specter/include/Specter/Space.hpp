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
    std::unique_ptr<View> m_contentView;
public:
    Space(ViewResources& res, View& parentView, Toolbar::Position toolbarPos);
    std::unique_ptr<View> setContentView(std::unique_ptr<View>&& view);
    void mouseDown(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
    void mouseUp(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
    void mouseMove(const boo::SWindowCoord&);
    void resized(const boo::SWindowRect& root, const boo::SWindowRect& sub);
    void resetResources(ViewResources& res);
    void draw(boo::IGraphicsCommandQueue* gfxQ);
};

}

#endif // SPECTER_SPACE_HPP
