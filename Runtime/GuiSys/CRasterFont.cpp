#include "Runtime/GuiSys/CRasterFont.hpp"

#include <algorithm>

#include "Runtime/CSimplePool.hpp"
#include "Runtime/Graphics/CGX.hpp"
#include "Runtime/Graphics/CTexture.hpp"
#include "Runtime/GuiSys/CDrawStringOptions.hpp"
#include "Runtime/GuiSys/CTextRenderBuffer.hpp"

namespace metaforce {
CRasterFont::CRasterFont(metaforce::CInputStream& in, metaforce::IObjectStore& store) {
  u32 magic = 0;
  in.Get(reinterpret_cast<u8*>(&magic), 4);
  if (magic != SBIG('FONT'))
    return;

  u32 version = in.ReadLong();
  x4_monoWidth = in.ReadLong();
  x8_monoHeight = in.ReadLong();

  if (version >= 1)
    x8c_baseline = in.ReadLong();
  else
    x8c_baseline = x8_monoHeight;

  if (version >= 2)
    x90_lineMargin = in.ReadLong();

  bool tmp1 = in.ReadBool();
  bool tmp2 = in.ReadBool();

  u32 tmp3 = in.ReadLong();
  u32 tmp4 = in.ReadLong();
  std::string name = in.Get<std::string>();
  u32 txtrId = (version == 5 ? in.ReadLongLong() : in.ReadLong());
  x30_fontInfo = CFontInfo(tmp1, tmp2, tmp3, tmp4, name.c_str());
  x80_texture = store.GetObj({FOURCC('TXTR'), txtrId});
  x2c_mode = CTexture::EFontType(in.ReadLong());

  u32 glyphCount = in.ReadLong();
  xc_glyphs.reserve(glyphCount);

  for (u32 i = 0; i < glyphCount; ++i) {
    char16_t chr = in.ReadShort();
    float startU = in.ReadFloat();
    float startV = in.ReadFloat();
    float endU = in.ReadFloat();
    float endV = in.ReadFloat();
    s32 layer = 0;
    s32 a, b, c, cellWidth, cellHeight, baseline, kernStart;
    if (version < 4) {
      a = in.ReadInt32();
      b = in.ReadInt32();
      c = in.ReadInt32();
      cellWidth = in.ReadInt32();
      cellHeight = in.ReadInt32();
      baseline = in.ReadInt32();
      kernStart = in.ReadInt32();
    } else {
      layer = in.ReadInt8();
      a = in.ReadInt8();
      b = in.ReadInt8();
      c = in.ReadInt8();
      cellWidth = in.ReadInt8();
      cellHeight = in.ReadInt8();
      baseline = in.ReadInt8();
      kernStart = in.ReadInt16();
    }
    xc_glyphs.emplace_back(
        chr, CGlyph(a, b, c, startU, startV, endU, endV, cellWidth, cellHeight, baseline, kernStart, layer));
  }

  std::sort(xc_glyphs.begin(), xc_glyphs.end(), [=](auto& a, auto& b) -> bool { return a.first < b.first; });

  u32 kernCount = in.ReadLong();
  x1c_kerning.reserve(kernCount);

  for (u32 i = 0; i < kernCount; ++i) {
    char16_t first = in.ReadShort();
    char16_t second = in.ReadShort();
    s32 howMuch = in.ReadInt32();
    x1c_kerning.emplace_back(first, second, howMuch);
  }

  if (magic == SBIG('FONT') && version <= 4)
    x0_initialized = true;
}

const CGlyph* CRasterFont::InternalGetGlyph(char16_t chr) const {
  const auto iter =
      std::find_if(xc_glyphs.cbegin(), xc_glyphs.cend(), [chr](const auto& entry) { return entry.first == chr; });

  if (iter == xc_glyphs.cend()) {
    return nullptr;
  }

  return &iter->second;
}

void CRasterFont::SinglePassDrawString(const CDrawStringOptions& opts, int x, int y, int& xout, int& yout,
                                       CTextRenderBuffer* renderBuf, const char16_t* str, s32 length) const {
  if (!x0_initialized)
    return;

  const char16_t* chr = str;
  const CGlyph* prevGlyph = nullptr;
  while (*chr != u'\0') {
    const CGlyph* glyph = GetGlyph(*chr);
    if (glyph) {
      if (opts.x0_direction == ETextDirection::Horizontal) {
        x += glyph->GetLeftPadding();

        if (prevGlyph != nullptr) {
          x += KernLookup(x1c_kerning, prevGlyph->GetKernStart(), *chr);
        }

        int left = 0;
        int top = 0;

        if (renderBuf) {
          left += x;
          top += y - glyph->GetBaseline();
          renderBuf->AddCharacter(zeus::CVector2i(left, top), *chr, opts.x4_colors[2]);
        }
        x += glyph->GetRightPadding() + glyph->GetAdvance();
      }
    }
    prevGlyph = glyph;
    chr++;
    if (length == -1)
      continue;

    if ((chr - str) >= length)
      break;
  }

  xout = x;
  yout = y;
}

void CRasterFont::DrawSpace(const CDrawStringOptions& opts, int x, int y, int& xout, int& yout, int len) const {
  if (opts.x0_direction != ETextDirection::Horizontal)
    return;

  xout = x + len;
  yout = y;
}

void CRasterFont::DrawString(const CDrawStringOptions& opts, int x, int y, int& xout, int& yout,
                             CTextRenderBuffer* renderBuf, const char16_t* str, int len) const {
  if (!x0_initialized)
    return;

  if (renderBuf != nullptr) {
    CGraphicsPalette pal(EPaletteFormat::RGB5A3, 4);
    u16* data = pal.Lock();
    data[0] = bswap16(zeus::CColor(0.f, 0.f, 0.f, 0.f).toRGB5A3());
    data[1] = bswap16(opts.x4_colors[0].toRGB5A3());
    data[2] = bswap16(opts.x4_colors[1].toRGB5A3());
    data[3] = bswap16(zeus::CColor(0.f, 0.f, 0.f, 0.f).toRGB5A3());
    pal.UnLock();
    renderBuf->AddPaletteChange(pal);
  }

  SinglePassDrawString(opts, x, y, xout, yout, renderBuf, str, len);
}

void CRasterFont::GetSize(const CDrawStringOptions& opts, int& width, int& height, const char16_t* str, int len) const {
  width = 0;
  height = 0;

  const char16_t* chr = str;
  const CGlyph* prevGlyph = nullptr;
  int prevWidth = 0;
  while (*chr != u'\0') {
    const CGlyph* glyph = GetGlyph(*chr);

    if (glyph) {
      if (opts.x0_direction == ETextDirection::Horizontal) {
        int advance = 0;
        if (prevGlyph)
          advance = KernLookup(x1c_kerning, prevGlyph->GetKernStart(), *chr);

        int curWidth = prevWidth + (glyph->GetLeftPadding() + glyph->GetAdvance() + glyph->GetRightPadding() + advance);
        int curHeight = glyph->GetBaseline() - (x8_monoHeight + glyph->GetCellHeight());

        width = curWidth;
        prevWidth = curWidth;

        if (curHeight > height)
          height = curHeight;
      }
    }

    prevGlyph = glyph;
    chr++;
    if (len == -1)
      continue;

    if ((chr - str) >= len)
      break;
  }
}

bool CRasterFont::IsFinishedLoading() const {
  if (!x80_texture || !x80_texture.IsLoaded())
    return false;
  return true;
}

void CRasterFont::SetupRenderState() {
  static const GX::VtxDescList skDescList[3] = {
    {GX::VA_POS, GX::DIRECT},
    {GX::VA_TEX0, GX::DIRECT},
    {GX::VA_NULL, GX::NONE}
  };

  x80_texture->Load(GX::TEXMAP0, EClampMode::Clamp);
  CGX::SetTevKAlphaSel(GX::TEVSTAGE0, GX::TEV_KASEL_K0_A);
  CGX::SetTevKColorSel(GX::TEVSTAGE0, GX::TEV_KCSEL_K0);
  CGX::SetTevColorIn(GX::TEVSTAGE0, GX::CC_ZERO, GX::CC_TEXC, GX::CC_KONST, GX::CC_ZERO);
  CGX::SetTevAlphaIn(GX::TEVSTAGE0, GX::CA_ZERO, GX::CA_TEXA, GX::CA_KONST, GX::CA_ZERO);
  CGX::SetStandardTevColorAlphaOp(GX::TEVSTAGE0);
  CGX::SetTevDirect(GX::TEVSTAGE0);
  CGX::SetVtxDescv(skDescList);
  CGX::SetNumChans(0);
  CGX::SetNumTexGens(1);
  CGX::SetNumTevStages(1);
  CGX::SetNumIndStages(0);
  CGX::SetTevOrder(GX::TEVSTAGE0, GX::TEXCOORD0, GX::TEXMAP0, GX::COLOR_NULL);
  CGX::SetTexCoordGen(GX::TEXCOORD0, GX::TG_MTX2x4, GX::TG_TEX0, GX::IDENTITY, false, GX::PTIDENTITY);
}
std::unique_ptr<IObj> FRasterFontFactory([[maybe_unused]] const SObjectTag& tag, CInputStream& in,
                                         const CVParamTransfer& vparms, [[maybe_unused]] CObjectReference* selfRef) {
  CSimplePool* sp = vparms.GetOwnedObj<CSimplePool*>();
  return TToken<CRasterFont>::GetIObjObjectFor(std::make_unique<CRasterFont>(in, *sp));
}

} // namespace metaforce
