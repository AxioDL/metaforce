#ifndef SPECTER_PATHBUTTONS_HPP
#define SPECTER_PATHBUTTONS_HPP

#include "Button.hpp"
#include "ScrollView.hpp"

namespace specter
{

struct IPathButtonsBinding
{
    virtual void pathButtonActivated(size_t idx)=0;
};

class PathButtons : public ScrollView
{
    struct ContentView : public View
    {
        PathButtons& m_pb;
        boo::SWindowRect m_scissorRect;

        void mouseDown(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
        void mouseUp(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
        void mouseMove(const boo::SWindowCoord&);
        void mouseLeave(const boo::SWindowCoord&);

        int nominalWidth() const
        {
            int ret = 0;
            for (PathButton& b : m_pb.m_pathButtons)
                ret += b.m_button.m_view->nominalWidth() + 2;
            return ret;
        }
        int nominalHeight() const
        {
            return m_pb.m_pathButtons.size() ? m_pb.m_pathButtons[0].m_button.m_view->nominalHeight() : 0;
        }

        void resized(const boo::SWindowRect& root, const boo::SWindowRect& sub, const boo::SWindowRect& scissor);
        void draw(boo::IGraphicsCommandQueue* gfxQ);

        ContentView(ViewResources& res, PathButtons& pb)
        : View(res, pb), m_pb(pb) {}
    };
    ViewChild<std::unique_ptr<ContentView>> m_contentView;

    int m_pathButtonPending = -1;
    IPathButtonsBinding& m_binding;
    bool m_fillContainer;
    struct PathButton final : IButtonBinding
    {
        PathButtons& m_pb;
        size_t m_idx;
        ViewChild<std::unique_ptr<Button>> m_button;
        PathButton(PathButtons& pb, ViewResources& res, size_t idx, const hecl::SystemString& str)
        : m_pb(pb), m_idx(idx)
        {
            m_button.m_view.reset(new Button(res, pb, this, hecl::SystemUTF8Conv(str).str()));
        }
        std::string_view name(const Control* control) const {return m_button.m_view->getText();}
        void activated(const Button* button, const boo::SWindowCoord&) {m_pb.m_pathButtonPending = m_idx;}
    };
    friend struct PathButton;
    std::vector<PathButton> m_pathButtons;

public:
    PathButtons(ViewResources& res, View& parentView, IPathButtonsBinding& binding, bool fillContainer=false);

    void setButtons(const std::vector<hecl::SystemString>& comps);
    void setMultiplyColor(const zeus::CColor& color);

    /* Fill all available space in container when requested */
    void containerResized(const boo::SWindowRect& root, const boo::SWindowRect& sub);
};

}

#endif // SPECTER_PATHBUTTONS_HPP
