#pragma once

#include <memory>
#include <vector>

#include "CToken.hpp"
#include "GCNTypes.hpp"
#include "Graphics/CCubeModel.hpp"
#include "Graphics/CTexture.hpp"
#include "Runtime/Factory/IObjectStore.hpp"

namespace metaforce {
class CCubeSurface;
class CCubeMaterial;

struct CModelFlags {
  /**
   * 2: add color
   * >6: additive
   * >4: blend
   * else opaque
   */
  u8 x0_blendMode = 0;
  u8 x1_matSetIdx = 0;
  /**
   * 0x1: depth equal
   * 0x2: depth update
   * 0x4: render without texture lock
   * 0x8: depth greater
   * 0x10: depth non-inclusive
   */
  u16 x2_flags = 0;
  /**
   * Set into kcolor slot specified by material
   */
  zeus::CColor x4_color;

  constexpr CModelFlags() = default;
  constexpr CModelFlags(u8 blendMode, u8 shadIdx, u16 flags, const zeus::CColor& col)
  : x0_blendMode(blendMode), x1_matSetIdx(shadIdx), x2_flags(flags), x4_color(col) {}

  bool operator==(const CModelFlags& other) const {
    return x0_blendMode == other.x0_blendMode && x1_matSetIdx == other.x1_matSetIdx && x2_flags == other.x2_flags &&
           x4_color == other.x4_color;
  }

  bool operator!=(const CModelFlags& other) const { return !operator==(other); }
};

class CModel {
public:
  struct SShader {
    std::vector<TCachedToken<CTexture>> x0_textures;
    u8* x10_data;

    explicit SShader(u8* data) : x10_data(data) {}

    void UnlockTextures();
  };

private:
  static u32 sTotalMemory;
  static u32 sFrameCounter;
  static bool sIsTextureTimeoutEnabled;
  static CModel* sThisFrameList;
  static CModel* sOneFrameList;
  static CModel* sTwoFrameList;

  std::unique_ptr<u8[]> x0_data;
  u32 x4_dataLen;
  std::vector<CCubeSurface> x8_surfaces; // was rstl::vector<void*>
  std::vector<SShader> x18_matSets;
  std::unique_ptr<CCubeModel> x28_modelInst = nullptr;
  u16 x2c_currentMatIdx = 0;
  u16 x2e_lastFrame = 0; // Last frame that the model switched materials
  CModel* x30_prev = nullptr;
  CModel* x34_next;
  u32 x38_lastFrame;

  /* Resident copies of maintained data */
  std::vector<zeus::CVector3f> m_positions;
  std::vector<zeus::CVector3f> m_normals;
  std::vector<zeus::CColor> m_colors;
  std::vector<zeus::CVector2f> m_floatUVs;
  std::vector<zeus::CVector2f> m_shortUVs;

public:
  CModel(std::unique_ptr<u8[]> in, u32 dataLen, IObjectStore* store);

  void UpdateLastFrame();
  void MoveToThisFrameList();
  void RemoveFromList();
  void VerifyCurrentShader(u32 matIdx);
  void Touch(u32 matIdx);
  void Draw(CModelFlags flags) const;
  void Draw(TVectorRef positions, TVectorRef normals, CModelFlags flags);
  void DrawSortedParts(CModelFlags flags);
  void DrawUnsortedParts(CModelFlags flags);
  bool IsLoaded(u32 matIdx);

  TVectorRef GetPositions() const;
  TVectorRef GetNormals() const;
  u32 GetNumMaterialSets() const { return x18_matSets.size(); }
  bool IsOpaque() const { return x28_modelInst->x3c_firstSortedSurf == nullptr; }
  const zeus::CAABox& GetAABB() const { return x28_modelInst->x20_worldAABB; }

  static void FrameDone();
  static void EnableTextureTimeout();
  static void DisableTextureTimeout();
};

CFactoryFnReturn FModelFactory(const metaforce::SObjectTag& tag, std::unique_ptr<u8[]>&& in, u32 len,
                               const metaforce::CVParamTransfer& vparms, CObjectReference* selfRef);
} // namespace metaforce
