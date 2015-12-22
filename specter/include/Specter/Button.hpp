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
    Zeus::CColor m_textColor;
    std::string m_textStr;
    std::unique_ptr<TextView> m_text;

    SolidShaderVert m_verts[28];
    boo::IGraphicsBufferD* m_bVertsBuf = nullptr;
    boo::IVertexFormat* m_bVtxFmt = nullptr; /* OpenGL only */
    boo::IShaderDataBinding* m_bShaderBinding = nullptr;

    RectangleConstraint m_constraint;
    int m_nomWidth, m_nomHeight;
    bool m_pressed = false;
    bool m_hovered = false;

    void setInactive();
    void setHover();
    void setPressed();
    void setDisabled();

public:
    class Resources
    {
        friend class ViewResources;
        friend class Button;

        void init(boo::IGraphicsDataFactory* factory, const ThemeData& theme);
    };

    Button(ViewResources& res, View& parentView,
           IButtonBinding* controlBinding, const std::string& text,
           Style style=Style::Block, RectangleConstraint constraint=RectangleConstraint());
    Button(ViewResources& res, View& parentView,
           IButtonBinding* controlBinding, const std::string& text,
           const Zeus::CColor& textColor, Style style=Style::Block,
           RectangleConstraint constraint=RectangleConstraint());
    void mouseDown(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
    void mouseUp(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
    void mouseEnter(const boo::SWindowCoord&);
    void mouseLeave(const boo::SWindowCoord&);
    void resized(const boo::SWindowRect& root, const boo::SWindowRect& sub);
    void draw(boo::IGraphicsCommandQueue* gfxQ);

    void setText(const std::string& text, const Zeus::CColor& textColor);
    void setText(const std::string& text);
    const std::string& getText() const {return m_textStr;}
    void colorGlyphs(const Zeus::CColor& newColor);
    int nominalWidth() const {return m_nomWidth;}
    int nominalHeight() const {return m_nomHeight;}

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
