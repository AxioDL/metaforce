#include "Kyoto/Text/CTextRenderBuffer.hpp"
#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CGraphicsPalette.hpp"
#include "Kyoto/Streams/CMemoryInStream.hpp"
#include "Kyoto/Streams/CMemoryStreamOut.hpp"
#include "Kyoto/Text/CFontImageDef.hpp"
#include "Kyoto/Text/CRasterFont.hpp"
#include "Kyoto/Text/CTextColor.hpp"
#include "rstl/math.hpp"
#include <dolphin/gx/GXVert.h>
#include <limits.h>
#include <string.h>

CTextRenderBuffer::CTextRenderBuffer(EMode mode)
: mMode(mode)
, mBlobSize(0)
, mCurBytecodeOffset(0)
, mActiveFont(-1)
, mActivePalette(-1)
, mQueuedFont(-1)
, mQueuedPalette(-1)
, mNextPalette(0)
#if VERSION >= VERSION_GM8P_00
, mBounds(CVector2i(0, 0), CVector2i(0, 0))
, mBoundsDirty(false)
#endif
{
}

#if VERSION >= VERSION_GM8P_00

int CTextRenderBuffer::GetNextAvailablePalette() const {
  if (mNextPalette >= 64) {
    mNextPalette = 0;
  }
  if (mNextPalette < mPalettes.size()) {
    mPalettes[mNextPalette] = SFontPalette(
        kFM_None, rstl::auto_ptr< CGraphicsPalette >(rs_new CGraphicsPalette(kPF_RGB5A3, 16)),
        rstl::auto_ptr< CGraphicsPalette >(rs_new CGraphicsPalette(kPF_RGB5A3, 16)),
        rstl::auto_ptr< CGraphicsPalette >(rs_new CGraphicsPalette(kPF_RGB5A3, 16)),
        rstl::auto_ptr< CGraphicsPalette >(rs_new CGraphicsPalette(kPF_RGB5A3, 16)));
  } else {
    mPalettes.push_back(SFontPalette(
        kFM_None, rstl::auto_ptr< CGraphicsPalette >(rs_new CGraphicsPalette(kPF_RGB5A3, 16)),
        rstl::auto_ptr< CGraphicsPalette >(rs_new CGraphicsPalette(kPF_RGB5A3, 16)),
        rstl::auto_ptr< CGraphicsPalette >(rs_new CGraphicsPalette(kPF_RGB5A3, 16)),
        rstl::auto_ptr< CGraphicsPalette >(rs_new CGraphicsPalette(kPF_RGB5A3, 16))));
  }
  return mNextPalette++;
}

int CTextRenderBuffer::GetMatchingPaletteIndex(EFontMode mode,
                                               const CGraphicsPalette& palette) const {
  for (int i = 0; i < mPalettes.size(); ++i) {
    const SFontPalette& cached = mPalettes[i];
    if (cached.mMode == mode && !memcmp(cached.mColors, palette.GetPaletteData(), 8)) {
      return i;
    }
  }
  return -1;
}

#else

CGraphicsPalette* CTextRenderBuffer::GetNextAvailablePalette() const {
  if (mNextPalette >= 64) {
    mNextPalette = 0;
  } else {
    mPalettes.push_back(rstl::auto_ptr< CGraphicsPalette >(rs_new CGraphicsPalette(kPF_RGB5A3, 4)));
  }

  ++mNextPalette;
  CGraphicsPalette* ret = mPalettes[mNextPalette - 1].get();
  return ret;
}

int CTextRenderBuffer::GetMatchingPaletteIndex(const CGraphicsPalette& palette) const {
  for (int i = 0; i < mPalettes.size(); ++i) {
    if (!memcmp(mPalettes[i]->GetPaletteData(), palette.GetPaletteData(), 8)) {
      return i;
    }
  }
  return -1;
}

#endif

void CTextRenderBuffer::AddFontChange(const TToken< CRasterFont >& font) {
  if (mMode == kM_BufferFill) {
    CMemoryStreamOut out(GetOutStream(), GetCurLen(), CMemoryStreamOut::kOS_NotOwned, 64);
    bool found = false;

    for (int fontIndex = 0; fontIndex < mFonts.size(); ++fontIndex) {
      if (mFonts[fontIndex].GetRef() == font.GetRef()) {
        out.WriteUint8(kC_FontChange);
        out.WriteInt8(fontIndex);
        found = true;
        break;
      }
    }

    if (!found) {
      mFonts.reserve(mFonts.size() + 1);
      int fontIndex = mFonts.size();
      mFonts.push_back(font);
      out.WriteUint8(kC_FontChange);
      out.WriteInt8(fontIndex);
    }
    mCurBytecodeOffset += out.GetWrittenBytes();
  } else {
    // Command + index
    mBlobSize += sizeof(char) + sizeof(char);
  }
}

#if VERSION >= VERSION_GM8P_00
void SetFontPalette(EFontMode mode, int layer, CGraphicsPalette& layerPalette,
                    const CGraphicsPalette& colorPalette) {
  ushort* layerColors = static_cast< ushort* >(layerPalette.Lock());
  const ushort* const colors = colorPalette.GetPaletteData();
  if (mode == kFM_OneLayer || mode == kFM_OneLayerOutline) {
    memcpy(layerColors, colors, 8);
  } else {
    for (int i = 0; i < 16; ++i) {
      switch (mode) {
      case kFM_TwoLayers: {
        int firstMask = 1;
        firstMask <<= layer * 2;
        int secondMask = 2;
        secondMask <<= layer * 2;
        bool first = (i & firstMask) != 0;
        bool second = (i & secondMask) != 0;
        if (first && second) {
          layerColors[i] = colors[1];
        } else if (second) {
          layerColors[i] = CColor::FromRGB5A3(colors[1]).WithAlphaModulatedBy(0.66f).ToRGB5A3();
        } else if (first) {
          layerColors[i] = CColor::FromRGB5A3(colors[1]).WithAlphaModulatedBy(0.33f).ToRGB5A3();
        } else {
          layerColors[i] = colors[0];
        }
        break;
      }
      case kFM_FourLayers:
        if ((i >> layer) & 1) {
          layerColors[i] = colors[1];
        } else {
          layerColors[i] = colors[0];
        }
        break;
      case kFM_TwoLayersOutline: {
        int firstMask = 1;
        firstMask <<= layer * 2;
        int secondMask = 2;
        secondMask <<= layer * 2;
        bool first = (i & firstMask) != 0;
        bool second = (i & secondMask) != 0;
        if (first) {
          layerColors[i] = colors[1];
        } else if (second) {
          layerColors[i] = colors[2];
        } else {
          layerColors[i] = colors[0];
        }
        break;
      }
      }
    }
  }
  layerPalette.UnLock();
}

void CTextRenderBuffer::AddPaletteChange(const CGraphicsPalette& palette, EFontMode mode) {
#else
void CTextRenderBuffer::AddPaletteChange(const CGraphicsPalette& palette) {
#endif
  if (mMode == kM_BufferFill) {
    CMemoryStreamOut out(GetOutStream(), GetCurLen(), CMemoryStreamOut::kOS_NotOwned, 64);

#if VERSION >= VERSION_GM8P_00
    int paletteIndex = GetMatchingPaletteIndex(mode, palette);
#else
    int paletteIndex = GetMatchingPaletteIndex(palette);
#endif
    if (paletteIndex == -1) {
      GetNextAvailablePalette();
      paletteIndex = mNextPalette - 1;
#if VERSION >= VERSION_GM8P_00
      SFontPalette& destPalette = mPalettes[paletteIndex];
      destPalette.mMode = mode;
      memcpy(destPalette.mColors, palette.GetPaletteData(), 8);
      SetFontPalette(mode, 0, *destPalette.mPalette0, palette);
      SetFontPalette(mode, 1, *destPalette.mPalette1, palette);
      SetFontPalette(mode, 2, *destPalette.mPalette2, palette);
      SetFontPalette(mode, 3, *destPalette.mPalette3, palette);
#else
      CGraphicsPalette* destPalette = mPalettes[paletteIndex].get();
      void* data = destPalette->Lock();
      memcpy(data, palette.GetPaletteData(), 8);
      destPalette->UnLock();
#endif
    }

    out.WriteUint8(kC_PaletteChange);
    out.WriteInt8(paletteIndex);
    mCurBytecodeOffset += out.GetWrittenBytes();
  } else {
    // Command + index
    mBlobSize += sizeof(char) + sizeof(char);
  }
}

void CTextRenderBuffer::AddCharacter(const CVector2i& offset, short chr, uint color) {
  if (mMode == kM_BufferFill) {
    CMemoryStreamOut out(GetOutStream(), GetCurLen(), CMemoryStreamOut::kOS_NotOwned, 64);
    int tmp = mCurBytecodeOffset;
    mPrimOffsets.reserve(mPrimOffsets.size() + 1);
    mPrimOffsets.push_back(tmp);
    out.WriteUint8(kC_CharacterRender);
    out.Put< short >(offset.GetX());
    out.Put< short >(offset.GetY());
    out.Put< short >(chr);
    out.Put< int >(color);
    mCurBytecodeOffset += out.GetWrittenBytes();
  } else {
    // Command + x + y + char + color
    mBlobSize += sizeof(char) + sizeof(short) + sizeof(short) + sizeof(short) + sizeof(CTextColor);
  }
#if VERSION >= VERSION_GM8P_00
  mBoundsDirty = true;
#endif
}

void CTextRenderBuffer::AddImage(const CVector2i& offset, const CFontImageDef& image) {
  if (mMode == kM_BufferFill) {
    CMemoryStreamOut out(GetOutStream(), GetCurLen(), CMemoryStreamOut::kOS_NotOwned, 64);
    const int tmp = mCurBytecodeOffset;
    mPrimOffsets.reserve(mPrimOffsets.size() + 1);
    mPrimOffsets.push_back(tmp);
    mImages.reserve(mImages.size() + 1);
    int imageIdx = mImages.size();
    mImages.push_back(image);
    out.WriteUint8(kC_ImageRender);
    out.Put< short >(offset.GetX());
    out.Put< short >(offset.GetY());
    out.WriteInt8(imageIdx);
    out.Put< uint >(CColor::White().GetColor_u32());
    mCurBytecodeOffset += out.GetWrittenBytes();
  } else {
    // Command + x + y + index + color
    mBlobSize += sizeof(char) + sizeof(short) + sizeof(short) + sizeof(char) + sizeof(uint);
  }
#if VERSION >= VERSION_GM8P_00
  mBoundsDirty = true;
#endif
}

void CTextRenderBuffer::Render(const CColor& color, float time) const {
  mActiveFont = -1;
  mActivePalette = -1;
  CMemoryInStream in(mBytecode.data(), mBlobSize, CMemoryInStream::kOS_NotOwned);
  while (in.GetReadPosition() < mBlobSize) {
    switch (static_cast< ECmd >(in.Get< uchar >())) {
    case kC_CharacterRender: {
      if (mQueuedFont != -1) {
        TToken< CRasterFont > font = mFonts[mQueuedFont];
        if (font.IsLoaded()) {
          font->SetupRenderState();
          mQueuedFont = -1;
        }
      }
#if VERSION < VERSION_GM8P_00
      if (mQueuedPalette != -1) {
        mPalettes[mQueuedPalette]->Load();
        mQueuedPalette = -1;
      }
#endif
      short x = in.Get< short >();
      short y = in.Get< short >();
      short chr = in.Get< short >();
      uint chrColor = in.Get< uint >();
      if (mActiveFont != -1) {
        TToken< CRasterFont > font = mFonts[mActiveFont];
        if (font.IsLoaded()) {
#if VERSION >= VERSION_GM8P_00
          const CGlyph* glyph = font->GetGlyph(chr);
          if (glyph != nullptr) {
            int layer = glyph->GetLayer();
            if (mQueuedPalette != -1 || layer != -1) {
              int paletteIndex = mQueuedPalette != -1 ? mQueuedPalette : mActivePalette;
              if (paletteIndex != -1) {
                const SFontPalette& palette = mPalettes[paletteIndex];
                switch (layer) {
                case 0:
                  palette.mPalette0->Load();
                  break;
                case 1:
                  palette.mPalette1->Load();
                  break;
                case 2:
                  palette.mPalette2->Load();
                  break;
                case 3:
                  palette.mPalette3->Load();
                  break;
                }
                mQueuedPalette = -1;
              }
            }
#else
          if (font->HasGlyph(chr)) {
            const CGlyph* glyph = font->GetGlyph(chr);
#endif
            CGX::SetTevKColor(GX_KCOLOR0, CColor::Modulate(CColor(chrColor), color).GetGXColor());
#if VERSION >= VERSION_GM8P_00
            static const GXVtxDescList skDescList[] = {
                {GX_VA_POS, GX_DIRECT}, {GX_VA_TEX0, GX_DIRECT}, {GX_VA_NULL, GX_NONE}};
            CGX::SetVtxDescv(skDescList);
            CGX::SetNumChans(0);
            CGX::SetNumTexGens(1);
#endif
            CGX::Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);
            GXPosition3f32(x, 0.f, y);
            GXTexCoord2f32(glyph->GetStartU(), glyph->GetStartV());
            GXPosition3f32(x + glyph->GetCellWidth(), 0.f, y);
            GXTexCoord2f32(glyph->GetEndU(), glyph->GetStartV());
            GXPosition3f32(x, 0.f, y + glyph->GetCellHeight());
            GXTexCoord2f32(glyph->GetStartU(), glyph->GetEndV());
            GXPosition3f32(x + glyph->GetCellWidth(), 0.f, y + glyph->GetCellHeight());
            GXTexCoord2f32(glyph->GetEndU(), glyph->GetEndV());
            CGX::End();
          }
        }
      }
      break;
    }
    case kC_ImageRender: {
      short x = in.Get< short >();
      short y = in.Get< short >();
      schar imageIndex = in.Get< schar >();
      uint imageColor = in.Get< uint >();
      const CFontImageDef& image = mImages[imageIndex];
      TToken< CTexture > texture =
          image.GetImages()[static_cast< int >(time * image.GetFps()) % image.GetImages().size()];
      if (texture.IsLoaded()) {
        texture->Load(GX_TEXMAP0, CTexture::kCM_Clamp);
        short width = image.GetMonoWidth();
        short height = image.GetHeight();
        float cropXHalf = image.GetScale().GetX() / 2.f;
        float cropYHalf = image.GetScale().GetY() / 2.f;
        CGX::SetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
        CGX::SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
        CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_ZERO);
        CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_KONST, GX_CA_ZERO);
        CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE0);
        static const GXVtxDescList skDescList[] = {
            {GX_VA_POS, GX_DIRECT}, {GX_VA_TEX0, GX_DIRECT}, {GX_VA_NULL, GX_NONE}};
        CGX::SetVtxDescv(skDescList);
        CGX::SetNumChans(0);
        CGX::SetNumTexGens(1);
        CGX::SetNumTevStages(1);
        CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
        CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, false,
                            GX_PTIDENTITY);
        CGX::SetTevKColor(GX_KCOLOR0, CColor::Modulate(CColor(imageColor), color).GetGXColor());
        CGX::Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);
        GXPosition3f32(x, 0.f, y);
        GXTexCoord2f32(0.5f - cropXHalf, 0.5f + cropYHalf);
        GXPosition3f32(x + width, 0.f, y);
        GXTexCoord2f32(0.5f + cropXHalf, 0.5f + cropYHalf);
        GXPosition3f32(x, 0.f, y + height);
        GXTexCoord2f32(0.5f - cropXHalf, 0.5f - cropYHalf);
        GXPosition3f32(x + width, 0.f, y + height);
        GXTexCoord2f32(0.5f + cropXHalf, 0.5f - cropYHalf);
        CGX::End();
        mQueuedFont = mActiveFont;
        mQueuedPalette = mActivePalette;
      }
      break;
    }
    case kC_FontChange:
      mActiveFont = mQueuedFont = in.Get< schar >();
      break;
    case kC_PaletteChange:
      mActivePalette = mQueuedPalette = in.Get< schar >();
      break;
    }
  }
}

void CTextRenderBuffer::VerifyBuffer() {
  if (mBytecode.empty()) {
    mBytecode.resize(mBlobSize);
  }
}

void CTextRenderBuffer::SetMode(EMode mode) { mMode = mode; }

void* CTextRenderBuffer::GetOutStream() {
  VerifyBuffer();
  return mBytecode.data() + mCurBytecodeOffset;
}

size_t CTextRenderBuffer::GetCurLen() {
  VerifyBuffer();
  return mBlobSize - mCurBytecodeOffset;
}

CTextRenderBuffer::Primitive CTextRenderBuffer::GetPrimitive(int index) const {
  CMemoryInStream in(mBytecode.data() + mPrimOffsets[index], mBlobSize - mPrimOffsets[index]);
  switch (static_cast< ECmd >(in.Get< uchar >())) {
  case kC_CharacterRender: {
    short x = in.Get< short >();
    short y = in.Get< short >();
    short chr = in.Get< short >();
    uint color = in.Get< uint >();
    return Primitive(kC_CharacterRender, x, y, chr, color, 0);
  }
  case kC_ImageRender: {
    short x = in.Get< short >();
    short y = in.Get< short >();
    schar image = in.Get< schar >();
    uint color = in.Get< uint >();
    return Primitive(kC_ImageRender, x, y, 0, color, image);
  }
  default:
    return Primitive(kC_Invalid, 0, 0, 0, 0, 0);
  }
}

void CTextRenderBuffer::SetPrimitive(const Primitive& prim, int index) {
  CMemoryStreamOut out(mBytecode.data() + mPrimOffsets[index], mBlobSize - mPrimOffsets[index],
                       CMemoryStreamOut::kOS_NotOwned, 64);
  switch (prim.mCmd) {
  case kC_CharacterRender:
    out.WriteUint8(kC_CharacterRender);
    out.Put< short >(prim.mX);
    out.Put< short >(prim.mY);
    out.Put< short >(prim.mChar);
    out.Put< uint >(prim.mColor);
    break;
  case kC_ImageRender:
    out.WriteUint8(kC_ImageRender);
    out.Put< short >(prim.mX);
    out.Put< short >(prim.mY);
    out.WriteInt8(prim.mIndex);
    out.Put< uint >(prim.mColor);
    break;
  }
#if VERSION >= VERSION_GM8P_00
  mBoundsDirty = true;
#endif
}

bool CTextRenderBuffer::HasSpaceAvailable(const CVector2i& origin, const CVector2i& extent) {
  rstl::pair< CVector2i, CVector2i > bounds = AccumulateTextBounds();
  if (bounds.first.GetX() > bounds.second.GetX()) {
    return true;
  }
  CVector2i offset(0, 0);
  CVector2i size = bounds.second - bounds.first;
  if (offset.GetY() < origin.GetY()) {
    return false;
  }
  return size.GetY() <= extent.GetY();
}

#if VERSION >= VERSION_GM8P_00
void CTextRenderBuffer::RecalculateTextBounds() const {
#else
rstl::pair< CVector2i, CVector2i > CTextRenderBuffer::AccumulateTextBounds() {
#endif
  CVector2i min(INT_MAX, INT_MAX);
  CVector2i max(-INT_MAX - 1, -INT_MAX - 1);
  CMemoryInStream in(mBytecode.data(), mBlobSize, CMemoryInStream::kOS_NotOwned);
  while (in.GetReadPosition() < mCurBytecodeOffset) {
    switch (static_cast< ECmd >(in.Get< uchar >())) {
    case kC_CharacterRender: {
      short x = in.Get< short >();
      short y = in.Get< short >();
      short chr = in.Get< short >();
      in.Get< uint >();
      if (mActiveFont != -1) {
        TToken< CRasterFont > font = mFonts[mActiveFont];
        if (font.IsLoaded() && font->HasGlyph(chr)) {
          const CGlyph* glyph = font->GetGlyph(chr);
          short maxX = x + glyph->GetCellWidth();
          short maxY = y + glyph->GetCellHeight();
          max[0] = rstl::max_val< int >(max[0], maxX);
          max[1] = rstl::max_val< int >(max[1], maxY);
          min[0] = rstl::min_val< int >(min[0], x);
          min[1] = rstl::min_val< int >(min[1], y);
        }
      }
      break;
    }
    case kC_ImageRender: {
      short x = in.Get< short >();
      short y = in.Get< short >();
      schar imageIndex = in.Get< schar >();
      in.Get< uint >();
      const CFontImageDef& image = mImages[imageIndex];
      short maxX = x + image.GetMonoWidth();
      short maxY = y + image.GetHeight();
      max[0] = rstl::max_val< int >(max[0], maxX);
      max[1] = rstl::max_val< int >(max[1], maxY);
      min[0] = rstl::min_val< int >(min[0], x);
      min[1] = rstl::min_val< int >(min[1], y);
      break;
    }
    case kC_FontChange:
      mActiveFont = in.Get< schar >();
      break;
    case kC_PaletteChange:
      in.Get< schar >();
      break;
    }
  }
#if VERSION >= VERSION_GM8P_00
  mBounds = rstl::pair< CVector2i, CVector2i >(min, max);
  mBoundsDirty = false;
#else
  return rstl::pair< CVector2i, CVector2i >(min, max);
#endif
}

#if VERSION >= VERSION_GM8P_00
const rstl::pair< CVector2i, CVector2i >& CTextRenderBuffer::AccumulateTextBounds() {
  if (mBoundsDirty) {
    RecalculateTextBounds();
  }
  return mBounds;
}
#endif
