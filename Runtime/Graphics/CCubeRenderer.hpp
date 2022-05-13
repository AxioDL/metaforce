#pragma once

#include "Runtime/Graphics/CCubeModel.hpp"
#include "Runtime/Graphics/CPVSVisSet.hpp"
#include "Runtime/Graphics/CTexture.hpp"
#include "Runtime/Graphics/IRenderer.hpp"
#include "Runtime/CRandom16.hpp"
#include "Runtime/Graphics/CFont.hpp"

#include <list>

namespace metaforce {
class IObjectStore;
class IFactory;

class CCubeRenderer final : public IRenderer {
  // TODO function for controlling x318_26_requestRGBA6
  // then these can be removed
  friend class CMorphBallShadow;
  friend class CWorldTransManager;

  struct CAreaListItem {
    const std::vector<CMetroidModelInstance>* x0_geometry;
    const CAreaRenderOctTree* x4_octTree;
    /* originally auto_ptrs of vectors */
    std::unique_ptr<std::vector<TCachedToken<CTexture>>> x8_textures;
    std::unique_ptr<std::vector<std::unique_ptr<CCubeModel>>> x10_models;
    s32 x18_areaIdx;
    /* Per-area octree-word major, light bits minor */
    std::vector<u32> x1c_lightOctreeWords;

    CAreaListItem(const std::vector<CMetroidModelInstance>* geom, const CAreaRenderOctTree* octTree,
                  std::unique_ptr<std::vector<TCachedToken<CTexture>>>&& textures,
                  std::unique_ptr<std::vector<std::unique_ptr<CCubeModel>>>&& models, s32 areaIdx);
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

private:
  IFactory& x8_factory;
  IObjectStore& xc_store;
  CFont x10_font{1.f};
  u32 x18_primVertCount = 0;
  std::list<CAreaListItem> x1c_areaListItems;
  // TODO x34...x40
  zeus::CFrustum x44_frustumPlanes; // {zeus::skIdentityMatrix4f, 1.5707964f, 1.f, 1.f, false, 100.f}
  TDrawableCallback xa8_drawableCallback = nullptr;
  void* xac_drawableCallbackUserData = nullptr;
  zeus::CPlane xb0_viewPlane{0.f, 1.f, 0.f, 0.f};
  enum class EPVSMode : u8 { Mask, PVS, PVSAndMask } xc0_pvsMode = EPVSMode::Mask;
  std::optional<CPVSVisSet> xc8_pvs;
  bool xdc_{};
  u32 xe0_pvsAreaIdx = UINT32_MAX;
  CTexture xe4_blackTex{ETexelFormat::RGB565, 4, 4, 1, "Black Texture"};
  std::unique_ptr<CTexture> x14c_reflectionTex;
  CTexture x150_reflectionTex{ETexelFormat::IA8, 32, 32, 1, "Reflection Texture"};
  CTexture x1b8_fogVolumeRamp{ETexelFormat::I8, 256, 256, 1, "Fog Volume Ramp Texture"};
  CTexture x220_sphereRamp{ETexelFormat::I8, 32, 32, 1, "Sphere Ramp Texture"};
  CGraphicsPalette x288_thermoPalette{EPaletteFormat::RGB565, 16};
  CRandom16 x2a8_thermalRand{20};
  std::list<CFogVolumeListItem> x2ac_fogVolumes;
  std::list<std::pair<zeus::CVector3f, float>> x2c4_spaceWarps;
  u32 x2dc_reflectionAge = 2;
  zeus::CColor x2e0_primColor = zeus::skWhite;
  zeus::CVector3f x2e4_primNormal = zeus::skForward;
  float x2f0_thermalVisorLevel = 1.f;
  zeus::CColor x2f4_thermColor{1.f, 0.f, 1.f, 1.f};
  float x2f8_thermColdScale = 0.f; // ??? byte in code
  zeus::CColor x2fc_tevReg1Color{1.f, 0.f, 1.f, 1.f};
  std::vector<CLight> x300_dynamicLights;
  u32 x310_phazonSuitMaskCountdown = 0;
  std::unique_ptr<CTexture> x314_phazonSuitMask;
  bool x318_24_refectionDirty : 1 = false;
  bool x318_25_drawWireframe : 1 = false;
  bool x318_26_requestRGBA6 : 1 = false;
  bool x318_27_currentRGBA6 : 1 = false;
  bool x318_28_disableFog : 1 = false;
  bool x318_29_thermalVisor : 1 = false;
  bool x318_30_inAreaDraw : 1 = false;
  bool x318_31_persistRGBA6 : 1 = false;

  CTexture m_thermalRandomStatic{ETexelFormat::IA4, 640, 448, 1, "Thermal Random Static"};

  void GenerateReflectionTex();
  void GenerateFogVolumeRampTex();
  void GenerateSphereRampTex();
  void LoadThermoPalette();

  void ReallyDrawPhazonSuitIndirectEffect(const zeus::CColor& vertColor, CTexture& maskTex, CTexture& indTex,
                                          const zeus::CColor& modColor, float scale, float offX, float offY);
  void ReallyDrawPhazonSuitEffect(const zeus::CColor& modColor, CTexture& maskTex);
  void DoPhazonSuitIndirectAlphaBlur(float blurRadius, float f2, const TLockedToken<CTexture>& indTex);

public:
  CCubeRenderer(IObjectStore& store, IFactory& resFac);
  ~CCubeRenderer() override;

  void AddWorldSurfaces(CCubeModel& model);
  void AddStaticGeometry(const std::vector<CMetroidModelInstance>* geometry, const CAreaRenderOctTree* octTree,
                         s32 areaIdx) override;
  void EnablePVS(const CPVSVisSet& set, u32 areaIdx) override;
  void DisablePVS() override;
  void RemoveStaticGeometry(const std::vector<CMetroidModelInstance>* geometry) override;
  void DrawUnsortedGeometry(s32 areaIdx, s32 mask, s32 targetMask) override;
  void DrawSortedGeometry(s32 areaIdx, s32 mask, s32 targetMask) override;
  void DrawStaticGeometry(s32 areaIdx, s32 mask, s32 targetMask) override;
  void DrawAreaGeometry(s32 areaIdx, s32 mask, s32 targetMask) override;
  void PostRenderFogs() override;
  void SetModelMatrix(const zeus::CTransform& xf) override;
  void AddParticleGen(CParticleGen& gen) override;
  void AddParticleGen(CParticleGen& gen, const zeus::CVector3f& pos, const zeus::CAABox& bounds) override;
  void AddPlaneObject(void* obj, const zeus::CAABox& aabb, const zeus::CPlane& plane, s32 type) override;
  void AddDrawable(void* obj, const zeus::CVector3f& pos, const zeus::CAABox& aabb, s32 mode,
                   EDrawableSorting sorting) override;
  void SetDrawableCallback(TDrawableCallback cb, void* ctx) override;
  void SetWorldViewpoint(const zeus::CTransform& xf) override;
  void SetPerspective(float fovy, float aspect, float znear, float zfar) override;
  void SetPerspective(float fovy, float width, float height, float znear, float zfar) override;
  std::pair<zeus::CVector2f, zeus::CVector2f> SetViewportOrtho(bool centered, float znear, float zfar) override;
  void SetClippingPlanes(const zeus::CFrustum& frustum) override;
  void SetViewport(s32 left, s32 right, s32 width, s32 height) override;
  void SetDepthReadWrite(bool read, bool write) override {
    CGraphics::SetDepthWriteMode(read, ERglEnum::LEqual, write);
  }
  void SetBlendMode_AdditiveAlpha() override {
    CGraphics::SetBlendMode(ERglBlendMode::Blend, ERglBlendFactor::SrcAlpha, ERglBlendFactor::One, ERglLogicOp::Clear);
  }
  void SetBlendMode_AlphaBlended() override {
    CGraphics::SetBlendMode(ERglBlendMode::Blend, ERglBlendFactor::SrcAlpha, ERglBlendFactor::InvSrcAlpha,
                            ERglLogicOp::Clear);
  }
  void SetBlendMode_ColorMultiply() override {
    CGraphics::SetBlendMode(ERglBlendMode::Blend, ERglBlendFactor::Zero, ERglBlendFactor::SrcColor, ERglLogicOp::Clear);
  }
  void SetBlendMode_InvertDst() override {
    CGraphics::SetBlendMode(ERglBlendMode::Blend, ERglBlendFactor::InvSrcColor, ERglBlendFactor::Zero,
                            ERglLogicOp::Clear);
  }
  void SetBlendMode_InvertSrc() override {
    CGraphics::SetBlendMode(ERglBlendMode::Logic, ERglBlendFactor::One, ERglBlendFactor::Zero, ERglLogicOp::InvCopy);
  }
  void SetBlendMode_NoColorWrite() override {
    CGraphics::SetBlendMode(ERglBlendMode::Blend, ERglBlendFactor::Zero, ERglBlendFactor::One, ERglLogicOp::Clear);
  }
  void SetBlendMode_Replace() override {
    CGraphics::SetBlendMode(ERglBlendMode::Blend, ERglBlendFactor::One, ERglBlendFactor::Zero, ERglLogicOp::Clear);
  }
  void SetBlendMode_AdditiveDestColor() override {
    CGraphics::SetBlendMode(ERglBlendMode::Blend, ERglBlendFactor::SrcColor, ERglBlendFactor::One, ERglLogicOp::Clear);
  }
  void BeginScene() override;
  void EndScene() override;
  void SetDebugOption(EDebugOption, s32) override;
  void BeginPrimitive(EPrimitiveType, s32) override;
  void BeginLines(int) override;
  void BeginLineStrip(int) override;
  void BeginTriangles(int) override;
  void BeginTriangleStrip(int) override;
  void BeginTriangleFan(int) override;
  void PrimVertex(const zeus::CVector3f&) override;
  void PrimNormal(const zeus::CVector3f&) override;
  void PrimColor(float, float, float, float) override;
  void PrimColor(const zeus::CColor&) override;
  void EndPrimitive() override;
  void SetAmbientColor(const zeus::CColor& color) override;
  void DrawString(const char* string, s32, s32) override;
  u32 GetFPS() override;
  void CacheReflection(TReflectionCallback cb, void* ctx, bool clearAfter) override;
  void DrawSpaceWarp(const zeus::CVector3f& pt, float strength) override;
  void DrawThermalModel(CModel& model, const zeus::CColor& multCol, const zeus::CColor& addCol,
                        TConstVectorRef positions, TConstVectorRef normals, const CModelFlags& flags) override;
  void DrawModelDisintegrate(CModel& model, CTexture& tex, const zeus::CColor& color, TConstVectorRef positions,
                             TConstVectorRef normals, float t) override;
  void DrawModelFlat(CModel& model, const CModelFlags& flags, bool unsortedOnly, TConstVectorRef positions,
                     TConstVectorRef normals) override;
  void SetWireframeFlags(s32 flags) override;
  void SetWorldFog(ERglFogMode mode, float startz, float endz, const zeus::CColor& color) override;
  void RenderFogVolume(const zeus::CColor& color, const zeus::CAABox& aabb, const TLockedToken<CModel>* model,
                       const CSkinnedModel* sModel) override;
  void SetThermal(bool thermal, float level, const zeus::CColor& color) override;
  void SetThermalColdScale(float scale) override;
  void DoThermalBlendCold() override;
  void DoThermalBlendHot() override;
  u32 GetStaticWorldDataSize() override;
  void SetGXRegister1Color(const zeus::CColor& color) override;
  void SetWorldLightFadeLevel(float level) override;
  void PrepareDynamicLights(const std::vector<CLight>& lights) override;

  // Non-virtual functions
  void SetupRendererStates(bool depthWrite);
  void AllocatePhazonSuitMaskTexture();
  void DrawPhazonSuitIndirectEffect(const zeus::CColor& nonIndirectMod, const TLockedToken<CTexture>& indTex,
                                    const zeus::CColor& indirectMod, float blurRadius, float scale, float offX,
                                    float offY);
  void DrawXRayOutline(const zeus::CAABox& aabb);
  std::list<CAreaListItem>::iterator FindStaticGeometry(const std::vector<CMetroidModelInstance>* geometry);
  void FindOverlappingWorldModels(std::vector<u32>& modelBits, const zeus::CAABox& aabb) const;
  s32 DrawOverlappingWorldModelIDs(s32 alphaVal, const std::vector<u32>& modelBits, const zeus::CAABox& aabb);
  void DrawOverlappingWorldModelShadows(s32 alphaVal, const std::vector<u32>& modelBits, const zeus::CAABox& aabb,
                                        float alpha);
  void RenderBucketItems(const CAreaListItem* lights);
  void DrawRenderBucketsDebug() {}

  void HandleUnsortedModel(CAreaListItem* areaItem, CCubeModel& model, const CModelFlags& flags);
  void HandleUnsortedModelWireframe(CAreaListItem* areaItem, CCubeModel& model);

  void ActivateLightsForModel(const CAreaListItem* areaItem, CCubeModel& model);

  void DoThermalModelDraw(CCubeModel& model, const zeus::CColor& multCol, const zeus::CColor& addCol,
                          TConstVectorRef positions, TConstVectorRef normals, const CModelFlags& flags);

  // Getters
  [[nodiscard]] bool IsInAreaDraw() const { return x318_30_inAreaDraw; }
  [[nodiscard]] bool IsReflectionDirty() const { return x318_24_refectionDirty; }
  void SetReflectionDirty(bool v) { x318_24_refectionDirty = v; }
  [[nodiscard]] bool IsThermalVisorActive() const { return x318_29_thermalVisor; }
  CTexture* GetRealReflection() {
    x2dc_reflectionAge = 0;
    if (x14c_reflectionTex) {
      return x14c_reflectionTex.get();
    }

    return &xe4_blackTex;
  }

  static void SetupCGraphicsState();
};
} // namespace metaforce
