#include "Runtime/Graphics/CTexture.hpp"

#include <array>

#include "Runtime/CSimplePool.hpp"
#include "Runtime/CToken.hpp"
#include "Runtime/Graphics/CGraphics.hpp"
#include "Runtime/CTextureCache.hpp"
#include "Runtime/GameGlobalObjects.hpp"

namespace metaforce {
namespace {
logvisor::Module Log("metaforce::CTextureBoo");

/* GX uses this upsampling technique to extract full 8-bit range */
constexpr u8 Convert3To8(u8 v) {
  /* Swizzle bits: 00000123 -> 12312312 */
  return static_cast<u8>((u32{v} << 5) | (u32{v} << 2) | (u32{v} >> 1));
}

constexpr u8 Convert4To8(u8 v) {
  /* Swizzle bits: 00001234 -> 12341234 */
  return static_cast<u8>((u32{v} << 4) | u32{v});
}

constexpr u8 Convert5To8(u8 v) {
  /* Swizzle bits: 00012345 -> 12345123 */
  return static_cast<u8>((u32{v} << 3) | (u32{v} >> 2));
}

constexpr u8 Convert6To8(u8 v) {
  /* Swizzle bits: 00123456 -> 12345612 */
  return static_cast<u8>((u32{v} << 2) | (u32{v} >> 4));
}
} // Anonymous namespace

size_t CTexture::ComputeMippedTexelCount() const {
  size_t w = x4_w;
  size_t h = x6_h;
  size_t ret = w * h;
  for (u32 i = x8_mips; i > 1; --i) {
    if (w > 1)
      w /= 2;
    if (h > 1)
      h /= 2;
    ret += w * h;
  }
  return ret;
}

size_t CTexture::ComputeMippedBlockCountDXT1() const {
  size_t w = x4_w / 4;
  size_t h = x6_h / 4;
  size_t ret = w * h;
  for (u32 i = x8_mips; i > 1; --i) {
    if (w > 1)
      w /= 2;
    if (h > 1)
      h /= 2;
    ret += w * h;
  }
  return ret;
}

void CTexture::BuildI4FromGCN(CInputStream& in) {
  m_booTex = hsh::create_texture2d({x4_w, x6_h}, hsh::RGBA8_UNORM, x8_mips, [&](void* data, std::size_t size) {
    RGBA8* buf = reinterpret_cast<RGBA8*>(data);
    int w = x4_w;
    int h = x6_h;
    RGBA8* targetMip = buf;
    for (u32 mip = 0; mip < x8_mips; ++mip) {
      int bwidth = (w + 7) / 8;
      int bheight = (h + 7) / 8;
      for (int by = 0; by < bheight; ++by) {
        int baseY = by * 8;
        for (int bx = 0; bx < bwidth; ++bx) {
          int baseX = bx * 8;
          for (int y = 0; y < 8; ++y) {
            RGBA8* target = targetMip + (baseY + y) * w + baseX;
            u8 source[4];
            in.readBytesToBuf(source, 4);
            for (int x = 0; x < 8; ++x) {
              target[x].r = Convert4To8(source[x / 2] >> ((x & 1) ? 0 : 4) & 0xf);
              target[x].g = target[x].r;
              target[x].b = target[x].r;
              target[x].a = target[x].r;
            }
          }
        }
      }
      targetMip += w * h;
      if (w > 1)
        w /= 2;
      if (h > 1)
        h /= 2;
    }
  });
}

void CTexture::BuildI8FromGCN(CInputStream& in) {
  m_booTex = hsh::create_texture2d({x4_w, x6_h}, hsh::RGBA8_UNORM, x8_mips, [&](void* data, std::size_t size) {
    RGBA8* buf = reinterpret_cast<RGBA8*>(data);
    int w = x4_w;
    int h = x6_h;
    RGBA8* targetMip = buf;
    for (u32 mip = 0; mip < x8_mips; ++mip) {
      int bwidth = (w + 7) / 8;
      int bheight = (h + 3) / 4;
      for (int by = 0; by < bheight; ++by) {
        int baseY = by * 4;
        for (int bx = 0; bx < bwidth; ++bx) {
          int baseX = bx * 8;
          for (int y = 0; y < 4; ++y) {
            RGBA8* target = targetMip + (baseY + y) * w + baseX;
            u8 source[8];
            in.readBytesToBuf(source, 8);
            for (int x = 0; x < 8; ++x) {
              target[x].r = source[x];
              target[x].g = source[x];
              target[x].b = source[x];
              target[x].a = source[x];
            }
          }
        }
      }
      targetMip += w * h;
      if (w > 1)
        w /= 2;
      if (h > 1)
        h /= 2;
    }
  });
}

void CTexture::BuildIA4FromGCN(CInputStream& in) {
  m_booTex = hsh::create_texture2d({x4_w, x6_h}, hsh::RGBA8_UNORM, x8_mips, [&](void* data, std::size_t size) {
    RGBA8* buf = reinterpret_cast<RGBA8*>(data);
    int w = x4_w;
    int h = x6_h;
    RGBA8* targetMip = buf;
    for (u32 mip = 0; mip < x8_mips; ++mip) {
      int bwidth = (w + 7) / 8;
      int bheight = (h + 3) / 4;
      for (int by = 0; by < bheight; ++by) {
        int baseY = by * 4;
        for (int bx = 0; bx < bwidth; ++bx) {
          int baseX = bx * 8;
          for (int y = 0; y < 4; ++y) {
            RGBA8* target = targetMip + (baseY + y) * w + baseX;
            u8 source[8];
            in.readBytesToBuf(source, 8);
            for (int x = 0; x < 8; ++x) {
              u8 intensity = Convert4To8(source[x] >> 4 & 0xf);
              target[x].r = intensity;
              target[x].g = intensity;
              target[x].b = intensity;
              target[x].a = Convert4To8(source[x] & 0xf);
            }
          }
        }
      }
      targetMip += w * h;
      if (w > 1)
        w /= 2;
      if (h > 1)
        h /= 2;
    }
  });
}

void CTexture::BuildIA8FromGCN(CInputStream& in) {
  m_booTex = hsh::create_texture2d({x4_w, x6_h}, hsh::RGBA8_UNORM, x8_mips, [&](void* data, std::size_t size) {
    RGBA8* buf = reinterpret_cast<RGBA8*>(data);
    int w = x4_w;
    int h = x6_h;
    RGBA8* targetMip = buf;
    for (u32 mip = 0; mip < x8_mips; ++mip) {
      int bwidth = (w + 3) / 4;
      int bheight = (h + 3) / 4;
      for (int by = 0; by < bheight; ++by) {
        int baseY = by * 4;
        for (int bx = 0; bx < bwidth; ++bx) {
          int baseX = bx * 4;
          for (int y = 0; y < 4; ++y) {
            RGBA8* target = targetMip + (baseY + y) * w + baseX;
            u16 source[4];
            in.readBytesToBuf(source, 8);
            for (int x = 0; x < 4; ++x) {
              u8 intensity = source[x] >> 8;
              target[x].r = intensity;
              target[x].g = intensity;
              target[x].b = intensity;
              target[x].a = source[x] & 0xff;
            }
          }
        }
      }
      targetMip += w * h;
      if (w > 1)
        w /= 2;
      if (h > 1)
        h /= 2;
    }
  });
}

static std::vector<CTexture::RGBA8> DecodePalette(int numEntries, CInputStream& in) {
  std::vector<CTexture::RGBA8> ret;
  ret.reserve(numEntries);

  enum class EPaletteType { IA8, RGB565, RGB5A3 };

  EPaletteType format = EPaletteType(in.readUint32Big());
  in.readUint32Big();
  switch (format) {
  case EPaletteType::IA8: {
    for (int e = 0; e < numEntries; ++e) {
      u8 intensity = in.readUByte();
      u8 alpha = in.readUByte();
      ret.push_back({intensity, intensity, intensity, alpha});
    }
    break;
  }
  case EPaletteType::RGB565: {
    for (int e = 0; e < numEntries; ++e) {
      u16 texel = in.readUint16Big();
      ret.push_back({Convert5To8(texel >> 11 & 0x1f), Convert6To8(texel >> 5 & 0x3f), Convert5To8(texel & 0x1f), 0xff});
    }
    break;
  }
  case EPaletteType::RGB5A3: {
    for (int e = 0; e < numEntries; ++e) {
      u16 texel = in.readUint16Big();
      if (texel & 0x8000) {
        ret.push_back(
            {Convert5To8(texel >> 10 & 0x1f), Convert5To8(texel >> 5 & 0x1f), Convert5To8(texel & 0x1f), 0xff});
      } else {
        ret.push_back({Convert4To8(texel >> 8 & 0xf), Convert4To8(texel >> 4 & 0xf), Convert4To8(texel & 0xf),
                       Convert3To8(texel >> 12 & 0x7)});
      }
    }
    break;
  }
  }
  return ret;
}

void CTexture::BuildC4FromGCN(CInputStream& in) {
  m_booTex = hsh::create_texture2d({x4_w, x6_h}, hsh::RGBA8_UNORM, x8_mips, [&](void* data, std::size_t size) {
    RGBA8* buf = reinterpret_cast<RGBA8*>(data);
    std::vector<RGBA8> palette = DecodePalette(16, in);

    int w = x4_w;
    int h = x6_h;
    RGBA8* targetMip = buf;
    for (u32 mip = 0; mip < x8_mips; ++mip) {
      int bwidth = (w + 7) / 8;
      int bheight = (h + 7) / 8;
      for (int by = 0; by < bheight; ++by) {
        int baseY = by * 8;
        for (int bx = 0; bx < bwidth; ++bx) {
          int baseX = bx * 8;
          for (int y = 0; y < 8; ++y) {
            RGBA8* target = targetMip + (baseY + y) * w + baseX;
            u8 source[4];
            in.readBytesToBuf(source, 4);
            for (int x = 0; x < 8; ++x)
              target[x] = palette[source[x / 2] >> ((x & 1) ? 0 : 4) & 0xf];
          }
        }
      }
      targetMip += w * h;
      if (w > 1)
        w /= 2;
      if (h > 1)
        h /= 2;
    }
  });
}

void CTexture::BuildC8FromGCN(CInputStream& in) {
  m_booTex = hsh::create_texture2d({x4_w, x6_h}, hsh::RGBA8_UNORM, x8_mips, [&](void* data, std::size_t size) {
    RGBA8* buf = reinterpret_cast<RGBA8*>(data);
    std::vector<RGBA8> palette = DecodePalette(256, in);

    int w = x4_w;
    int h = x6_h;
    RGBA8* targetMip = buf;
    for (u32 mip = 0; mip < x8_mips; ++mip) {
      int bwidth = (w + 7) / 8;
      int bheight = (h + 3) / 4;
      for (int by = 0; by < bheight; ++by) {
        int baseY = by * 4;
        for (int bx = 0; bx < bwidth; ++bx) {
          int baseX = bx * 8;
          for (int y = 0; y < 4; ++y) {
            RGBA8* target = targetMip + (baseY + y) * w + baseX;
            u8 source[8];
            in.readBytesToBuf(source, 8);
            for (int x = 0; x < 8; ++x)
              target[x] = palette[source[x]];
          }
        }
      }
      targetMip += w * h;
      if (w > 1)
        w /= 2;
      if (h > 1)
        h /= 2;
    }
  });
}

void CTexture::BuildC14X2FromGCN(CInputStream& in) {}

void CTexture::BuildRGB565FromGCN(CInputStream& in) {
  m_booTex = hsh::create_texture2d({x4_w, x6_h}, hsh::RGBA8_UNORM, x8_mips, [&](void* data, std::size_t size) {
    RGBA8* buf = reinterpret_cast<RGBA8*>(data);
    std::vector<RGBA8> palette = DecodePalette(256, in);

    int w = x4_w;
    int h = x6_h;
    RGBA8* targetMip = buf;
    for (u32 mip = 0; mip < x8_mips; ++mip) {
      int bwidth = (w + 3) / 4;
      int bheight = (h + 3) / 4;
      for (int by = 0; by < bheight; ++by) {
        int baseY = by * 4;
        for (int bx = 0; bx < bwidth; ++bx) {
          int baseX = bx * 4;
          for (int y = 0; y < 4; ++y) {
            RGBA8* target = targetMip + (baseY + y) * w + baseX;
            for (int x = 0; x < 4; ++x) {
              u16 texel = in.readUint16Big();
              target[x].r = Convert5To8(texel >> 11 & 0x1f);
              target[x].g = Convert6To8(texel >> 5 & 0x3f);
              target[x].b = Convert5To8(texel & 0x1f);
              target[x].a = 0xff;
            }
          }
        }
      }
      targetMip += w * h;
      if (w > 1)
        w /= 2;
      if (h > 1)
        h /= 2;
    }
  });
}

void CTexture::BuildRGB5A3FromGCN(CInputStream& in) {
  m_booTex = hsh::create_texture2d({x4_w, x6_h}, hsh::RGBA8_UNORM, x8_mips, [&](void* data, std::size_t size) {
    RGBA8* buf = reinterpret_cast<RGBA8*>(data);
    std::vector<RGBA8> palette = DecodePalette(256, in);

    int w = x4_w;
    int h = x6_h;
    RGBA8* targetMip = buf;
    for (u32 mip = 0; mip < x8_mips; ++mip) {
      int bwidth = (w + 3) / 4;
      int bheight = (h + 3) / 4;
      for (int by = 0; by < bheight; ++by) {
        int baseY = by * 4;
        for (int bx = 0; bx < bwidth; ++bx) {
          int baseX = bx * 4;
          for (int y = 0; y < 4; ++y) {
            RGBA8* target = targetMip + (baseY + y) * w + baseX;
            for (int x = 0; x < 4; ++x) {
              u16 texel = in.readUint16Big();
              if (texel & 0x8000) {
                target[x].r = Convert5To8(texel >> 10 & 0x1f);
                target[x].g = Convert5To8(texel >> 5 & 0x1f);
                target[x].b = Convert5To8(texel & 0x1f);
                target[x].a = 0xff;
              } else {
                target[x].r = Convert4To8(texel >> 8 & 0xf);
                target[x].g = Convert4To8(texel >> 4 & 0xf);
                target[x].b = Convert4To8(texel & 0xf);
                target[x].a = Convert3To8(texel >> 12 & 0x7);
              }
            }
          }
        }
      }
      targetMip += w * h;
      if (w > 1)
        w /= 2;
      if (h > 1)
        h /= 2;
    }
  });
}

void CTexture::BuildRGBA8FromGCN(CInputStream& in) {
  m_booTex = hsh::create_texture2d({x4_w, x6_h}, hsh::RGBA8_UNORM, x8_mips, [&](void* data, std::size_t size) {
    RGBA8* buf = reinterpret_cast<RGBA8*>(data);
    std::vector<RGBA8> palette = DecodePalette(256, in);

    int w = x4_w;
    int h = x6_h;
    RGBA8* targetMip = buf;
    for (u32 mip = 0; mip < x8_mips; ++mip) {
      int bwidth = (w + 3) / 4;
      int bheight = (h + 3) / 4;
      for (int by = 0; by < bheight; ++by) {
        int baseY = by * 4;
        for (int bx = 0; bx < bwidth; ++bx) {
          int baseX = bx * 4;
          for (int c = 0; c < 2; ++c) {
            for (int y = 0; y < 4; ++y) {
              RGBA8* target = targetMip + (baseY + y) * w + baseX;
              u8 source[8];
              in.readBytesToBuf(source, 8);
              for (int x = 0; x < 4; ++x) {
                if (c) {
                  target[x].g = source[x * 2];
                  target[x].b = source[x * 2 + 1];
                } else {
                  target[x].a = source[x * 2];
                  target[x].r = source[x * 2 + 1];
                }
              }
            }
          }
        }
      }
      targetMip += w * h;
      if (w > 1)
        w /= 2;
      if (h > 1)
        h /= 2;
    }
  });
}

void CTexture::BuildRGBA8(const void* dataIn, size_t length) {
  m_booTex = hsh::create_texture2d({x4_w, x6_h}, hsh::RGBA8_UNORM, x8_mips, [&](void* data, std::size_t size) {
    std::memcpy(data, dataIn, length);
  });
}

void CTexture::BuildC8(const void* data, size_t length) {
  uint32_t nentries = hecl::SBig(*reinterpret_cast<const uint32_t*>(data));
  const u8* paletteTexels = reinterpret_cast<const u8*>(data) + 4;
  const u8* texels = reinterpret_cast<const u8*>(data) + 4 + nentries * 4;

  m_paletteTex = hsh::create_texture2d({nentries, 1}, hsh::RGBA8_UNORM, 1, [&](void* data, std::size_t size) {
    std::memcpy(data, paletteTexels, nentries * 4);
  });

  m_booTex = hsh::create_texture2d({x4_w, x6_h}, hsh::R8_UNORM, x8_mips, [&](void* data, std::size_t size) {
    std::memcpy(data, texels, size);
  });
}

void CTexture::BuildC8Font(const void* data, EFontType ftype) {
  size_t texelCount = ComputeMippedTexelCount();

  size_t layerCount = 1;
  switch (ftype) {
  case EFontType::OneLayer:
  case EFontType::OneLayerOutline:
    layerCount = 1;
    break;
  case EFontType::FourLayers:
    layerCount = 4;
    break;
  case EFontType::TwoLayersOutlines:
  case EFontType::TwoLayers:
  case EFontType::TwoLayersOutlines2:
    layerCount = 2;
    break;
  default:
    break;
  }

  const uint32_t nentries = hecl::SBig(*reinterpret_cast<const uint32_t*>(data));
  const u8* texels = reinterpret_cast<const u8*>(data) + 4 + nentries * 4;
  auto buf = std::make_unique<RGBA8[]>(texelCount * layerCount);

  size_t w = x4_w;
  size_t h = x6_h;
  RGBA8* bufCur = buf.get();
  for (size_t i = 0; i < x8_mips; ++i) {
    size_t tCount = w * h;
    RGBA8* l0 = bufCur;
    RGBA8* l1 = bufCur + tCount;
    RGBA8* l2 = bufCur + tCount * 2;
    RGBA8* l3 = bufCur + tCount * 3;
    for (size_t j = 0; j < tCount; ++j) {
      u8 texel = texels[j];
      switch (ftype) {
      case EFontType::OneLayer:
        l0[j].r = (texel & 0x1) ? 0xff : 0;
        l0[j].a = 0xff;
        break;
      case EFontType::OneLayerOutline:
        l0[j].r = (texel & 0x1) ? 0xff : 0;
        l0[j].g = (texel & 0x2) ? 0xff : 0;
        l0[j].a = 0xff;
        break;
      case EFontType::FourLayers:
        l0[j].r = (texel & 0x1) ? 0xff : 0;
        l0[j].a = 0xff;
        l1[j].r = (texel & 0x2) ? 0xff : 0;
        l1[j].a = 0xff;
        l2[j].r = (texel & 0x4) ? 0xff : 0;
        l2[j].a = 0xff;
        l3[j].r = (texel & 0x8) ? 0xff : 0;
        l3[j].a = 0xff;
        break;
      case EFontType::TwoLayersOutlines:
        l0[j].r = (texel & 0x1) ? 0xff : 0;
        l0[j].g = (texel & 0x2) ? 0xff : 0;
        l0[j].a = 0xff;
        l1[j].r = (texel & 0x4) ? 0xff : 0;
        l1[j].g = (texel & 0x8) ? 0xff : 0;
        l1[j].a = 0xff;
        break;
      case EFontType::TwoLayers:
        l0[j].r = (texel & 0x1) ? 0xff : 0;
        l0[j].a = 0xff;
        l1[j].r = (texel & 0x2) ? 0xff : 0;
        l1[j].a = 0xff;
        break;
      case EFontType::TwoLayersOutlines2:
        l0[j].r = (texel & 0x4) ? 0xff : 0;
        l0[j].g = (texel & 0x1) ? 0xff : 0;
        l0[j].a = 0xff;
        l1[j].r = (texel & 0x8) ? 0xff : 0;
        l1[j].g = (texel & 0x2) ? 0xff : 0;
        l1[j].a = 0xff;
        break;
      default:
        break;
      }
    }
    texels += tCount;
    bufCur += tCount * layerCount;
    if (w > 1)
      w /= 2;
    if (h > 1)
      h /= 2;
  }

  m_booTex = hsh::create_texture2d_array({x4_w, x6_h}, layerCount, hsh::RGBA8_UNORM, x8_mips, [&](void* data, std::size_t size) {
    std::memcpy(data, buf.get(), size);
  });
}

void CTexture::BuildDXT1(const void* dataIn, size_t length) {
  m_booTex = hsh::create_texture2d({x4_w, x6_h}, hsh::BC1_UNORM, x8_mips, [&](void* data, std::size_t size) {
    std::memcpy(data, dataIn, size);
  });
}

void CTexture::BuildDXT3(const void* dataIn, size_t length) {
  m_booTex = hsh::create_texture2d({x4_w, x6_h}, hsh::BC2_UNORM, x8_mips, [&](void* data, std::size_t size) {
    std::memcpy(data, dataIn, size);
  });
}

CTexture::CTexture(ETexelFormat fmt, s16 w, s16 h, s32 mips) : x0_fmt(fmt), x4_w(w), x6_h(h), x8_mips(mips) {
  /*
  x64_ = sMangleMipmaps;
  InitBitmapBuffers(fmt, w, h, mips);
  InitTextureObjs();
  */
}

CTexture::CTexture(std::unique_ptr<u8[]>&& in, u32 length, bool otex, const CTextureInfo* inf) {
  m_textureInfo = inf;
  std::unique_ptr<u8[]> owned = std::move(in);
  athena::io::MemoryReader r(owned.get(), length);
  x0_fmt = ETexelFormat(r.readUint32Big());
  x4_w = r.readUint16Big();
  x6_h = r.readUint16Big();
  x8_mips = r.readUint32Big();

  switch (x0_fmt) {
  case ETexelFormat::I4:
    BuildI4FromGCN(r);
    break;
  case ETexelFormat::I8:
    BuildI8FromGCN(r);
    break;
  case ETexelFormat::IA4:
    BuildIA4FromGCN(r);
    break;
  case ETexelFormat::IA8:
    BuildIA8FromGCN(r);
    break;
  case ETexelFormat::C4:
    BuildC4FromGCN(r);
    break;
  case ETexelFormat::C8:
    BuildC8FromGCN(r);
    break;
  case ETexelFormat::C14X2:
    BuildC14X2FromGCN(r);
    break;
  case ETexelFormat::RGB565:
    BuildRGB565FromGCN(r);
    break;
  case ETexelFormat::RGB5A3:
    BuildRGB5A3FromGCN(r);
    break;
  case ETexelFormat::RGBA8:
    BuildRGBA8FromGCN(r);
    break;
//  case ETexelFormat::CMPR:
//    BuildDXT1FromGCN(r);
//    break;
  case ETexelFormat::RGBA8PC:
    BuildRGBA8(owned.get() + 12, length - 12);
    break;
  case ETexelFormat::C8PC:
    BuildC8(owned.get() + 12, length - 12);
    otex = true;
    break;
  case ETexelFormat::CMPRPC:
    BuildDXT1(owned.get() + 12, length - 12);
    break;
  case ETexelFormat::CMPRPCA:
    BuildDXT3(owned.get() + 12, length - 12);
    break;
  default:
    Log.report(logvisor::Fatal, FMT_STRING("invalid texture type {} for boo"), int(x0_fmt));
  }

  if (otex)
    m_otex = std::move(owned);
}

void CTexture::Load(int slot, EClampMode clamp) const {}

std::unique_ptr<u8[]> CTexture::BuildMemoryCardTex(u32& sizeOut, ETexelFormat& fmtOut,
                                                   std::unique_ptr<u8[]>& paletteOut) const {
  if (!m_otex)
    Log.report(logvisor::Fatal, FMT_STRING("MemoryCard TXTR not loaded with 'otex'"));

  size_t texelCount = x4_w * x6_h;
  std::unique_ptr<u8[]> ret;
  if (x0_fmt == ETexelFormat::RGBA8PC) {
    sizeOut = texelCount * 2;
    fmtOut = ETexelFormat::RGB5A3;
    ret.reset(new u8[sizeOut]);
    u16* texel = reinterpret_cast<u16*>(ret.get());

    int w = x4_w;
    int h = x6_h;
    const RGBA8* sourceMip = reinterpret_cast<const RGBA8*>(m_otex.get() + 12);
    int bwidth = (w + 3) / 4;
    int bheight = (h + 3) / 4;
    for (int by = 0; by < bheight; ++by) {
      int baseY = by * 4;
      for (int bx = 0; bx < bwidth; ++bx) {
        int baseX = bx * 4;
        for (int y = 0; y < 4; ++y) {
          const RGBA8* source = sourceMip + (x6_h - (baseY + y) - 1) * w + baseX;
          for (int x = 0; x < 4; ++x) {
            if (source[x].a == 0xff) {
              *texel++ = hecl::SBig(u16((source[x].r >> 3 << 10) | (source[x].g >> 3 << 5) | (source[x].b >> 3)));
            } else {
              *texel++ = hecl::SBig(u16((source[x].r >> 4 << 8) | (source[x].g >> 4 << 4) | (source[x].b >> 4) |
                                        (source[x].a >> 5 << 12)));
            }
          }
        }
      }
    }
  } else if (x0_fmt == ETexelFormat::C8PC) {
    sizeOut = texelCount;
    fmtOut = ETexelFormat::C8;
    ret.reset(new u8[sizeOut]);
    paletteOut.reset(new u8[512]);
    u8* texel = ret.get();
    u16* paletteColors = reinterpret_cast<u16*>(paletteOut.get());

    int w = x4_w;
    int h = x6_h;
    const u8* data = m_otex.get() + 12;
    u32 nentries = hecl::SBig(*reinterpret_cast<const u32*>(data));
    const RGBA8* paletteTexels = reinterpret_cast<const RGBA8*>(data + 4);
    const u8* sourceMip = data + 4 + nentries * 4;

    for (u32 i = 0; i < 256; ++i) {
      u16& color = paletteColors[i];
      if (i >= nentries) {
        color = 0;
      } else {
        const RGBA8& colorIn = paletteTexels[i];
        if (colorIn.a == 0xff) {
          color = hecl::SBig(u16((colorIn.r >> 3 << 10) | (colorIn.g >> 3 << 5) | (colorIn.b >> 3) | 0x8000));
        } else {
          color = hecl::SBig(
              u16((colorIn.r >> 4 << 8) | (colorIn.g >> 4 << 4) | (colorIn.b >> 4) | (colorIn.a >> 5 << 12)));
        }
      }
    }

    int bwidth = (w + 7) / 8;
    int bheight = (h + 3) / 4;
    for (int by = 0; by < bheight; ++by) {
      int baseY = by * 4;
      for (int bx = 0; bx < bwidth; ++bx) {
        int baseX = bx * 8;
        for (int y = 0; y < 4; ++y) {
          const u8* source = sourceMip + (x6_h - (baseY + y) - 1) * w + baseX;
          for (int x = 0; x < 8; ++x)
            *texel++ = source[x];
        }
      }
    }
  } else
    Log.report(logvisor::Fatal, FMT_STRING("MemoryCard texture may only use RGBA8PC or C8PC format"));

  return ret;
}

hsh::texture2d_array CTexture::GetFontTexture(EFontType tp) {
  if (m_ftype != tp && x0_fmt == ETexelFormat::C8PC) {
    m_ftype = tp;
    BuildC8Font(m_otex.get() + 12, m_ftype);
  }
  return m_booTex.get();
}

CFactoryFnReturn FTextureFactory(const metaforce::SObjectTag& tag, std::unique_ptr<u8[]>&& in, u32 len,
                                 const metaforce::CVParamTransfer& vparms, CObjectReference* selfRef) {
  u32 u32Owned = vparms.GetOwnedObj<u32>();
  const CTextureInfo* inf = nullptr;
  if (g_TextureCache)
    inf = g_TextureCache->GetTextureInfo(tag.id);
  return TToken<CTexture>::GetIObjObjectFor(
      std::make_unique<CTexture>(std::move(in), len, u32Owned == SBIG('OTEX'), inf));
}

} // namespace metaforce
