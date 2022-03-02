#include "DolphinCTexture.hpp"

u32 GXGetTexBufferSize(u16 width, u16 height, u32 format, bool mipmap, u8 max_lod) {
  s32 shiftX = 0;
  s32 shiftY = 0;
  switch (format) {
  case GX::TF_I4:
  case GX::TF_C4:
  case GX::TF_CMPR:
  case GX::CTF_R4:
  case GX::CTF_Z4:
    shiftX = 3;
    shiftY = 3;
    break;
  case GX::TF_I8:
  case GX::TF_IA4:
  case GX::TF_C8:
  case GX::TF_Z8:
  case GX::CTF_RA4:
  case GX::CTF_A8:
  case GX::CTF_R8:
  case GX::CTF_G8:
  case GX::CTF_B8:
  case GX::CTF_Z8M:
  case GX::CTF_Z8L:
    shiftX = 3;
    shiftY = 2;
    break;
  case GX::TF_IA8:
  case GX::TF_RGB565:
  case GX::TF_RGB5A3:
  case GX::TF_RGBA8:
  case GX::TF_C14X2:
  case GX::TF_Z16:
  case GX::TF_Z24X8:
  case GX::CTF_RA8:
  case GX::CTF_RG8:
  case GX::CTF_GB8:
  case GX::CTF_Z16L:
    shiftX = 2;
    shiftY = 2;
    break;
  default:
    break;
  }
  u32 bitSize = format == GX::TF_RGBA8 || format == GX::TF_Z24X8 ? 64 : 32;
  u32 bufLen = 0;
  if (mipmap) {
    while (max_lod != 0) {
      const u32 tileX = ((width + (1 << shiftX) - 1) >> shiftX);
      const u32 tileY = ((height + (1 << shiftY) - 1) >> shiftY);
      bufLen += bitSize * tileX * tileY;

      if (width == 1 && height == 1) {
        return bufLen;
      }

      width = (width < 2) ? 1 : width / 2;
      height = (height < 2) ? 1 : height / 2;
      --max_lod;
    };
  } else {
    const u32 tileX = ((width + (1 << shiftX) - 1) >> shiftX);
    const u32 tileY = ((height + (1 << shiftY) - 1) >> shiftY);
    bufLen = bitSize * tileX * tileY;
  }

  return bufLen;
}
namespace metaforce::WIP {

CTexture::CTexture(ETexelFormat fmt, s16 w, s16 h, s32 mips)
: x0_fmt(fmt)
, x4_w(w)
, x6_h(h)
, x8_mips(mips)
, x9_bitsPerPixel(TexelFormatBitsPerPixel(fmt))
, x64_frameAllocated(sCurrentFrameCount) {
  InitBitmapBuffers(fmt, w, h, mips);
  InitTextureObjs();
}

CTexture::CTexture(CInputStream& in, EAutoMipmap automip, EBlackKey blackKey) {
  x64_frameAllocated = sCurrentFrameCount;
  x0_fmt = ETexelFormat(in.ReadLong());
  x4_w = in.ReadShort();
  x6_h = in.ReadShort();
  x8_mips = in.ReadLong();

  bool hasPalette = (x0_fmt == ETexelFormat::C4 || x0_fmt == ETexelFormat::C8 || x0_fmt == ETexelFormat::C14X2);
  if (hasPalette) {
    x10_graphicsPalette = std::make_unique<CGraphicsPalette>(in);
    xa_25_hasPalette = true;
  }
  x9_bitsPerPixel = TexelFormatBitsPerPixel(x0_fmt);
  InitBitmapBuffers(x0_fmt, x4_w, x6_h, x8_mips);
  u32 bufLen = 0;
  if (x8_mips > 0) {
    for (u32 i = 0; i < x8_mips; ++i) {
      u32 curMip = i & 3;
      const u32 width = ROUND_UP_4(x4_w >> curMip);
      const u32 height = ROUND_UP_4(x6_h >> curMip);
      bufLen += (width * height * x9_bitsPerPixel) / 8;
    }
  }

  for (u32 i = 0, len = 0; i < bufLen; i += len) {
    len = bufLen - i;
    if (len > 256) {
      len = 256;
    }

    in.Get(x44_aramToken_x4_buff.get() + i, len);
    //DCFlushRangeNoSync(x44_aramToken_x4_buff.get() + i, ROUND_UP_32(len));
  }

  if (sMangleMips) {
    for (u32 i = 0; i < x8_mips; ++i) {
      MangleMipmap(i);
    }
  }

  InitTextureObjs();
}

void CTexture::InitBitmapBuffers(ETexelFormat fmt, s16 width, s16 height, s32 mips) {
  switch (fmt) {
  case ETexelFormat::I4:
    x18_gxFormat = GX::TF_I4;
    break;
  case ETexelFormat::I8:
    x18_gxFormat = GX::TF_I8;
    break;
  case ETexelFormat::IA4:
    x18_gxFormat = GX::TF_IA4;
    break;
  case ETexelFormat::IA8:
    x18_gxFormat = GX::TF_IA8;
    break;
  case ETexelFormat::C4:
    x1c_gxCIFormat = GX::TF_C4;
    break;
  case ETexelFormat::C8:
    x1c_gxCIFormat = GX::TF_C8;
    break;
  case ETexelFormat::C14X2:
    x1c_gxCIFormat = GX::TF_C14X2;
    break;
  case ETexelFormat::RGB565:
    x18_gxFormat = GX::TF_RGB565;
    break;
  case ETexelFormat::RGB5A3:
    x18_gxFormat = GX::TF_RGB5A3;
    break;
  case ETexelFormat::RGBA8:
    x18_gxFormat = GX::TF_RGBA8;
    break;
  case ETexelFormat::CMPR:
    x18_gxFormat = GX::TF_CMPR;
    break;
  default:
    break;
  }

  u32 format = (x0_fmt == ETexelFormat::C4 || x0_fmt == ETexelFormat::C8 || x0_fmt == ETexelFormat::C14X2)
                   ? x1c_gxCIFormat
                   : x18_gxFormat;

  /* I have no idea what they're doing with that last argument... */
  xc_memoryAllocated = GXGetTexBufferSize(width, height, format, mips > 1, static_cast<u8>(mips > 1) & 11);
  x44_aramToken_x4_buff.reset(new u8[xc_memoryAllocated]);
  /*x44_aramToken.PostConstruct(buf, xc_memoryAllocated, 1);*/
  CountMemory();
}
void CTexture::InitTextureObjs() {}

void CTexture::CountMemory() {
  if (xa_28_counted) {
    return;
  }

  xa_28_counted = true;
  sTotalAllocatedMemory += xc_memoryAllocated;
}

void CTexture::MangleMipmap(u32 mip) {
  // TODO(phil): Mangle mipmap
}

u32 CTexture::TexelFormatBitsPerPixel(ETexelFormat fmt) {
  switch (fmt) {
  case ETexelFormat::I4:
  case ETexelFormat::C4:
  case ETexelFormat::CMPR:
    return 4;
  case ETexelFormat::I8:
  case ETexelFormat::IA4:
  case ETexelFormat::C8:
    return 8;
  case ETexelFormat::IA8:
  case ETexelFormat::C14X2:
  case ETexelFormat::RGB565:
  case ETexelFormat::RGB5A3:
    return 16;
  case ETexelFormat::RGBA8:
    return 32;
  default:
    return 0;
  }
}

u32 CTexture::sCurrentFrameCount = 0;
u32 CTexture::sTotalAllocatedMemory = 0;
} // namespace metaforce::WIP