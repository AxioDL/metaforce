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

#if VERSION < VERSION_GM8P_00 || VERSION == VERSION_GM8E_02

CTextRenderBuffer::CTextRenderBuffer(EMode mode)
: x0_mode(mode)
, x44_blobSize(0)
, x48_curBytecodeOffset(0)
, x4c_activeFont(-1)
, x4d_activePalette(-1)
, x4e_queuedFont(-1)
, x4f_queuedPalette(-1)
, x254_nextPalette(0) {}

CGraphicsPalette* CTextRenderBuffer::GetNextAvailablePalette() const {
  if (x254_nextPalette >= 64) {
    x254_nextPalette = 0;
  } else {
    x50_palettes.push_back(
        rstl::auto_ptr< CGraphicsPalette >(rs_new CGraphicsPalette(kPF_RGB5A3, 4)));
  }

  ++x254_nextPalette;
  CGraphicsPalette* ret = x50_palettes[x254_nextPalette - 1].get();
  return ret;
}

int CTextRenderBuffer::GetMatchingPaletteIndex(const CGraphicsPalette& palette) const {
  for (int i = 0; i < x50_palettes.size(); ++i) {
    if (!memcmp(x50_palettes[i]->GetPaletteData(), palette.GetPaletteData(), 8)) {
      return i;
    }
  }
  return -1;
}

#endif

void CTextRenderBuffer::AddFontChange(const TToken< CRasterFont >& font) {
  if (x0_mode == kM_BufferFill) {
    CMemoryStreamOut out(GetOutStream(), GetCurLen(), CMemoryStreamOut::kOS_NotOwned, 64);
    bool found = false;

    for (int fontIndex = 0; fontIndex < x4_fonts.size(); ++fontIndex) {
      if (x4_fonts[fontIndex].GetRef() == font.GetRef()) {
        out.WriteUint8(kC_FontChange);
        out.WriteInt8(fontIndex);
        found = true;
        break;
      }
    }

    if (!found) {
      x4_fonts.reserve(x4_fonts.size() + 1);
      int fontIndex = x4_fonts.size();
      x4_fonts.push_back(font);
      out.WriteUint8(kC_FontChange);
      out.WriteInt8(fontIndex);
    }
    x48_curBytecodeOffset += out.GetWrittenBytes();
  } else {
    // Command + index
    x44_blobSize += sizeof(char) + sizeof(char);
  }
}

#if VERSION < VERSION_GM8P_00 || VERSION == VERSION_GM8E_02

void CTextRenderBuffer::AddPaletteChange(const CGraphicsPalette& palette) {
  if (x0_mode == kM_BufferFill) {
    CMemoryStreamOut out(GetOutStream(), GetCurLen(), CMemoryStreamOut::kOS_NotOwned, 64);

    int paletteIndex = GetMatchingPaletteIndex(palette);
    if (paletteIndex == -1) {
      GetNextAvailablePalette();
      paletteIndex = x254_nextPalette - 1;
      CGraphicsPalette* destPalette = x50_palettes[paletteIndex].get();
      void* data = destPalette->Lock();
      memcpy(data, palette.GetPaletteData(), 8);
      destPalette->UnLock();
    }

    out.WriteUint8(kC_PaletteChange);
    out.WriteInt8(paletteIndex);
    x48_curBytecodeOffset += out.GetWrittenBytes();
  } else {
    // Command + index
    x44_blobSize += sizeof(char) + sizeof(char);
  }
}

#endif

void CTextRenderBuffer::AddCharacter(const CVector2i& offset, short chr, uint color) {
  if (x0_mode == kM_BufferFill) {
    CMemoryStreamOut out(GetOutStream(), GetCurLen(), CMemoryStreamOut::kOS_NotOwned, 64);
    int tmp = x48_curBytecodeOffset;
    x24_primOffsets.reserve(x24_primOffsets.size() + 1);
    x24_primOffsets.push_back(tmp);
    out.WriteUint8(kC_CharacterRender);
    out.WriteInt16(offset.GetX());
    out.WriteInt16(offset.GetY());
    out.WriteInt16(chr);
    out.WriteInt32(color);
    x48_curBytecodeOffset += out.GetWrittenBytes();
  } else {
    // Command + x + y + char + color
    x44_blobSize +=
        sizeof(char) + sizeof(short) + sizeof(short) + sizeof(short) + sizeof(CTextColor);
  }
}

void CTextRenderBuffer::AddImage(const CVector2i& offset, const CFontImageDef& image) {
  if (x0_mode == kM_BufferFill) {
    CMemoryStreamOut out(GetOutStream(), GetCurLen(), CMemoryStreamOut::kOS_NotOwned, 64);
    const int tmp = x48_curBytecodeOffset;
    x24_primOffsets.reserve(x24_primOffsets.size() + 1);
    x24_primOffsets.push_back(tmp);
    x14_images.reserve(x14_images.size() + 1);
    int imageIdx = x14_images.size();
    x14_images.push_back(image);
    out.WriteUint8(kC_ImageRender);
    out.WriteInt16(offset.GetX());
    out.WriteInt16(offset.GetY());
    out.WriteInt8(imageIdx);
    out.WriteUint32(CColor::White().GetColor_u32());
    x48_curBytecodeOffset += out.GetWrittenBytes();
  } else {
    // Command + x + y + index + color
    x44_blobSize += sizeof(char) + sizeof(short) + sizeof(short) + sizeof(char) + sizeof(uint);
  }
}

#if VERSION < VERSION_GM8P_00 || VERSION == VERSION_GM8E_02

void CTextRenderBuffer::Render(const CColor& color, float time) const {
  x4c_activeFont = -1;
  x4d_activePalette = -1;
  CMemoryInStream in(x34_bytecode.data(), x44_blobSize, CMemoryInStream::kOS_NotOwned);
  while (in.GetReadPosition() < x44_blobSize) {
    switch (static_cast< ECmd >(in.Get< uchar >())) {
    case kC_CharacterRender: {
      if (x4e_queuedFont != -1) {
        TToken< CRasterFont > font = x4_fonts[x4e_queuedFont];
        if (font.IsLoaded()) {
          font->SetupRenderState();
          x4e_queuedFont = -1;
        }
      }
      if (x4f_queuedPalette != -1) {
        x50_palettes[x4f_queuedPalette]->Load();
        x4f_queuedPalette = -1;
      }
      short x = in.Get< short >();
      short y = in.Get< short >();
      short chr = in.Get< short >();
      uint chrColor = in.Get< uint >();
      if (x4c_activeFont != -1) {
        TToken< CRasterFont > font = x4_fonts[x4c_activeFont];
        if (font.IsLoaded() && font->HasGlyph(chr)) {
          const CGlyph* glyph = font->GetGlyph(chr);
          CGX::SetTevKColor(GX_KCOLOR0, CColor::Modulate(CColor(chrColor), color).GetGXColor());
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
      break;
    }
    case kC_ImageRender: {
      short x = in.Get< short >();
      short y = in.Get< short >();
      schar imageIndex = in.Get< schar >();
      uint imageColor = in.Get< uint >();
      const CFontImageDef& image = x14_images[imageIndex];
      TToken< CTexture > texture =
          image.GetImages()[static_cast< int >(time * image.GetFps()) % image.GetImages().size()];
      if (texture.IsLoaded()) {
        texture->Load(GX_TEXMAP0, CTexture::kCM_Clamp);
        short width = image.GetMonoWidth();
        short height = image.GetMonoHeight();
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
        x4e_queuedFont = x4c_activeFont;
        x4f_queuedPalette = x4d_activePalette;
      }
      break;
    }
    case kC_FontChange:
      x4c_activeFont = x4e_queuedFont = in.Get< schar >();
      break;
    case kC_PaletteChange:
      x4d_activePalette = x4f_queuedPalette = in.Get< schar >();
      break;
    }
  }
}

#endif

void CTextRenderBuffer::VerifyBuffer() {
  if (x34_bytecode.empty()) {
    x34_bytecode.resize(x44_blobSize);
  }
}

void CTextRenderBuffer::SetMode(EMode mode) { x0_mode = mode; }

void* CTextRenderBuffer::GetOutStream() {
  VerifyBuffer();
  return x34_bytecode.data() + x48_curBytecodeOffset;
}

size_t CTextRenderBuffer::GetCurLen() {
  VerifyBuffer();
  return x44_blobSize - x48_curBytecodeOffset;
}

CTextRenderBuffer::Primitive CTextRenderBuffer::GetPrimitive(int index) const {
  CMemoryInStream in(x34_bytecode.data() + x24_primOffsets[index],
                     x44_blobSize - x24_primOffsets[index]);
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
  CMemoryStreamOut out(x34_bytecode.data() + x24_primOffsets[index],
                       x44_blobSize - x24_primOffsets[index], CMemoryStreamOut::kOS_NotOwned, 64);
  switch (prim.x4_cmd) {
  case kC_CharacterRender:
    out.WriteUint8(kC_CharacterRender);
    out.WriteInt16(prim.x8_x);
    out.WriteInt16(prim.xa_y);
    out.WriteInt16(prim.xc_char);
    out.WriteUint32(prim.x0_color);
    break;
  case kC_ImageRender:
    out.WriteUint8(kC_ImageRender);
    out.WriteInt16(prim.x8_x);
    out.WriteInt16(prim.xa_y);
    out.WriteInt8(prim.xe_index);
    out.WriteUint32(prim.x0_color);
    break;
  }
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

rstl::pair< CVector2i, CVector2i > CTextRenderBuffer::AccumulateTextBounds() {
  CVector2i min(INT_MAX, INT_MAX);
  CVector2i max(-INT_MAX - 1, -INT_MAX - 1);
  CMemoryInStream in(x34_bytecode.data(), x44_blobSize, CMemoryInStream::kOS_NotOwned);
  while (in.GetReadPosition() < x48_curBytecodeOffset) {
    switch (static_cast< ECmd >(in.Get< uchar >())) {
    case kC_CharacterRender: {
      short x = in.Get< short >();
      short y = in.Get< short >();
      short chr = in.Get< short >();
      in.Get< uint >();
      if (x4c_activeFont != -1) {
        TToken< CRasterFont > font = x4_fonts[x4c_activeFont];
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
      const CFontImageDef& image = x14_images[imageIndex];
      short maxX = x + image.GetMonoWidth();
      short maxY = y + image.GetMonoHeight();
      max[0] = rstl::max_val< int >(max[0], maxX);
      max[1] = rstl::max_val< int >(max[1], maxY);
      min[0] = rstl::min_val< int >(min[0], x);
      min[1] = rstl::min_val< int >(min[1], y);
      break;
    }
    case kC_FontChange:
      x4c_activeFont = in.Get< schar >();
      break;
    case kC_PaletteChange:
      in.Get< schar >();
      break;
    }
  }
  return rstl::pair< CVector2i, CVector2i >(min, max);
}
