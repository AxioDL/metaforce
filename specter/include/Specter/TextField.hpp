#ifndef SPECTER_TEXTFIELD_HPP
#define SPECTER_TEXTFIELD_HPP

#include "Control.hpp"
#include "TextView.hpp"

namespace Specter
{

class TextField : public Control
{
    std::string m_textStr;
    std::unique_ptr<TextView> m_text;

    SolidShaderVert m_verts[32];
    boo::IGraphicsBufferD* m_bVertsBuf = nullptr;
    boo::IVertexFormat* m_bVtxFmt = nullptr; /* OpenGL only */
    boo::IShaderDataBinding* m_bShaderBinding = nullptr;

    int m_nomWidth = 0;
    int m_nomHeight = 0;

    size_t m_selectionStart = 0;
    size_t m_selectionCount = 0;
    size_t m_cursorPos = 0;
    size_t m_cursorFrames = 0;

    bool m_active = false;

    void setInactive();
    void setHover();
    void setDisabled();

public:
    TextField(ViewResources& res, View& parentView, IStringBinding* strBind);

    const std::string& getText() const {return m_textStr;}
    void setText(const std::string& str);

    void mouseDown(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
    void mouseUp(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
    void mouseMove(const boo::SWindowCoord&);
    void mouseEnter(const boo::SWindowCoord&);
    void mouseLeave(const boo::SWindowCoord&);
    void charKeyDown(unsigned long, boo::EModifierKey, bool);
    void specialKeyDown(boo::ESpecialKey, boo::EModifierKey, bool);
    void think();
    void resized(const boo::SWindowRect& rootView, const boo::SWindowRect& sub);
    void draw(boo::IGraphicsCommandQueue* gfxQ);

    int nominalWidth() const {return m_nomWidth;}
    int nominalHeight() const {return m_nomHeight;}

    void setActive(bool active);
    void setCursorPos(size_t pos);

    void setSelectionRange(size_t start, size_t count);
    void clearSelectionRange();

    void setMultiplyColor(const Zeus::CColor& color)
    {
        View::setMultiplyColor(color);
        m_viewVertBlock.m_color = color;
        m_viewVertBlockBuf->load(&m_viewVertBlock, sizeof(ViewBlock));
        m_text->setMultiplyColor(color);
    }
};

}

#endif // SPECTER_TEXTFIELD_HPP
