#include "Runtime/Graphics/CTexture.hpp"

#include <array>

#include "Runtime/CBasics.hpp"
#include "Runtime/CSimplePool.hpp"
#include "Runtime/CTextureCache.hpp"
#include "Runtime/CToken.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Graphics/CGraphics.hpp"

#include <logvisor/logvisor.hpp>

namespace metaforce {
namespace {
logvisor::Module Log("metaforce::CTextureBoo");

struct RGBA8 {
  u8 r;
  u8 g;
  u8 b;
  u8 a;
};

struct DXT1Block {
  u16 color1;
  u16 color2;
  std::array<u8, 4> lines;
};

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

void CTexture::BuildI4FromGCN(CInputStream& in, aurora::zstring_view label) {
  const size_t texelCount = ComputeMippedTexelCount();
  std::unique_ptr<RGBA8[]> buf(new RGBA8[texelCount]);

  int w = x4_w;
  int h = x6_h;
  RGBA8* targetMip = buf.get();
  for (u32 mip = 0; mip < x8_mips; ++mip) {
    const int bwidth = (w + 7) / 8;
    const int bheight = (h + 7) / 8;
    for (int by = 0; by < bheight; ++by) {
      const int baseY = by * 8;
      for (int bx = 0; bx < bwidth; ++bx) {
        const int baseX = bx * 8;
        for (int y = 0; y < std::min(h, 8); ++y) {
          RGBA8* target = targetMip + (baseY + y) * w + baseX;
          std::array<u8, 4> source;
          in.Get(source.data(), std::min(size_t(w) / 4, source.size()));
          for (size_t x = 0; x < std::min(w, 8); ++x) {
            target[x].r = Convert4To8(source[x / 2] >> ((x & 1) ? 0 : 4) & 0xf);
            target[x].g = target[x].r;
            target[x].b = target[x].r;
            target[x].a = target[x].r;
          }
        }
      }
    }
    targetMip += w * h;
    if (w > 1) {
      w /= 2;
    }
    if (h > 1) {
      h /= 2;
    }
  }

  m_tex = aurora::gfx::new_static_texture_2d(x4_w, x6_h, x8_mips, aurora::gfx::TextureFormat::RGBA8,
                                             {reinterpret_cast<const uint8_t*>(buf.get()), texelCount * 4}, label);
}

void CTexture::BuildI8FromGCN(CInputStream& in, aurora::zstring_view label) {
  const size_t texelCount = ComputeMippedTexelCount();
  std::unique_ptr<RGBA8[]> buf(new RGBA8[texelCount]);

  int w = x4_w;
  int h = x6_h;
  RGBA8* targetMip = buf.get();
  for (u32 mip = 0; mip < x8_mips; ++mip) {
    const int bwidth = (w + 7) / 8;
    const int bheight = (h + 3) / 4;
    for (int by = 0; by < bheight; ++by) {
      const int baseY = by * 4;
      for (int bx = 0; bx < bwidth; ++bx) {
        const int baseX = bx * 8;
        for (int y = 0; y < 4; ++y) {
          RGBA8* target = targetMip + (baseY + y) * w + baseX;
          std::array<u8, 8> source;
          in.Get(source.data(), source.size());
          for (size_t x = 0; x < source.size(); ++x) {
            target[x].r = source[x];
            target[x].g = source[x];
            target[x].b = source[x];
            target[x].a = source[x];
          }
        }
      }
    }
    targetMip += w * h;
    if (w > 1) {
      w /= 2;
    }
    if (h > 1) {
      h /= 2;
    }
  }

  m_tex = aurora::gfx::new_static_texture_2d(x4_w, x6_h, x8_mips, aurora::gfx::TextureFormat::RGBA8,
                                             {reinterpret_cast<const uint8_t*>(buf.get()), texelCount * 4}, label);
}

void CTexture::BuildIA4FromGCN(CInputStream& in, aurora::zstring_view label) {
  const size_t texelCount = ComputeMippedTexelCount();
  std::unique_ptr<RGBA8[]> buf(new RGBA8[texelCount]);

  int w = x4_w;
  int h = x6_h;
  RGBA8* targetMip = buf.get();
  for (u32 mip = 0; mip < x8_mips; ++mip) {
    const int bwidth = (w + 7) / 8;
    const int bheight = (h + 3) / 4;
    for (int by = 0; by < bheight; ++by) {
      const int baseY = by * 4;
      for (int bx = 0; bx < bwidth; ++bx) {
        const int baseX = bx * 8;
        for (int y = 0; y < 4; ++y) {
          RGBA8* target = targetMip + (baseY + y) * w + baseX;
          std::array<u8, 8> source;
          in.Get(source.data(), source.size());
          for (size_t x = 0; x < source.size(); ++x) {
            const u8 intensity = Convert4To8(source[x] >> 4 & 0xf);
            target[x].r = intensity;
            target[x].g = intensity;
            target[x].b = intensity;
            target[x].a = Convert4To8(source[x] & 0xf);
          }
        }
      }
    }
    targetMip += w * h;
    if (w > 1) {
      w /= 2;
    }
    if (h > 1) {
      h /= 2;
    }
  }

  m_tex = aurora::gfx::new_static_texture_2d(x4_w, x6_h, x8_mips, aurora::gfx::TextureFormat::RGBA8,
                                             {reinterpret_cast<const uint8_t*>(buf.get()), texelCount * 4}, label);
}

void CTexture::BuildIA8FromGCN(CInputStream& in, aurora::zstring_view label) {
  const size_t texelCount = ComputeMippedTexelCount();
  std::unique_ptr<RGBA8[]> buf(new RGBA8[texelCount]);

  int w = x4_w;
  int h = x6_h;
  RGBA8* targetMip = buf.get();
  for (u32 mip = 0; mip < x8_mips; ++mip) {
    const int bwidth = (w + 3) / 4;
    const int bheight = (h + 3) / 4;
    for (int by = 0; by < bheight; ++by) {
      const int baseY = by * 4;
      for (int bx = 0; bx < bwidth; ++bx) {
        const int baseX = bx * 4;
        for (int y = 0; y < 4; ++y) {
          RGBA8* target = targetMip + (baseY + y) * w + baseX;
          std::array<u16, 4> source;
          in.Get(reinterpret_cast<u8*>(source.data()), sizeof(source));
          for (size_t x = 0; x < source.size(); ++x) {
            const u8 intensity = source[x] >> 8;
            target[x].r = intensity;
            target[x].g = intensity;
            target[x].b = intensity;
            target[x].a = source[x] & 0xff;
          }
        }
      }
    }
    targetMip += w * h;
    if (w > 1) {
      w /= 2;
    }
    if (h > 1) {
      h /= 2;
    }
  }

  m_tex = aurora::gfx::new_static_texture_2d(x4_w, x6_h, x8_mips, aurora::gfx::TextureFormat::RGBA8,
                                             {reinterpret_cast<const uint8_t*>(buf.get()), texelCount * 4}, label);
}

static std::vector<RGBA8> DecodePalette(int numEntries, CInputStream& in) {
  std::vector<RGBA8> ret;
  ret.reserve(numEntries);

  enum class EPaletteType { IA8, RGB565, RGB5A3 };

  EPaletteType format = EPaletteType(in.ReadLong());
  in.ReadLong();
  switch (format) {
  case EPaletteType::IA8: {
    for (int e = 0; e < numEntries; ++e) {
      u8 intensity = in.ReadUint8();
      u8 alpha = in.ReadUint8();
      ret.push_back({intensity, intensity, intensity, alpha});
    }
    break;
  }
  case EPaletteType::RGB565: {
    for (int e = 0; e < numEntries; ++e) {
      u16 texel = in.ReadShort();
      ret.push_back({Convert5To8(texel >> 11 & 0x1f), Convert6To8(texel >> 5 & 0x3f), Convert5To8(texel & 0x1f), 0xff});
    }
    break;
  }
  case EPaletteType::RGB5A3: {
    for (int e = 0; e < numEntries; ++e) {
      u16 texel = in.ReadShort();
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

void CTexture::BuildC4FromGCN(CInputStream& in, aurora::zstring_view label) {
  const size_t texelCount = ComputeMippedTexelCount();
  std::unique_ptr<RGBA8[]> buf(new RGBA8[texelCount]);
  std::vector<RGBA8> palette = DecodePalette(16, in);

  int w = x4_w;
  int h = x6_h;
  RGBA8* targetMip = buf.get();
  for (u32 mip = 0; mip < x8_mips; ++mip) {
    const int bwidth = (w + 7) / 8;
    const int bheight = (h + 7) / 8;
    for (int by = 0; by < bheight; ++by) {
      const int baseY = by * 8;
      for (int bx = 0; bx < bwidth; ++bx) {
        const int baseX = bx * 8;
        for (int y = 0; y < 8; ++y) {
          RGBA8* target = targetMip + (baseY + y) * w + baseX;
          std::array<u8, 4> source;
          in.Get(source.data(), source.size());
          for (size_t x = 0; x < 8; ++x) {
            target[x] = palette[source[x / 2] >> ((x & 1) ? 0 : 4) & 0xf];
          }
        }
      }
    }
    targetMip += w * h;
    if (w > 1) {
      w /= 2;
    }
    if (h > 1) {
      h /= 2;
    }
  }

  m_tex = aurora::gfx::new_static_texture_2d(x4_w, x6_h, x8_mips, aurora::gfx::TextureFormat::RGBA8,
                                             {reinterpret_cast<const uint8_t*>(buf.get()), texelCount * 4}, label);
}

void CTexture::BuildC8FromGCN(CInputStream& in, aurora::zstring_view label) {
  const size_t texelCount = ComputeMippedTexelCount();
  std::unique_ptr<RGBA8[]> buf(new RGBA8[texelCount]);
  std::vector<RGBA8> palette = DecodePalette(256, in);

  int w = x4_w;
  int h = x6_h;
  RGBA8* targetMip = buf.get();
  for (u32 mip = 0; mip < x8_mips; ++mip) {
    const int bwidth = (w + 7) / 8;
    const int bheight = (h + 3) / 4;
    for (int by = 0; by < bheight; ++by) {
      const int baseY = by * 4;
      for (int bx = 0; bx < bwidth; ++bx) {
        const int baseX = bx * 8;
        for (int y = 0; y < 4; ++y) {
          RGBA8* target = targetMip + (baseY + y) * w + baseX;
          std::array<u8, 8> source;
          in.Get(source.data(), source.size());
          for (size_t x = 0; x < source.size(); ++x) {
            target[x] = palette[source[x]];
          }
        }
      }
    }
    targetMip += w * h;
    if (w > 1) {
      w /= 2;
    }
    if (h > 1) {
      h /= 2;
    }
  }

  m_tex = aurora::gfx::new_static_texture_2d(x4_w, x6_h, x8_mips, aurora::gfx::TextureFormat::RGBA8,
                                             {reinterpret_cast<const uint8_t*>(buf.get()), texelCount * 4}, label);
}

void CTexture::BuildC14X2FromGCN(CInputStream& in, aurora::zstring_view label) {
  Log.report(logvisor::Fatal, FMT_STRING("C14X2 not implemented"));
}

void CTexture::BuildRGB565FromGCN(CInputStream& in, aurora::zstring_view label) {
  const size_t texelCount = ComputeMippedTexelCount();
  std::unique_ptr<RGBA8[]> buf(new RGBA8[texelCount]);

  int w = x4_w;
  int h = x6_h;
  RGBA8* targetMip = buf.get();
  for (u32 mip = 0; mip < x8_mips; ++mip) {
    const int bwidth = (w + 3) / 4;
    const int bheight = (h + 3) / 4;
    for (int by = 0; by < bheight; ++by) {
      const int baseY = by * 4;
      for (int bx = 0; bx < bwidth; ++bx) {
        const int baseX = bx * 4;
        for (int y = 0; y < 4; ++y) {
          RGBA8* target = targetMip + (baseY + y) * w + baseX;
          for (size_t x = 0; x < 4; ++x) {
            const u16 texel = in.ReadShort();
            target[x].r = Convert5To8(texel >> 11 & 0x1f);
            target[x].g = Convert6To8(texel >> 5 & 0x3f);
            target[x].b = Convert5To8(texel & 0x1f);
            target[x].a = 0xff;
          }
        }
      }
    }
    targetMip += w * h;
    if (w > 1) {
      w /= 2;
    }
    if (h > 1) {
      h /= 2;
    }
  }

  m_tex = aurora::gfx::new_static_texture_2d(x4_w, x6_h, x8_mips, aurora::gfx::TextureFormat::RGBA8,
                                             {reinterpret_cast<const uint8_t*>(buf.get()), texelCount * 4}, label);
}

void CTexture::BuildRGB5A3FromGCN(CInputStream& in, aurora::zstring_view label) {
  size_t texelCount = ComputeMippedTexelCount();
  std::unique_ptr<RGBA8[]> buf(new RGBA8[texelCount]);

  int w = x4_w;
  int h = x6_h;
  RGBA8* targetMip = buf.get();
  for (u32 mip = 0; mip < x8_mips; ++mip) {
    const int bwidth = (w + 3) / 4;
    const int bheight = (h + 3) / 4;
    for (int by = 0; by < bheight; ++by) {
      const int baseY = by * 4;
      for (int bx = 0; bx < bwidth; ++bx) {
        const int baseX = bx * 4;
        for (int y = 0; y < 4; ++y) {
          RGBA8* target = targetMip + (baseY + y) * w + baseX;
          for (size_t x = 0; x < 4; ++x) {
            const u16 texel = in.ReadShort();
            if ((texel & 0x8000) != 0) {
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
    if (w > 1) {
      w /= 2;
    }
    if (h > 1) {
      h /= 2;
    }
  }

  m_tex = aurora::gfx::new_static_texture_2d(x4_w, x6_h, x8_mips, aurora::gfx::TextureFormat::RGBA8,
                                             {reinterpret_cast<const uint8_t*>(buf.get()), texelCount * 4}, label);
}

void CTexture::BuildRGBA8FromGCN(CInputStream& in, aurora::zstring_view label) {
  const size_t texelCount = ComputeMippedTexelCount();
  std::unique_ptr<RGBA8[]> buf(new RGBA8[texelCount]);

  int w = x4_w;
  int h = x6_h;
  RGBA8* targetMip = buf.get();
  for (u32 mip = 0; mip < x8_mips; ++mip) {
    const int bwidth = (w + 3) / 4;
    const int bheight = (h + 3) / 4;
    for (int by = 0; by < bheight; ++by) {
      const int baseY = by * 4;
      for (int bx = 0; bx < bwidth; ++bx) {
        const int baseX = bx * 4;
        for (int c = 0; c < 2; ++c) {
          for (int y = 0; y < 4; ++y) {
            RGBA8* target = targetMip + (baseY + y) * w + baseX;
            std::array<u8, 8> source;
            in.Get(source.data(), source.size());
            for (size_t x = 0; x < 4; ++x) {
              if (c != 0) {
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
    if (w > 1) {
      w /= 2;
    }
    if (h > 1) {
      h /= 2;
    }
  }

  m_tex = aurora::gfx::new_static_texture_2d(x4_w, x6_h, x8_mips, aurora::gfx::TextureFormat::RGBA8,
                                             {reinterpret_cast<const uint8_t*>(buf.get()), texelCount * 4}, label);
}

void CTexture::BuildDXT1FromGCN(CInputStream& in, aurora::zstring_view label) {
  const size_t blockCount = ComputeMippedBlockCountDXT1();
  std::unique_ptr<DXT1Block[]> buf(new DXT1Block[blockCount]);

  int w = x4_w / 4;
  int h = x6_h / 4;
  DXT1Block* targetMip = buf.get();
  for (u32 mip = 0; mip < x8_mips; ++mip) {
    const int bwidth = (w + 1) / 2;
    const int bheight = (h + 1) / 2;
    for (int by = 0; by < bheight; ++by) {
      const int baseY = by * 2;
      for (int bx = 0; bx < bwidth; ++bx) {
        const int baseX = bx * 2;
        for (int y = 0; y < 2; ++y) {
          DXT1Block* target = targetMip + (baseY + y) * w + baseX;
          std::array<DXT1Block, 2> source;
          in.Get(reinterpret_cast<u8*>(source.data()), sizeof(source));
          for (size_t x = 0; x < source.size(); ++x) {
            target[x].color1 = CBasics::SwapBytes(source[x].color1);
            target[x].color2 = CBasics::SwapBytes(source[x].color2);
            for (size_t i = 0; i < 4; ++i) {
              std::array<u8, 4> ind;
              const u8 packed = source[x].lines[i];
              ind[3] = packed & 0x3;
              ind[2] = (packed >> 2) & 0x3;
              ind[1] = (packed >> 4) & 0x3;
              ind[0] = (packed >> 6) & 0x3;
              target[x].lines[i] = ind[0] | (ind[1] << 2) | (ind[2] << 4) | (ind[3] << 6);
            }
          }
        }
      }
    }
    targetMip += w * h;

    if (w > 1) {
      w /= 2;
    }
    if (h > 1) {
      h /= 2;
    }
  }

  m_tex = aurora::gfx::new_static_texture_2d(x4_w, x6_h, x8_mips, aurora::gfx::TextureFormat::DXT1,
                                             {reinterpret_cast<const uint8_t*>(buf.get()), blockCount * 8}, label);
}

void CTexture::BuildRGBA8(const void* data, size_t length, aurora::zstring_view label) {
  size_t texelCount = ComputeMippedTexelCount();
  size_t expectedSize = texelCount * 4;
  if (expectedSize > length)
    Log.report(logvisor::Fatal, FMT_STRING("insufficient TXTR length ({}/{})"), length, expectedSize);

  m_tex = aurora::gfx::new_static_texture_2d(x4_w, x6_h, x8_mips, aurora::gfx::TextureFormat::RGBA8,
                                             {reinterpret_cast<const uint8_t*>(data), expectedSize}, label);
}

void CTexture::BuildC8(const void* data, size_t length, aurora::zstring_view label) {
  size_t texelCount = ComputeMippedTexelCount();
  if (texelCount > length)
    Log.report(logvisor::Fatal, FMT_STRING("insufficient TXTR length ({}/{})"), length, texelCount);

  uint32_t nentries = CBasics::SwapBytes(*reinterpret_cast<const uint32_t*>(data));
  const u8* paletteTexels = reinterpret_cast<const u8*>(data) + 4;
  const u8* texels = reinterpret_cast<const u8*>(data) + 4 + nentries * 4;
  m_paletteTex = aurora::gfx::new_static_texture_2d(nentries, 1, 1, aurora::gfx::TextureFormat::RGBA8,
                                                    {paletteTexels, nentries * 4}, label);
  m_tex = aurora::gfx::new_static_texture_2d(x4_w, x6_h, x8_mips, aurora::gfx::TextureFormat::R8, {texels, texelCount},
                                             label);
}

void CTexture::BuildC8Font(const void* data, EFontType ftype, aurora::zstring_view label) {
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

  const uint32_t nentries = CBasics::SwapBytes(*reinterpret_cast<const uint32_t*>(data));
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

  // TODO array tex
  //  CGraphics::CommitResources([&](boo::IGraphicsDataFactory::Context& ctx) {
  //    m_booTex = ctx.newStaticArrayTexture(x4_w, x6_h, layerCount, x8_mips, boo::TextureFormat::RGBA8,
  //                                         boo::TextureClampMode::Repeat, buf.get(), texelCount * layerCount * 4)
  //                   .get();
  //    return true;
  //  } BooTrace);
}

void CTexture::BuildDXT1(const void* data, size_t length, aurora::zstring_view label) {
  m_tex = aurora::gfx::new_static_texture_2d(x4_w, x6_h, x8_mips, aurora::gfx::TextureFormat::DXT1,
                                             {reinterpret_cast<const uint8_t*>(data), length}, label);
}

void CTexture::BuildDXT3(const void* data, size_t length, aurora::zstring_view label) {
  m_tex = aurora::gfx::new_static_texture_2d(x4_w, x6_h, x8_mips, aurora::gfx::TextureFormat::DXT3,
                                             {reinterpret_cast<const uint8_t*>(data), length}, label);
}

static std::string_view TextureFormatString(ETexelFormat format) {
  switch (format) {
  case ETexelFormat::I4:
    return "I4"sv;
  case ETexelFormat::I8:
    return "I8"sv;
  case ETexelFormat::IA4:
    return "IA4"sv;
  case ETexelFormat::IA8:
    return "IA8"sv;
  case ETexelFormat::C4:
    return "C4"sv;
  case ETexelFormat::C8:
    return "C8"sv;
  case ETexelFormat::C14X2:
    return "C14X2"sv;
  case ETexelFormat::RGB565:
    return "RGB565"sv;
  case ETexelFormat::RGB5A3:
    return "RGB5A3"sv;
  case ETexelFormat::RGBA8:
    return "RGBA8"sv;
  case ETexelFormat::CMPR:
    return "CMPR"sv;
  case ETexelFormat::RGBA8PC:
    return "RGBA8PC"sv;
  case ETexelFormat::C8PC:
    return "C8PC"sv;
  case ETexelFormat::CMPRPC:
    return "CMPRPC"sv;
  case ETexelFormat::CMPRPCA:
    return "CMPRPCA"sv;
  default:
    return "Invalid"sv;
  }
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

void CTexture::InitBitmapBuffers(ETexelFormat fmt, s16 width, s16 height, s32 mips) {}
void CTexture::InitTextureObjs() {}

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

CTexture::CTexture(std::unique_ptr<u8[]>&& in, u32 length, bool otex, const CTextureInfo* inf, CAssetId id) {
  std::unique_ptr<u8[]> owned = std::move(in);
  CMemoryInStream r(owned.get(), length, CMemoryInStream::EOwnerShip::NotOwned);
  x0_fmt = ETexelFormat(r.ReadLong());
  x4_w = r.ReadShort();
  x6_h = r.ReadShort();
  x8_mips = r.ReadLong();
  x9_bitsPerPixel = TexelFormatBitsPerPixel(x0_fmt);
  m_textureInfo = inf;

  auto label = fmt::format(FMT_STRING("TXTR {:08X} ({})"), id.Value(), TextureFormatString(x0_fmt));
  switch (x0_fmt) {
  case ETexelFormat::I4:
    BuildI4FromGCN(r, label);
    break;
  case ETexelFormat::I8:
    BuildI8FromGCN(r, label);
    break;
  case ETexelFormat::IA4:
    BuildIA4FromGCN(r, label);
    break;
  case ETexelFormat::IA8:
    BuildIA8FromGCN(r, label);
    break;
  case ETexelFormat::C4:
    BuildC4FromGCN(r, label);
    break;
  case ETexelFormat::C8:
    BuildC8FromGCN(r, label);
    break;
  case ETexelFormat::C14X2:
    BuildC14X2FromGCN(r, label);
    break;
  case ETexelFormat::RGB565:
    BuildRGB565FromGCN(r, label);
    break;
  case ETexelFormat::RGB5A3:
    BuildRGB5A3FromGCN(r, label);
    break;
  case ETexelFormat::RGBA8:
    BuildRGBA8FromGCN(r, label);
    break;
  case ETexelFormat::CMPR:
    if (aurora::gfx::get_dxt_compression_supported()) {
      BuildDXT1FromGCN(r, label);
    } else {
      Log.report(logvisor::Error, FMT_STRING("BC/DXT1 compression is not supported on your GPU, unable to load {}"),
                 label);
      x0_fmt = ETexelFormat::RGBA8PC;
      x8_mips = 1;
      std::unique_ptr<u8[]> data = std::make_unique<u8[]>(x4_w * x6_h * 4);
      /* Build a fake texture to use */
      for (u32 i = 0; i < (x4_w * x6_h) * 4; i += 4) {
        data[i + 0] = 0xFF;
        data[i + 1] = 0x00;
        data[i + 2] = 0xFF;
        data[i + 3] = 0xFF;
      }
      m_tex = aurora::gfx::new_static_texture_2d(x4_w, x6_h, x8_mips, aurora::gfx::TextureFormat::RGBA8,
                                                 {reinterpret_cast<const uint8_t*>(data.get()), (x4_w * x6_h * 4ul)},
                                                 label);
    }
    break;
  case ETexelFormat::RGBA8PC:
    BuildRGBA8(owned.get() + 12, length - 12, label);
    break;
  case ETexelFormat::C8PC:
    BuildC8(owned.get() + 12, length - 12, label);
    otex = true;
    break;
  case ETexelFormat::CMPRPC:
    BuildDXT1(owned.get() + 12, length - 12, label);
    break;
  case ETexelFormat::CMPRPCA:
    BuildDXT3(owned.get() + 12, length - 12, label);
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
              *texel++ =
                  CBasics::SwapBytes(u16((source[x].r >> 3 << 10) | (source[x].g >> 3 << 5) | (source[x].b >> 3)));
            } else {
              *texel++ = CBasics::SwapBytes(u16((source[x].r >> 4 << 8) | (source[x].g >> 4 << 4) | (source[x].b >> 4) |
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
    u32 nentries = CBasics::SwapBytes(*reinterpret_cast<const u32*>(data));
    const RGBA8* paletteTexels = reinterpret_cast<const RGBA8*>(data + 4);
    const u8* sourceMip = data + 4 + nentries * 4;

    for (u32 i = 0; i < 256; ++i) {
      u16& color = paletteColors[i];
      if (i >= nentries) {
        color = 0;
      } else {
        const RGBA8& colorIn = paletteTexels[i];
        if (colorIn.a == 0xff) {
          color = CBasics::SwapBytes(u16((colorIn.r >> 3 << 10) | (colorIn.g >> 3 << 5) | (colorIn.b >> 3) | 0x8000));
        } else {
          color = CBasics::SwapBytes(
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

const aurora::gfx::TextureHandle& CTexture::GetFontTexture(EFontType tp) {
  if (m_ftype != tp && x0_fmt == ETexelFormat::C8PC) {
    m_ftype = tp;
    BuildC8Font(m_otex.get() + 12, m_ftype, "Font (TODO)");
  }
  return m_tex;
}

CFactoryFnReturn FTextureFactory(const metaforce::SObjectTag& tag, std::unique_ptr<u8[]>&& in, u32 len,
                                 const metaforce::CVParamTransfer& vparms, CObjectReference* selfRef) {
  u32 u32Owned = vparms.GetOwnedObj<u32>();
  const CTextureInfo* inf = nullptr;
  if (g_TextureCache)
    inf = g_TextureCache->GetTextureInfo(tag.id);
  return TToken<CTexture>::GetIObjObjectFor(
      std::make_unique<CTexture>(std::move(in), len, u32Owned == SBIG('OTEX'), inf, tag.id));
}

} // namespace metaforce
