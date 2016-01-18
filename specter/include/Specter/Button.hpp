#ifndef SPECTER_BUTTON_HPP
#define SPECTER_BUTTON_HPP

#include "Specter/TextView.hpp"
#include "Specter/Control.hpp"

namespace Specter
{

class Button : public Control
{
public:
    enum class Style
    {
        Block,
        Text,
    };

private:
    Style m_style;
    IButtonBinding::MenuStyle m_menuStyle = IButtonBinding::MenuStyle::None;
    Zeus::CColor m_textColor;
    std::string m_textStr;
    std::unique_ptr<TextView> m_text;
    boo::ITexture* m_icon = nullptr;

    SolidShaderVert m_verts[40];
    VertexBufferBinding m_vertsBinding;

    RectangleConstraint m_constraint;
    int m_nomWidth, m_nomHeight;
    int m_textWidth;

    struct ButtonTarget : View
    {
        Button& m_button;

        bool m_pressed = false;
        bool m_hovered = false;

        void setInactive();
        void setHover();
        void setPressed();
        void setDisabled();

        void mouseDown(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
        void mouseUp(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
        void mouseEnter(const boo::SWindowCoord&);
        void mouseLeave(const boo::SWindowCoord&);
        ButtonTarget(ViewResources& res, Button& button) : View(res, button), m_button(button) {}
    };
    ViewChild<std::unique_ptr<ButtonTarget>> m_buttonTarget;

    struct MenuTarget : View
    {
        Button& m_button;

        bool m_pressed = false;
        bool m_hovered = false;

        void setInactive();
        void setHover();
        void setPressed();
        void setDisabled();

        void mouseDown(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
        void mouseUp(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
        void mouseEnter(const boo::SWindowCoord&);
        void mouseLeave(const boo::SWindowCoord&);
        MenuTarget(ViewResources& res, Button& button) : View(res, button), m_button(button) {}
    };
    ViewChild<std::unique_ptr<MenuTarget>> m_menuTarget;

    ViewChild<std::unique_ptr<View>> m_modalMenu;

public:
    class Resources
    {
        friend class ViewResources;
        friend class Button;

        void init(boo::IGraphicsDataFactory* factory, const IThemeData& theme);
    };

    ~Button() {closeMenu({});}
    Button(ViewResources& res, View& parentView,
           IButtonBinding* controlBinding, const std::string& text, boo::ITexture* icon=nullptr,
           Style style=Style::Block, RectangleConstraint constraint=RectangleConstraint());
    Button(ViewResources& res, View& parentView,
           IButtonBinding* controlBinding, const std::string& text, const Zeus::CColor& textColor,
           boo::ITexture* icon=nullptr, Style style=Style::Block,
           RectangleConstraint constraint=RectangleConstraint());
    void mouseDown(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
    void mouseUp(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
    void mouseMove(const boo::SWindowCoord&);
    void mouseLeave(const boo::SWindowCoord&);
    void think();
    void resized(const boo::SWindowRect& root, const boo::SWindowRect& sub);
    void draw(boo::IGraphicsCommandQueue* gfxQ);

    void setText(const std::string& text, const Zeus::CColor& textColor);
    void setText(const std::string& text);
    void setIcon(boo::ITexture* icon);
    const std::string& getText() const {return m_textStr;}
    void colorGlyphs(const Zeus::CColor& newColor);
    int nominalWidth() const {return m_nomWidth;}
    int nominalHeight() const {return m_nomHeight;}

    void closeMenu(const boo::SWindowCoord& coord);
    ViewChild<std::unique_ptr<View>>& getMenu() {return m_modalMenu;}

    void setMultiplyColor(const Zeus::CColor& color)
    {
        View::setMultiplyColor(color);
        m_viewVertBlock.m_color = color;
        m_viewVertBlockBuf->load(&m_viewVertBlock, sizeof(ViewBlock));
        m_text->setMultiplyColor(color);
    }
};

}

#endif // SPECTER_BUTTON_HPP
