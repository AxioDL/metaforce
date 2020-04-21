#pragma once

#include <functional>
#include <list>
#include <optional>
#include <unordered_map>
#include <utility>
#include <vector>

#include "Runtime/CRandom16.hpp"
#include "Runtime/rstl.hpp"
#include "Runtime/Graphics/CDrawable.hpp"
#include "Runtime/Graphics/CDrawablePlaneObject.hpp"
#include "Runtime/Graphics/CPVSVisSet.hpp"
#include "Runtime/Graphics/CTexture.hpp"
#include "Runtime/Graphics/IRenderer.hpp"
#include "Runtime/Graphics/Shaders/CFogVolumeFilter.hpp"
#include "Runtime/Graphics/Shaders/CFogVolumePlaneShader.hpp"
#include "Runtime/Graphics/Shaders/CPhazonSuitFilter.hpp"
#include "Runtime/Graphics/Shaders/CSpaceWarpFilter.hpp"
#include "Runtime/Graphics/Shaders/CThermalColdFilter.hpp"
#include "Runtime/Graphics/Shaders/CThermalHotFilter.hpp"
#include "Runtime/World/CGameArea.hpp"

#include <zeus/CAABox.hpp>
#include <zeus/CColor.hpp>
#include <zeus/CFrustum.hpp>
#include <zeus/CPlane.hpp>
#include <zeus/CRectangle.hpp>
#include <zeus/CTransform.hpp>
#include <zeus/CVector3f.hpp>

namespace urde {
class CBooModel;
class CMemorySys;
class CParticleGen;
class CTexture;
class IFactory;
class IObjectStore;

class Buckets;

enum class EWorldShadowMode {
  None,
  WorldOnActorShadow,
  BallOnWorldShadow,
  BallOnWorldIds,
  MAX
};

class CBooRenderer final : public IRenderer {
  friend class CBooModel;
  friend class CGameArea;
  friend class CModel;
  friend class CMorphBallShadow;
  friend class CWorldTransManager;

  struct CAreaListItem {
    const std::vector<CMetroidModelInstance>* x0_geometry;
    const CAreaRenderOctTree* x4_octTree;
    /* originally auto_ptrs of vectors */
    std::unordered_map<CAssetId, TCachedToken<CTexture>> x8_textures;
    std::vector<CBooModel*> x10_models;
    int x18_areaIdx;
    /* Per-area octree-word major, light bits minor */
    std::vector<u32> x1c_lightOctreeWords;
    const SShader* m_shaderSet;

    CAreaListItem(const std::vector<CMetroidModelInstance>* geom, const CAreaRenderOctTree* octTree,
                  std::unordered_map<CAssetId, TCachedToken<CTexture>>&& textures, std::vector<CBooModel*>&& models,
                  int areaIdx, const SShader* shaderSet);
    ~CAreaListItem();
  };

  struct CFogVolumeListItem {
    zeus::CTransform x0_transform;
    zeus::CColor x30_color;
    zeus::CAABox x34_aabb;
    TLockedToken<CModel> x4c_model;
    // bool x58_b; Optional for model token
    const CSkinnedModel* x5c_skinnedModel = nullptr;
    CFogVolumeListItem(const zeus::CTransform& xf, const zeus::CColor& color, const zeus::CAABox& aabb,
                       const TLockedToken<CModel>* model, const CSkinnedModel* sModel)
    : x0_transform(xf), x30_color(color), x34_aabb(aabb), x5c_skinnedModel(sModel) {
      if (model)
        x4c_model = *model;
    }
  };

  IFactory& x8_factory;
  IObjectStore& xc_store;
  TLockedToken<CTexture> m_staticEntropy;
  // CFont x10_fnt;
  u32 x18_ = 0;
  std::list<CAreaListItem> x1c_areaListItems;
  zeus::CFrustum x44_frustumPlanes;

  TDrawableCallback xa8_drawableCallback;
  void* xac_callbackContext;

  zeus::CPlane xb0_viewPlane = {0.f, 1.f, 0.f, 0.f};

  enum class EPVSMode { Mask, PVS, PVSAndMask } xc4_pvsMode = EPVSMode::Mask;
  std::optional<CPVSVisSet> xc8_pvs;
  u32 xe0_pvsAreaIdx = 0;

  // boo::ITextureS* xe4_blackTex = nullptr;
  bool xee_24_ : 1 = true;

  boo::ObjToken<boo::ITexture> m_clearTexture;
  boo::ObjToken<boo::ITexture> m_blackTexture;
  boo::ObjToken<boo::ITexture> m_whiteTexture;
  std::unordered_map<zeus::CColor, boo::ObjToken<boo::ITexture>> m_colorTextures;

  boo::ObjToken<boo::ITextureR> x14c_reflectionTex;
  // boo::ITextureS* x150_mirrorRamp = nullptr;
  boo::ObjToken<boo::ITextureS> x1b8_fogVolumeRamp;
  boo::ObjToken<boo::ITextureS> x220_sphereRamp;
  TLockedToken<CTexture> m_thermoPaletteTex;
  boo::ObjToken<boo::ITexture> x288_thermoPalette;
  TLockedToken<CTexture> m_ballFadeTex;
  boo::ObjToken<boo::ITexture> m_ballFade;
  boo::ObjToken<boo::ITextureR> m_ballShadowId;
  boo::ObjToken<boo::IGraphicsBufferS> m_scanLinesEvenVBO;
  boo::ObjToken<boo::IGraphicsBufferS> m_scanLinesOddVBO;
  int m_ballShadowIdW = 64;
  int m_ballShadowIdH = 64;

  CRandom16 x2a8_thermalRand;
  std::list<CFogVolumeListItem> x2ac_fogVolumes;
  std::list<CFogVolumePlaneShader> m_fogVolumePlaneShaders;
  std::list<CFogVolumePlaneShader>::iterator m_nextFogVolumePlaneShader;
  std::list<CFogVolumeFilter> m_fogVolumeFilters;
  std::list<CFogVolumeFilter>::iterator m_nextFogVolumeFilter;
  std::list<std::pair<zeus::CVector3f, float>> x2c4_spaceWarps;
  u32 x2dc_reflectionAge = 2;
  zeus::CColor x2e0_ = zeus::skWhite;
  zeus::CVector3f x2e4_ = {0.f, 1.f, 0.f};

  CSpaceWarpFilter m_spaceWarpFilter;

  float x2f0_thermalVisorLevel;
  zeus::CColor x2f4_thermColor;
  float x2f8_thermColdScale = 0.f;
  zeus::CColor x2fc_tevReg1Color = {1.f, 0.f, 1.f, 1.f};
  std::optional<CThermalColdFilter> m_thermColdFilter;
  std::optional<CThermalHotFilter> m_thermHotFilter;

  std::vector<CLight> x300_dynamicLights;

  u32 x310_phazonSuitMaskCountdown = 0;
  // std::unique_ptr<CTexture> x314_phazonSuitMask;
  CPhazonSuitFilter m_phazonSuitFilter;

  bool x318_24_refectionDirty : 1 = false;
  bool x318_25_drawWireframe : 1 = false;
  bool x318_26_requestRGBA6 : 1 = false;
  bool x318_27_currentRGBA6 : 1 = false;
  bool x318_28_disableFog : 1 = false;
  bool x318_29_thermalVisor : 1 = false;
  bool x318_30_inAreaDraw : 1 = false;
  bool x318_31_persistRGBA6 : 1 = false;
  bool m_thermalHotPass : 1 = false;

  void GenerateFogVolumeRampTex(boo::IGraphicsDataFactory::Context& ctx);
  void GenerateSphereRampTex(boo::IGraphicsDataFactory::Context& ctx);
  void GenerateScanLinesVBO(boo::IGraphicsDataFactory::Context& ctx);
  void LoadThermoPalette();
  void LoadBallFade();

  void ActivateLightsForModel(CAreaListItem* item, CBooModel& model);
  void RenderBucketItems(CAreaListItem* item);
  void HandleUnsortedModel(CAreaListItem* item, CBooModel& model, const CModelFlags& flags);
  static void CalcDrawFogFan(const zeus::CPlane* planes, size_t numPlanes, const zeus::CVector3f* verts,
                             size_t numVerts, size_t iteration, size_t level, CFogVolumePlaneShader& fogVol);
  static void DrawFogSlices(const zeus::CPlane* planes, size_t numPlanes, size_t iteration,
                            const zeus::CVector3f& center, float delta, CFogVolumePlaneShader& fogVol);
  static void RenderFogVolumeModel(const zeus::CAABox& aabb, const CModel* model, const zeus::CTransform& modelMtx,
                                   const zeus::CTransform& viewMtx, const CSkinnedModel* sModel, int pass,
                                   CFogVolumePlaneShader* fvs);
  void SetupRendererStates() const;

  void ReallyDrawPhazonSuitIndirectEffect(const zeus::CColor& vertColor, /*const CTexture& maskTex,*/
                                          const CTexture& indTex, const zeus::CColor& modColor, float scale, float offX,
                                          float offY);
  void ReallyDrawPhazonSuitEffect(const zeus::CColor& modColor /*, const CTexture& maskTex*/);
  void DoPhazonSuitIndirectAlphaBlur(float blurRadius /*, float f2*/, const TLockedToken<CTexture>& indTex);

public:
  CBooRenderer(IObjectStore& store, IFactory& resFac);
  ~CBooRenderer() override;

  void AddWorldSurfaces(CBooModel& model);

  std::list<CAreaListItem>::iterator FindStaticGeometry(const std::vector<CMetroidModelInstance>*);
  void AddStaticGeometry(const std::vector<CMetroidModelInstance>* geometry, const CAreaRenderOctTree* octTree,
                         int areaIdx, const SShader* shaderSet) override;
  void EnablePVS(const CPVSVisSet& set, u32 areaIdx) override;
  void DisablePVS() override;
  void UpdateAreaUniforms(int areaIdx, EWorldShadowMode shadowMode = EWorldShadowMode::None,
                          bool activateLights = true, int cubeFace = -1, const CModelFlags* ballShadowFlags = nullptr);
  void RemoveStaticGeometry(const std::vector<CMetroidModelInstance>*) override;
  void DrawAreaGeometry(int areaIdx, int mask, int targetMask) override;
  void DrawUnsortedGeometry(int areaIdx, int mask, int targetMask, bool shadowRender = false) override;
  void DrawSortedGeometry(int areaIdx, int mask, int targetMask) override;
  void DrawStaticGeometry(int areaIdx, int mask, int targetMask) override;
  void DrawModelFlat(const CModel& model, const CModelFlags& flags, bool unsortedOnly) override;
  void PostRenderFogs() override;
  void SetModelMatrix(const zeus::CTransform& xf) override;
  void AddParticleGen(CParticleGen& gen) override;
  void AddParticleGen(CParticleGen& gen, const zeus::CVector3f& pos, const zeus::CAABox& bounds) override;
  void AddPlaneObject(void* obj, const zeus::CAABox& aabb, const zeus::CPlane& plane, int type) override;
  void AddDrawable(void* obj, const zeus::CVector3f& pos, const zeus::CAABox& aabb, int mode,
                   EDrawableSorting sorting) override;
  void SetDrawableCallback(TDrawableCallback cb, void* ctx) override;
  void SetWorldViewpoint(const zeus::CTransform& xf) override;
  void SetPerspective(float fovy, float width, float height, float znear, float zfar) override;
  void SetPerspective(float fovy, float aspect, float znear, float zfar) override;
  std::pair<zeus::CVector2f, zeus::CVector2f> SetViewportOrtho(bool centered, float znear, float zfar) override;
  void SetClippingPlanes(const zeus::CFrustum& frustum) override;
  void SetViewport(int left, int bottom, int width, int height) override;
  // void SetDepthReadWrite(bool, bool);
  // void SetBlendMode_AdditiveAlpha();
  // void SetBlendMode_AlphaBlended();
  // void SetBlendMode_NoColorWrite();
  // void SetBlendMode_ColorMultiply();
  // void SetBlendMode_InvertDst();
  // void SetBlendMode_InvertSrc();
  // void SetBlendMode_Replace();
  // void SetBlendMode_AdditiveDestColor();
  void SetDebugOption(EDebugOption, int) override;
  void BeginScene() override;
  void EndScene() override;
  // void BeginPrimitive(EPrimitiveType, int);
  // void BeginLines(int);
  // void BeginLineStrip(int);
  // void BeginTriangles(int);
  // void BeginTriangleStrip(int);
  // void BeginTriangleFan(int);
  // void PrimVertex(const zeus::CVector3f&);
  // void PrimNormal(const zeus::CVector3f&);
  // void PrimColor(float, float, float, float);
  // void PrimColor(const zeus::CColor&);
  // void EndPrimitive();
  void SetAmbientColor(const zeus::CColor& color) override;
  void DrawString(const char* string, int, int) override;
  u32 GetFPS() override;
  void CacheReflection(TReflectionCallback cb, void* ctx, bool clearAfter) override;
  void DrawSpaceWarp(const zeus::CVector3f& pt, float strength) override;
  void DrawThermalModel(const CModel& model, const zeus::CColor& multCol, const zeus::CColor& addCol) override;
  void DrawXRayOutline(const zeus::CAABox&) override;
  void SetWireframeFlags(int flags) override;
  void SetWorldFog(ERglFogMode mode, float startz, float endz, const zeus::CColor& color) override;
  void RenderFogVolume(const zeus::CColor& color, const zeus::CAABox& aabb, const TLockedToken<CModel>* model,
                       const CSkinnedModel* sModel) override;
  void SetThermal(bool thermal, float level, const zeus::CColor& color) override;
  void SetThermalColdScale(float scale) override;
  void DoThermalBlendCold() override;
  void DoThermalBlendHot() override;
  u32 GetStaticWorldDataSize() override;
  void PrepareDynamicLights(const std::vector<CLight>& lights) override;
  void SetGXRegister1Color(const zeus::CColor& color) override;
  void SetWorldLightFadeLevel(float level) override;
  void SetWorldLightMultiplyColor(const zeus::CColor& col) override;
  void DrawPhazonSuitIndirectEffect(const zeus::CColor& nonIndirectMod, const TLockedToken<CTexture>& indTex,
                                    const zeus::CColor& indirectMod, float blurRadius, float indScale, float indOffX,
                                    float indOffY);
  void AllocatePhazonSuitMaskTexture();

  void ReallyRenderFogVolume(const zeus::CColor& color, const zeus::CAABox& aabb, const CModel* model,
                             const CSkinnedModel* sModel);

  const boo::ObjToken<boo::ITexture>& GetThermoPalette() const { return x288_thermoPalette; }
  const boo::ObjToken<boo::ITextureS>& GetFogRampTex() const { return x1b8_fogVolumeRamp; }
  const boo::ObjToken<boo::ITexture>& GetRandomStaticEntropyTex() const { return m_staticEntropy->GetBooTexture(); }
  const boo::ObjToken<boo::IGraphicsBufferS>& GetScanLinesEvenVBO() const { return m_scanLinesEvenVBO; }
  const boo::ObjToken<boo::IGraphicsBufferS>& GetScanLinesOddVBO() const { return m_scanLinesOddVBO; }

  const boo::ObjToken<boo::ITexture>& GetClearTexture() const { return m_clearTexture; }
  const boo::ObjToken<boo::ITexture>& GetBlackTexture() const { return m_blackTexture; }
  const boo::ObjToken<boo::ITexture>& GetWhiteTexture() const { return m_whiteTexture; }

  boo::ObjToken<boo::ITexture> GetColorTexture(const zeus::CColor& color);

  static void BindMainDrawTarget() { CGraphics::g_BooMainCommandQueue->setRenderTarget(CGraphics::g_SpareTexture); }
  void BindReflectionDrawTarget() { CGraphics::g_BooMainCommandQueue->setRenderTarget(x14c_reflectionTex); }
  void BindBallShadowIdTarget() {
    CGraphics::g_BooMainCommandQueue->setRenderTarget(m_ballShadowId);
    SetViewport(0, 0, m_ballShadowIdW, m_ballShadowIdH);
  }
  void ResolveBallShadowIdTarget() {
    CGraphics::g_BooMainCommandQueue->resolveBindTexture(
        m_ballShadowId, boo::SWindowRect(0, 0, m_ballShadowIdW, m_ballShadowIdH), false, 0, true, false);
  }

  void FindOverlappingWorldModels(std::vector<u32>& modelBits, const zeus::CAABox& aabb) const;
  int DrawOverlappingWorldModelIDs(int alphaVal, const std::vector<u32>& modelBits, const zeus::CAABox& aabb);
  void DrawOverlappingWorldModelShadows(int alphaVal, const std::vector<u32>& modelBits, const zeus::CAABox& aabb,
                                        float alpha);

  bool IsThermalVisorActive() const { return x318_29_thermalVisor; }
  bool IsThermalVisorHotPass() const { return m_thermalHotPass; }
};

} // namespace urde
