#include "Specter/TextField.hpp"
#include "Specter/RootView.hpp"
#include "Specter/ViewResources.hpp"

namespace Specter
{

TextField::TextField(ViewResources& res, View& parentView, IStringBinding* strBind)
: Control(res, parentView, strBind)
{
    m_bVertsBuf = res.m_factory->newDynamicBuffer(boo::BufferUse::Vertex, sizeof(SolidShaderVert), 32);

    if (!res.m_viewRes.m_texVtxFmt)
    {
        boo::VertexElementDescriptor vdescs[] =
        {
            {m_bVertsBuf, nullptr, boo::VertexSemantic::Position4},
            {m_bVertsBuf, nullptr, boo::VertexSemantic::Color}
        };
        m_bVtxFmt = res.m_factory->newVertexFormat(2, vdescs);
        boo::IGraphicsBuffer* bufs[] = {m_viewVertBlockBuf};
        m_bShaderBinding = res.m_factory->newShaderDataBinding(res.m_viewRes.m_solidShader,
                                                               m_bVtxFmt, m_bVertsBuf, nullptr,
                                                               nullptr, 1, bufs, 0, nullptr);
    }
    else
    {
        boo::IGraphicsBuffer* bufs[] = {m_viewVertBlockBuf};
        m_bShaderBinding = res.m_factory->newShaderDataBinding(res.m_viewRes.m_solidShader,
                                                               res.m_viewRes.m_texVtxFmt,
                                                               m_bVertsBuf, nullptr,
                                                               nullptr, 1, bufs, 0, nullptr);
    }
    commitResources(res);

    m_verts[0].m_color = rootView().themeData().textfield1Inactive();
    m_verts[1].m_color = rootView().themeData().textfield2Inactive();
    m_verts[2].m_color = rootView().themeData().textfield1Inactive();
    m_verts[3].m_color = rootView().themeData().textfield2Inactive();
    m_verts[4].m_color = rootView().themeData().textfield2Inactive();
    for (int i=5 ; i<28 ; ++i)
        m_verts[i].m_color = res.themeData().textfield2Inactive();
    for (int i=28 ; i<32 ; ++i)
        m_verts[i].m_color = res.themeData().textfieldSelection();
    m_bVertsBuf->load(m_verts, sizeof(m_verts));

    m_text.reset(new TextView(res, *this, res.m_mainFont, TextView::Alignment::Left, 1024));
    setText("Test");
}

void TextField::setText(const std::string& str)
{
    clearSelectionRange();
    auto it = str.cbegin();
    for (; it != str.cend() ; ++it)
        if (*it == '\n')
            break;
    m_textStr.assign(str.cbegin(), it);
    m_text->typesetGlyphs(m_textStr, rootView().themeData().fieldText());
}

void TextField::setInactive()
{
    m_verts[0].m_color = rootView().themeData().textfield1Inactive();
    m_verts[1].m_color = rootView().themeData().textfield2Inactive();
    m_verts[2].m_color = rootView().themeData().textfield1Inactive();
    m_verts[3].m_color = rootView().themeData().textfield2Inactive();
    m_verts[4].m_color = rootView().themeData().textfield2Inactive();
    m_bVertsBuf->load(m_verts, sizeof(m_verts));
}

void TextField::setHover()
{
    m_verts[0].m_color = rootView().themeData().textfield1Hover();
    m_verts[1].m_color = rootView().themeData().textfield2Hover();
    m_verts[2].m_color = rootView().themeData().textfield1Hover();
    m_verts[3].m_color = rootView().themeData().textfield2Hover();
    m_verts[4].m_color = rootView().themeData().textfield2Hover();
    m_bVertsBuf->load(m_verts, sizeof(m_verts));
}

void TextField::setDisabled()
{
    m_verts[0].m_color = rootView().themeData().textfield1Disabled();
    m_verts[1].m_color = rootView().themeData().textfield2Disabled();
    m_verts[2].m_color = rootView().themeData().textfield1Disabled();
    m_verts[3].m_color = rootView().themeData().textfield2Disabled();
    m_verts[4].m_color = rootView().themeData().textfield2Disabled();
    m_bVertsBuf->load(m_verts, sizeof(m_verts));
}

void TextField::mouseDown(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod)
{
    if (!m_active)
    {
        rootView().setActiveTextView(this);
        if (!m_selectionCount)
            setSelectionRange(0, m_textStr.size());
    }
    else
        setCursorPos(m_text->reverseSelectGlyph(coord.pixel[0] - m_text->subRect().location[0]));
}

void TextField::mouseUp(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mod)
{
}

void TextField::mouseMove(const boo::SWindowCoord& coord)
{
}

void TextField::mouseEnter(const boo::SWindowCoord& coord)
{
    setHover();
    rootView().window()->setCursor(boo::EMouseCursor::IBeam);
}

void TextField::mouseLeave(const boo::SWindowCoord& coord)
{
    setInactive();
    rootView().window()->setCursor(boo::EMouseCursor::Pointer);
}

void TextField::charKeyDown(unsigned long charCode, boo::EModifierKey mods, bool isRepeat)
{
    if (m_selectionCount)
    {
        std::string newStr(m_textStr.cbegin(), m_textStr.cbegin() + m_selectionStart);
        utf8proc_uint8_t theChar[5] = {};
        utf8proc_ssize_t sz = utf8proc_encode_char(charCode, theChar);
        if (sz > 0)
            newStr += (char*)theChar;
        newStr.append(m_textStr.cbegin() + m_selectionStart + m_selectionCount, m_textStr.cend());
        setText(newStr);
    }
    else
    {
        std::string newStr(m_textStr.cbegin(), m_textStr.cbegin() + m_cursorPos);
        utf8proc_uint8_t theChar[5] = {};
        utf8proc_ssize_t sz = utf8proc_encode_char(charCode, theChar);
        if (sz > 0)
            newStr += (char*)theChar;
        newStr.append(m_textStr.cbegin() + m_cursorPos, m_textStr.cend());
        setText(newStr);
    }
}

void TextField::specialKeyDown(boo::ESpecialKey key, boo::EModifierKey mods, bool isRepeat)
{
    if (key == boo::ESpecialKey::Left)
    {
        if (m_selectionCount)
            m_cursorPos = m_selectionStart;
        setCursorPos(m_cursorPos==0 ? 0 : (m_cursorPos-1));
    }
    else if (key == boo::ESpecialKey::Right)
    {
        if (m_selectionCount)
            m_cursorPos = m_selectionStart + m_selectionCount - 1;
        setCursorPos(m_cursorPos+1);
    }
}

void TextField::think()
{
    ++m_cursorFrames;
}

void TextField::setActive(bool active)
{
    m_active = active;
    if (!active)
        clearSelectionRange();
}

void TextField::setCursorPos(size_t pos)
{
    clearSelectionRange();
    m_cursorPos = std::min(pos, m_textStr.size());
    m_cursorFrames = 0;

    float pf = rootView().viewRes().pixelFactor();
    int offset1 = 4 * pf + m_text->queryReverseAdvance(m_cursorPos);
    int offset2 = offset1 + 2 * pf;
    m_verts[28].m_pos.assign(offset1, 18 * pf, 0);
    m_verts[29].m_pos.assign(offset1, 4 * pf, 0);
    m_verts[30].m_pos.assign(offset2, 18 * pf, 0);
    m_verts[31].m_pos.assign(offset2, 4 * pf, 0);
    m_bVertsBuf->load(m_verts, sizeof(m_verts));
}

void TextField::setSelectionRange(size_t start, size_t count)
{
    m_selectionStart = std::min(start, m_textStr.size()-1);
    m_selectionCount = std::min(count, m_textStr.size()-m_selectionStart);

    ViewResources& res = rootView().viewRes();
    float pf = res.pixelFactor();
    int offset1 = 5 * pf;
    int offset2 = offset1;
    std::vector<TextView::RenderGlyph>& glyphs = m_text->accessGlyphs();
    offset1 += glyphs[m_selectionStart].m_pos[0][0];
    offset2 += glyphs[m_selectionStart+m_selectionCount-1].m_pos[2][0];
    for (size_t i=0 ; i<glyphs.size() ; ++i)
    {
        if (i >= m_selectionStart && i< m_selectionStart + m_selectionCount)
            glyphs[i].m_color = rootView().themeData().selectedFieldText();
        else
            glyphs[i].m_color = rootView().themeData().fieldText();
    }
    m_text->updateGlyphs();

    m_verts[28].m_pos.assign(offset1, 18 * pf, 0);
    m_verts[29].m_pos.assign(offset1, 4 * pf, 0);
    m_verts[30].m_pos.assign(offset2, 18 * pf, 0);
    m_verts[31].m_pos.assign(offset2, 4 * pf, 0);
    m_bVertsBuf->load(m_verts, sizeof(m_verts));
}

void TextField::clearSelectionRange()
{
    m_selectionStart = 0;
    m_selectionCount = 0;

    std::vector<TextView::RenderGlyph>& glyphs = m_text->accessGlyphs();
    for (size_t i=0 ; i<glyphs.size() ; ++i)
        glyphs[i].m_color = rootView().themeData().fieldText();
    m_text->updateGlyphs();
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

    m_bVertsBuf->load(m_verts, sizeof(m_verts));

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
    gfxQ->setShaderDataBinding(m_bShaderBinding);
    gfxQ->setDrawPrimitive(boo::Primitive::TriStrips);
    gfxQ->draw(0, 28);
    if (m_active)
    {
        if (!m_selectionCount)
        {
            if (m_cursorFrames % 60 < 30)
                gfxQ->draw(28, 4);
        }
        else
            gfxQ->draw(28, 4);
    }
    m_text->draw(gfxQ);
}

}
