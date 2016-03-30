#ifndef SPECTER_TEXTFIELD_HPP
#define SPECTER_TEXTFIELD_HPP

#include "Control.hpp"
#include "TextView.hpp"
#include <boo/IWindow.hpp>

namespace specter
{

class TextField : public ITextInputView
{
    bool m_hasTextSet = false;
    bool m_hasMarkSet = false;
    std::string m_textStr;
    std::string m_deferredTextStr;
    std::string m_deferredMarkStr;
    std::unique_ptr<TextView> m_text;
    std::unique_ptr<TextView> m_errText;

    SolidShaderVert m_verts[41];
    VertexBufferBinding m_vertsBinding;

    int m_nomWidth = 0;
    int m_nomHeight = 0;

    bool m_hasSelectionClear = false;
    bool m_hasSelectionSet = false;
    bool m_hasCursorSet = false;
    size_t m_selectionStart = 0;
    size_t m_deferredSelectionStart = 0;
    size_t m_selectionCount = 0;
    size_t m_deferredSelectionCount = 0;
    
    size_t m_markReplStart = 0;
    size_t m_deferredMarkReplStart = 0;
    size_t m_markReplCount = 0;
    size_t m_deferredMarkReplCount = 0;
    
    size_t m_markSelStart = 0;
    size_t m_deferredMarkSelStart = 0;
    size_t m_markSelCount = 0;
    size_t m_deferredMarkSelCount = 0;
    
    size_t m_cursorPos = 0;
    size_t m_deferredCursorPos = SIZE_MAX;
    size_t m_cursorFrames = 0;
    size_t m_clickFrames = 15;
    size_t m_clickFrames2 = 15;
    size_t m_errorFrames = 360;

    size_t m_dragStart = 0;
    size_t m_dragging = 0;

    bool m_active = false;
    bool m_error = false;

    enum class BGState
    {
        Inactive,
        Hover,
        Disabled
    } m_bgState = BGState::Inactive;
    void setInactive();
    void setHover();
    void setDisabled();
    void refreshBg();

public:
    TextField(ViewResources& res, View& parentView, IStringBinding* strBind);

    const std::string& getText() const {return m_textStr;}
    void setText(const std::string& str);

    void clipboardCopy();
    void clipboardCut();
    void clipboardPaste();

    void mouseDown(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
    void mouseUp(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
    void mouseMove(const boo::SWindowCoord&);
    void mouseEnter(const boo::SWindowCoord&);
    void mouseLeave(const boo::SWindowCoord&);
    void specialKeyDown(boo::ESpecialKey, boo::EModifierKey, bool);
    
    bool hasMarkedText() const;
    std::pair<int,int> markedRange() const;
    std::pair<int,int> selectedRange() const;
    void setMarkedText(const std::string& str,
                       const std::pair<int,int>& selectedRange,
                       const std::pair<int,int>& replacementRange);
    void unmarkText();
    
    std::string substringForRange(const std::pair<int,int>& range,
                                  std::pair<int,int>& actualRange) const;
    void insertText(const std::string& str, const std::pair<int,int>& range);
    int characterIndexAtPoint(const boo::SWindowCoord& point) const;
    boo::SWindowRect rectForCharacterRange(const std::pair<int,int>& range,
                                           std::pair<int,int>& actualRange) const;
    
    void think();
    void resized(const boo::SWindowRect& rootView, const boo::SWindowRect& sub);
    void draw(boo::IGraphicsCommandQueue* gfxQ);

    int nominalWidth() const {return m_nomWidth;}
    int nominalHeight() const {return m_nomHeight;}

    void setActive(bool active);
    void setCursorPos(size_t pos);
    void setErrorState(const std::string& message);
    void clearErrorState();

    void setSelectionRange(size_t start, size_t count);
    void clearSelectionRange();

    void setMultiplyColor(const zeus::CColor& color)
    {
        View::setMultiplyColor(color);
        m_viewVertBlock.m_color = color;
        if (m_viewVertBlockBuf)
            m_viewVertBlockBuf->load(&m_viewVertBlock, sizeof(ViewBlock));
        m_text->setMultiplyColor(color);
        if (m_errText)
            m_errText->setMultiplyColor(color);
    }

private:
    void _setCursorPos();
    void _reallySetCursorPos(size_t pos);
    void _setSelectionRange();
    void _reallySetSelectionRange(size_t start, size_t len);
    void _reallySetMarkRange(size_t start, size_t len);
    void _clearSelectionRange();
    void _setText();
    void _setMarkedText();
};

}

#endif // SPECTER_TEXTFIELD_HPP
