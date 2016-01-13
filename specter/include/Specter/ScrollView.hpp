#ifndef SPECTER_SCROLLVIEW_HPP
#define SPECTER_SCROLLVIEW_HPP

#include "Button.hpp"
#include "IViewManager.hpp"

namespace Specter
{
class ViewResources;
class Button;

class ScrollView : public View
{
public:
    enum class Style
    {
        Plain,
        ThinIndicator,
        SideButtons
    };

private:
    Style m_style;
    ScissorViewChild<View*> m_contentView;
    int m_scroll[2] = {};
    int m_targetScroll[2] = {};

    size_t m_consecutiveIdx = 0;
    double m_consecutiveScroll[16][2] = {};

    bool m_drawInd = false;
    bool m_drawSideButtons = false;

    SolidShaderVert m_verts[4];
    VertexBufferBinding m_vertsBinding;

    enum class SideButtonState
    {
        None,
        ScrollLeft,
        ScrollRight
    } m_sideButtonState = SideButtonState::None;
    struct SideButtonBinding : IButtonBinding
    {
        ScrollView& m_sv;
        std::string m_leftName, m_rightName;
        SideButtonBinding(ScrollView& sv, IViewManager& vm)
        : m_sv(sv),
          m_leftName(vm.translateOr("scroll_left", "Scroll Left")),
          m_rightName(vm.translateOr("scroll_right", "Scroll Right")) {}
        const char* name(const Control* control) const
        {
            return (control == reinterpret_cast<Control*>(m_sv.m_sideButtons[0].m_view.get())) ?
                   m_leftName.c_str() : m_rightName.c_str();
        }
        void down(const Button* button, const boo::SWindowCoord& coord)
        {
            if (button == m_sv.m_sideButtons[0].m_view.get())
                m_sv.m_sideButtonState = SideButtonState::ScrollRight;
            else
                m_sv.m_sideButtonState = SideButtonState::ScrollLeft;
        }
        void up(const Button* button, const boo::SWindowCoord& coord)
        {
            m_sv.m_sideButtonState = SideButtonState::None;
        }
    } m_sideButtonBind;
    ViewChild<std::unique_ptr<Button>> m_sideButtons[2];

    bool _scroll(const boo::SScrollDelta& scroll);
    int scrollAreaWidth() const;

public:
    ScrollView(ViewResources& res, View& parentView, Style style);
    void setContentView(View* v)
    {
        m_contentView.m_view = v;
        updateSize();
    }

    void mouseDown(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
    void mouseUp(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
    void mouseMove(const boo::SWindowCoord&);
    void mouseEnter(const boo::SWindowCoord&);
    void mouseLeave(const boo::SWindowCoord&);
    void scroll(const boo::SWindowCoord& coord, const boo::SScrollDelta& scroll);
    int getScrollX() const {return m_scroll[0];}
    int getScrollY() const {return m_scroll[1];}

    int nominalWidth() const {return subRect().size[0];}
    int nominalHeight() const {return subRect().size[1];}

    void setMultiplyColor(const Zeus::CColor& color);

    void think();
    void resized(const boo::SWindowRect& root, const boo::SWindowRect& sub);
    void draw(boo::IGraphicsCommandQueue* gfxQ);
};

}

#endif // SPECTER_SCROLLVIEW_HPP
