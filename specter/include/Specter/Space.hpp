#ifndef SPECTER_SPACE_HPP
#define SPECTER_SPACE_HPP

#include "View.hpp"
#include "Toolbar.hpp"
#include "SplitView.hpp"

namespace Specter
{
class Space;
struct ISplitSpaceController;

struct ISpaceController
{
    virtual bool spaceSplitAllowed() const {return false;}
    virtual ISplitSpaceController* spaceSplit(SplitView::Axis axis, int thisSlot) {return nullptr;}
    virtual ISpaceController* spaceJoin(int keepSlot) {return nullptr;}
};

struct ISplitSpaceController
{
    virtual SplitView* splitView()=0;
    virtual void setSplit(float split)=0;
};

class Space : public View
{
    ISpaceController& m_controller;
    Toolbar::Position m_tbPos;
    ViewChild<std::unique_ptr<Toolbar>> m_toolbar;
    ViewChild<View*> m_contentView;

    bool m_cornerDrag = false;
    int m_cornerDragPoint[2];

    struct CornerView : View
    {
        Space& m_space;
        VertexBufferBinding m_vertexBinding;
        bool m_flip;
        CornerView(ViewResources& res, Space& space, const Zeus::CColor& triColor);
        void mouseEnter(const boo::SWindowCoord&);
        void mouseLeave(const boo::SWindowCoord&);
        void mouseDown(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
        void mouseUp(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
        void resized(const boo::SWindowRect& root, const boo::SWindowRect& sub, bool flip);
        void draw(boo::IGraphicsCommandQueue* gfxQ);
    };
    friend struct CornerView;
    ViewChild<std::unique_ptr<CornerView>> m_cornerView;

public:
    Space(ViewResources& res, View& parentView, ISpaceController& controller, Toolbar::Position toolbarPos);
    View* setContentView(View* view);
    Toolbar* toolbar() {return m_toolbar.m_view.get();}
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
        if (m_contentView.m_view)
            m_contentView.m_view->setMultiplyColor(color);
        if (m_toolbar.m_view)
            m_toolbar.m_view->setMultiplyColor(color);
    }

    virtual const Zeus::CColor& spaceTriangleColor() const
    {
        static const Zeus::CColor defaultColor = {0.75, 0.75, 0.75, 1.0};
        return defaultColor;
    }
};

}

#endif // SPECTER_SPACE_HPP
