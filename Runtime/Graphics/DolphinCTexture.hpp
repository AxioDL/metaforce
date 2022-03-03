#pragma once

#include "Runtime/CFactoryMgr.hpp"
#include "Runtime/Graphics/CGraphics.hpp"
#include "Runtime/Graphics/CGraphicsPalette.hpp"
#include "Runtime/IObj.hpp"
#include "Runtime/Streams/CInputStream.hpp"
#include "Runtime/Graphics/GX.hpp"

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

  enum class EBlackKey { Zero, One };

private:
  static bool sMangleMips;
  static u32 sCurrentFrameCount;
  static u32 sTotalAllocatedMemory;
  ETexelFormat x0_fmt = ETexelFormat::Invalid;
  u16 x4_w = 0;
  u16 x6_h = 0;
  u8 x8_mips = 0;
  u8 x9_bitsPerPixel = 0;
  bool xa_24_locked : 1 = false;
  bool xa_25_canLoadPalette : 1 = false;
  bool xa_26_isPowerOfTwo : 1 = false;
  bool xa_27_noSwap : 1 = true;
  bool xa_28_counted : 1 = false;
  bool xa_29_canLoadObj : 1 = false;
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
  void UncountMemory();
  void MangleMipmap(u32 mip);
  static u32 TexelFormatBitsPerPixel(ETexelFormat fmt);

public:
  CTexture(ETexelFormat, s16, s16, s32);
  CTexture(CInputStream& in, EAutoMipmap automip = EAutoMipmap::Zero, EBlackKey blackKey = EBlackKey::Zero);

  [[nodiscard]] ETexelFormat GetTextureFormat() const { return x0_fmt; }
  [[nodiscard]] s16 GetWidth(s32 mip) const { return x4_w; }
  [[nodiscard]] s16 GetHeight(s32 mip) const { return x6_h; }
  [[nodiscard]] u8 GetNumberOfMipMaps() const { return x8_mips; }
  [[nodiscard]] u32 GetBitDepth() const { return x9_bitsPerPixel; }
  [[nodiscard]] u32 GetMemoryAllocated() const { return xc_memoryAllocated; }
  [[nodiscard]] const std::unique_ptr<CGraphicsPalette>& GetPalette() const { return x10_graphicsPalette; }
  [[nodiscard]] bool HasPalette() const { return x10_graphicsPalette != nullptr; }
  [[nodiscard]] void* Lock();
  void UnLock();
  void Load(GX::TexMapID id, EClampMode clamp);
  void LoadMipLevel(s32 mip, GX::TexMapID id, EClampMode clamp);
  // void UnloadBitmapData(u32) const;
  // void TryReloadBitmapData(CResFactory&) const;
  // void LoadToMRAM() const;
  // void LoadToARAM() const;
  // bool IsARAMTransferInProgress() const { return false; }
  void MakeSwappable();
  [[nodiscard]] const void* GetConstBitMapData(s32 mip) const;
  [[nodiscard]] void* GetBitMapData(s32 mip) const;

  [[nodiscard]] bool IsCITexture() const {
    return x0_fmt == ETexelFormat::C4 || x0_fmt == ETexelFormat::C8 || x0_fmt == ETexelFormat::C14X2;
  }

  static void InvalidateTexMap(GX::TexMapID id);
  static void SetMangleMips(bool b) { sMangleMips = b; }
  static void SetCurrentFrameCount(u32 frameCount) { sCurrentFrameCount = frameCount; }
};

CFactoryFnReturn FTextureFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& vparms,
                                 CObjectReference* selfRef);
} // namespace metaforce::WIP
