#include "Specter/TextField.hpp"
#include "Specter/RootView.hpp"
#include "Specter/ViewResources.hpp"

namespace Specter
{

TextField::TextField(ViewResources& res, View& parentView, IStringBinding* strBind)
: ITextInputView(res, parentView, strBind)
{
    m_vertsBinding.initSolid(res, 41, m_viewVertBlockBuf);
    commitResources(res);

    for (int i=28 ; i<32 ; ++i)
        m_verts[i].m_color = res.themeData().textfieldSelection();
    setInactive();
    m_vertsBinding.load(m_verts, sizeof(m_verts));

    m_text.reset(new TextView(res, *this, res.m_mainFont, TextView::Alignment::Left, 1024));
    if (strBind)
        setText(strBind->getDefault(this));
}

void TextField::_setText()
{
    if (m_hasTextSet)
    {
        _clearSelectionRange();
        m_textStr = m_deferredTextStr;
        m_text->typesetGlyphs(m_textStr, m_error ? rootView().themeData().uiText() :
                                                   rootView().themeData().fieldText());
        if (m_controlBinding && dynamic_cast<IStringBinding*>(m_controlBinding))
            static_cast<IStringBinding&>(*m_controlBinding).changed(this, m_textStr);
        m_hasTextSet = false;
        if (m_deferredMarkStr.size())
            m_hasMarkSet = true;
    }
}
    
void TextField::_setMarkedText()
{
    if (m_hasMarkSet)
    {
        m_markReplStart = m_deferredMarkReplStart;
        m_markReplCount = m_deferredMarkReplCount;
        m_markSelStart = m_deferredMarkSelStart;
        m_markSelCount = m_deferredMarkSelCount;
        
        size_t repPoint;
        size_t repEnd;
        if (m_selectionCount)
        {
            repPoint = m_selectionStart;
            repEnd = m_selectionStart;
        }
        else
        {
            repPoint = m_cursorPos;
            repEnd = m_cursorPos;
        }
        
        if (m_markReplStart != SIZE_MAX)
        {
            repPoint += m_markReplStart;
            repEnd += m_markReplStart + m_markReplCount;
        }
        
        size_t len = UTF8Iterator(m_textStr.cbegin()).countTo(m_textStr.cend());
        repPoint = std::min(repPoint, len);
        repEnd = std::min(repEnd, len);
        std::string compStr(m_textStr.cbegin(), (UTF8Iterator(m_textStr.cbegin()) + repPoint).iter());
        compStr += m_deferredMarkStr;
        compStr += std::string((UTF8Iterator(m_textStr.cbegin()) + repEnd).iter(), m_textStr.cend());
        m_text->typesetGlyphs(compStr, m_error ? rootView().themeData().uiText() :
                                                 rootView().themeData().fieldText());
        
        size_t pos = m_cursorPos;
        if (m_deferredMarkStr.size())
            pos += m_markSelStart;
        if (m_markSelCount)
            _reallySetMarkRange(pos, m_markSelCount);
        else
            _reallySetCursorPos(pos);
        
        std::vector<TextView::RenderGlyph>& glyphs = m_text->accessGlyphs();
        size_t defLen = UTF8Iterator(m_deferredMarkStr.cbegin()).countTo(m_deferredMarkStr.cend());
        for (auto it=glyphs.begin()+repPoint ; it<glyphs.begin()+repPoint+defLen ; ++it)
            it->m_color = rootView().themeData().fieldMarkedText();
        m_text->updateGlyphs();
        
        m_hasMarkSet = false;

    }
}
    
void TextField::setText(const std::string& str)
{
    std::unique_lock<std::recursive_mutex> lk(m_textInputLk);
    UTF8Iterator it(str.cbegin());
    for (; it.iter() != str.cend() ; ++it)
        if (*it.iter() == '\n')
            break;
    m_deferredTextStr.assign(str.cbegin(), it.iter());
    m_hasTextSet = true;
}

void TextField::setInactive()
{
    const IThemeData& theme = rootView().themeData();
    if (m_error)
    {
        m_verts[0].m_color = theme.textfield1Inactive() * Zeus::CColor::skRed;
        m_verts[1].m_color = theme.textfield2Inactive() * Zeus::CColor::skRed;
        m_verts[2].m_color = theme.textfield1Inactive() * Zeus::CColor::skRed;
        m_verts[3].m_color = theme.textfield2Inactive() * Zeus::CColor::skRed;
        m_verts[4].m_color = theme.textfield2Inactive() * Zeus::CColor::skRed;
        for (int i=5 ; i<28 ; ++i)
            m_verts[i].m_color = theme.textfield2Inactive() * Zeus::CColor::skRed;
    }
    else
    {
        m_verts[0].m_color = theme.textfield1Inactive();
        m_verts[1].m_color = theme.textfield2Inactive();
        m_verts[2].m_color = theme.textfield1Inactive();
        m_verts[3].m_color = theme.textfield2Inactive();
        m_verts[4].m_color = theme.textfield2Inactive();
        for (int i=5 ; i<28 ; ++i)
            m_verts[i].m_color = theme.textfield2Inactive();
    }
    m_vertsBinding.load(m_verts, sizeof(m_verts));
    m_bgState = BGState::Inactive;
}

void TextField::setHover()
{
    const IThemeData& theme = rootView().themeData();
    if (m_error)
    {
        m_verts[0].m_color = theme.textfield1Hover() * Zeus::CColor::skRed;
        m_verts[1].m_color = theme.textfield2Hover() * Zeus::CColor::skRed;
        m_verts[2].m_color = theme.textfield1Hover() * Zeus::CColor::skRed;
        m_verts[3].m_color = theme.textfield2Hover() * Zeus::CColor::skRed;
        m_verts[4].m_color = theme.textfield2Hover() * Zeus::CColor::skRed;
        for (int i=5 ; i<28 ; ++i)
            m_verts[i].m_color = theme.textfield2Inactive() * Zeus::CColor::skRed;
    }
    else
    {
        m_verts[0].m_color = theme.textfield1Hover();
        m_verts[1].m_color = theme.textfield2Hover();
        m_verts[2].m_color = theme.textfield1Hover();
        m_verts[3].m_color = theme.textfield2Hover();
        m_verts[4].m_color = theme.textfield2Hover();
        for (int i=5 ; i<28 ; ++i)
            m_verts[i].m_color = theme.textfield2Inactive();
    }
    m_vertsBinding.load(m_verts, sizeof(m_verts));
    m_bgState = BGState::Hover;
}

void TextField::setDisabled()
{
    const IThemeData& theme = rootView().themeData();
    if (m_error)
    {
        m_verts[0].m_color = theme.textfield1Disabled() * Zeus::CColor::skRed;
        m_verts[1].m_color = theme.textfield2Disabled() * Zeus::CColor::skRed;
        m_verts[2].m_color = theme.textfield1Disabled() * Zeus::CColor::skRed;
        m_verts[3].m_color = theme.textfield2Disabled() * Zeus::CColor::skRed;
        m_verts[4].m_color = theme.textfield2Disabled() * Zeus::CColor::skRed;
        for (int i=5 ; i<28 ; ++i)
            m_verts[i].m_color = theme.textfield2Disabled() * Zeus::CColor::skRed;
    }
    else
    {
        m_verts[0].m_color = theme.textfield1Disabled();
        m_verts[1].m_color = theme.textfield2Disabled();
        m_verts[2].m_color = theme.textfield1Disabled();
        m_verts[3].m_color = theme.textfield2Disabled();
        m_verts[4].m_color = theme.textfield2Disabled();
        for (int i=5 ; i<28 ; ++i)
            m_verts[i].m_color = theme.textfield2Disabled();
    }
    m_vertsBinding.load(m_verts, sizeof(m_verts));
    m_bgState = BGState::Disabled;
}

void TextField::refreshBg()
{
    switch (m_bgState)
    {
    case BGState::Inactive:
        setInactive(); break;
    case BGState::Hover:
        setHover(); break;
    case BGState::Disabled:
        setDisabled(); break;
    }
}

void TextField::mouseDown(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod)
{
    std::unique_lock<std::recursive_mutex> lk(m_textInputLk);
    if (!m_active)
    {
        rootView().setActiveTextView(this);
    }
    else if (m_clickFrames2 < 15)
    {
        size_t len = UTF8Iterator(m_textStr.cbegin()).countTo(m_textStr.cend());
        setSelectionRange(0, len);
    }
    else if (m_clickFrames < 15)
    {
        size_t startPos = m_text->reverseSelectGlyph(coord.pixel[0] - m_text->subRect().location[0]);
        std::pair<size_t,size_t> range = m_text->queryWholeWordRange(startPos);
        setSelectionRange(range.first, range.second);
        m_clickFrames2 = 0;
    }
    else
    {
        size_t startPos = m_text->reverseSelectGlyph(coord.pixel[0] - m_text->subRect().location[0]);
        setCursorPos(startPos);
        m_dragging |= size_t(1 << int(button));
        m_dragStart = startPos;
        rootView().setActiveDragView(this);
    }
    m_clickFrames = 0;
}

void TextField::mouseUp(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod)
{
    m_dragging &= ~(1 << int(button));
    if (m_dragging == 0)
        rootView().setActiveDragView(nullptr);
}

void TextField::mouseMove(const boo::SWindowCoord& coord)
{
    if (m_dragging != 0)
    {
        std::unique_lock<std::recursive_mutex> lk(m_textInputLk);
        size_t thisPos = m_text->reverseSelectGlyph(coord.pixel[0] - m_text->subRect().location[0]);
        size_t minPos = std::min(m_dragStart, thisPos);
        size_t maxPos = std::max(m_dragStart, thisPos);
        if (minPos != maxPos)
            setSelectionRange(minPos, maxPos-minPos);
        else
            setCursorPos(minPos);
    }
}

void TextField::mouseEnter(const boo::SWindowCoord& coord)
{
    setHover();
    rootView().setTextFieldHover(true);
}

void TextField::mouseLeave(const boo::SWindowCoord& coord)
{
    setInactive();
    rootView().setTextFieldHover(false);
}

void TextField::clipboardCopy()
{
    std::unique_lock<std::recursive_mutex> lk(m_textInputLk);
    
    if (!m_selectionCount)
        return;

    UTF8Iterator begin(m_textStr.cbegin());
    begin += m_selectionStart;
    UTF8Iterator end(begin.iter());
    end += m_selectionCount;

    rootView().window()->clipboardCopy(boo::EClipboardType::UTF8String,
                                       (uint8_t*)&*begin.iter(), end.iter() - begin.iter());
}
    
void TextField::clipboardCut()
{
    std::unique_lock<std::recursive_mutex> lk(m_textInputLk);
    
    if (!m_selectionCount)
        return;
    
    UTF8Iterator begin(m_textStr.cbegin());
    begin += m_selectionStart;
    UTF8Iterator end(begin.iter());
    end += m_selectionCount;
    
    rootView().window()->clipboardCopy(boo::EClipboardType::UTF8String,
                                       (uint8_t*)&*begin.iter(), end.iter() - begin.iter());
    
    std::string newStr(m_textStr.cbegin(), (UTF8Iterator(m_textStr.cbegin()) + m_selectionStart).iter());
    newStr.append((UTF8Iterator(m_textStr.cbegin()) + m_selectionStart + m_selectionCount).iter(), m_textStr.cend());
    size_t selStart = m_selectionStart;
    setText(newStr);
    setCursorPos(selStart);
}
    
static std::string SanitizeUTF8TextLine(const char* string, size_t len)
{
    const char* it = string;
    utf8proc_int32_t ch;
    utf8proc_ssize_t sz;
    std::string ret;
    ret.reserve(len);
    for (sz = utf8proc_iterate((utf8proc_uint8_t*)it, -1, &ch);
         it < string+len;
         it += sz, sz = utf8proc_iterate((utf8proc_uint8_t*)it, -1, &ch))
    {
        if (sz <= 0)
            break;
        if (ch >= 0x20)
            ret.append(it, sz);
    }
    return ret;
}

void TextField::clipboardPaste()
{
    std::unique_lock<std::recursive_mutex> lk(m_textInputLk);
    
    size_t retSz;
    std::unique_ptr<uint8_t[]> retData =
    rootView().window()->clipboardPaste(boo::EClipboardType::UTF8String, retSz);
    std::string saniData = SanitizeUTF8TextLine((char*)retData.get(), retSz);

    if (retData && saniData.size())
    {
        if (m_selectionCount)
        {
            std::string newStr(m_textStr.cbegin(), (UTF8Iterator(m_textStr.cbegin()) + m_selectionStart).iter());
            newStr.append(saniData);
            size_t newSel = UTF8Iterator(newStr.cbegin()).countTo(newStr.cend());
            newStr.append((UTF8Iterator(m_textStr.cbegin()) + m_selectionStart + m_selectionCount).iter(), m_textStr.cend());
            setText(newStr);
            setCursorPos(newSel);
        }
        else
        {
            std::string newStr(m_textStr.cbegin(), (UTF8Iterator(m_textStr.cbegin()) + m_cursorPos).iter());
            newStr.append(saniData);
            size_t newSel = UTF8Iterator(newStr.cbegin()).countTo(newStr.cend());
            newStr.append((UTF8Iterator(m_textStr.cbegin()) + m_cursorPos).iter(), m_textStr.cend());
            setText(newStr);
            setCursorPos(newSel);
        }
    }
}

void TextField::specialKeyDown(boo::ESpecialKey key, boo::EModifierKey mods, bool isRepeat)
{
    std::unique_lock<std::recursive_mutex> lk(m_textInputLk);
    if (m_deferredMarkStr.size())
        return;
    
    if (key == boo::ESpecialKey::Left)
    {
        if ((mods & boo::EModifierKey::Shift) != boo::EModifierKey::None)
        {
            if (m_cursorPos)
            {
                size_t origPos = m_cursorPos;
                if (m_selectionCount)
                {
                    if (m_cursorPos == m_selectionStart)
                        setSelectionRange(m_cursorPos-1, m_selectionCount+1);
                    else
                        setSelectionRange(m_selectionStart, m_selectionCount-1);
                }
                else
                    setSelectionRange(m_cursorPos-1, 1);
                m_cursorPos = origPos - 1;
            }
        }
        else
        {
            if (m_selectionCount)
                m_cursorPos = m_selectionStart;
            setCursorPos(m_cursorPos==0 ? 0 : (m_cursorPos-1));
        }
    }
    else if (key == boo::ESpecialKey::Right)
    {
        if ((mods & boo::EModifierKey::Shift) != boo::EModifierKey::None)
        {
            size_t len = UTF8Iterator(m_textStr.cbegin()).countTo(m_textStr.cend());
            if (m_cursorPos < len)
            {
                size_t origPos = m_cursorPos;
                if (m_selectionCount)
                {
                    if (m_cursorPos == m_selectionStart)
                        setSelectionRange(m_cursorPos+1, m_selectionCount-1);
                    else
                        setSelectionRange(m_selectionStart, m_selectionCount+1);
                }
                else
                    setSelectionRange(m_cursorPos, 1);
                m_cursorPos = origPos + 1;
            }
        }
        else
        {
            if (m_selectionCount)
                m_cursorPos = m_selectionStart + m_selectionCount - 1;
            setCursorPos(m_cursorPos+1);
        }
    }
    else if (key == boo::ESpecialKey::Backspace)
    {
        if (m_selectionCount)
        {
            std::string newStr(m_textStr.cbegin(), (UTF8Iterator(m_textStr.cbegin()) + m_selectionStart).iter());
            newStr.append((UTF8Iterator(m_textStr.cbegin()) + m_selectionStart + m_selectionCount).iter(), m_textStr.cend());
            size_t selStart = m_selectionStart;
            setText(newStr);
            setCursorPos(selStart);
        }
        else if (m_cursorPos > 0)
        {
            std::string newStr(m_textStr.cbegin(), (UTF8Iterator(m_textStr.cbegin()) + (m_cursorPos-1)).iter());
            newStr.append((UTF8Iterator(m_textStr.cbegin()) + m_cursorPos).iter(), m_textStr.cend());
            setText(newStr);
            setCursorPos(m_cursorPos-1);
        }
    }
    else if (key == boo::ESpecialKey::Delete)
    {
        size_t len = UTF8Iterator(m_textStr.cbegin()).countTo(m_textStr.cend());
        if (m_selectionCount)
        {
            std::string newStr(m_textStr.cbegin(), (UTF8Iterator(m_textStr.cbegin()) + m_selectionStart).iter());
            newStr.append((UTF8Iterator(m_textStr.cbegin()) + m_selectionStart + m_selectionCount).iter(), m_textStr.cend());
            size_t selStart = m_selectionStart;
            setText(newStr);
            setCursorPos(selStart);
        }
        else if (m_cursorPos < len)
        {
            std::string newStr(m_textStr.cbegin(), (UTF8Iterator(m_textStr.cbegin()) + m_cursorPos).iter());
            newStr.append((UTF8Iterator(m_textStr.cbegin()) + (m_cursorPos+1)).iter(), m_textStr.cend());
            setText(newStr);
            setCursorPos(m_cursorPos);
        }
    }
}
    
bool TextField::hasMarkedText() const
{
    std::unique_lock<std::recursive_mutex> lk(m_textInputLk);
    return m_deferredMarkStr.size() != 0;
}
std::pair<int,int> TextField::markedRange() const
{
    std::unique_lock<std::recursive_mutex> lk(m_textInputLk);
    if (m_deferredMarkStr.empty())
        return {-1, 0};
    return {m_cursorPos, UTF8Iterator(m_deferredMarkStr.cbegin()).countTo(m_deferredMarkStr.cend())};
}
std::pair<int,int> TextField::selectedRange() const
{
    std::unique_lock<std::recursive_mutex> lk(m_textInputLk);
    if (!m_deferredSelectionCount)
        return {-1, 0};
    return {m_deferredSelectionStart, m_deferredSelectionCount};
}
void TextField::setMarkedText(const std::string& str,
                              const std::pair<int,int>& selectedRange,
                              const std::pair<int,int>& replacementRange)
{
    std::unique_lock<std::recursive_mutex> lk(m_textInputLk);
    m_deferredMarkStr = SanitizeUTF8TextLine(str.data(), str.size());
    m_deferredMarkSelStart = selectedRange.first;
    m_deferredMarkSelCount = selectedRange.second;
    m_deferredMarkReplStart = replacementRange.first;
    m_deferredMarkReplCount = replacementRange.second;
    m_hasMarkSet = true;
}
void TextField::unmarkText()
{
    std::unique_lock<std::recursive_mutex> lk(m_textInputLk);
    m_deferredMarkStr.clear();
    m_deferredMarkReplStart = 0;
    m_deferredMarkReplCount = 0;
    m_deferredMarkSelStart = 0;
    m_deferredMarkSelCount = 0;
    m_hasMarkSet = true;
}

std::string TextField::substringForRange(const std::pair<int,int>& range,
                                         std::pair<int,int>& actualRange) const
{
    std::unique_lock<std::recursive_mutex> lk(m_textInputLk);
    UTF8Iterator begin(m_deferredTextStr.cbegin());
    size_t curLen = UTF8Iterator(m_deferredTextStr.cbegin()).countTo(m_deferredTextStr.cend());
    if (range.first >= curLen)
        return std::string();
    begin += range.first;
    size_t endIdx = std::min(size_t(range.first + range.second), curLen);
    UTF8Iterator end(m_deferredTextStr.cbegin());
    end += endIdx;
    actualRange.first = range.first;
    actualRange.second = endIdx;
    return std::string(begin.iter(), end.iter());
}
void TextField::insertText(const std::string& str, const std::pair<int,int>& range)
{
    std::string saniStr = SanitizeUTF8TextLine(str.data(), str.size());
    
    std::unique_lock<std::recursive_mutex> lk(m_textInputLk);
    size_t curLen = UTF8Iterator(m_deferredTextStr.cbegin()).countTo(m_deferredTextStr.cend());
    if (range.first < 0 || range.first >= curLen)
    {
        size_t beginPos = m_deferredCursorPos;
        if (m_selectionCount)
            beginPos = m_selectionCount;
        beginPos = std::min(beginPos, curLen);
        std::string newStr(m_deferredTextStr.cbegin(), (UTF8Iterator(m_deferredTextStr.cbegin())+beginPos).iter());
        newStr += saniStr;
        size_t newPos = UTF8Iterator(newStr.cbegin()).countTo(newStr.cend());
        newStr += std::string((UTF8Iterator(m_deferredTextStr.cbegin())+beginPos).iter(), m_deferredTextStr.cend());
        setText(newStr);
        setCursorPos(newPos);
        unmarkText();
        return;
    }
    
    std::string newStr(m_deferredTextStr.cbegin(), (UTF8Iterator(m_deferredTextStr.cbegin()) + range.first).iter());
    newStr += saniStr;
    size_t newSel = UTF8Iterator(newStr.cbegin()).countTo(newStr.cend());
    size_t endIdx = range.first + range.second;
    if (endIdx >= newSel)
        endIdx = newSel - 1;
    newStr.append((UTF8Iterator(m_deferredTextStr.cbegin()) + endIdx).iter(), m_deferredTextStr.cend());
    setText(newStr);
    setCursorPos(newSel);
    unmarkText();
}
int TextField::characterIndexAtPoint(const boo::SWindowCoord& point) const
{
    std::unique_lock<std::recursive_mutex> lk(m_textInputLk);
    return m_text->reverseSelectGlyph(point.pixel[0]);
}
boo::SWindowRect TextField::rectForCharacterRange(const std::pair<int,int>& range,
                                                  std::pair<int,int>& actualRange) const
{
    std::unique_lock<std::recursive_mutex> lk(m_textInputLk);
    UTF8Iterator begin(m_textStr.cbegin());
    size_t curLen = UTF8Iterator(m_textStr.cbegin()).countTo(m_textStr.cend());
    if (range.first >= curLen)
    {
        const std::vector<TextView::RenderGlyph>& glyphs = m_text->accessGlyphs();
        const TextView::RenderGlyph& g = glyphs.back();
        return {subRect().location[0] + int(g.m_pos[3][0]), subRect().location[1] + int(g.m_pos[3][1]), 0, 0};
    }
    begin += range.first;
    size_t endIdx = std::min(size_t(range.first + range.second), curLen);
    UTF8Iterator end(m_textStr.cbegin());
    end += endIdx;
    actualRange.first = range.first;
    actualRange.second = endIdx;
    const std::vector<TextView::RenderGlyph>& glyphs = m_text->accessGlyphs();
    const TextView::RenderGlyph& g1 = glyphs[range.first];
    const TextView::RenderGlyph& g2 = glyphs[endIdx];
    return {subRect().location[0] + int(g1.m_pos[1][0]), subRect().location[1] + int(g1.m_pos[1][1]),
            int(g2.m_pos[3][0]-g1.m_pos[1][0]), int(g2.m_pos[0][1]-g1.m_pos[1][1])};
}

void TextField::think()
{
    ++m_cursorFrames;
    ++m_clickFrames;
    ++m_clickFrames2;
    ++m_errorFrames;

    if (m_error && m_errorFrames <= 360)
    {
        Zeus::CColor errMult;
        Zeus::CColor errBg;
        if (m_errorFrames < 300)
        {
            errMult = m_viewVertBlock.m_color;
            errBg = rootView().themeData().tooltipBackground() * m_viewVertBlock.m_color;
        }
        else if (m_errorFrames >= 360)
        {
            errMult = Zeus::CColor::skClear;
            errBg = rootView().themeData().tooltipBackground();
            errBg[3] = 0.0;
        }
        else
        {
            float t = (m_errorFrames - 300) / 60.0;
            errMult = Zeus::CColor::lerp(m_viewVertBlock.m_color, Zeus::CColor::skClear, t);
            errBg = Zeus::CColor::lerp(rootView().themeData().tooltipBackground() * m_viewVertBlock.m_color,
                                       Zeus::CColor::skClear, t);
        }
        for (size_t i=32 ; i<41 ; ++i)
            m_verts[i].m_color = errBg;
        m_vertsBinding.load(m_verts, sizeof(m_verts));

        m_errText->setMultiplyColor(errMult);
    }
    
    std::unique_lock<std::recursive_mutex> lk(m_textInputLk);
    _setText();
    _setSelectionRange();
    _clearSelectionRange();
    _setCursorPos();
    _setMarkedText();
}

void TextField::setActive(bool active)
{
    std::unique_lock<std::recursive_mutex> lk(m_textInputLk);
    m_active = active;
    if (!active)
    {
        clearSelectionRange();
        rootView().window()->claimKeyboardFocus(nullptr);
    }
    else if (!m_selectionCount)
    {
        size_t len = UTF8Iterator(m_textStr.cbegin()).countTo(m_textStr.cend());
        setSelectionRange(0, len);
    }
}
    
void TextField::_reallySetCursorPos(size_t pos)
{
    float pf = rootView().viewRes().pixelFactor();
    int offset1 = 4 * pf + m_text->queryReverseAdvance(pos);
    int offset2 = offset1 + 2 * pf;
    const Zeus::CColor& selColor = rootView().viewRes().themeData().textfieldSelection();
    m_verts[28].m_pos.assign(offset1, 18 * pf, 0);
    m_verts[28].m_color = selColor;
    m_verts[29].m_pos.assign(offset1, 4 * pf, 0);
    m_verts[29].m_color = selColor;
    m_verts[30].m_pos.assign(offset2, 18 * pf, 0);
    m_verts[30].m_color = selColor;
    m_verts[31].m_pos.assign(offset2, 4 * pf, 0);
    m_verts[31].m_color = selColor;
    m_vertsBinding.load(m_verts, sizeof(m_verts));
    
    int focusRect[2] = {subRect().location[0] + offset1, subRect().location[1]};
    rootView().window()->claimKeyboardFocus(focusRect);
}
    
void TextField::_setCursorPos()
{
    if (m_hasCursorSet)
    {
        m_hasSelectionClear = true;
        _clearSelectionRange();
        m_cursorPos = std::min(m_deferredCursorPos, UTF8Iterator(m_textStr.cbegin()).countTo(m_textStr.cend()));
        m_deferredCursorPos = m_cursorPos;
        m_cursorFrames = 0;
        _reallySetCursorPos(m_cursorPos);
        m_hasCursorSet = false;
    }
}

void TextField::setCursorPos(size_t pos)
{
    std::unique_lock<std::recursive_mutex> lk(m_textInputLk);
    m_deferredCursorPos = pos;
    m_hasCursorSet = true;
}

void TextField::setErrorState(const std::string& message)
{
    m_error = true;
    if (m_selectionCount)
        _reallySetSelectionRange(m_selectionStart, m_selectionCount);
    else
        clearSelectionRange();
    refreshBg();

    m_errText.reset(new TextView(rootView().viewRes(), *this, rootView().viewRes().m_mainFont));
    m_errText->typesetGlyphs(message, rootView().themeData().uiText());

    updateSize();
    m_errorFrames = 0;
}

void TextField::clearErrorState()
{
    m_error = false;
    if (m_selectionCount)
        _reallySetSelectionRange(m_selectionStart, m_selectionCount);
    else
        clearSelectionRange();
    refreshBg();
    m_errText.reset();
    m_errorFrames = 360;
}
    
void TextField::_reallySetSelectionRange(size_t start, size_t len)
{
    ViewResources& res = rootView().viewRes();
    float pf = res.pixelFactor();
    int offset1 = 5 * pf;
    int offset2 = offset1;
    std::vector<TextView::RenderGlyph>& glyphs = m_text->accessGlyphs();
    offset1 += glyphs[start].m_pos[0][0];
    offset2 += glyphs[start+len-1].m_pos[2][0];
    const Zeus::CColor& selColor = rootView().themeData().selectedFieldText();
    const Zeus::CColor& deselColor = m_error ? rootView().themeData().uiText() :
                                               rootView().themeData().fieldText();

    for (size_t i=0 ; i<glyphs.size() ; ++i)
    {
        if (i >= start && i < start + len)
            glyphs[i].m_color = selColor;
        else
            glyphs[i].m_color = deselColor;
    }
    m_text->updateGlyphs();
    
    m_verts[28].m_pos.assign(offset1, 18 * pf, 0);
    m_verts[29].m_pos.assign(offset1, 4 * pf, 0);
    m_verts[30].m_pos.assign(offset2, 18 * pf, 0);
    m_verts[31].m_pos.assign(offset2, 4 * pf, 0);
    m_vertsBinding.load(m_verts, sizeof(m_verts));
    
    int focusRect[2] = {subRect().location[0] + offset1, subRect().location[1]};
    rootView().window()->claimKeyboardFocus(focusRect);
}
    
void TextField::_reallySetMarkRange(size_t start, size_t len)
{
    ViewResources& res = rootView().viewRes();
    float pf = res.pixelFactor();
    int offset1 = 5 * pf;
    int offset2 = offset1;
    std::vector<TextView::RenderGlyph>& glyphs = m_text->accessGlyphs();
    offset1 += glyphs[start].m_pos[0][0];
    offset2 += glyphs[start+len-1].m_pos[2][0];
    
    const Zeus::CColor& selColor = rootView().themeData().textfieldMarkSelection();
    m_verts[28].m_pos.assign(offset1, 18 * pf, 0);
    m_verts[28].m_color = selColor;
    m_verts[29].m_pos.assign(offset1, 4 * pf, 0);
    m_verts[29].m_color = selColor;
    m_verts[30].m_pos.assign(offset2, 18 * pf, 0);
    m_verts[30].m_color = selColor;
    m_verts[31].m_pos.assign(offset2, 4 * pf, 0);
    m_verts[31].m_color = selColor;
    m_vertsBinding.load(m_verts, sizeof(m_verts));
    
    int focusRect[2] = {subRect().location[0] + offset1, subRect().location[1]};
    rootView().window()->claimKeyboardFocus(focusRect);
}

void TextField::_setSelectionRange()
{
    if (m_hasSelectionSet)
    {
        size_t len = UTF8Iterator(m_textStr.cbegin()).countTo(m_textStr.cend());
        m_selectionStart = std::min(m_deferredSelectionStart, len-1);
        m_deferredSelectionStart = m_selectionStart;
        m_selectionCount = std::min(m_deferredSelectionCount, len-m_selectionStart);
        m_deferredSelectionCount = m_selectionCount;
        _reallySetSelectionRange(m_selectionStart, m_selectionCount);
        m_hasSelectionSet = false;
    }
}
    
void TextField::setSelectionRange(size_t start, size_t count)
{
    std::unique_lock<std::recursive_mutex> lk(m_textInputLk);

    if (!count)
    {
        setCursorPos(start);
        return;
    }

    m_deferredSelectionStart = start;
    m_deferredSelectionCount = count;
    m_hasSelectionSet = true;
    m_hasSelectionClear = false;
}

void TextField::_clearSelectionRange()
{
    if (m_hasSelectionClear)
    {
        m_selectionStart = 0;
        m_selectionCount = 0;
        
        const Zeus::CColor& deselColor = m_error ? rootView().themeData().uiText() :
                                                   rootView().themeData().fieldText();

        std::vector<TextView::RenderGlyph>& glyphs = m_text->accessGlyphs();
        for (size_t i=0 ; i<glyphs.size() ; ++i)
            glyphs[i].m_color = deselColor;
        m_text->updateGlyphs();
        
        m_hasSelectionClear = false;
    }
}
    
void TextField::clearSelectionRange()
{
    std::unique_lock<std::recursive_mutex> lk(m_textInputLk);
    m_deferredSelectionStart = 0;
    m_deferredSelectionCount = 0;
    m_hasSelectionClear = true;
    m_hasSelectionSet = false;
}

void TextField::resized(const boo::SWindowRect& root, const boo::SWindowRect& sub)
{
    float pf = rootView().viewRes().pixelFactor();
    int width = sub.size[0];
    int height = 20 * pf;
    boo::SWindowRect newRect = sub;
    newRect.size[1] = height;
    View::resized(root, newRect);

    m_verts[0].m_pos.assign(1, height+1, 0);
    m_verts[1].m_pos.assign(1, 1, 0);
    m_verts[2].m_pos.assign(width+1, height+1, 0);
    m_verts[3].m_pos.assign(width+1, 1, 0);
    m_verts[4].m_pos.assign(width+1, 1, 0);

    m_verts[5].m_pos.assign(1, height+1, 0);
    m_verts[6].m_pos.assign(1, height+1, 0);
    m_verts[7].m_pos.assign(0, height+1, 0);
    m_verts[8].m_pos.assign(1, 1, 0);
    m_verts[9].m_pos.assign(0, 1, 0);
    m_verts[10].m_pos.assign(0, 1, 0);

    m_verts[11].m_pos.assign(width+2, height+1, 0);
    m_verts[12].m_pos.assign(width+2, height+1, 0);
    m_verts[13].m_pos.assign(width+1, height+1, 0);
    m_verts[14].m_pos.assign(width+2, 1, 0);
    m_verts[15].m_pos.assign(width+1, 1, 0);
    m_verts[16].m_pos.assign(width+1, 1, 0);

    m_verts[17].m_pos.assign(1, height+2, 0);
    m_verts[18].m_pos.assign(1, height+2, 0);
    m_verts[19].m_pos.assign(1, height+1, 0);
    m_verts[20].m_pos.assign(width+1, height+2, 0);
    m_verts[21].m_pos.assign(width+1, height+1, 0);
    m_verts[22].m_pos.assign(width+1, height+1, 0);

    m_verts[23].m_pos.assign(1, 1, 0);
    m_verts[24].m_pos.assign(1, 1, 0);
    m_verts[25].m_pos.assign(1, 0, 0);
    m_verts[26].m_pos.assign(width+1, 1, 0);
    m_verts[27].m_pos.assign(width+1, 0, 0);

    if (m_error)
    {
        boo::SWindowRect errRect = sub;
        errRect.location[1] -= 16 * pf;
        errRect.location[0] += 5 * pf;
        m_errText->resized(root, errRect);

        int eX = 0;
        int eY = -22 * pf;
        int eWidth = m_errText->nominalWidth() + 10 * pf;
        int eHeight = 20 * pf;
        m_verts[32].m_pos.assign(eX, eY + eHeight, 0);
        m_verts[33].m_pos.assign(eX, eY, 0);
        m_verts[34].m_pos.assign(eX + eWidth, eY + eHeight, 0);
        m_verts[35].m_pos.assign(eX + eWidth, eY, 0);
        m_verts[36] = m_verts[35];
        m_verts[37].m_pos.assign(eX + 7 * pf, eY + eHeight + 7 * pf, 0);
        m_verts[38] = m_verts[37];
        m_verts[39].m_pos.assign(eX, eY + eHeight, 0);
        m_verts[40].m_pos.assign(eX + 14 * pf, eY + eHeight, 0);
        for (size_t i=32 ; i<41 ; ++i)
            m_verts[i].m_color = Zeus::CColor::skClear;
    }

    m_vertsBinding.load(m_verts, sizeof(m_verts));

    m_nomWidth = width;
    m_nomHeight = height;

    boo::SWindowRect textRect = sub;
    textRect.location[0] += 5 * pf;
    textRect.location[1] += 7 * pf;
    m_text->resized(root, textRect);
}

void TextField::draw(boo::IGraphicsCommandQueue* gfxQ)
{
    View::draw(gfxQ);
    gfxQ->setShaderDataBinding(m_vertsBinding);
    gfxQ->draw(0, 28);
    if (m_active)
    {
        if (!m_selectionCount && !m_markSelCount)
        {
            if (m_cursorFrames % 60 < 30)
                gfxQ->draw(28, 4);
        }
        else
            gfxQ->draw(28, 4);
    }

    if (m_error)
    {
        gfxQ->draw(32, 9);
        m_errText->draw(gfxQ);
    }

    m_text->draw(gfxQ);
}

}
