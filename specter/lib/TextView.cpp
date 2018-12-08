#include "specter/RootView.hpp"
#include "specter/TextView.hpp"
#include "specter/ViewResources.hpp"
#include "utf8proc.h"
#include "hecl/Pipeline.hpp"

#include <freetype/internal/internal.h>
#include <freetype/internal/ftobjs.h>

namespace specter {
static logvisor::Module Log("specter::TextView");

void TextView::Resources::init(boo::IGraphicsDataFactory::Context& ctx, FontCache* fcache) {
  m_fcache = fcache;
  m_regular = hecl::conv->convert(ctx, Shader_SpecterTextViewShader{});
  m_subpixel = hecl::conv->convert(ctx, Shader_SpecterTextViewShaderSubpixel{});
}

void TextView::_commitResources(size_t capacity) {
  auto& res = rootView().viewRes();
  auto fontTex = m_fontAtlas.texture(res.m_factory);
  View::commitResources(res, [&](boo::IGraphicsDataFactory::Context& ctx) {
    buildResources(ctx, res);

    if (capacity) {
      m_glyphBuf = res.m_textRes.m_glyphPool.allocateBlock(res.m_factory, capacity);

      boo::ObjToken<boo::IShaderPipeline> shader;
      if (m_fontAtlas.subpixel())
        shader = res.m_textRes.m_subpixel;
      else
        shader = res.m_textRes.m_regular;

      auto vBufInfo = m_glyphBuf.getBufferInfo();
      auto uBufInfo = m_viewVertBlockBuf.getBufferInfo();
      boo::ObjToken<boo::IGraphicsBuffer> uBufs[] = {uBufInfo.first.get()};
      size_t uBufOffs[] = {size_t(uBufInfo.second)};
      size_t uBufSizes[] = {sizeof(ViewBlock)};
      boo::ObjToken<boo::ITexture> texs[] = {fontTex.get()};

      m_shaderBinding = ctx.newShaderDataBinding(shader, {}, vBufInfo.first.get(), nullptr, 1, uBufs, nullptr, uBufOffs,
                                                 uBufSizes, 1, texs, nullptr, nullptr, 0, vBufInfo.second);
    }
    return true;
  });
}

TextView::TextView(ViewResources& res, View& parentView, const FontAtlas& font, Alignment align, size_t capacity)
: View(res, parentView), m_capacity(capacity), m_fontAtlas(font), m_align(align) {
  if (size_t(hecl::VertexBufferPool<RenderGlyph>::bucketCapacity()) < capacity)
    Log.report(logvisor::Fatal, "bucket overflow [%" PRISize "/%" PRISize "]", capacity,
               hecl::VertexBufferPool<RenderGlyph>::bucketCapacity());

  _commitResources(0);
}

TextView::TextView(ViewResources& res, View& parentView, FontTag font, Alignment align, size_t capacity)
: TextView(res, parentView, res.m_textRes.m_fcache->lookupAtlas(font), align, capacity) {}

TextView::RenderGlyph::RenderGlyph(int& adv, const FontAtlas::Glyph& glyph, const zeus::CColor& defaultColor) {
  m_pos[0].assign(adv + glyph.m_leftPadding, glyph.m_verticalOffset + glyph.m_height, 0.f);
  m_pos[1].assign(adv + glyph.m_leftPadding, glyph.m_verticalOffset, 0.f);
  m_pos[2].assign(adv + glyph.m_leftPadding + glyph.m_width, glyph.m_verticalOffset + glyph.m_height, 0.f);
  m_pos[3].assign(adv + glyph.m_leftPadding + glyph.m_width, glyph.m_verticalOffset, 0.f);
  m_uv[0].assign(glyph.m_uv[0], glyph.m_uv[1], glyph.m_layerFloat);
  m_uv[1].assign(glyph.m_uv[0], glyph.m_uv[3], glyph.m_layerFloat);
  m_uv[2].assign(glyph.m_uv[2], glyph.m_uv[1], glyph.m_layerFloat);
  m_uv[3].assign(glyph.m_uv[2], glyph.m_uv[3], glyph.m_layerFloat);
  m_color = defaultColor;
  adv += glyph.m_advance;
}

int TextView::DoKern(FT_Pos val, const FontAtlas& atlas) {
  if (!val)
    return 0;
  val = FT_MulFix(val, atlas.FT_Xscale());

  FT_Pos orig_x = val;

  /* we scale down kerning values for small ppem values */
  /* to avoid that rounding makes them too big.         */
  /* `25' has been determined heuristically.            */
  if (atlas.FT_XPPem() < 25)
    val = FT_MulDiv(orig_x, atlas.FT_XPPem(), 25);

  return FT_PIX_ROUND(val) >> 6;
}

void TextView::typesetGlyphs(std::string_view str, const zeus::CColor& defaultColor) {
  UTF8Iterator it(str.begin());
  size_t charLen = str.size() ? std::min(it.countTo(str.end()), m_capacity) : 0;
  if (charLen > m_curSize) {
    m_curSize = charLen;
    _commitResources(charLen);
  }

  uint32_t lCh = -1;
  m_glyphs.clear();
  m_glyphs.reserve(charLen);
  m_glyphInfo.clear();
  m_glyphInfo.reserve(charLen);
  int adv = 0;

  if (charLen) {
    for (; it.iter() < str.end(); ++it) {
      utf8proc_int32_t ch = *it;
      if (ch == -1) {
        Log.report(logvisor::Warning, "invalid UTF-8 char");
        break;
      }
      if (ch == '\n' || ch == '\0')
        break;

      const FontAtlas::Glyph* glyph = m_fontAtlas.lookupGlyph(ch);
      if (!glyph)
        continue;

      if (lCh != -1)
        adv += DoKern(m_fontAtlas.lookupKern(lCh, glyph->m_glyphIdx), m_fontAtlas);
      m_glyphs.emplace_back(adv, *glyph, defaultColor);
      m_glyphInfo.emplace_back(ch, glyph->m_width, glyph->m_height, adv);

      lCh = glyph->m_glyphIdx;

      if (m_glyphs.size() == m_capacity)
        break;
    }
  }

  if (m_align == Alignment::Right) {
    int adj = -adv;
    for (RenderGlyph& g : m_glyphs) {
      g.m_pos[0][0] += adj;
      g.m_pos[1][0] += adj;
      g.m_pos[2][0] += adj;
      g.m_pos[3][0] += adj;
    }
  } else if (m_align == Alignment::Center) {
    int adj = -adv / 2;
    for (RenderGlyph& g : m_glyphs) {
      g.m_pos[0][0] += adj;
      g.m_pos[1][0] += adj;
      g.m_pos[2][0] += adj;
      g.m_pos[3][0] += adj;
    }
  }

  m_width = adv;
  invalidateGlyphs();
  updateSize();
}

void TextView::typesetGlyphs(std::wstring_view str, const zeus::CColor& defaultColor) {
  size_t charLen = std::min(str.size(), m_capacity);
  if (charLen > m_curSize) {
    m_curSize = charLen;
    _commitResources(charLen);
  }

  uint32_t lCh = -1;
  m_glyphs.clear();
  m_glyphs.reserve(charLen);
  m_glyphInfo.clear();
  m_glyphInfo.reserve(charLen);
  int adv = 0;

  for (wchar_t ch : str) {
    if (ch == L'\n')
      break;

    const FontAtlas::Glyph* glyph = m_fontAtlas.lookupGlyph(ch);
    if (!glyph)
      continue;

    if (lCh != -1)
      adv += DoKern(m_fontAtlas.lookupKern(lCh, glyph->m_glyphIdx), m_fontAtlas);
    m_glyphs.emplace_back(adv, *glyph, defaultColor);
    m_glyphInfo.emplace_back(ch, glyph->m_width, glyph->m_height, adv);

    lCh = glyph->m_glyphIdx;

    if (m_glyphs.size() == m_capacity)
      break;
  }

  if (m_align == Alignment::Right) {
    int adj = -adv;
    for (RenderGlyph& g : m_glyphs) {
      g.m_pos[0][0] += adj;
      g.m_pos[1][0] += adj;
      g.m_pos[2][0] += adj;
      g.m_pos[3][0] += adj;
    }
  } else if (m_align == Alignment::Center) {
    int adj = -adv / 2;
    for (RenderGlyph& g : m_glyphs) {
      g.m_pos[0][0] += adj;
      g.m_pos[1][0] += adj;
      g.m_pos[2][0] += adj;
      g.m_pos[3][0] += adj;
    }
  }

  m_width = adv;
  invalidateGlyphs();
  updateSize();
}

void TextView::colorGlyphs(const zeus::CColor& newColor) {
  for (RenderGlyph& glyph : m_glyphs)
    glyph.m_color = newColor;
  invalidateGlyphs();
}

void TextView::colorGlyphsTypeOn(const zeus::CColor& newColor, float startInterval, float fadeTime) {}

void TextView::invalidateGlyphs() {
  if (m_glyphBuf) {
    RenderGlyph* out = m_glyphBuf.access();
    size_t i = 0;
    for (RenderGlyph& glyph : m_glyphs)
      out[i++] = glyph;
  }
}

void TextView::think() {}

void TextView::resized(const boo::SWindowRect& root, const boo::SWindowRect& sub) { View::resized(root, sub); }

void TextView::draw(boo::IGraphicsCommandQueue* gfxQ) {
  View::draw(gfxQ);
  if (m_glyphs.size()) {
    gfxQ->setShaderDataBinding(m_shaderBinding);
    gfxQ->drawInstances(0, 4, m_glyphs.size());
  }
}

std::pair<int, int> TextView::queryGlyphDimensions(size_t pos) const {
  if (pos >= m_glyphInfo.size())
    Log.report(logvisor::Fatal, "TextView::queryGlyphWidth(%" PRISize ") out of bounds: %" PRISize, pos,
               m_glyphInfo.size());

  return m_glyphInfo[pos].m_dims;
}

size_t TextView::reverseSelectGlyph(int x) const {
  size_t ret = 0;
  size_t idx = 1;
  int minDelta = abs(x);
  for (const RenderGlyphInfo& info : m_glyphInfo) {
    int thisDelta = abs(info.m_adv - x);
    if (thisDelta < minDelta) {
      minDelta = thisDelta;
      ret = idx;
    }
    ++idx;
  }
  return ret;
}

int TextView::queryReverseAdvance(size_t idx) const {
  if (idx > m_glyphInfo.size())
    Log.report(logvisor::Fatal, "TextView::queryReverseGlyph(%" PRISize ") out of inclusive bounds: %" PRISize, idx,
               m_glyphInfo.size());
  if (!idx)
    return 0;
  return m_glyphInfo[idx - 1].m_adv;
}

std::pair<size_t, size_t> TextView::queryWholeWordRange(size_t idx) const {
  if (idx > m_glyphInfo.size())
    Log.report(logvisor::Fatal, "TextView::queryWholeWordRange(%" PRISize ") out of inclusive bounds: %" PRISize, idx,
               m_glyphInfo.size());
  if (m_glyphInfo.empty())
    return {0, 0};

  if (idx == m_glyphInfo.size())
    --idx;

  size_t begin = idx;
  while (begin > 0 && !m_glyphInfo[begin - 1].m_space)
    --begin;

  size_t end = idx;
  while (end < m_glyphInfo.size() && !m_glyphInfo[end].m_space)
    ++end;

  return {begin, end - begin};
}

} // namespace specter
