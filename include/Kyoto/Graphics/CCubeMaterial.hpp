#ifndef _CCUBEMATERIAL
#define _CCUBEMATERIAL

#include "Kyoto/Basics/CBasics.hpp"
#include "types.h"

#include "Kyoto/Graphics/CModelFlags.hpp"
#include "Kyoto/Math/CVector3f.hpp"

enum EStateFlags {
  kStateFlag_KonstValues = (1 << 3),
  kStateFlag_DepthSorting = (1 << 4),
  kStateFlag_AlphaTest = (1 << 5),
  kStateFlag_Reflection = (1 << 6),
  kStateFlag_DepthWrite = (1 << 7),
  kStateFlag_ReflectionSurfaceEye = (1 << 8),
  kStateFlag_ShadowOccluderMesh = (1 << 9),
  kStateFlag_ReflectionIndirectTexture = (1 << 10),
  kStateFlag_Lightmap = (1 << 11),
  kStateFlag_LightmapUvArray = (1 << 13),
  kStateFlag_TextureSlotMask = static_cast< uint >(~kStateFlag_LightmapUvArray),
};

class CCubeSurface;
class CCubeModel;
class CCubeMaterial {
public:
  explicit CCubeMaterial(const void* data) : mData(data) {}
  static void ResetCachedMaterials();
  static void EnsureViewDepStateCached(const CCubeSurface* surface);
  static void EnsureTevsDirect();
  static void KillCachedViewDepState();

  inline const uchar* GetData() const { return static_cast< const uchar* >(mData); }
  uint GetFlags() const { return CBasics::SwapBytes(*reinterpret_cast< const uint* >(GetData())); }
  bool IsFlagSet(const EStateFlags flag) const { return (GetFlags() & flag) != 0; }
  void SetCurrent(const CModelFlags& flags, const CCubeSurface& surface,
                  const CCubeModel& mode) const;
  void SetCurrentBlack() const;
  uint GetTextureCount() const {
    const uchar* data = GetData();
    data += sizeof(uint);
    const int ret = CBasics::SwapBytes(*reinterpret_cast< const uint* >(data));
    return ret;
  }
  uint GetVertexDesc() const {
    const uchar* data = GetData();
    data += (GetTextureCount() * sizeof(uint));
    data += sizeof(uint) + sizeof(uint);
    return CBasics::SwapBytes(*reinterpret_cast< const uint* >(data));
  }

  // TODO: Figure out wtf is going on here
  uint GetVertexDescLwzx() const {
    return CBasics::SwapBytes(static_cast< const uint* >(mData)[GetTextureCount() + 2]);
  }

  uint GetCompressedBlend() const;

  static const CVector3f& GetViewingReflection() { return sViewingFrom; }

private:
  static void SetupBlendMode(uint blendFactors, const CModelFlags& flags, bool alphaTest);
  static uint HandleReflection(bool usesTevReg2, GXTexMapID indTexSlot, int indMtxScaleExp,
                               uint tevCount, uint texCount, uint tcgCount, uint finalKColorCount,
                               uint& finalCCFlags, uint& finalACFlags);

  static const CCubeModel* sLastModelCached;
  static const CCubeModel* sRenderingModel;
  static CVector3f sViewingFrom;
  const void* mData;
};

#endif // _CCUBEMATERIAL
