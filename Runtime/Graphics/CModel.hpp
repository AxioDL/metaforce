#pragma once

#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

#include "DataSpec/DNACommon/CMDL.hpp"
#include "DataSpec/DNAMP1/CMDLMaterials.hpp"
#include "Runtime/CFactoryMgr.hpp"
#include "Runtime/CToken.hpp"
#include "Runtime/RetroTypes.hpp"
#include "Runtime/Graphics/CTexture.hpp"
#include "Runtime/Graphics/Shaders/CModelShaders.hpp"

#include "hecl/HMDLMeta.hpp"
#include "zeus/CAABox.hpp"
#include "zeus/CColor.hpp"

namespace metaforce {
class CLight;
class CModel;
class CPoseAsTransforms;
class CSkinRules;
class IObjectStore;

struct CModelFlags {
  u8 x0_blendMode = 0; /* 2: add color, >6: additive, >4: blend, else opaque */
  u8 x1_matSetIdx = 0;
  u16 x2_flags = 0;      /* Flags */
  zeus::CColor x4_color; /* Set into kcolor slot specified by material */
  EPostType m_postType = EPostType::Normal;
  // For PostType::ThermalHot and Disintegrate
  zeus::CColor m_addColor = zeus::skClear;
  // For PostType::MBShadow
  zeus::CAABox m_mbShadowBox;

  constexpr CModelFlags() = default;
  constexpr CModelFlags(u8 blendMode, u8 shadIdx, u16 flags, const zeus::CColor& col)
  : x0_blendMode(blendMode), x1_matSetIdx(shadIdx), x2_flags(flags), x4_color(col) {}

  /* Flags
      0x1: depth lequal
      0x2: depth update
      0x4: render without texture lock
      0x8: depth greater
      0x10: depth non-inclusive
   */

  bool operator==(const CModelFlags& other) const {
    return x0_blendMode == other.x0_blendMode && x1_matSetIdx == other.x1_matSetIdx && x2_flags == other.x2_flags &&
           x4_color == other.x4_color;
  }

  bool operator!=(const CModelFlags& other) const { return !operator==(other); }
};

/* metaforce addition: doesn't require hacky stashing of
 * pointers within loaded CMDL buffer */
struct CBooSurface {
  DataSpec::DNACMDL::SurfaceHeader_2 m_data;
  size_t selfIdx;
  class CBooModel* m_parent = nullptr;
  CBooSurface* m_next = nullptr;

  zeus::CAABox GetBounds() const {
    if (!m_data.aabbSz)
      return zeus::CAABox(m_data.centroid, m_data.centroid);
    else
      return zeus::CAABox(m_data.aabb[0], m_data.aabb[1]);
  }
};

using MaterialSet = DataSpec::DNAMP1::HMDLMaterialSet;

struct GeometryUniformLayout {
  size_t m_skinBankCount = 0;
  size_t m_weightVecCount = 0;

  GeometryUniformLayout(const CModel* model, const MaterialSet* matSet);
  void Update(const CModelFlags& flags, const CSkinRules* cskr, const CPoseAsTransforms* pose,
              const MaterialSet* matSet, std::vector<hsh::dynamic_owner<hsh::uniform_buffer_typeless>>& buf,
              const CBooModel* parent) const;

  hsh::dynamic_owner<hsh::uniform_buffer_typeless> AllocateVertUniformBuffer() const;
};

struct SShader {
  std::unordered_map<CAssetId, TCachedToken<CTexture>> x0_textures;
  MaterialSet m_matSet;
  std::optional<GeometryUniformLayout> m_geomLayout;
  int m_matSetIdx;
  explicit SShader(int idx) : m_matSetIdx(idx) { x0_textures.clear(); }
  void InitializeLayout(const CModel* model) { m_geomLayout.emplace(model, &m_matSet); }
  void UnlockTextures();
};

struct VertexFormat {
  uint8_t NSkinSlots;
  uint8_t NCol;
  uint8_t NUv;
  uint8_t NWeight;
  explicit VertexFormat(const hecl::HMDLMeta& meta)
  : NSkinSlots(meta.weightCount * 4), NCol(meta.colorCount), NUv(meta.uvCount), NWeight(meta.weightCount) {}
};

class CBooModel {
  friend class CBooRenderer;
  friend class CGameArea;
  friend class CMetroidModelInstance;
  friend class CModel;
  friend class CSkinnedModel;
  friend struct GeometryUniformLayout;
  friend class CModelShaders;
  friend struct ModelInstance;

public:
  enum class ESurfaceSelection { UnsortedOnly, SortedOnly, All };

private:
  CBooModel* m_next = nullptr;
  CBooModel* m_prev = nullptr;
  size_t m_uniUpdateCount = 0;
  TToken<CModel> m_modelTok;
  CModel* m_model;
  std::vector<CBooSurface>* x0_surfaces;
  const MaterialSet* x4_matSet;
  const GeometryUniformLayout* m_geomLayout;
  VertexFormat m_vtxFmt;
  int m_matSetIdx = -1;
  std::unordered_map<CAssetId, TCachedToken<CTexture>> x1c_textures;
  zeus::CAABox x20_aabb;
  CBooSurface* x38_firstUnsortedSurface = nullptr;
  CBooSurface* x3c_firstSortedSurface = nullptr;
  bool x40_24_texturesLoaded : 1 = false;
  bool x40_25_modelVisible : 1 = false;
  u8 x41_mask;
  u32 x44_areaInstanceIdx = UINT32_MAX;

  struct UVAnimationBuffer {
    static void ProcessAnimation(CModelShaders::TCGMatrix& tcg, const MaterialSet::Material::PASS& anim);
    static void Update(std::vector<hsh::dynamic_owner<hsh::uniform_buffer<CModelShaders::TCGMatrixUniform>>>& uniforms,
                       const MaterialSet* matSet, const CModelFlags& flags, const CBooModel* parent);
  };

  CModelShaders::FragmentUniform m_lightingData{};
  bool m_lightsActive = false;

  /* metaforce addition: boo! */
  size_t m_uniformDataSize = 0;
  std::vector<ModelInstance> m_instances;
  ModelInstance m_ballShadowInstance;

  hsh::vertex_buffer_typeless m_staticVbo;
  hsh::index_buffer_typeless m_staticIbo;

  hsh::texture2d m_lastDrawnShadowMap;
  hsh::texture2d m_lastDrawnOneTexture;
  hsh::texturecube m_lastDrawnReflectionCube;

  ModelInstance* PushNewModelInstance(int sharedLayoutBuf = -1, boo::IGraphicsDataFactory::Context* ctx = nullptr);
  void DrawAlphaSurfaces(const CModelFlags& flags) const;
  void DrawNormalSurfaces(const CModelFlags& flags) const;
  void DrawSurfaces(const CModelFlags& flags) const;
  void DrawSurface(const CBooSurface& surf, const CModelFlags& flags) const;
  void WarmupDrawSurface(const CBooSurface& surf, const CModelFlags& flags) const;

  static inline zeus::CVector3f g_PlayerPosition;
  static inline float g_ModSeconds = 0.0f;
  static inline float g_TransformedTime = 0.0f;
  static inline float g_TransformedTime2 = 0.0f;
  static inline CBooModel* g_LastModelCached = nullptr;

  static inline bool g_DummyTextures = false;
  static inline bool g_RenderModelBlack = false;

public:
  ~CBooModel();
  CBooModel(TToken<CModel>& token, CModel* parent, std::vector<CBooSurface>* surfaces, SShader& shader,
            hsh::vertex_buffer_typeless vbo, hsh::index_buffer<u32> ibo, const zeus::CAABox& aabb, u8 renderMask,
            int numInsts, VertexFormat vtxFmt);

  static void MakeTexturesFromMats(const MaterialSet& matSet,
                                   std::unordered_map<CAssetId, TCachedToken<CTexture>>& toksOut, IObjectStore& store);
  void MakeTexturesFromMats(std::unordered_map<CAssetId, TCachedToken<CTexture>>& toksOut, IObjectStore& store);

  bool IsOpaque() const { return x3c_firstSortedSurface == nullptr; }
  void ActivateLights(const std::vector<CLight>& lights);
  void SetAmbientColor(const zeus::CColor& color) { m_lightingData.ambient = color; }
  void DisableAllLights();
  void RemapMaterialData(SShader& shader);
  bool TryLockTextures();
  void UnlockTextures();
  void SyncLoadTextures();
  void Touch(int shaderIdx);
  void VerifyCurrentShader(int shaderIdx);
  hsh::dynamic_owner<hsh::vertex_buffer_typeless>* UpdateUniformData(const CModelFlags& flags, const CSkinRules* cskr,
                                                                     const CPoseAsTransforms* pose,
                                                                     int sharedLayoutBuf = -1,
                                                         boo::IGraphicsDataFactory::Context* ctx = nullptr);
  void DrawAlpha(const CModelFlags& flags, const CSkinRules* cskr, const CPoseAsTransforms* pose);
  void DrawNormal(const CModelFlags& flags, const CSkinRules* cskr, const CPoseAsTransforms* pose);
  void Draw(const CModelFlags& flags, const CSkinRules* cskr, const CPoseAsTransforms* pose);
  void DrawFlat(ESurfaceSelection sel, EExtendedShader extendedIdx) const;

  void LockParent() { m_modelTok.Lock(); }
  void UnlockParent() { m_modelTok.Unlock(); }

  const MaterialSet::Material& GetMaterialByIndex(int idx) const { return x4_matSet->materials.at(idx); }

  void ClearUniformCounter() { m_uniUpdateCount = 0; }
  static void ClearModelUniformCounters();

  static inline bool g_DrawingOccluders = false;
  static void SetDrawingOccluders(bool occ) { g_DrawingOccluders = occ; }

  static void SetNewPlayerPositionAndTime(const zeus::CVector3f& pos);

  static inline zeus::CVector3f g_ReflectViewPos;
  static void KillCachedViewDepState();
  static void EnsureViewDepStateCached(const CBooModel& model, const CBooSurface* surf,
                                       hsh::dynamic_owner<hsh::uniform_buffer<CModelShaders::ReflectMtx>>& buf);

  static hsh::texture2d g_shadowMap;
  static zeus::CTransform g_shadowTexXf;
  static void EnableShadowMaps(hsh::texture2d map, const zeus::CTransform& texXf);
  static void DisableShadowMaps();

  static hsh::texture2d g_disintegrateTexture;
  static void SetDisintegrateTexture(hsh::texture2d map) { g_disintegrateTexture = map; }

  static hsh::texturecube g_reflectionCube;
  static void SetReflectionCube(hsh::texturecube map) { g_reflectionCube = map; }

  static void SetDummyTextures(bool b) { g_DummyTextures = b; }
  static void SetRenderModelBlack(bool b) { g_RenderModelBlack = b; }

  static void Shutdown();

  const zeus::CAABox& GetAABB() const { return x20_aabb; }
  void WarmupDrawSurfaces(const CModelFlags& unsortedFlags, const CModelFlags& sortedFlags) const;
};

class CModel {
  friend class CBooModel;
  friend struct GeometryUniformLayout;
  // std::unique_ptr<u8[]> x0_data;
  // u32 x4_dataLen;
  TToken<CModel> m_selfToken; /* DO NOT LOCK! */
  zeus::CAABox m_aabb;
  u32 m_flags;
  std::vector<CBooSurface> x8_surfaces;
  std::vector<SShader> x18_matSets;
  std::unique_ptr<CBooModel> x28_modelInst;
  // CModel* x30_next = nullptr;
  // CModel* x34_prev = nullptr;
  int x38_lastFrame;

  /* metaforce addition: boo2! */
  hsh::owner<hsh::vertex_buffer_typeless> m_staticVbo;
  hecl::HMDLMeta m_hmdlMeta;
  std::unique_ptr<uint8_t[]> m_dynamicVertexData;
  hsh::owner<hsh::index_buffer<u32>> m_ibo;

public:
  using MaterialSet = DataSpec::DNAMP1::HMDLMaterialSet;

  CModel(std::unique_ptr<u8[]>&& in, u32 dataLen, IObjectStore* store, CObjectReference* selfRef);
  void DrawSortedParts(const CModelFlags& flags) const;
  void DrawUnsortedParts(const CModelFlags& flags) const;
  void Draw(const CModelFlags& flags) const;
  bool IsLoaded(int shaderIdx) const;
  void Touch(int shaderIdx) { x28_modelInst->Touch(shaderIdx); }

  const zeus::CAABox& GetAABB() const { return m_aabb; }
  CBooModel& GetInstance() { return *x28_modelInst; }
  const CBooModel& GetInstance() const { return *x28_modelInst; }
  std::unique_ptr<CBooModel> MakeNewInstance(int shaderIdx, int subInsts, bool lockParent = true);
  void UpdateLastFrame() const { const_cast<CModel&>(*this).x38_lastFrame = CGraphics::GetFrameCounter(); }
  u32 GetNumMaterialSets() const { return x18_matSets.size(); }

  zeus::CVector3f GetPoolVertex(size_t idx) const;
  zeus::CVector3f GetPoolNormal(size_t idx) const;
  void ApplyVerticesCPU(hsh::dynamic_owner<hsh::vertex_buffer_typeless>& vertBuf,
                        const std::vector<std::pair<zeus::CVector3f, zeus::CVector3f>>& vn) const;
  void RestoreVerticesCPU(hsh::dynamic_owner<hsh::vertex_buffer_typeless>& vertBuf) const;

  void _WarmupShaders();
  static void WarmupShaders(const SObjectTag& cmdlTag);

  const uint8_t* GetDynamicVertexData() const { return m_dynamicVertexData.get(); }
  const hecl::HMDLMeta& GetHMDLMeta() const { return m_hmdlMeta; }
};

CFactoryFnReturn FModelFactory(const metaforce::SObjectTag& tag, std::unique_ptr<u8[]>&& in, u32 len,
                               const metaforce::CVParamTransfer& vparms, CObjectReference* selfRef);

template <typename F>
constexpr auto MapVertData(const hecl::HMDLMeta& meta, F&& Func) {
#define WEIGHT_COUNT(uvs, weights)                                                                                     \
  case weights: {                                                                                                      \
    using VertData = CModelShaders::VertData<0, uvs, weights>;                                                         \
    assert(sizeof(VertData) == meta.vertStride && "Vert data stride mismatch");                                        \
    return Func.template operator()<VertData>();                                                                         \
    break;                                                                                                             \
  }
#define UV_COUNT(uvs)                                                                                                  \
  case uvs:                                                                                                            \
    switch (meta.weightCount) {                                                                                        \
      WEIGHT_COUNT(uvs, 0);                                                                                            \
      WEIGHT_COUNT(uvs, 1);                                                                                            \
      WEIGHT_COUNT(uvs, 2);                                                                                            \
      WEIGHT_COUNT(uvs, 3);                                                                                            \
      WEIGHT_COUNT(uvs, 4);                                                                                            \
      WEIGHT_COUNT(uvs, 5);                                                                                            \
    default:                                                                                                           \
      assert(false && "Unhandled weight count");                                                                       \
      break;                                                                                                           \
    }                                                                                                                  \
    break;
  switch (meta.uvCount) {
    UV_COUNT(0)
    UV_COUNT(1)
    UV_COUNT(2)
    UV_COUNT(3)
    UV_COUNT(4)
    UV_COUNT(5)
#undef UV_COUNT
#undef WEIGHT_COUNT
  default:
    assert(false && "Unhandled UV count");
  }
  // fallback
  return Func.template operator()<CModelShaders::VertData<0, 0, 0>>();
}
} // namespace metaforce
