#include "Kyoto/Text/CRasterFont.hpp"
#include "Kyoto/Basics/CBasics.hpp"

#include "Kyoto/CFactoryFnReturn.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/CVParamTransfer.hpp"
#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CGraphicsPalette.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/Text/CDrawStringOptions.hpp"
#include "Kyoto/Text/CTextRenderBuffer.hpp"
#include "Kyoto/Text/TextCommon.hpp"

#include "rstl/algorithm.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"

CRasterFont::CRasterFont(CInputStream& in, IObjectStore* store)
: mInitialized(false)
, mMonoWidth(16)
, mMonoHeight(16)
, mMode(kFM_OneLayer)
, mLineMargin(0) {
  if (in.ReadInt32() == 'FONT') {
    int version = in.ReadInt32();
    if (version >= 0 && version <= 2) {
      mMonoWidth = in.ReadInt32();
      mMonoHeight = in.ReadInt32();
      if (version >= 1) {
        mBaseline = in.ReadInt32();
      } else {
        mBaseline = mMonoHeight;
      }

      if (version >= 2) {
        mLineMargin = in.ReadInt32();
      }

      bool fInfoA = in.ReadBool();
      bool fInfoB = in.ReadBool();
      int fInfoC = in.ReadInt32();
      int fontSize = in.ReadInt32();
      rstl::string fontName(in);
      CAssetId fontId = in.ReadInt32();

      if (store != nullptr) {
        mTexture = store->GetObj(SObjectTag('TXTR', fontId));
        mTexture->Lock();
      }

      mFontInfo = CFontInfo(fInfoA, fInfoB, fInfoC, fontSize, fontName.data());

      int mode = in.ReadInt32();
      switch (mode) {
      case 0:
        mMode = kFM_OneLayer;
        break;
      case 1:
        mMode = kFM_OneLayerOutline;
        break;
      }

      int glyphCount = in.ReadInt32();
      mGlyphs.reserve(glyphCount);

      for (int i = 0; i < glyphCount; ++i) {
        wchar_t chr = in.Get< ushort >();
        float startU = in.ReadFloat();
        float startV = in.ReadFloat();
        float endU = in.ReadFloat();
        float endV = in.ReadFloat();
        int a = in.ReadInt32();
        int b = in.ReadInt32();
        int c = in.ReadInt32();
        int cellWidth = in.ReadInt32();
        int cellHeight = in.ReadInt32();
        int baseline = in.ReadInt32();
        int kernStart = in.ReadInt32();
        mGlyphs.push_back(
            rstl::pair< wchar_t, CGlyph >(chr, CGlyph(a, b, c, startU, startV, endU, endV,
                                                      cellWidth, cellHeight, baseline, kernStart)));
      }
      rstl::sort_by_key(mGlyphs);

      int kerningCount = in.ReadInt32();
      mKerning.reserve(kerningCount);

      for (int i = 0; i < kerningCount; ++i) {
        short first = in.Get< short >();
        short second = in.Get< short >();
        int howMuch = in.ReadInt32();
        mKerning.push_back(CKernPair(first, second, howMuch));
      }

      mInitialized = true;
    }
  }
}

EFontMode CRasterFont::GetMode() const { return mMode; }

void CRasterFont::GetSize(const CDrawStringOptions& options, int& width, int& height,
                          const wchar_t* str, int length) const {
  width = 0;
  height = 0;
  int curWidth = 0;
  const CGlyph* prevGlyph = nullptr;
#if NONMATCHING
  for (const wchar_t* ptr = str; (length == -1 || ptr - str < length) && *ptr != 0; ++ptr) {
#else
  for (const wchar_t* ptr = str; *ptr != 0 && (length == -1 || ptr - str < length); ++ptr) {
#endif
    const CGlyph* glyph = GetGlyph(*ptr);
    if (glyph != nullptr) {
      int kerning =
          prevGlyph != nullptr ? KernLookup(mKerning, prevGlyph->GetKernStart(), *ptr) : 0;
      int newWidth = curWidth + glyph->GetA() + glyph->GetB() + glyph->GetC() + kerning;
      int newHeight = mMonoHeight - glyph->GetBaseLine() + glyph->GetCellHeight();
      if (options.GetTextDirection() == kTD_Horizontal) {
        width = newWidth;
        curWidth = newWidth;
        if (newHeight > height) {
          height = newHeight;
        }
      }
    }
    prevGlyph = glyph;
  }
}

int CRasterFont::GetMonoWidth() const { return mMonoWidth; }
int CRasterFont::GetMonoHeight() const { return mMonoHeight; }
int CRasterFont::GetCarriageAdvance() { return GetMonoHeight() + GetLineMargin(); }

const CGlyph* CRasterFont::GetGlyph(wchar_t c) const { return InternalGetGlyph(c); }

void CRasterFont::DrawString(const CDrawStringOptions& options, int x, int y, int& xOut, int& yOut,
                             CTextRenderBuffer* buffer, const wchar_t* str, int length) const {
  if (!mInitialized) {
    return;
  }

  if (buffer != nullptr) {
    CGraphicsPalette pal(kPF_RGB5A3, 4);
    ushort* data = reinterpret_cast< ushort* >(pal.Lock());
    data[0] = CBasics::SwapBytes(CColor(0.f, 0.f, 0.f, 0.f).ToRGB5A3());
    data[1] = CBasics::SwapBytes(CColor(options.GetPaletteEntry(0)).ToRGB5A3());
    data[2] = CBasics::SwapBytes(CColor(options.GetPaletteEntry(1)).ToRGB5A3());
    data[3] = CBasics::SwapBytes(CColor(0.f, 0.f, 0.f, 0.f).ToRGB5A3());
    pal.UnLock();
    buffer->AddPaletteChange(pal);
  }

  SinglePassDrawString(options, x, y, xOut, yOut, buffer, str, length);
}

void CRasterFont::DrawSpace(const CDrawStringOptions& options, int x, int y, int& xOut, int& yOut,
                            int length) const {
  if (options.GetTextDirection() != kTD_Horizontal) {
    return;
  }

  xOut = x + length;
  yOut = y;
}

int CRasterFont::KernLookup(const rstl::vector< CKernPair >& kern, const int start, const int chr) {

  rstl::vector< CKernPair >::const_iterator it = kern.begin() + start;
  for (; it != kern.end() && it->GetFirst() == kern[start].GetFirst(); ++it) {
    if (it->GetSecond() == chr) {
      return it->GetHowMuch();
    }
  }

  return 0;
}

void CRasterFont::SinglePassDrawString(const CDrawStringOptions& options, const int x, const int y,
                                       int& xOut, int& yOut, CTextRenderBuffer* buffer,
                                       const wchar_t* str, const int length) const {
  if (mInitialized) {
    int curX = x;
    const CGlyph* prevGlyph = nullptr;
#if NONMATCHING
    for (const wchar_t* ptr = str; (length == -1 || ptr - str < length) && *ptr != 0; ++ptr) {
#else
    for (const wchar_t* ptr = str; *ptr != 0 && (length == -1 || (ptr - str) < length); ++ptr) {
#endif
      const CGlyph* curGlyph = GetGlyph(*ptr);
      if (curGlyph != nullptr) {
        int xOffset = 0;
        int yOffset = 0;
        if (options.GetTextDirection() == kTD_Horizontal) {
          curX += curGlyph->GetA();
          if (prevGlyph != nullptr) {
            curX += KernLookup(mKerning, prevGlyph->GetKernStart(), *ptr);
          }
          xOffset = 0;
          yOffset = 0;
        }

        if (buffer) {
          buffer->AddCharacter(CVector2i(curX + xOffset, yOffset + (y - curGlyph->GetBaseLine())),
                               *ptr, options.GetPaletteEntry(2));
        }

        if (options.GetTextDirection() == kTD_Horizontal) {
          curX += curGlyph->GetB() + curGlyph->GetC();
        }
      }
      prevGlyph = curGlyph;
    }

    xOut = curX;
    yOut = y;
  }
}

const CGlyph* CRasterFont::InternalGetGlyph(const wchar_t chr) const {
  rstl::vector< rstl::pair< wchar_t, CGlyph > >::const_iterator it =
      rstl::find_by_key(mGlyphs, chr);

  if (it == mGlyphs.end()) {
    return nullptr;
  }

  return &it->second;
}

const CFactoryFnReturn FRasterFontFactory(const SObjectTag& tag, CInputStream& in,
                                    const CVParamTransfer& xfer) {
  const rstl::rc_ptr< IVParamObj > obj = xfer.mObj;
  CSimplePool* pool = static_cast< TObjOwnerParam< CSimplePool* >* >(obj.GetPtr())->GetData();

  return rs_new CRasterFont(in, pool);
}

void CRasterFont::SetupRenderState() {
  static const GXVtxDescList skDescList[] = {
      {GX_VA_POS, GX_DIRECT},
      {GX_VA_TEX0, GX_DIRECT},
      {GX_VA_NULL, GX_NONE},
  };

  TLockedToken< CTexture > texture = *mTexture;
  texture->Load(GX_TEXMAP0, CTexture::kCM_Clamp);
  CGX::SetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
  CGX::SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_ZERO);
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_KONST, GX_CA_ZERO);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE0);
  CGX::SetTevDirect(GX_TEVSTAGE0);
  CGX::SetVtxDescv(skDescList);
  CGX::SetNumChans(0);
  CGX::SetNumTexGens(1);
  CGX::SetNumTevStages(1);
  CGX::SetNumIndStages(0);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
}

int CRasterFont::GetBaseLine() const { return mBaseline; }
int CRasterFont::GetLineMargin() { return mLineMargin; }

bool CRasterFont::IsFinishedLoading() { return mTexture && mTexture->IsLoaded(); }

void CRasterFont::SetTexture(TToken< CTexture > texture) {
  mTexture = texture;
  mTexture->Lock();
}
