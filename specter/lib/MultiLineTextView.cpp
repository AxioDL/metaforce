#include "Specter/MultiLineTextView.hpp"
#include "Specter/ViewResources.hpp"

namespace Specter
{
static LogVisor::LogModule Log("Specter::MultiLineTextView");

MultiLineTextView::MultiLineTextView(ViewResources& res,
                                     View& parentView,
                                     const FontAtlas& font,
                                     size_t lineCapacity,
                                     float lineHeight)
: View(res, parentView),
  m_viewSystem(res),
  m_fontAtlas(font),
  m_lineCapacity(lineCapacity),
  m_lineHeight(lineHeight)
{
    commitResources(res);
}

MultiLineTextView::MultiLineTextView(ViewResources& res,
                                     View& parentView,
                                     FontTag font,
                                     size_t lineCapacity,
                                     float lineHeight)
: MultiLineTextView(res,
                    parentView,
                    res.m_textRes.m_fcache->lookupAtlas(font),
                    lineCapacity,
                    lineHeight) {}

void MultiLineTextView::typesetGlyphs(const std::string& str,
                                      const Zeus::CColor& defaultColor)
{
    m_lines.clear();
    size_t rem = str.size() + 1;
    const utf8proc_uint8_t* it = reinterpret_cast<const utf8proc_uint8_t*>(str.data());

    size_t lineCount = 0;
    while (rem)
    {
        utf8proc_int32_t ch;
        utf8proc_ssize_t sz = utf8proc_iterate(it, -1, &ch);
        if (sz < 0)
            Log.report(LogVisor::FatalError, "invalid UTF-8 char");
        if (ch == '\n' || ch == '\0')
            ++lineCount;
        rem -= sz;
        it += sz;
    }

    m_lines.reserve(lineCount);
    rem = str.size() + 1;
    it = reinterpret_cast<const utf8proc_uint8_t*>(str.data());
    const utf8proc_uint8_t* beginIt = it;

    while (rem)
    {
        utf8proc_int32_t ch;
        utf8proc_ssize_t sz = utf8proc_iterate(it, -1, &ch);
        if (ch == '\n' || ch == '\0')
        {
            m_lines.emplace_back(new TextView(m_viewSystem, *this, m_fontAtlas, m_lineCapacity));
            m_lines.back()->typesetGlyphs(std::string((char*)beginIt, it - beginIt), defaultColor);
            beginIt = it + 1;
        }
        rem -= sz;
        it += sz;
    }

    updateSize();
}

void MultiLineTextView::typesetGlyphs(const std::wstring& str,
                                      const Zeus::CColor& defaultColor)
{
    m_lines.clear();
    size_t rem = str.size() + 1;
    auto it = str.cbegin();

    size_t lineCount = 0;
    while (rem)
    {
        if (*it == L'\n' || *it == L'\0')
            ++lineCount;
        --rem;
        ++it;
    }

    m_lines.reserve(lineCount);
    rem = str.size() + 1;
    it = str.cbegin();
    auto beginIt = it;

    while (rem)
    {
        if (*it == L'\n' || *it == L'\0')
        {
            m_lines.emplace_back(new TextView(m_viewSystem, *this, m_fontAtlas, m_lineCapacity));
            m_lines.back()->typesetGlyphs(std::wstring(beginIt, it), defaultColor);
            beginIt = it + 1;
        }
        --rem;
        ++it;
    }

    updateSize();
}

void MultiLineTextView::resized(const boo::SWindowRect& root, const boo::SWindowRect& sub)
{
    View::resized(root, sub);
    unsigned lHeight = unsigned(m_lineHeight * m_fontAtlas.FT_LineHeight()) >> 6;
    unsigned decumHeight = lHeight * m_lines.size();
    boo::SWindowRect tsub = sub;
    tsub.location[1] += decumHeight;
    tsub.size[1] = 10;
    for (std::unique_ptr<TextView>& tv : m_lines)
    {
        tsub.location[1] -= lHeight;
        tv->resized(root, tsub);
    }
}

void MultiLineTextView::draw(boo::IGraphicsCommandQueue* gfxQ)
{
    View::draw(gfxQ);
    for (std::unique_ptr<TextView>& tv : m_lines)
        tv->draw(gfxQ);
}

}
