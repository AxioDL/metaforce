#pragma once

#include "Runtime/Graphics/CGraphics.hpp"
#include "Runtime/Graphics/CGraphicsPalette.hpp"
#include "Runtime/Streams/CInputStream.hpp"


namespace metaforce::WIP {
class CTexture {
  class CDumpedBitmapDataReloader {
    int x0_;
    u32 x4_;
    int x8_;
    u32 xc_;
    bool x10_;
    int x14_;
    void* x18_;
  };
public:
  enum class EClampMode {
    Clamp,
    Repeat,
    Mirror,
  };

  enum class EAutoMipmap {
    Zero,
    One,
  };

  enum class EBlackKey {
    Zero,
    One
  };

private:
  static constexpr bool sMangleMips = false;
  static u32 sCurrentFrameCount;
  static u32 sTotalAllocatedMemory;
  ETexelFormat x0_fmt = ETexelFormat::Invalid;
  u16 x4_w = 0;
  u16 x6_h = 0;
  u8 x8_mips = 0;
  u8 x9_bitsPerPixel = 0;
  bool xa_24_ : 1 = false;
  bool xa_25_hasPalette : 1 = false;
  bool xa_26_ : 1 = false;
  bool xa_27_ : 1 = true;
  bool xa_28_counted : 1 = false;
  bool xa_29_ : 1 = false;
  u32 xc_memoryAllocated = 0;
  std::unique_ptr<CGraphicsPalette> x10_graphicsPalette;
  std::unique_ptr<CDumpedBitmapDataReloader> x14_bitmapReloader;
  u32 x18_gxFormat = GX::TF_RGB565;
  u32 x1c_gxCIFormat = GX::TF_C8;
  /* GXTexObj x20_texObj */
  EClampMode x40_clampMode = EClampMode::Repeat;
  /* CARAMToken x44_aramToken */
  std::unique_ptr<u8[]> x44_aramToken_x4_buff;
  u32 x64_frameAllocated{};

  void InitBitmapBuffers(ETexelFormat fmt, s16 width, s16 height, s32 mips);
  void InitTextureObjs();
  void CountMemory();
  void MangleMipmap(u32 mip);
  static u32 TexelFormatBitsPerPixel(ETexelFormat fmt);
public:
  CTexture(ETexelFormat, s16, s16, s32);
  CTexture(CInputStream& in, EAutoMipmap automip = EAutoMipmap::Zero, EBlackKey blackKey = EBlackKey::Zero);

  const void* GetConstBitMapData(s32 mip) const { /* TODO: get bitmap data for specified mipmap */ return nullptr; }
  void* GetBitMapData(s32 mip) const { return const_cast<void*>(GetConstBitMapData(mip)); }
};
} // namespace metaforce::WIP

