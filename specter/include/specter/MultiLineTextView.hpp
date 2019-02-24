#pragma once

#include "View.hpp"
#include "TextView.hpp"
#include "FontCache.hpp"

namespace specter {

class MultiLineTextView : public View {
  ViewResources& m_viewSystem;
  std::vector<std::unique_ptr<TextView>> m_lines;
  const FontAtlas& m_fontAtlas;
  TextView::Alignment m_align;
  size_t m_lineCapacity;
  float m_lineHeight;
  int m_width;
  std::string LineWrap(std::string_view str, int wrap);
  std::wstring LineWrap(std::wstring_view str, int wrap);

public:
  MultiLineTextView(ViewResources& res, View& parentView, const FontAtlas& font,
                    TextView::Alignment align = TextView::Alignment::Left, size_t lineCapacity = 256,
                    float lineHeight = 1.0);
  MultiLineTextView(ViewResources& res, View& parentView, FontTag font,
                    TextView::Alignment align = TextView::Alignment::Left, size_t lineCapacity = 256,
                    float lineHeight = 1.0);

  void typesetGlyphs(std::string_view str, const zeus::CColor& defaultColor = zeus::skWhite, unsigned wrap = 0);
  void typesetGlyphs(std::wstring_view str, const zeus::CColor& defaultColor = zeus::skWhite,
                     unsigned wrap = 0);

  void colorGlyphs(const zeus::CColor& newColor);

  void setMultiplyColor(const zeus::CColor& color) {
    for (std::unique_ptr<TextView>& l : m_lines)
      l->setMultiplyColor(color);
  }

  void resized(const boo::SWindowRect& root, const boo::SWindowRect& sub);
  void draw(boo::IGraphicsCommandQueue* gfxQ);

  int nominalWidth() const { return m_width; }
  int nominalHeight() const { return (int(m_lineHeight * m_fontAtlas.FT_LineHeight()) >> 6) * m_lines.size(); }
};

} // namespace specter
