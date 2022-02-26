#include "CCubeModel.hpp"

#include "Runtime/Factory/CSimplePool.hpp"
#include "Graphics/CCubeMaterial.hpp"
#include "Graphics/CCubeSurface.hpp"
#include "Graphics/CGraphics.hpp"
#include "Graphics/CModel.hpp"

namespace metaforce {
bool CCubeModel::sRenderModelBlack = false;
bool CCubeModel::sRenderModelShadow = false;
bool CCubeModel::sUsingPackedLightmaps = false;
const CTexture* CCubeModel::sShadowTexture = nullptr;

static bool sDrawingOccluders = false;
static bool sDrawingWireframe = false;

static zeus::CTransform sTextureProjectionTransform;
static u8 sChannel0DisableLightMask = 0;
static u8 sChannel1EnableLightMask = 0;

static zeus::CVector3f sPlayerPosition;

CCubeModel::CCubeModel(std::vector<CCubeSurface>* surfaces, std::vector<TCachedToken<CTexture>>* textures,
                       u8* materialData, std::vector<zeus::CVector3f>* positions, std::vector<zeus::CColor>* colors,
                       std::vector<zeus::CVector3f>* normals, std::vector<zeus::CVector2f>* texCoords,
                       std::vector<zeus::CVector2f>* packedTexCoords, const zeus::CAABox& aabb, u8 flags, bool b1,
                       u32 idx)
: x0_modelInstance(surfaces, materialData, positions, colors, normals, texCoords, packedTexCoords)
, x1c_textures(textures)
, x20_worldAABB(aabb)
, x40_24_texturesLoaded(!b1)
, x41_visorFlags(flags)
, x44_idx(idx) {
  for (auto& surf : *x0_modelInstance.Surfaces()) {
    surf.SetParent(this);
  }

  for (u32 i = x0_modelInstance.Surfaces()->size(); i > 0; --i) {
    auto& surf = (*x0_modelInstance.Surfaces())[i - 1];
    const auto matFlags = GetMaterialByIndex(surf.GetMaterialIndex()).GetFlags();
    if (!matFlags.IsSet(CCubeMaterialFlagBits::fDepthSorting)) {
      surf.SetNextSurface(x38_firstUnsortedSurf);
      x38_firstUnsortedSurf = &surf;
    } else {
      surf.SetNextSurface(x3c_firstSortedSurf);
      x3c_firstSortedSurf = &surf;
    }
  }
}

CCubeMaterial CCubeModel::GetMaterialByIndex(u32 idx) {
  u32 materialOffset = 0;
  const u8* matData = x0_modelInstance.GetMaterialPointer();
  matData += (x1c_textures->size() + 1) * 4;
  if (idx != 0) {
    materialOffset = SBig(*reinterpret_cast<const u32*>(matData + (idx * 4)));
  }

  u32 materialCount = SBig(*reinterpret_cast<const u32*>(matData));
  return CCubeMaterial(matData + materialOffset + (materialCount * 4) + 4);
}

bool CCubeModel::TryLockTextures() {
  if (!x40_24_texturesLoaded) {
    bool texturesPumped = false;
    for (auto& texture : *x1c_textures) {
      texture.Lock();
      bool loadTexture = true;
      if (!texture.HasReference()) {
        if (!texture.IsLocked() || texture.IsNull()) {
          loadTexture = false;
        } else {
          texture.GetObj();
        }
      }
      if (loadTexture) {
        // texture->LoadToMRAM();
        texturesPumped = true;
      }
    }
    if (!texturesPumped) {
      x40_24_texturesLoaded = true;
    }
  }
  return x40_24_texturesLoaded;
}

void CCubeModel::UnlockTextures() {
  for (auto& token : *x1c_textures) {
    token.Unlock();
  }
}

void CCubeModel::RemapMaterialData(u8* data, std::vector<TCachedToken<CTexture>>& textures) {
  x0_modelInstance.SetMaterialPointer(data);
  x1c_textures = &textures;
  x40_24_texturesLoaded = false;
}

void CCubeModel::MakeTexturesFromMats(const u8* ptr, std::vector<TCachedToken<CTexture>>& textures, IObjectStore* store,
                                      bool b1) {
  const u32* curId = reinterpret_cast<const u32*>(ptr + 4);
  u32 textureCount = SBig(*reinterpret_cast<const u32*>(ptr));
  textures.reserve(textureCount);
  for (u32 i = 0; i < textureCount; ++i) {
    textures.emplace_back(store->GetObj({FOURCC('TXTR'), SBig(curId[i])}));

    if (!b1 && textures.back().IsNull()) {
      textures.back().GetObj();
    }
  }
}

void CCubeModel::Draw(const CModelFlags& flags) {
  CCubeMaterial::KillCachedViewDepState();
  SetArraysCurrent();
  DrawSurfaces(flags);
}

void CCubeModel::Draw(TVectorRef positions, TVectorRef normals, const CModelFlags& flags) {
  CCubeMaterial::KillCachedViewDepState();
  SetSkinningArraysCurrent(positions, normals);
  DrawSurfaces(flags);
}

void CCubeModel::DrawAlpha(const CModelFlags& flags) {
  CCubeMaterial::KillCachedViewDepState();
  SetArraysCurrent();
  DrawAlphaSurfaces(flags);
}

void CCubeModel::DrawAlphaSurfaces(const CModelFlags& flags) {
  if (sDrawingWireframe) {
    const auto* surface = x3c_firstSortedSurf;
    while (surface != nullptr) {
      DrawSurfaceWireframe(*surface);
      surface = surface->GetNextSurface();
    }
  } else if (TryLockTextures()) {
    const auto* surface = x3c_firstSortedSurf;
    while (surface != nullptr) {
      DrawSurface(*surface, flags);
      surface = surface->GetNextSurface();
    }
  }
}

void CCubeModel::DrawFlat(TVectorRef positions, TVectorRef normals, ESurfaceSelection surfaces) {
  if (positions == nullptr) {
    SetArraysCurrent();
  } else {
    SetSkinningArraysCurrent(positions, normals);
  }
  if (surfaces != ESurfaceSelection::Sorted) {
    const auto* surface = x38_firstUnsortedSurf;
    while (surface != nullptr) {
      const auto mat = GetMaterialByIndex(surface->GetMaterialIndex());
      // TODO draw
    }
  }
  if (surfaces != ESurfaceSelection::Unsorted) {
    const auto* surface = x3c_firstSortedSurf;
    while (surface != nullptr) {
      const auto mat = GetMaterialByIndex(surface->GetMaterialIndex());
      // TODO draw
    }
  }
}

void CCubeModel::DrawNormal(TVectorRef positions, TVectorRef normals, ESurfaceSelection surfaces) {
  // TODO bunch of CGX stuff, what is it doing?
  DrawFlat(positions, normals, surfaces);
}

void CCubeModel::DrawNormal(const CModelFlags& flags) {
  CCubeMaterial::KillCachedViewDepState();
  SetArraysCurrent();
  DrawNormalSurfaces(flags);
}

void CCubeModel::DrawNormalSurfaces(const CModelFlags& flags) {
  if (sDrawingWireframe) {
    const auto* surface = x38_firstUnsortedSurf;
    while (surface != nullptr) {
      DrawSurfaceWireframe(*surface);
      surface = surface->GetNextSurface();
    }
  } else if (TryLockTextures()) {
    const auto* surface = x38_firstUnsortedSurf;
    while (surface != nullptr) {
      DrawSurface(*surface, flags);
      surface = surface->GetNextSurface();
    }
  }
}

void CCubeModel::DrawSurface(const CCubeSurface& surface, const CModelFlags& flags) {
  auto mat = GetMaterialByIndex(surface.GetMaterialIndex());
  if (!mat.GetFlags().IsSet(CCubeMaterialFlagBits::fShadowOccluderMesh) || sDrawingOccluders) {
    mat.SetCurrent(flags, surface, *this);
    // TODO draw
  }
}

void CCubeModel::DrawSurfaces(const CModelFlags& flags) {
  if (sDrawingWireframe) {
    const auto* surface = x38_firstUnsortedSurf;
    while (surface != nullptr) {
      DrawSurfaceWireframe(*surface);
      surface = surface->GetNextSurface();
    }
    surface = x3c_firstSortedSurf;
    while (surface != nullptr) {
      DrawSurfaceWireframe(*surface);
      surface = surface->GetNextSurface();
    }
  } else if (TryLockTextures()) {
    const auto* surface = x38_firstUnsortedSurf;
    while (surface != nullptr) {
      DrawSurface(*surface, flags);
      surface = surface->GetNextSurface();
    }
    surface = x3c_firstSortedSurf;
    while (surface != nullptr) {
      DrawSurface(*surface, flags);
      surface = surface->GetNextSurface();
    }
  }
}

void CCubeModel::DrawSurfaceWireframe(const CCubeSurface& surface) {
  auto mat = GetMaterialByIndex(surface.GetMaterialIndex());
  // TODO convert vertices to line strips and draw
}

void CCubeModel::EnableShadowMaps(const CTexture& shadowTex, const zeus::CTransform& textureProjXf, u8 chan0DisableMask,
                                  u8 chan1EnableLightMask) {
  sRenderModelShadow = true;
  sShadowTexture = &shadowTex;
  sTextureProjectionTransform = textureProjXf;
  sChannel0DisableLightMask = chan0DisableMask;
  sChannel1EnableLightMask = chan1EnableLightMask;
}

void CCubeModel::DisableShadowMaps() { sRenderModelShadow = false; }

void CCubeModel::SetArraysCurrent() {
  if (x0_modelInstance.GetVertexPointer() != nullptr) {
    // TODO set vertices active
  }
  if (x0_modelInstance.GetNormalPointer() != nullptr) {
    // TODO set normals active
  }
  SetStaticArraysCurrent();
}

void CCubeModel::SetDrawingOccluders(bool v) { sDrawingOccluders = v; }

void CCubeModel::SetModelWireframe(bool v) { sDrawingWireframe = v; }

void CCubeModel::SetNewPlayerPositionAndTime(const zeus::CVector3f& pos, const CStopwatch& time) {
  sPlayerPosition = pos;
  CCubeMaterial::KillCachedViewDepState();
  // TODO time
}

void CCubeModel::SetRenderModelBlack(bool v) {
  sRenderModelBlack = v;
  // TODO another value is set here, but always 0?
}

void CCubeModel::SetSkinningArraysCurrent(TVectorRef positions, TVectorRef normals) {
  // TODO activate vertices, normals & colors
  SetStaticArraysCurrent();
}

void CCubeModel::SetStaticArraysCurrent() {
  // TODO activate colors
  const auto* packedTexCoords = x0_modelInstance.GetPackedTCPointer();
  const auto* texCoords = x0_modelInstance.GetTCPointer();
  if (packedTexCoords == nullptr) {
    sUsingPackedLightmaps = false;
  }
  if (sUsingPackedLightmaps) {
    // TODO activate packed TCs for texture 0
  } else if (texCoords != nullptr) {
    // TODO activate TCs for texture 0
  }
  if (texCoords != nullptr) {
    for (int i = 1; i < 8; ++i) {
      // TODO activate TCs for textures 1-7
    }
  }
  CCubeMaterial::KillCachedViewDepState();
}

void CCubeModel::SetUsingPackedLightmaps(bool v) {
  sUsingPackedLightmaps = v;
  if (v) {
    // TODO activate packed TCs for texture 0
  } else {
    // TODO activate TCs for texture 0
  }
}

} // namespace metaforce
