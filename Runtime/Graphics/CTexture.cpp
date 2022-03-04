#include "Graphics/CTexture.hpp"

#include "CToken.hpp"

#include <zeus/Math.hpp>
#include <magic_enum.hpp>

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

namespace metaforce {
static std::array<CTexture*, GX::MAX_TEXMAP> sLoadedTextures{};

CTexture::CTexture(ETexelFormat fmt, u16 w, u16 h, s32 mips, std::string_view label)
: x0_fmt(fmt)
, x4_w(w)
, x6_h(h)
, x8_mips(mips)
, x9_bitsPerPixel(TexelFormatBitsPerPixel(fmt))
, x64_frameAllocated(sCurrentFrameCount)
, m_label(fmt::format(FMT_STRING("{} ({})"), label, magic_enum::enum_name(fmt))) {
  InitBitmapBuffers(fmt, w, h, mips);
  InitTextureObjs(false);
}

CTexture::CTexture(CInputStream& in, std::string_view label, EAutoMipmap automip, EBlackKey blackKey)
: x0_fmt(ETexelFormat(in.ReadLong()))
, x4_w(in.ReadShort())
, x6_h(in.ReadShort())
, x8_mips(in.ReadLong())
, x64_frameAllocated(sCurrentFrameCount)
, m_label(fmt::format(FMT_STRING("{} ({})"), label, magic_enum::enum_name(x0_fmt))) {
  bool hasPalette = (x0_fmt == ETexelFormat::C4 || x0_fmt == ETexelFormat::C8 || x0_fmt == ETexelFormat::C14X2);
  if (hasPalette) {
    x10_graphicsPalette = std::make_unique<CGraphicsPalette>(in);
    xa_25_canLoadPalette = true;
  }
  x9_bitsPerPixel = TexelFormatBitsPerPixel(x0_fmt);
  InitBitmapBuffers(x0_fmt, x4_w, x6_h, x8_mips);
  u32 bufLen = 0;
  if (x8_mips > 0) {
    for (u32 i = 0; i < x8_mips; ++i) {
      u32 curMip = i & 63;
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

    auto image_ptr = /*x44_aramToken.GetMRAMSafe() */ x44_aramToken_x4_buff.get();
    in.Get(image_ptr + i, len);
    // DCFlushRangeNoSync(x44_aramToken_x4_buff.get() + i, ROUND_UP_32(len));
  }

  if (sMangleMips) {
    for (u32 i = 0; i < x8_mips; ++i) {
      MangleMipmap(i);
    }
  }

  InitTextureObjs(true);
}

u8* CTexture::Lock() {
  xa_24_locked = true;
  return GetBitMapData(0);
}

void CTexture::UnLock() {
  xa_24_locked = false;
  CountMemory();
  // DCFlushRange(x44_aramToken.GetMRAMSafe(), ROUND_UP_32(xc_memoryAllocated));
}

void CTexture::Load(GX::TexMapID id, EClampMode clamp) {
  if (sLoadedTextures[id] != this || xa_29_canLoadObj) {
    auto* image_ptr = /*x44_aramToken.GetMRAMSafe() */ x44_aramToken_x4_buff.get();
    CountMemory();
    if (HasPalette()) {
      x10_graphicsPalette->Load();
      xa_25_canLoadPalette = false;
    }
    xa_29_canLoadObj = false;
    if (x40_clampMode != clamp) {
      x40_clampMode = !xa_26_isPowerOfTwo ? EClampMode::Clamp : clamp;
      // GXInitTexObjWrapMode(x20_texObj, static_cast<u32>(x40_clampMode), static_cast<u32>(x40_clampMode));
    }

    // GXInitObjectData(x20_texObj, image_ptr);
    // GXLoadObj(x20_texObj, id);
    sLoadedTextures[id] = this;
    x64_frameAllocated = sCurrentFrameCount;
  }
}

void CTexture::LoadMipLevel(s32 mip, GX::TexMapID id, EClampMode clamp) {
  auto image_ptr = /*x44_aramToken.GetMRAMSafe() */ x44_aramToken_x4_buff.get();
  u32 width = x4_w;
  u32 height = x6_h;
  u32 iVar15 = 0;
  u32 offset = 0;
  if (mip > 0) {
    for (u32 i = 0; i < mip; ++i) {
      offset += ROUND_UP_32(x9_bitsPerPixel * (ROUND_UP_4(width) * ROUND_UP_4(height)));
      width /= 2;
      height /= 2;
    }
  }

  // GXTexObj texObj;
  // GXInitTexObj(&texObj, image_ptr + offset, width, height, x18_gxFormat);
  // GXInitTexObjLod(&texObj, GX_LINEAR, GX_LINEAR, 0.f, 1.f, 0.f, false, false, GX_ANISO_1);
  if (HasPalette()) {
    x10_graphicsPalette->Load();
    xa_25_canLoadPalette = false;
  }
  // GXLoadTexObj(&texObj, mapId);
  x64_frameAllocated = sCurrentFrameCount;
  sLoadedTextures[id] = nullptr;
}

void CTexture::MakeSwappable() {
  if (!xa_27_noSwap) {
    return;
  }

  xa_27_noSwap = false;
}

const u8* CTexture::GetConstBitMapData(s32 mip) const {
  u32 buffOffset = 0;
  if (x8_mips > 0) {
    for (u32 i = 0; i < x8_mips; ++i) {
      buffOffset += (x9_bitsPerPixel >> 3) * (x4_w >> (i & 0x3f)) * (x6_h >> (i & 0x3f));
    }
  }
  return x44_aramToken_x4_buff.get() + buffOffset; /* x44_aramToken.GetMRAMSafe() + buffOffset*/
}

u8* CTexture::GetBitMapData(s32 mip) const { return const_cast<u8*>(GetConstBitMapData(mip)); }

void CTexture::InitBitmapBuffers(ETexelFormat fmt, u16 width, u16 height, s32 mips) {
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

  xc_memoryAllocated = GXGetTexBufferSize(width, height, format, mips > 1, mips > 1 ? 11 : 0);
  x44_aramToken_x4_buff = std::make_unique<u8[]>(xc_memoryAllocated);
  /*x44_aramToken.PostConstruct(buf, xc_memoryAllocated, 1);*/
  CountMemory();
}

void CTexture::InitTextureObjs(bool write) {
  xa_26_isPowerOfTwo = zeus::floorPowerOfTwo(x4_w) == x4_w && zeus::floorPowerOfTwo(x6_h) == x6_h;

  if (!xa_26_isPowerOfTwo) {
    x40_clampMode = EClampMode::Clamp;
  }

  CountMemory();
  if (IsCITexture()) {
    // GXInitTexObjCI(x20_texObj, x44_aramToken_x4_buff.get(), x4_w, x6_h, x1c_gxCIFormat, u32(x40_clampMode),
    //                u32(x40_clampMode), x8_mips > 1, 0);
    // TODO
  } else {
    // GXInitTexObj(x20_texObj, x44_aramToken_x4_buff.get(), x4_w, x6_h, x1c_gxCIFormat, u32(x40_clampMode),
    //              u32(x40_clampMode), x8_mips > 1);
    // GXInitTexObjLOD(x20_texObj, x8_mips > 1 ? GX_LIN_MIP_LIN : GX_LINEAR, 0.f, static_cast<float>(x8_mips) - 1.f,
    // 0.f,
    //                 false, false, x8_mips > 1 ? GX_ANISO_4 : GX_ANISO_1);
    x20_texObj = aurora::gfx::new_dynamic_texture_2d(x4_w, x6_h, x8_mips, x0_fmt, m_label);
    if (write) {
      aurora::gfx::write_texture(x20_texObj, {x44_aramToken_x4_buff.get(), xc_memoryAllocated});
    }
  }
  xa_29_canLoadObj = true;
}

void CTexture::CountMemory() {
  if (xa_28_counted) {
    return;
  }

  xa_28_counted = true;
  sTotalAllocatedMemory += xc_memoryAllocated;
}

void CTexture::UncountMemory() {
  if (!xa_28_counted) {
    return;
  }

  xa_28_counted = false;
  sTotalAllocatedMemory -= xc_memoryAllocated;
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

bool CTexture::sMangleMips = false;
u32 CTexture::sCurrentFrameCount = 0;
u32 CTexture::sTotalAllocatedMemory = 0;

void CTexture::InvalidateTexMap(GX::TexMapID id) { sLoadedTextures[id] = nullptr; }

CFactoryFnReturn FTextureFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& vparms,
                                 CObjectReference* selfRef) {
  const auto label = fmt::format("{} {}", tag.type, tag.id);
  return TToken<CTexture>::GetIObjObjectFor(std::make_unique<CTexture>(in, label));
}
} // namespace metaforce
