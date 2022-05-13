#pragma once

#include "CToken.hpp"
#include "GCNTypes.hpp"
#include "Graphics/CTexture.hpp"
#include "Graphics/CModel.hpp"
#include "IObjectStore.hpp"

namespace metaforce {
class CCubeModel;
class CCubeSurface;

enum class CCubeMaterialFlagBits : u32 {
  fKonstValues = 0x8,
  fDepthSorting = 0x10,
  fAlphaTest = 0x20,
  fSamusReflection = 0x40,
  fDepthWrite = 0x80,
  fSamusReflectionSurfaceEye = 0x100,
  fShadowOccluderMesh = 0x200,
  fSamusReflectionIndirectTexture = 0x400,
  fLightmap = 0x800,
  fLightmapUvArray = 0x2000,
  fTextureSlotMask = 0xffff0000
};
using CCubeMaterialFlags = Flags<CCubeMaterialFlagBits>;

class CCubeMaterial {
  const u8* x0_data;

public:
  explicit CCubeMaterial(const u8* data) : x0_data(data) {}

  void SetCurrent(const CModelFlags& flags, const CCubeSurface& surface, CCubeModel& model);

  [[nodiscard]] u32 GetCompressedBlend() {
    const u32* ptr = reinterpret_cast<const u32*>(x0_data + (GetTextureCount() * 4) + 16);
    if (GetFlags() & CCubeMaterialFlagBits::fKonstValues) {
      ptr += SBig(*ptr) + 1;
    }
    return SBig(*ptr);
  }
  [[nodiscard]] CCubeMaterialFlags GetFlags() const {
    return CCubeMaterialFlags(SBig(*reinterpret_cast<const u32*>(x0_data)));
  }
  [[nodiscard]] u32 GetVatFlags() const {
    return SBig(*reinterpret_cast<const u32*>(x0_data + 8 + (GetTextureCount() * 4)));
  }
  [[nodiscard]] u32 GetUsedTextureSlots() const { return static_cast<u32>(GetFlags()) >> 16; }
  [[nodiscard]] u32 GetTextureCount() const { return SBig(*reinterpret_cast<const u32*>(x0_data + 4)); }
  [[nodiscard]] u32 GetVertexDesc() const {
    return SBig(*reinterpret_cast<const u32*>(x0_data + (GetTextureCount() * 4) + 8));
  }

  static void ResetCachedMaterials();
  static void EnsureViewDepStateCached(const CCubeSurface* surface);
  static void KillCachedViewDepState();
  static void EnsureTevsDirect();

private:
  void SetCurrentBlack();

  static void SetupBlendMode(u32 blendFactors, const CModelFlags& flags, bool alphaTest);
  static void HandleDepth(CModelFlagsFlags modelFlags, CCubeMaterialFlags matFlags);
  static u32 HandleColorChannels(u32 chanCount, u32 firstChan);
  static void HandleTev(u32 tevCur, const u32* materialDataCur, const u32* texMapTexCoordFlags, bool shadowMapsEnabled);
  static u32 HandleAnimatedUV(const u32* uvAnim, GX::TexMtx texMtx, GX::PTTexMtx pttTexMtx);
  static void HandleTransparency(u32& finalTevCount, u32& finalKColorCount, const CModelFlags& modelFlags,
                                 u32 blendFactors, u32& finalCCFlags, u32& finalACFlags);
  static u32 HandleReflection(bool usesTevReg2, u32 indTexSlot, u32 r5, u32 finalTevCount, u32 texCount, u32 tcgCount,
                              u32 finalKColorCount, u32& finalCCFlags, u32& finalACFlags);
  static void DoPassthru(u32 finalTevCount);
  static void DoModelShadow(u32 texCount, u32 tcgCount);
};
} // namespace metaforce
