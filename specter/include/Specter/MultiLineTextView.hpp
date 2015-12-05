#ifndef SPECTER_MULTILINETEXTVIEW_HPP
#define SPECTER_MULTILINETEXTVIEW_HPP

#include "View.hpp"
#include "TextView.hpp"
#include "FontCache.hpp"

namespace Specter
{

class MultiLineTextView : public View
{
    ViewResources& m_viewSystem;
    std::vector<std::unique_ptr<TextView>> m_lines;
    const FontAtlas& m_fontAtlas;
    size_t m_lineCapacity;
    float m_lineHeight;
public:
    MultiLineTextView(ViewResources& res, View& parentView, const FontAtlas& font, size_t lineCapacity=256, float lineHeight=1.0);
    MultiLineTextView(ViewResources& res, View& parentView, FontTag font, size_t lineCapacity=256, float lineHeight=1.0);

    void typesetGlyphs(const std::string& str,
                       const Zeus::CColor& defaultColor=Zeus::CColor::skWhite);
    void typesetGlyphs(const std::wstring& str,
                       const Zeus::CColor& defaultColor=Zeus::CColor::skWhite);

    void resized(const boo::SWindowRect& rootView, const boo::SWindowRect& sub);
    void draw(boo::IGraphicsCommandQueue* gfxQ);
};

}

#endif // SPECTER_MULTILINETEXTVIEW_HPP
