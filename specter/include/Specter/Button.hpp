#ifndef SPECTER_BUTTON_HPP
#define SPECTER_BUTTON_HPP

#include "Specter/TextView.hpp"
#include "Specter/Control.hpp"

namespace Specter
{

class Button : public Control
{
    std::string m_textStr;
    std::unique_ptr<TextView> m_text;
    SolidShaderVert m_verts[28];

    ViewBlock m_bBlock;
    boo::IGraphicsBufferD* m_bBlockBuf;

    boo::IGraphicsBufferD* m_bVertsBuf;
    boo::IVertexFormat* m_bVtxFmt; /* OpenGL only */
    boo::IShaderDataBinding* m_bShaderBinding;

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
           std::unique_ptr<IButtonBinding>&& controlBinding, const std::string& text);
    void mouseDown(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
    void mouseUp(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
    void mouseEnter(const boo::SWindowCoord&);
    void mouseLeave(const boo::SWindowCoord&);
    void resized(const boo::SWindowRect& rootView, const boo::SWindowRect& sub);
    void draw(boo::IGraphicsCommandQueue* gfxQ);

    void setText(const std::string& text);
    int nominalWidth() const {return m_nomWidth;}
    int nominalHeight() const {return m_nomHeight;}
};

}

#endif // SPECTER_BUTTON_HPP
