#pragma once

#include "Runtime/CFactoryMgr.hpp"
#include "Runtime/Graphics/CGraphics.hpp"
#include "Runtime/Graphics/CGraphicsPalette.hpp"
#include "Runtime/Graphics/GX.hpp"
#include "Runtime/IObj.hpp"
#include "Runtime/Streams/CInputStream.hpp"
#include "GX.hpp"

namespace metaforce {
enum class EClampMode : std::underlying_type_t<GXTexWrapMode> {
  Clamp = GX_CLAMP,
  Repeat = GX_REPEAT,
  Mirror = GX_MIRROR,
};

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
  enum class EAutoMipmap {
    Zero,
    One,
  };

  enum class EBlackKey { Zero, One };

  enum class EFontType {
    None = -1,
    OneLayer = 0,        /* Fill bit0 */
    OneLayerOutline = 1, /* Fill bit0, Outline bit1 */
    FourLayers = 2,
    TwoLayersOutlines = 3, /* Fill bit0/2, Outline bit1/3 */
    TwoLayers = 4,         /* Fill bit0/1 and copied to bit2/3 */
    TwoLayersOutlines2 = 8 /* Fill bit2/3, Outline bit0/1 */
  };

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
  GX::TextureFormat x18_gxFormat = GX::TF_RGB565;
  GXCITexFmt x1c_gxCIFormat = GX_TF_C8;
  GXTexObj x20_texObj;
  EClampMode x40_clampMode = EClampMode::Repeat;
  std::unique_ptr<u8[]> x44_aramToken_x4_buff; // was CARAMToken
  u32 x64_frameAllocated{};

  // Metaforce additions
  std::string m_label;
  bool m_needsTexObjDataLoad = true;

  void InitBitmapBuffers(ETexelFormat fmt, u16 width, u16 height, s32 mips);
  void InitTextureObjs();
  void CountMemory();
  void UncountMemory();
  void MangleMipmap(u32 mip);
  static u32 TexelFormatBitsPerPixel(ETexelFormat fmt);

public:
  // Label parameters are new for Metaforce
  CTexture(ETexelFormat fmt, u16 w, u16 h, s32 mips, std::string_view label);
  CTexture(CInputStream& in, std::string_view label, EAutoMipmap automip = EAutoMipmap::Zero,
           EBlackKey blackKey = EBlackKey::Zero);

  [[nodiscard]] ETexelFormat GetTextureFormat() const { return x0_fmt; }
  [[nodiscard]] u16 GetWidth() const { return x4_w; }
  [[nodiscard]] u16 GetHeight() const { return x6_h; }
  [[nodiscard]] u8 GetNumberOfMipMaps() const { return x8_mips; }
  [[nodiscard]] u32 GetBitDepth() const { return x9_bitsPerPixel; }
  [[nodiscard]] u32 GetMemoryAllocated() const { return xc_memoryAllocated; }
  [[nodiscard]] const std::unique_ptr<CGraphicsPalette>& GetPalette() const { return x10_graphicsPalette; }
  [[nodiscard]] bool HasPalette() const { return x10_graphicsPalette != nullptr; }
  [[nodiscard]] u8* Lock();
  void UnLock();
  void Load(GX::TexMapID id, EClampMode clamp);
  void LoadMipLevel(float lod, GX::TexMapID id, EClampMode clamp); // was an s32 mip parameter, adjusted to use lod
  // void UnloadBitmapData(u32) const;
  // void TryReloadBitmapData(CResFactory&) const;
  // void LoadToMRAM() const;
  // void LoadToARAM() const;
  // bool IsARAMTransferInProgress() const { return false; }
  void MakeSwappable();

  [[nodiscard]] const u8* GetConstBitMapData(s32 mip) const;
  [[nodiscard]] u8* GetBitMapData(s32 mip) const;
  [[nodiscard]] bool IsCITexture() const {
    return x0_fmt == ETexelFormat::C4 || x0_fmt == ETexelFormat::C8 || x0_fmt == ETexelFormat::C14X2;
  }

  static void InvalidateTexMap(GX::TexMapID id);
  static void SetMangleMips(bool b) { sMangleMips = b; }
  static void SetCurrentFrameCount(u32 frameCount) { sCurrentFrameCount = frameCount; }
};

CFactoryFnReturn FTextureFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& vparms,
                                 CObjectReference* selfRef);
} // namespace metaforce
