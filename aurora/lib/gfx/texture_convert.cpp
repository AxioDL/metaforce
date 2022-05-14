#include "texture_convert.hpp"

namespace aurora::gfx {
static logvisor::Module Log("aurora::gfx");

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

constexpr u8 S3TCBlend(u8 a_, u8 b_) {
  u32 a = a_;
  u32 b = b_;
  return static_cast<u8>((((a << 1) + a) + ((b << 2) + b)) >> 3);
}

constexpr u8 HalfBlend(u8 a, u8 b) { return static_cast<u8>((static_cast<u32>(a) + static_cast<u32>(b)) >> 1); }

static size_t ComputeMippedTexelCount(u32 w, u32 h, u32 mips) {
  size_t ret = w * h;
  for (u32 i = mips; i > 1; --i) {
    if (w > 1) {
      w /= 2;
    }
    if (h > 1) {
      h /= 2;
    }
    ret += w * h;
  }
  return ret;
}

static size_t ComputeMippedBlockCountDXT1(u32 w, u32 h, u32 mips) {
  w /= 4;
  h /= 4;
  size_t ret = w * h;
  for (u32 i = mips; i > 1; --i) {
    if (w > 1) {
      w /= 2;
    }
    if (h > 1) {
      h /= 2;
    }
    ret += w * h;
  }
  return ret;
}

template <typename T>
constexpr T bswap16(T val) noexcept {
#if __GNUC__
  return __builtin_bswap16(val);
#elif _WIN32
  return _byteswap_ushort(val);
#else
  return (val = (val << 8) | ((val >> 8) & 0xFF));
#endif
}

static ByteBuffer BuildI4FromGCN(uint32_t width, uint32_t height, uint32_t mips, ArrayRef<uint8_t> data) {
  const size_t texelCount = ComputeMippedTexelCount(width, height, mips);
  ByteBuffer buf{texelCount};

  uint32_t w = width;
  uint32_t h = height;
  u8* targetMip = buf.data();
  const uint8_t* in = data.data();
  for (uint32_t mip = 0; mip < mips; ++mip) {
    const uint32_t bwidth = (w + 7) / 8;
    const uint32_t bheight = (h + 7) / 8;
    for (uint32_t by = 0; by < bheight; ++by) {
      const uint32_t baseY = by * 8;
      for (uint32_t bx = 0; bx < bwidth; ++bx) {
        const uint32_t baseX = bx * 8;
        for (uint32_t y = 0; y < std::min(h, 8u); ++y) {
          u8* target = targetMip + (baseY + y) * w + baseX;
          for (uint32_t x = 0; x < std::min(w, 8u); ++x) {
            target[x] = Convert4To8(in[x / 2] >> ((x & 1) ? 0 : 4) & 0xf);
          }
          in += std::min<size_t>(w / 4, 4);
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

  return buf;
}

static ByteBuffer BuildI8FromGCN(uint32_t width, uint32_t height, uint32_t mips, ArrayRef<uint8_t> data) {
  const size_t texelCount = ComputeMippedTexelCount(width, height, mips);
  ByteBuffer buf{sizeof(RGBA8) * texelCount};

  uint32_t w = width;
  uint32_t h = height;
  auto* targetMip = reinterpret_cast<RGBA8*>(buf.data());
  const uint8_t* in = data.data();
  for (uint32_t mip = 0; mip < mips; ++mip) {
    const uint32_t bwidth = (w + 7) / 8;
    const uint32_t bheight = (h + 3) / 4;
    for (uint32_t by = 0; by < bheight; ++by) {
      const uint32_t baseY = by * 4;
      for (uint32_t bx = 0; bx < bwidth; ++bx) {
        const uint32_t baseX = bx * 8;
        for (uint32_t y = 0; y < 4; ++y) {
          RGBA8* target = targetMip + (baseY + y) * w + baseX;
          const auto n = std::min(w, 8u);
          for (size_t x = 0; x < n; ++x) {
            const auto v = in[x];
            target[x].r = v;
            target[x].g = v;
            target[x].b = v;
            target[x].a = v;
          }
          in += n;
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

  return buf;
}

ByteBuffer BuildIA4FromGCN(uint32_t width, uint32_t height, uint32_t mips, ArrayRef<uint8_t> data) {
  const size_t texelCount = ComputeMippedTexelCount(width, height, mips);
  ByteBuffer buf{sizeof(RGBA8) * texelCount};

  uint32_t w = width;
  uint32_t h = height;
  RGBA8* targetMip = reinterpret_cast<RGBA8*>(buf.data());
  const uint8_t* in = data.data();
  for (uint32_t mip = 0; mip < mips; ++mip) {
    const uint32_t bwidth = (w + 7) / 8;
    const uint32_t bheight = (h + 3) / 4;
    for (uint32_t by = 0; by < bheight; ++by) {
      const uint32_t baseY = by * 4;
      for (uint32_t bx = 0; bx < bwidth; ++bx) {
        const uint32_t baseX = bx * 8;
        for (uint32_t y = 0; y < 4; ++y) {
          RGBA8* target = targetMip + (baseY + y) * w + baseX;
          const auto n = std::min(w, 8u);
          for (size_t x = 0; x < n; ++x) {
            const u8 intensity = Convert4To8(in[x] >> 4 & 0xf);
            target[x].r = intensity;
            target[x].g = intensity;
            target[x].b = intensity;
            target[x].a = Convert4To8(in[x] & 0xf);
          }
          in += n;
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

  return buf;
}

ByteBuffer BuildIA8FromGCN(uint32_t width, uint32_t height, uint32_t mips, ArrayRef<uint8_t> data) {
  const size_t texelCount = ComputeMippedTexelCount(width, height, mips);
  ByteBuffer buf{sizeof(RGBA8) * texelCount};

  uint32_t w = width;
  uint32_t h = height;
  auto* targetMip = reinterpret_cast<RGBA8*>(buf.data());
  const auto* in = reinterpret_cast<const uint16_t*>(data.data());
  for (uint32_t mip = 0; mip < mips; ++mip) {
    const uint32_t bwidth = (w + 3) / 4;
    const uint32_t bheight = (h + 3) / 4;
    for (uint32_t by = 0; by < bheight; ++by) {
      const uint32_t baseY = by * 4;
      for (uint32_t bx = 0; bx < bwidth; ++bx) {
        const uint32_t baseX = bx * 4;
        for (uint32_t y = 0; y < 4; ++y) {
          RGBA8* target = targetMip + (baseY + y) * w + baseX;
          for (size_t x = 0; x < 4; ++x) {
            const auto texel = bswap16(in[x]);
            const u8 intensity = texel >> 8;
            target[x].r = intensity;
            target[x].g = intensity;
            target[x].b = intensity;
            target[x].a = texel & 0xff;
          }
          in += 4;
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

  return buf;
}

ByteBuffer BuildC4FromGCN(uint32_t width, uint32_t height, uint32_t mips, ArrayRef<uint8_t> data) {
  const size_t texelCount = ComputeMippedTexelCount(width, height, mips);
  ByteBuffer buf{texelCount * 2};

  uint32_t w = width;
  uint32_t h = height;
  u16* targetMip = reinterpret_cast<u16*>(buf.data());
  const uint8_t* in = data.data();
  for (uint32_t mip = 0; mip < mips; ++mip) {
    const uint32_t bwidth = (w + 7) / 8;
    const uint32_t bheight = (h + 7) / 8;
    for (uint32_t by = 0; by < bheight; ++by) {
      const uint32_t baseY = by * 8;
      for (uint32_t bx = 0; bx < bwidth; ++bx) {
        const uint32_t baseX = bx * 8;
        for (uint32_t y = 0; y < std::min(8u, h); ++y) {
          u16* target = targetMip + (baseY + y) * w + baseX;
          const auto n = std::min(w, 8u);
          for (size_t x = 0; x < n; ++x) {
            target[x] = in[x / 2] >> ((x & 1) ? 0 : 4) & 0xf;
          }
          in += n / 2;
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

  return buf;
}

ByteBuffer BuildC8FromGCN(uint32_t width, uint32_t height, uint32_t mips, ArrayRef<uint8_t> data) {
  const size_t texelCount = ComputeMippedTexelCount(width, height, mips);
  ByteBuffer buf{texelCount * 2};

  uint32_t w = width;
  uint32_t h = height;
  u16* targetMip = reinterpret_cast<u16*>(buf.data());
  const uint8_t* in = data.data();
  for (uint32_t mip = 0; mip < mips; ++mip) {
    const uint32_t bwidth = (w + 7) / 8;
    const uint32_t bheight = (h + 3) / 4;
    for (uint32_t by = 0; by < bheight; ++by) {
      const uint32_t baseY = by * 4;
      for (uint32_t bx = 0; bx < bwidth; ++bx) {
        const uint32_t baseX = bx * 8;
        for (uint32_t y = 0; y < 4; ++y) {
          u16* target = targetMip + (baseY + y) * w + baseX;
          const auto n = std::min(w, 8u);
          for (size_t x = 0; x < n; ++x) {
            target[x] = in[x];
          }
          in += n;
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

  return buf;
}

ByteBuffer BuildRGB565FromGCN(uint32_t width, uint32_t height, uint32_t mips, ArrayRef<uint8_t> data) {
  const size_t texelCount = ComputeMippedTexelCount(width, height, mips);
  ByteBuffer buf{sizeof(RGBA8) * texelCount};

  uint32_t w = width;
  uint32_t h = height;
  auto* targetMip = reinterpret_cast<RGBA8*>(buf.data());
  const auto* in = reinterpret_cast<const uint16_t*>(data.data());
  for (uint32_t mip = 0; mip < mips; ++mip) {
    const uint32_t bwidth = (w + 3) / 4;
    const uint32_t bheight = (h + 3) / 4;
    for (uint32_t by = 0; by < bheight; ++by) {
      const uint32_t baseY = by * 4;
      for (uint32_t bx = 0; bx < bwidth; ++bx) {
        const uint32_t baseX = bx * 4;
        for (uint32_t y = 0; y < std::min(4u, h); ++y) {
          RGBA8* target = targetMip + (baseY + y) * w + baseX;
          for (size_t x = 0; x < std::min(4u, w); ++x) {
            const auto texel = bswap16(in[x]);
            target[x].r = Convert5To8(texel >> 11 & 0x1f);
            target[x].g = Convert6To8(texel >> 5 & 0x3f);
            target[x].b = Convert5To8(texel & 0x1f);
            target[x].a = 0xff;
          }
          in += 4;
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

  return buf;
}

ByteBuffer BuildRGB5A3FromGCN(uint32_t width, uint32_t height, uint32_t mips, ArrayRef<uint8_t> data) {
  size_t texelCount = ComputeMippedTexelCount(width, height, mips);
  ByteBuffer buf{sizeof(RGBA8) * texelCount};

  uint32_t w = width;
  uint32_t h = height;
  auto* targetMip = reinterpret_cast<RGBA8*>(buf.data());
  const auto* in = reinterpret_cast<const uint16_t*>(data.data());
  for (uint32_t mip = 0; mip < mips; ++mip) {
    const uint32_t bwidth = (w + 3) / 4;
    const uint32_t bheight = (h + 3) / 4;
    for (uint32_t by = 0; by < bheight; ++by) {
      const uint32_t baseY = by * 4;
      for (uint32_t bx = 0; bx < bwidth; ++bx) {
        const uint32_t baseX = bx * 4;
        for (uint32_t y = 0; y < std::min(4u, h); ++y) {
          RGBA8* target = targetMip + (baseY + y) * w + baseX;
          for (size_t x = 0; x < std::min(4u, w); ++x) {
            const auto texel = bswap16(in[x]);
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
          in += 4;
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

  return buf;
}

ByteBuffer BuildRGBA8FromGCN(uint32_t width, uint32_t height, uint32_t mips, ArrayRef<uint8_t> data) {
  const size_t texelCount = ComputeMippedTexelCount(width, height, mips);
  ByteBuffer buf{sizeof(RGBA8) * texelCount};

  uint32_t w = width;
  uint32_t h = height;
  auto* targetMip = reinterpret_cast<RGBA8*>(buf.data());
  const uint8_t* in = data.data();
  for (uint32_t mip = 0; mip < mips; ++mip) {
    const uint32_t bwidth = (w + 3) / 4;
    const uint32_t bheight = (h + 3) / 4;
    for (uint32_t by = 0; by < bheight; ++by) {
      const uint32_t baseY = by * 4;
      for (uint32_t bx = 0; bx < bwidth; ++bx) {
        const uint32_t baseX = bx * 4;
        for (uint32_t c = 0; c < 2; ++c) {
          for (uint32_t y = 0; y < 4; ++y) {
            RGBA8* target = targetMip + (baseY + y) * w + baseX;
            for (size_t x = 0; x < 4; ++x) {
              if (c != 0) {
                target[x].g = in[x * 2];
                target[x].b = in[x * 2 + 1];
              } else {
                target[x].a = in[x * 2];
                target[x].r = in[x * 2 + 1];
              }
            }
            in += 8;
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

  return buf;
}

ByteBuffer BuildDXT1FromGCN(uint32_t width, uint32_t height, uint32_t mips, ArrayRef<uint8_t> data) {
  const size_t blockCount = ComputeMippedBlockCountDXT1(width, height, mips);
  ByteBuffer buf{sizeof(DXT1Block) * blockCount};

  uint32_t w = width / 4;
  uint32_t h = height / 4;
  auto* targetMip = reinterpret_cast<DXT1Block*>(buf.data());
  const auto* in = reinterpret_cast<const DXT1Block*>(data.data());
  for (uint32_t mip = 0; mip < mips; ++mip) {
    const uint32_t bwidth = (w + 1) / 2;
    const uint32_t bheight = (h + 1) / 2;
    for (uint32_t by = 0; by < bheight; ++by) {
      const uint32_t baseY = by * 2;
      for (uint32_t bx = 0; bx < bwidth; ++bx) {
        const uint32_t baseX = bx * 2;
        for (uint32_t y = 0; y < 2; ++y) {
          DXT1Block* target = targetMip + (baseY + y) * w + baseX;
          for (size_t x = 0; x < 2; ++x) {
            target[x].color1 = bswap16(in[x].color1);
            target[x].color2 = bswap16(in[x].color2);
            for (size_t i = 0; i < 4; ++i) {
              std::array<uint8_t, 4> ind;
              const uint8_t packed = in[x].lines[i];
              ind[3] = packed & 0x3;
              ind[2] = (packed >> 2) & 0x3;
              ind[1] = (packed >> 4) & 0x3;
              ind[0] = (packed >> 6) & 0x3;
              target[x].lines[i] = ind[0] | (ind[1] << 2) | (ind[2] << 4) | (ind[3] << 6);
            }
          }
          in += 2;
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

  return buf;
}

ByteBuffer BuildRGBA8FromCMPR(u32 width, u32 height, u32 mips, ArrayRef<u8> data) {
  const size_t texelCount = ComputeMippedTexelCount(width, height, mips);
  const size_t blockCount = ComputeMippedBlockCountDXT1(width, height, mips);
  ByteBuffer buf{sizeof(RGBA8) * texelCount};

  u32 h = height;
  u32 w = width;
  u8* dst = buf.data();
  const u8* src = data.data();
  for (u32 mip = 0; mip < mips; ++mip) {
    for (u32 yy = 0; yy < h; yy += 8) {
      for (u32 xx = 0; xx < w; xx += 8) {
        for (u32 yb = 0; yb < 8; yb += 4) {
          for (u32 xb = 0; xb < 8; xb += 4) {
            // CMPR difference: Big-endian color1/2
            const u16 color1 = bswap16(*reinterpret_cast<const u16*>(src));
            const u16 color2 = bswap16(*reinterpret_cast<const u16*>(src + 2));
            src += 4;

            // Fill in first two colors in color table.
            std::array<u8, 16> color_table{};

            color_table[0] = Convert5To8(static_cast<u8>((color1 >> 11) & 0x1F));
            color_table[1] = Convert6To8(static_cast<u8>((color1 >> 5) & 0x3F));
            color_table[2] = Convert5To8(static_cast<u8>(color1 & 0x1F));
            color_table[3] = 0xFF;

            color_table[4] = Convert5To8(static_cast<u8>((color2 >> 11) & 0x1F));
            color_table[5] = Convert6To8(static_cast<u8>((color2 >> 5) & 0x3F));
            color_table[6] = Convert5To8(static_cast<u8>(color2 & 0x1F));
            color_table[7] = 0xFF;
            if (color1 > color2) {
              // Predict gradients.
              color_table[8] = S3TCBlend(color_table[4], color_table[0]);
              color_table[9] = S3TCBlend(color_table[5], color_table[1]);
              color_table[10] = S3TCBlend(color_table[6], color_table[2]);
              color_table[11] = 0xFF;

              color_table[12] = S3TCBlend(color_table[0], color_table[4]);
              color_table[13] = S3TCBlend(color_table[1], color_table[5]);
              color_table[14] = S3TCBlend(color_table[2], color_table[6]);
              color_table[15] = 0xFF;
            } else {
              color_table[8] = HalfBlend(color_table[0], color_table[4]);
              color_table[9] = HalfBlend(color_table[1], color_table[5]);
              color_table[10] = HalfBlend(color_table[2], color_table[6]);
              color_table[11] = 0xFF;

              // CMPR difference: GX fills with an alpha 0 midway point here.
              color_table[12] = color_table[8];
              color_table[13] = color_table[9];
              color_table[14] = color_table[10];
              color_table[15] = 0;
            }

            for (u32 y = 0; y < 4; ++y) {
              u8 bits = src[y];
              for (u32 x = 0; x < 4; ++x) {
                if (xx + xb + x >= w || yy + yb + y >= h) {
                  continue;
                }
                u8* dstOffs = dst + ((yy + yb + y) * w + (xx + xb + x)) * 4;
                const u8* colorTableOffs = &color_table[static_cast<size_t>((bits >> 6) & 3) * 4];
                memcpy(dstOffs, colorTableOffs, 4);
                bits <<= 2;
              }
            }
            src += 4;
          }
        }
      }
    }
    dst += w * h * 4;
    if (w > 1) {
      w /= 2;
    }
    if (h > 1) {
      h /= 2;
    }
  }

  return buf;
}

ByteBuffer convert_texture(GX::TextureFormat format, uint32_t width, uint32_t height, uint32_t mips,
                           ArrayRef<uint8_t> data) {
  switch (format) {
  default:
    Log.report(logvisor::Fatal, FMT_STRING("convert_texture: unknown format supplied {}"), format);
    unreachable();
  case GX::TF_I4:
    return BuildI4FromGCN(width, height, mips, data);
  case GX::TF_I8:
    // No conversion
    return {};
  case GX::TF_IA4:
    return BuildIA4FromGCN(width, height, mips, data);
  case GX::TF_IA8:
    return BuildIA8FromGCN(width, height, mips, data);
  case GX::TF_C4:
    return BuildC4FromGCN(width, height, mips, data);
  case GX::TF_C8:
    return BuildC8FromGCN(width, height, mips, data);
  case GX::TF_C14X2:
    Log.report(logvisor::Fatal, FMT_STRING("convert_texture: C14X2 unimplemented"));
    unreachable();
  case GX::TF_RGB565:
    return BuildRGB565FromGCN(width, height, mips, data);
  case GX::TF_RGB5A3:
    return BuildRGB5A3FromGCN(width, height, mips, data);
  case GX::TF_RGBA8:
    return BuildRGBA8FromGCN(width, height, mips, data);
  case GX::TF_CMPR:
    if (gpu::g_device.HasFeature(wgpu::FeatureName::TextureCompressionBC)) {
      return BuildDXT1FromGCN(width, height, mips, data);
    } else {
      return BuildRGBA8FromCMPR(width, height, mips, data);
    }
  }
}
} // namespace aurora::gfx
