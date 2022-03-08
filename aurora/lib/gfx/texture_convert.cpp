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
  ByteBuffer buf{sizeof(RGBA8) * texelCount};

  uint32_t w = width;
  uint32_t h = height;
  auto* targetMip = reinterpret_cast<RGBA8*>(buf.data());
  const uint8_t* in = data.data();
  for (uint32_t mip = 0; mip < mips; ++mip) {
    const uint32_t bwidth = (w + 7) / 8;
    const uint32_t bheight = (h + 7) / 8;
    for (uint32_t by = 0; by < bheight; ++by) {
      const uint32_t baseY = by * 8;
      for (uint32_t bx = 0; bx < bwidth; ++bx) {
        const uint32_t baseX = bx * 8;
        for (uint32_t y = 0; y < std::min(h, 8u); ++y) {
          RGBA8* target = targetMip + (baseY + y) * w + baseX;
          for (uint32_t x = 0; x < std::min(w, 8u); ++x) {
            target[x].r = Convert4To8(in[x / 2] >> ((x & 1) ? 0 : 4) & 0xf);
            target[x].g = target[x].r;
            target[x].b = target[x].r;
            target[x].a = target[x].r;
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

ByteBuffer BuildC4FromGCN(uint32_t width, uint32_t height, uint32_t mips, ArrayRef<uint8_t> data, RGBA8* palette) {
  const size_t texelCount = ComputeMippedTexelCount(width, height, mips);
  ByteBuffer buf{sizeof(RGBA8) * texelCount};

  uint32_t w = width;
  uint32_t h = height;
  auto* targetMip = reinterpret_cast<RGBA8*>(buf.data());
  const uint8_t* in = data.data();
  for (uint32_t mip = 0; mip < mips; ++mip) {
    const uint32_t bwidth = (w + 7) / 8;
    const uint32_t bheight = (h + 7) / 8;
    for (uint32_t by = 0; by < bheight; ++by) {
      const uint32_t baseY = by * 8;
      for (uint32_t bx = 0; bx < bwidth; ++bx) {
        const uint32_t baseX = bx * 8;
        for (uint32_t y = 0; y < 8; ++y) {
          RGBA8* target = targetMip + (baseY + y) * w + baseX;
          const auto n = std::min(w, 8u);
          for (size_t x = 0; x < n; ++x) {
            target[x] = palette[in[x / 2] >> ((x & 1) ? 0 : 4) & 0xf];
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

ByteBuffer BuildC8FromGCN(uint32_t width, uint32_t height, uint32_t mips, ArrayRef<uint8_t> data, RGBA8* palette) {
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
            target[x] = palette[in[x]];
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
        for (uint32_t y = 0; y < 4; ++y) {
          RGBA8* target = targetMip + (baseY + y) * w + baseX;
          for (size_t x = 0; x < 4; ++x) {
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
        for (uint32_t y = 0; y < 4; ++y) {
          RGBA8* target = targetMip + (baseY + y) * w + baseX;
          for (size_t x = 0; x < 4; ++x) {
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

ByteBuffer convert_texture(metaforce::ETexelFormat format, uint32_t width, uint32_t height, uint32_t mips,
                           ArrayRef<uint8_t> data) {
  switch (format) {
  case metaforce::ETexelFormat::RGBA8PC:
  case metaforce::ETexelFormat::R8PC:
    return {};
  case metaforce::ETexelFormat::Invalid:
    Log.report(logvisor::Fatal, FMT_STRING("convert_texture: invalid format supplied"));
    unreachable();
  case metaforce::ETexelFormat::I4:
    return BuildI4FromGCN(width, height, mips, data);
  case metaforce::ETexelFormat::I8:
    return BuildI8FromGCN(width, height, mips, data);
  case metaforce::ETexelFormat::IA4:
    return BuildIA4FromGCN(width, height, mips, data);
  case metaforce::ETexelFormat::IA8:
    return BuildIA8FromGCN(width, height, mips, data);
  case metaforce::ETexelFormat::C4:
    Log.report(logvisor::Fatal, FMT_STRING("convert_texture: C4 unimplemented"));
    unreachable();
    // return BuildC4FromGCN(width, height, mips, data);
  case metaforce::ETexelFormat::C8:
    Log.report(logvisor::Fatal, FMT_STRING("convert_texture: C8 unimplemented"));
    unreachable();
    // return BuildC8FromGCN(width, height, mips, data);
  case metaforce::ETexelFormat::C14X2:
    Log.report(logvisor::Fatal, FMT_STRING("convert_texture: C14X2 unimplemented"));
    unreachable();
  case metaforce::ETexelFormat::RGB565:
    return BuildRGB565FromGCN(width, height, mips, data);
  case metaforce::ETexelFormat::RGB5A3:
    return BuildRGB5A3FromGCN(width, height, mips, data);
  case metaforce::ETexelFormat::RGBA8:
    return BuildRGBA8FromGCN(width, height, mips, data);
  case metaforce::ETexelFormat::CMPR:
    if (gpu::g_device.HasFeature(wgpu::FeatureName::TextureCompressionBC)) {
      return BuildDXT1FromGCN(width, height, mips, data);
    } else {
      Log.report(logvisor::Fatal, FMT_STRING("convert_texture: TODO implement CMPR to RGBA"));
      unreachable();
    }
  }
}
} // namespace aurora::gfx
