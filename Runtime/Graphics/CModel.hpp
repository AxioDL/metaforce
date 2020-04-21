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

#include <boo/graphicsdev/IGraphicsDataFactory.hpp>
#include <hecl/HMDLMeta.hpp>
#include <zeus/CAABox.hpp>
#include <zeus/CColor.hpp>

namespace urde {
class CLight;
class CModel;
class CPoseAsTransforms;
class CSkinRules;
class IObjectStore;

struct CModelFlags {
  u8 x0_blendMode = 0; /* 2: add color, >6: additive, >4: blend, else opaque */
  u8 x1_matSetIdx = 0;
  EExtendedShader m_extendedShader = EExtendedShader::Lighting;
  bool m_noCull = false;
  bool m_noZWrite = false;
  bool m_depthGreater = false;
  u16 x2_flags = 0;      /* Flags */
  zeus::CColor x4_color; /* Set into kcolor slot specified by material */
  zeus::CColor addColor = zeus::skClear;
  zeus::CAABox mbShadowBox;

  constexpr CModelFlags() = default;
  constexpr CModelFlags(u8 blendMode, u8 shadIdx, u16 flags, const zeus::CColor& col)
  : x0_blendMode(blendMode), x1_matSetIdx(shadIdx), x2_flags(flags), x4_color(col) {
    /* Blend mode will override this if the surface's original material is opaque */
    m_noZWrite = (x2_flags & 0x2) == 0;
    m_depthGreater = (x2_flags & 0x8) != 0;
  }

  /* Flags
      0x1: depth equal
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

/* urde addition: doesn't require hacky stashing of
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
  mutable std::vector<boo::ObjToken<boo::IGraphicsBufferD>> m_sharedBuffer;
  size_t m_geomBufferSize = 0;
  size_t m_skinBankCount = 0;
  size_t m_weightVecCount = 0;

  std::vector<size_t> m_skinOffs;
  std::vector<size_t> m_skinSizes;

  std::vector<size_t> m_uvOffs;
  std::vector<size_t> m_uvSizes;

  GeometryUniformLayout(const CModel* model, const MaterialSet* matSet);
  void Update(const CModelFlags& flags, const CSkinRules* cskr, const CPoseAsTransforms* pose,
              const MaterialSet* matSet, const boo::ObjToken<boo::IGraphicsBufferD>& buf,
              const CBooModel* parent) const;

  void ReserveSharedBuffers(boo::IGraphicsDataFactory::Context& ctx, int size);
  boo::ObjToken<boo::IGraphicsBufferD> GetSharedBuffer(int idx) const;
};

struct SShader {
  std::unordered_map<CAssetId, TCachedToken<CTexture>> x0_textures;
  std::unordered_map<int, CModelShaders::ShaderPipelines> m_shaders;
  MaterialSet m_matSet;
  std::optional<GeometryUniformLayout> m_geomLayout;
  int m_matSetIdx;
  explicit SShader(int idx) : m_matSetIdx(idx) {
    x0_textures.clear();
    m_shaders.clear();
  }
  void InitializeLayout(const CModel* model) { m_geomLayout.emplace(model, &m_matSet); }
  void UnlockTextures();
  CModelShaders::ShaderPipelines BuildShader(const hecl::HMDLMeta& meta, const MaterialSet::Material& mat);
  void BuildShaders(const hecl::HMDLMeta& meta, std::unordered_map<int, CModelShaders::ShaderPipelines>& shaders);
  void BuildShaders(const hecl::HMDLMeta& meta) { BuildShaders(meta, m_shaders); }
};

class CBooModel {
  friend class CBooRenderer;
  friend class CGameArea;
  friend class CMetroidModelInstance;
  friend class CModel;
  friend class CSkinnedModel;
  friend struct GeometryUniformLayout;

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
  int m_matSetIdx = -1;
  const std::unordered_map<int, CModelShaders::ShaderPipelines>* m_pipelines;
  std::unordered_map<CAssetId, TCachedToken<CTexture>> x1c_textures;
  zeus::CAABox x20_aabb;
  CBooSurface* x38_firstUnsortedSurface = nullptr;
  CBooSurface* x3c_firstSortedSurface = nullptr;
  bool x40_24_texturesLoaded : 1 = false;
  bool x40_25_modelVisible : 1 = false;
  u8 x41_mask;
  u32 x44_areaInstanceIdx = UINT32_MAX;

  struct UVAnimationBuffer {
    static void ProcessAnimation(u8*& bufOut, const MaterialSet::Material::PASS& anim);
    static void PadOutBuffer(u8*& bufStart, u8*& bufOut);
    static void Update(u8*& bufOut, const MaterialSet* matSet, const CModelFlags& flags, const CBooModel* parent);
  };

  CModelShaders::LightingUniform m_lightingData;

  /* urde addition: boo! */
  size_t m_uniformDataSize = 0;
  struct ModelInstance {
    boo::ObjToken<boo::IGraphicsBufferD> m_geomUniformBuffer;
    boo::ObjToken<boo::IGraphicsBufferD> m_uniformBuffer;
    std::vector<std::vector<boo::ObjToken<boo::IShaderDataBinding>>> m_shaderDataBindings;
    boo::ObjToken<boo::IGraphicsBufferD> m_dynamicVbo;

    boo::ObjToken<boo::IGraphicsBuffer> GetBooVBO(const CBooModel& model, boo::IGraphicsDataFactory::Context& ctx);
  };
  std::vector<ModelInstance> m_instances;
  ModelInstance m_ballShadowInstance;

  boo::ObjToken<boo::IGraphicsBufferS> m_staticVbo;
  boo::ObjToken<boo::IGraphicsBufferS> m_staticIbo;

  boo::ObjToken<boo::ITexture> m_lastDrawnShadowMap;
  boo::ObjToken<boo::ITexture> m_lastDrawnOneTexture;
  boo::ObjToken<boo::ITextureCubeR> m_lastDrawnReflectionCube;

  ModelInstance* PushNewModelInstance(int sharedLayoutBuf = -1);
  void DrawAlphaSurfaces(const CModelFlags& flags) const;
  void DrawNormalSurfaces(const CModelFlags& flags) const;
  void DrawSurfaces(const CModelFlags& flags) const;
  void DrawSurface(const CBooSurface& surf, const CModelFlags& flags) const;
  void WarmupDrawSurfaces() const;
  void WarmupDrawSurface(const CBooSurface& surf) const;

  static zeus::CVector3f g_PlayerPosition;
  static float g_ModSeconds;
  static float g_TransformedTime;
  static float g_TransformedTime2;
  static CBooModel* g_LastModelCached;

  static bool g_DummyTextures;
  static bool g_RenderModelBlack;

public:
  ~CBooModel();
  CBooModel(TToken<CModel>& token, CModel* parent, std::vector<CBooSurface>* surfaces, SShader& shader,
            const boo::ObjToken<boo::IGraphicsBufferS>& vbo, const boo::ObjToken<boo::IGraphicsBufferS>& ibo,
            const zeus::CAABox& aabb, u8 renderMask, int numInsts);

  static void MakeTexturesFromMats(const MaterialSet& matSet,
                                   std::unordered_map<CAssetId, TCachedToken<CTexture>>& toksOut,
                                   IObjectStore& store);
  void MakeTexturesFromMats(std::unordered_map<CAssetId, TCachedToken<CTexture>>& toksOut, IObjectStore& store);

  bool IsOpaque() const { return x3c_firstSortedSurface == nullptr; }
  void ActivateLights(const std::vector<CLight>& lights);
  void SetAmbientColor(const zeus::CColor& color) { m_lightingData.ambient = color; }
  void DisableAllLights();
  void RemapMaterialData(SShader& shader);
  void RemapMaterialData(SShader& shader, const std::unordered_map<int, CModelShaders::ShaderPipelines>& pipelines);
  bool TryLockTextures();
  void UnlockTextures();
  void SyncLoadTextures();
  void Touch(int shaderIdx);
  void VerifyCurrentShader(int shaderIdx);
  boo::ObjToken<boo::IGraphicsBufferD> UpdateUniformData(const CModelFlags& flags, const CSkinRules* cskr,
                                                         const CPoseAsTransforms* pose, int sharedLayoutBuf = -1);
  void DrawAlpha(const CModelFlags& flags, const CSkinRules* cskr, const CPoseAsTransforms* pose);
  void DrawNormal(const CModelFlags& flags, const CSkinRules* cskr, const CPoseAsTransforms* pose);
  void Draw(const CModelFlags& flags, const CSkinRules* cskr, const CPoseAsTransforms* pose);
  void DrawFlat(ESurfaceSelection sel, EExtendedShader extendedIdx) const;

  void LockParent() { m_modelTok.Lock(); }
  void UnlockParent() { m_modelTok.Unlock(); }

  const MaterialSet::Material& GetMaterialByIndex(int idx) const { return x4_matSet->materials.at(idx); }

  void ClearUniformCounter() { m_uniUpdateCount = 0; }
  static void ClearModelUniformCounters();

  static bool g_DrawingOccluders;
  static void SetDrawingOccluders(bool occ) { g_DrawingOccluders = occ; }

  static void SetNewPlayerPositionAndTime(const zeus::CVector3f& pos);

  static zeus::CVector3f g_ReflectViewPos;
  static void KillCachedViewDepState();
  static void EnsureViewDepStateCached(const CBooModel& model, const CBooSurface* surf, zeus::CMatrix4f* mtxsOut,
                                       float& alphaOut);

  static boo::ObjToken<boo::ITexture> g_shadowMap;
  static zeus::CTransform g_shadowTexXf;
  static void EnableShadowMaps(const boo::ObjToken<boo::ITexture>& map, const zeus::CTransform& texXf);
  static void DisableShadowMaps();

  static boo::ObjToken<boo::ITexture> g_disintegrateTexture;
  static void SetDisintegrateTexture(const boo::ObjToken<boo::ITexture>& map) { g_disintegrateTexture = map; }

  static boo::ObjToken<boo::ITextureCubeR> g_reflectionCube;
  static void SetReflectionCube(const boo::ObjToken<boo::ITextureCubeR>& map) { g_reflectionCube = map; }

  static void SetDummyTextures(bool b) { g_DummyTextures = b; }
  static void SetRenderModelBlack(bool b) { g_RenderModelBlack = b; }

  static void Shutdown();

  const zeus::CAABox& GetAABB() const { return x20_aabb; }
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

  /* urde addition: boo! */
  boo::ObjToken<boo::IGraphicsBufferS> m_staticVbo;
  hecl::HMDLMeta m_hmdlMeta;
  std::unique_ptr<uint8_t[]> m_dynamicVertexData;
  boo::ObjToken<boo::IGraphicsBufferS> m_ibo;

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

  size_t GetPoolVertexOffset(size_t idx) const;
  zeus::CVector3f GetPoolVertex(size_t idx) const;
  size_t GetPoolNormalOffset(size_t idx) const;
  zeus::CVector3f GetPoolNormal(size_t idx) const;
  void ApplyVerticesCPU(const boo::ObjToken<boo::IGraphicsBufferD>& vertBuf,
                        const std::vector<std::pair<zeus::CVector3f, zeus::CVector3f>>& vn) const;
  void RestoreVerticesCPU(const boo::ObjToken<boo::IGraphicsBufferD>& vertBuf) const;

  void _WarmupShaders();
  static void WarmupShaders(const SObjectTag& cmdlTag);
};

CFactoryFnReturn FModelFactory(const urde::SObjectTag& tag, std::unique_ptr<u8[]>&& in, u32 len,
                               const urde::CVParamTransfer& vparms, CObjectReference* selfRef);

} // namespace urde
