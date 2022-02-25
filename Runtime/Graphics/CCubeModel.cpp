#include "CCubeModel.hpp"

#include "CSimplePool.hpp"
#include "Graphics/CCubeMaterial.hpp"
#include "Graphics/CCubeSurface.hpp"
#include "Graphics/CGraphics.hpp"

namespace metaforce {

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
    if (!GetMaterialByIndex(surf.GetMaterialIndex()).IsFlagSet(EStateFlags::DepthSorting)) {
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

} // namespace metaforce
