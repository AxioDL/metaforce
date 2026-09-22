#include "Kyoto/Graphics/CCubeModel.hpp"
#include "Kyoto/Basics/CBasics.hpp"

#include "Kyoto/Graphics/CCubeSurface.hpp"
#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CGX_Impl.hpp" // IWYU pragma: keep
#include "Kyoto/Graphics/CGraphics.hpp"
#include "dolphin/gx/GXVert.h"

#include <string.h>

static bool sDrawingOccluders = false;
static bool sDrawingWireframe = false;
bool CCubeModel::sUsingPackedLightmaps = false;

inline uint GetMaterialOffset(const uchar* materialData, const int idx) {
  materialData += (idx * 4);
  return CBasics::SwapBytes(*reinterpret_cast< const uint* >(materialData - 4));
}

CCubeModel::CCubeModel(rstl::vector< void* >* surfaces,
                       rstl::vector< TCachedToken< CTexture > >* textures, const void* materialData,
                       const void* positions, const void* normals, const void* colors,
                       const void* uvs, const void* compressedUvs, const CAABox& bounds,
                       const uchar visorFlags, const bool texturesLoaded, const uint idx)
: x0_instance(*surfaces, materialData, positions, normals, colors, uvs, compressedUvs)
, x1c_textures(textures)
, x20_bounds(bounds)
, x38_firstUnsorted(nullptr)
, x3c_firstSorted(nullptr)
, x40_24_loadTextures(static_cast< uchar >(!texturesLoaded))
, x40_25_visible(false)
, x41_visorFlags(visorFlags)
, x44_idx(idx) {
  rstl::vector< void* >& surf = x0_instance.Surfaces();
  for (AUTO(it, surf.begin()); it != surf.end(); ++it) {
    CCubeSurface::SSurfaceData* data = static_cast< CCubeSurface::SSurfaceData* >(*it);
    data->mParent = this;
  }

  for (int i = surf.size(); i > 0; i--) {
    void*& data = surf[i - 1];
    uint materialIndex = static_cast< CCubeSurface::SSurfaceData* >(data)->mMaterialIndex;
    if (GetMaterialByIndex(materialIndex).IsFlagSet(kStateFlag_DepthSorting)) {
      static_cast< CCubeSurface::SSurfaceData* >(data)->mNextSurface = x3c_firstSorted.x0_rawdata;
      x3c_firstSorted.x0_rawdata = static_cast< uchar* >(data);
    } else {
      static_cast< CCubeSurface::SSurfaceData* >(data)->mNextSurface = x38_firstUnsorted.x0_rawdata;
      x38_firstUnsorted.x0_rawdata = static_cast< uchar* >(data);
    }
  }
}

void CCubeModel::MakeTexturesFromMats(const void* data,
                                      rstl::vector< TCachedToken< CTexture > >& textures,
                                      IObjectStore& store, const bool cache) {
  const uint* textureIds = static_cast< const uint* >(data);
  const uint textureCount = CBasics::SwapBytes(*static_cast< const int* >(data));
  textureIds++;
  textures.reserve(textureCount);

  for (int i = 0; i < textureCount; i++) {
    textures.push_back(store.GetObj(SObjectTag('TXTR', CBasics::SwapBytes(*textureIds))));
    if (!cache) {
      textures.back().ForceCache();
    }
    ++textureIds;
  }
}

void CCubeModel::SetStaticArraysCurrent() const {
  CGX::SetArray(GX_VA_CLR0, x0_instance.GetColorPointer(), sizeof(CColor));
  const void* packed = x0_instance.GetPackedTCPointer();
  const void* unpacked = x0_instance.GetTCPointer();
  if (!packed) {
    sUsingPackedLightmaps = false;
  }

  if (sUsingPackedLightmaps) {
    CGX::SetArray(GX_VA_TEX0, packed, sizeof(ushort) * 2);
  } else {
    CGX::SetArray(GX_VA_TEX0, unpacked, sizeof(CVector2f));
  }

  if (unpacked) {
    for (int i = 1; i <= GX_VA_TEX7 - GX_VA_TEX0; ++i) {
      CGX::SetArray(static_cast< GXAttr >(i + GX_VA_TEX0), unpacked, sizeof(CVector2f));
    }
  }

  CCubeMaterial::KillCachedViewDepState();
}

void CCubeModel::SetArraysCurrent() const {
  CGX::SetArray(GX_VA_POS, x0_instance.GetVertexPointer(), sizeof(CVector3f));
  const int stride = (x41_visorFlags & 1) ? sizeof(short) * 3 : sizeof(CVector3f);
  CGX::SetArray(GX_VA_NRM, x0_instance.GetNormalPointer(), stride);
  SetStaticArraysCurrent();
}

void CCubeModel::SetSkinningArraysCurrent(const float* positions, const float* normals) const {
  CGraphics::sRenderState.SetVtxState(positions, normals,
                                      static_cast< const uint* >(x0_instance.GetColorPointer()));
  SetStaticArraysCurrent();
}

void CCubeModel::SetUsingPackedLightmaps(const bool use) const {
  sUsingPackedLightmaps = use;
  if (sUsingPackedLightmaps) {
    CGX::SetArray(GX_VA_TEX0, x0_instance.GetPackedTCPointer(), sizeof(ushort) * 2);
  } else {
    CGX::SetArray(GX_VA_TEX0, x0_instance.GetTCPointer(), sizeof(CVector2f));
  }
}

CCubeMaterial CCubeModel::GetMaterialByIndex(const int idx) const {
  uint materialCount = 0;
  uint materialOffset = 0;
  const uchar* materialData = static_cast< const uchar* >(x0_instance.GetMaterialPointer()) +
                              (x1c_textures->size() + 1) * 4;
  materialCount = *reinterpret_cast< const uint* >(materialData++);
  materialCount = CBasics::SwapBytes(materialCount);
  materialData++;
  materialData++;
  materialData++;
  if (idx != 0) {
    materialOffset = GetMaterialOffset(materialData, idx);
  }

  materialData += (materialCount * 4);
  materialData += materialOffset;
  return CCubeMaterial(materialData);
}

void CCubeModel::DrawSurface(const CCubeSurface& surface, const CModelFlags& modelFlags) const {
  const CCubeMaterial material = GetMaterialByIndex(surface.GetMaterialIndex());
  if (material.IsFlagSet(kStateFlag_ShadowOccluderMesh) && !sDrawingOccluders) {
    return;
  }

  material.SetCurrent(modelFlags, surface, *this);
  CGX::CallDisplayList(surface.GetDisplayList(), surface.GetDisplayListSize());
}

static inline const ushort proxy_to_uint(const uchar* data) {
  uchar bytes[2];
  bytes[0] = data[0];
  bytes[1] = data[1];
#ifdef __MWERKS__
  return CBasics::SwapBytes(*reinterpret_cast< const ushort* >(bytes));
#else
  ushort value;
  memcpy(&value, bytes, sizeof(value));
  return CBasics::SwapBytes(value);
#endif
}

void CCubeModel::DrawSurfaceWireframe(const CCubeSurface& surface) const {
  const CCubeMaterial material = GetMaterialByIndex(surface.GetMaterialIndex());
  uint vertexAttributes;
  static uint sLastDesc = 0;
  static uint sAttrCount = 0;
  vertexAttributes = material.GetVertexDesc();

  if (vertexAttributes != sLastDesc) {
    sAttrCount = 0;
    for (int i = 0; i < 16; ++i, sLastDesc = vertexAttributes) {
      if ((vertexAttributes >> (i * 2)) & 3) {
        sAttrCount++;
      }
    }
  }

  const int attrCountTimes2 = sAttrCount * 2;
  static const GXVtxDescList sDesc[] = {
      {GX_VA_POS, GX_INDEX16},
      {GX_VA_NULL, GX_NONE},
  };
  CGX::SetVtxDescv(sDesc);
  CGX::SetTevDirect(GX_TEVSTAGE0);
  CGX::SetNumIndStages(0);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ONE);
  CGX::SetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
  CGX::SetNumChans(0);
  CGX::SetNumTexGens(1);
  CGX::SetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR);

  const int displayListSize = surface.GetDisplayListSize();
  const uchar* dispList = static_cast< const uchar* >(surface.GetDisplayList());
  for (int bytesRead = 0; bytesRead < displayListSize;) {
    const uchar pType = *dispList & 0xf8;
    if (!pType) {
      break;
    }
    bytesRead += 3;
    ushort elementCount = proxy_to_uint(dispList + 1);
    dispList += 3;
    if (elementCount < 3U) {
      break;
    }

    CGX::Begin(GX_LINESTRIP, GX_VTXFMT0, 4);
    GXPosition1x16(proxy_to_uint(dispList));
    GXPosition1x16(proxy_to_uint(dispList + attrCountTimes2));
    GXPosition1x16(proxy_to_uint(dispList + attrCountTimes2 * 2));
    GXPosition1x16(proxy_to_uint(dispList));
    bytesRead += elementCount * attrCountTimes2;
    dispList += attrCountTimes2 * 3;
    CGX::End();
    if (pType == GX_TRIANGLES) {
      elementCount -= 3;
      for (int j = 0; j < elementCount; j += 3) {
        CGX::Begin(GX_LINESTRIP, GX_VTXFMT0, 4);
        GXPosition1x16(proxy_to_uint(dispList));
        GXPosition1x16(proxy_to_uint(dispList + attrCountTimes2));
        GXPosition1x16(proxy_to_uint(dispList + attrCountTimes2 * 2));
        GXPosition1x16(proxy_to_uint(dispList));
        dispList += attrCountTimes2 * 3;
        CGX::End();
      }
    } else if (pType == GX_TRIANGLESTRIP) {
      elementCount -= 3;
      uint winding = 1;
      for (int j = 0; j < elementCount; ++j) {
        CGX::Begin(GX_LINESTRIP, GX_VTXFMT0, 3);
        const uchar* last = dispList - attrCountTimes2 * ((winding ^ 1) + 1);
        const uchar* first = dispList - attrCountTimes2 * (winding + 1);
        winding ^= 1;
        GXPosition1x16(proxy_to_uint(first));
        GXPosition1x16(proxy_to_uint(dispList));
        dispList += attrCountTimes2;
        GXPosition1x16(proxy_to_uint(last));
        CGX::End();
      }
    } else {
      if (pType != GX_TRIANGLEFAN) {
        return;
      }
      elementCount -= 3;
      const uchar* indices = dispList - attrCountTimes2 * 3;

      for (int j = 0; j < elementCount; ++j) {
        const uchar* previous = dispList - attrCountTimes2;
        CGX::Begin(GX_LINESTRIP, GX_VTXFMT0, 3);
        GXPosition1x16(proxy_to_uint(previous));
        GXPosition1x16(proxy_to_uint(dispList));
        dispList += attrCountTimes2;
        GXPosition1x16(proxy_to_uint(indices));
        CGX::End();
      }
    }
  }
}

bool CCubeModel::TryLockTextures() const {
  if (!x40_24_loadTextures) {
    bool texturesLoading = false;
    for (int i = 0; i < GetTextures().size(); ++i) {
      GetTextures()[i].Lock();
      if (!GetTextures()[i].TryCache()) {
        texturesLoading = true;
      } else if (!GetTextures()[i].GetObject()->LoadToMRAM()) {
        texturesLoading = true;
      }
    }

    if (!texturesLoading) {
      x40_24_loadTextures = true;
    }
  }

  return !!x40_24_loadTextures;
}

void CCubeModel::DrawSurfaces(const CModelFlags& flags) const {
  if (sDrawingWireframe) {
    for (CCubeSurface surface = GetNormalSurfaces(); surface.IsValid();
         surface = surface.GetNextSurface()) {
      DrawSurfaceWireframe(surface);
    }
    for (CCubeSurface surface = GetAlphaSurfaces(); surface.IsValid();
         surface = surface.GetNextSurface()) {
      DrawSurfaceWireframe(surface);
    }
  } else if ((flags.GetOtherFlags() & CModelFlags::kF_NoTextureLock) || TryLockTextures()) {
    for (CCubeSurface surface = GetNormalSurfaces(); surface.IsValid();
         surface = surface.GetNextSurface()) {
      DrawSurface(surface, flags);
    }
    for (CCubeSurface surface = GetAlphaSurfaces(); surface.IsValid();
         surface = surface.GetNextSurface()) {
      DrawSurface(surface, flags);
    }
  }
}

void CCubeModel::DrawNormalSurfaces(const CModelFlags& flags) const {
  if (sDrawingWireframe) {
    for (CCubeSurface surface = GetNormalSurfaces(); surface.IsValid();
         surface = surface.GetNextSurface()) {
      DrawSurfaceWireframe(surface);
    }
  } else if (TryLockTextures()) {
    for (CCubeSurface surface = GetNormalSurfaces(); surface.IsValid();
         surface = surface.GetNextSurface()) {
      DrawSurface(surface, flags);
    }
  }
}

void CCubeModel::DrawAlphaSurfaces(const CModelFlags& flags) const {
  if (sDrawingWireframe) {
    for (CCubeSurface surface = GetAlphaSurfaces(); surface.IsValid();
         surface = surface.GetNextSurface()) {
      DrawSurfaceWireframe(surface);
    }
  } else if (TryLockTextures()) {
    for (CCubeSurface surface = GetAlphaSurfaces(); surface.IsValid();
         surface = surface.GetNextSurface()) {
      DrawSurface(surface, flags);
    }
  }
}

void CCubeModel::DrawFlat(const float* positions, const float* normals,
                          ESurfaceSelection which) const {
  if (positions != nullptr) {
    SetSkinningArraysCurrent(positions, normals);
  } else {
    SetArraysCurrent();
  }

  if (which != kSS_Sorted) {
    for (CCubeSurface surface = x38_firstUnsorted; surface.IsValid();
         surface = surface.GetNextSurface()) {
      CCubeMaterial material = GetMaterialByIndex(surface.GetMaterialIndex());
      CGX::SetVtxDescv_Compressed(material.GetVertexDescLwzx());
      CGX::CallDisplayList(surface.GetDisplayList(), surface.GetDisplayListSize());
    }
  }

  if (which != kSS_Unsorted) {
    for (CCubeSurface surface = x3c_firstSorted; surface.IsValid();
         surface = surface.GetNextSurface()) {
      CCubeMaterial material = GetMaterialByIndex(surface.GetMaterialIndex());
      CGX::SetVtxDescv_Compressed(material.GetVertexDescLwzx());
      CGX::CallDisplayList(surface.GetDisplayList(), surface.GetDisplayListSize());
    }
  }
}

void CCubeModel::Draw(const CModelFlags& flags) const {
  CCubeMaterial::KillCachedViewDepState();
  SetArraysCurrent();
  DrawSurfaces(flags);
}

void CCubeModel::Draw(const float* positions, const float* normals,
                      const CModelFlags& flags) const {
  CCubeMaterial::KillCachedViewDepState();
  SetSkinningArraysCurrent(positions, normals);
  DrawSurfaces(flags);
}

void CCubeModel::DrawNormal(const CModelFlags& flags) const {
  CCubeMaterial::KillCachedViewDepState();
  SetArraysCurrent();
  DrawNormalSurfaces(flags);
}

void CCubeModel::DrawAlpha(const CModelFlags& flags) const {
  CCubeMaterial::KillCachedViewDepState();
  SetArraysCurrent();
  DrawAlphaSurfaces(flags);
}

void CCubeModel::SetDrawingOccluders(const bool drawOccluders) {
  sDrawingOccluders = drawOccluders;
}

void CCubeModel::SetModelWireframe(const bool drawWireframe) { sDrawingWireframe = drawWireframe; }

void CCubeModel::UnlockTextures() const {
  for (AUTO(texture, x1c_textures->begin()); texture != x1c_textures->end(); ++texture) {
    texture->Unlock();
  }

  x40_24_loadTextures = false;
}

void CCubeModel::RemapMaterialData(const void* data,
                                   rstl::vector< TCachedToken< CTexture > >* texture) {

  x0_instance.SetMaterialPointer(data);
  x1c_textures = texture;
  x40_24_loadTextures = false;
}

void CCubeModel::DrawNormal(const float* positions, const float* normals,
                            ESurfaceSelection which) const {
  CGX::SetNumIndStages(0);
  CGX::SetNumTevStages(1);
  CGX::SetNumTexGens(1);
  CGX::SetZMode(true, GX_LEQUAL, true);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE0);
  CGX::SetBlendMode(GX_BM_BLEND, GX_BL_ZERO, GX_BL_ONE, GX_LO_CLEAR);
  DrawFlat(positions, normals, which);
}
