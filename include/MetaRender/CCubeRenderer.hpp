#ifndef _CCUBERENDERER
#define _CCUBERENDERER

#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Math/CPlane.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "types.h"

#include <dolphin/gx/GXEnum.h>

#include "MetaRender/IRenderer.hpp"

#include "Kyoto/CRandom16.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Graphics/CGraphicsPalette.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/IObjectStore.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CFrustumPlanes.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Math/CVector2f.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/PVS/CPVSVisSet.hpp"
#include "Kyoto/TOneStatic.hpp"
#include "Kyoto/TToken.hpp"
#include "Kyoto/Text/CFont.hpp"

#include "Weapons/IWeaponRenderer.hpp"

#include "rstl/list.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/pair.hpp"
#include "rstl/single_ptr.hpp"
#include "rstl/vector.hpp"

class CCubeModel;
class CMetroidModelInstance;
class CModel;
class CSkinnedModel;
class CCubeSurface;
class CLight;
class CModelFlags;

class CCubeRenderer : public IRenderer, public IWeaponRenderer, public TOneStatic< CCubeRenderer > {
private:
  class CAreaListItem {
  public:
    CAreaListItem(const rstl::vector< CMetroidModelInstance >* geometry,
                  const CAreaRenderOctTree* octTree,
                  const rstl::auto_ptr< rstl::vector< TCachedToken< CTexture > > >& textures,
                  const rstl::auto_ptr< rstl::vector< rstl::auto_ptr< CCubeModel > > >& models,
                  int areaIdx);

    const rstl::vector< CMetroidModelInstance >* GetModelVector() const { return x0_geometry; }
    const rstl::auto_ptr< rstl::vector< TCachedToken< CTexture > > >& GetTextures() const {
      return x8_textures;
    }
    const rstl::auto_ptr< rstl::vector< rstl::auto_ptr< CCubeModel > > >& GetModelList() const {
      return x10_models;
    }
    int GetAreaId() const { return x18_areaIdx; }

    // private:
    const rstl::vector< CMetroidModelInstance >* x0_geometry;
    const CAreaRenderOctTree* x4_octTree;
    const rstl::auto_ptr< rstl::vector< TCachedToken< CTexture > > > x8_textures;
    const rstl::auto_ptr< rstl::vector< rstl::auto_ptr< CCubeModel > > > x10_models;
    int x18_areaIdx;
    rstl::vector< uint > x1c_lightOctreeWords;
  };

public:
  class CFogVolumeListItem {
  public:
    CFogVolumeListItem(const CTransform4f&, CColor, const CAABox&, const TLockedToken< CModel >*,
                       const CSkinnedModel*);

    CTransform4f x0_xf;
    CColor x30_color;
    CAABox x34_aabb;
    rstl::optional_object< TLockedToken< CModel > > x4c_model;
    const CSkinnedModel* x5c_skinnedModel;
  };

#if defined(TARGET_PC)
  CCubeRenderer(IObjectStore&, CResFactory&);
#else
  CCubeRenderer(IObjectStore&, COsContext&, CMemorySys&, CResFactory&);
#endif
  ~CCubeRenderer() override;
  void AddStaticGeometry(const rstl::vector< CMetroidModelInstance >* geometry,
                         const CAreaRenderOctTree* octTree, int areaIdx) override;
  void EnablePVS(const CPVSVisSet* set, int areaIdx) override;
  void DisablePVS() override;
  void RemoveStaticGeometry(const rstl::vector< CMetroidModelInstance >* geometry) override;
  void DrawUnsortedGeometry(int areaIdx, uint mask, uint targetMask) override;
  void DrawSortedGeometry(int areaIdx, uint mask, uint targetMask) override;
  void DrawStaticGeometry(int areaIdx, uint mask, uint targetMask) override;
  void DrawAreaGeometry(int areaIdx, uint mask, uint targetMask) override;
  void PostRenderFogs() override;
  void SetModelMatrix(const CTransform4f& xf) override;
  void AddParticleGen(const CParticleGen& gen) override;
  void AddParticleGen(const CParticleGen& gen, const CVector3f&, const CAABox&) override;
  void AddPlaneObject(const void* obj, const CAABox& aabb, const CPlane& plane, int type) override;
  void AddDrawable(const void* obj, const CVector3f& pos, const CAABox& bounds, int mode,
                   IRenderer::EDrawableSorting sorting) override;
  void SetDrawableCallback(TDrawableCallback cb, const void* ctx) override;
  void SetWorldViewpoint(const CTransform4f& xf) override;
  void SetPerspective(float, float, float, float, float) override;
  void SetPerspective(float, float, float, float) override;
  rstl::pair< CVector2f, CVector2f > SetViewportOrtho(bool centered, float znear,
                                                      float zfar) override;
  void SetClippingPlanes(const CFrustumPlanes&) override;
  void SetViewport(int left, int right, int width, int height) override;
  void SetDepthReadWrite(const bool read, const bool update) override;
  void SetBlendMode_AdditiveAlpha() override;
  void SetBlendMode_AlphaBlended() override;
  void SetBlendMode_NoColorWrite() override;
  void SetBlendMode_ColorMultiply() override;
  void SetBlendMode_InvertDst() override;
  void SetBlendMode_InvertSrc() override;
  void SetBlendMode_Replace() override;
  void SetBlendMode_AdditiveDestColor() override;
  void SetDebugOption(IRenderer::EDebugOption option, int value) override;
  void BeginScene() override;
  void EndScene() override;
  void BeginPrimitive(IRenderer::EPrimitiveType prim, int count) override;
  void BeginLines(int nverts) override;
  void BeginLineStrip(int nverts) override;
  void BeginTriangles(int nverts) override;
  void BeginTriangleStrip(int nverts) override;
  void BeginTriangleFan(int nverts) override;
  void PrimVertex(const CVector3f& vtx) override;
  void PrimNormal(const CVector3f& nrm) override;
  void PrimColor(float r, float g, float b, float a) override;
  void PrimColor(const CColor& color) override;
  void EndPrimitive() override;
  void SetAmbientColor(const CColor& color) override;
  void DrawString(const char*, int, int) override;
  float GetFPS() override;
  void CacheReflection(void (*)(void*, const CVector3f&), void*, bool) override;
  void DrawSpaceWarp(const CVector3f&, float) override;
  void DrawThermalModel(const CModel&, const CColor&, const CColor&, TModelPositions, TModelNormals,
                        const CModelFlags&) override;
  void DrawModelDisintegrate(const CModel&, const CTexture&, const CColor&, TModelPositions,
                             TModelNormals, float) override;
  void DrawModelFlat(const CModel&, const CModelFlags&, const bool, TModelPositions,
                     TModelNormals) override;
  void SetWireframeFlags(int) override;
  void SetWorldFog(ERglFogMode mode, float startz, float endz, const CColor& color) override;
  void RenderFogVolume(const CColor&, const CAABox&, const TLockedToken< CModel >*,
                       const CSkinnedModel*) override;
  void SetThermal(bool, float, const CColor&) override;
  void SetThermalColdScale(float) override;
  void DoThermalBlendCold() override;
  void DoThermalBlendHot() override;
  int GetStaticWorldDataSize() override;
  void SetGXRegister1Color(const CColor&) override;
  void SetWorldLightFadeLevel(float) override;
  CAABox GetAreaModelBounds(int areaIdx, int modelIdx) override;
  void PrepareDynamicLights(const rstl::vector< CLight >& lights) override;

  void DrawXRayOutline(const CAABox&, const float*, const float*);
  void FindOverlappingWorldModels(rstl::vector< uint >&, const CAABox&);
  int DrawOverlappingWorldModelIDs(int, rstl::vector< uint >&, const CAABox&, int, int);
  void DrawOverlappingWorldModelShadows(int, rstl::vector< uint >&, const CAABox&, int, int);

  bool GetThermal() const { return x318_29_thermalVisor; }
  bool GetInAreaDraw() const { return x318_30_inAreaDraw; }

  void AllocatePhazonSuitMaskTexture();
  void DrawPhazonSuitIndirectEffect(const CColor&,
                                    const rstl::optional_object< TCachedToken< CTexture > >&, float,
                                    float, float, float, const CColor& = CColor::White());
  void ReallyDrawPhazonSuitIndirectEffect(const CColor&, const CTexture&, const CTexture&,
                                          const CColor&, float, float, float);
  void ReallyDrawPhazonSuitEffect(const CColor&, const CTexture&);
  void DoPhazonSuitIndirectAlphaBlur(float, float);
  void CopyTex(const int, const bool, void*, const GXTexFmt, const bool);
  void DoThermalModelDraw(const CCubeModel&, const CColor&, const CColor&, TModelPositions,
                          TModelNormals, const CModelFlags&);
  void SetupRendererStates(bool depthWrite);
  void SetupCGraphicsStates();
  void AddWorldSurfaces(CCubeModel& model);
  void HandleUnsortedModel(const CAreaListItem*, CCubeModel&);
  void HandleUnsortedModelWireframe(const CAreaListItem*, CCubeModel&);
  void ActivateLightsForModel(const CAreaListItem*, const CCubeModel&);
  void RenderBucketItems(const CAreaListItem*);
  void DrawRenderBucketsDebug();
  static void DrawFogFan(const CVector3f*, int);
  static void DrawFogFans(const CPlane*, int, const CVector3f*, int, int, int);
  static void DrawFogSlices(const CPlane*, int, int, const CVector3f&, float);
  void ReallyRenderFogVolume(const CColor&, const CAABox&, const CModel*, const CSkinnedModel*);
  void _DrawSpaceWarp(const CVector3f&, float);
  static void* GetRenderToTexBuffer(int);

  void SetRequestRGBA6(bool req) { x318_26_requestRGBA6 = req; }
  bool IsRGBA6Current() const { return x318_27_currentRGBA6; }
  bool GetReflectionFlag() { return x318_24_reflectionDirty; }
  void SetReflectionFlag() { x318_24_reflectionDirty = true; }
  CTexture* GetRealReflection();
  const CPlane& GetViewPlane() const { return xb0_viewPlane; }
  const CTexture& GetZeroTexture() const { return xe4_blackTex; }
  const CTexture& GetSphereRamp() const { return x220_sphereRamp; }
  static CCubeRenderer* That() { return sRenderer; }

private:
  CResFactory& x8_factory;
  IObjectStore& xc_objStore;
  CFont x10_font;
  int x18_primVertCount;
  rstl::list< CAreaListItem > x1c_areaListItems;
  rstl::vector< CCubeSurface > x34_surfaces;
  CFrustumPlanes x44_frustumPlanes;
  TDrawableCallback xa8_drawableCallback;
  const void* xac_drawableCallbackUserData;
  CPlane xb0_viewPlane;
  uchar xc0_pvsMode; // bool?
  int xc4_pvsState;
  rstl::optional_object< CPVSVisSet > xc8_pvsVisSet;
  int xe0_pvsAreaIdx;
  CTexture xe4_blackTex;
  rstl::single_ptr< CTexture > x14c_reflectionTex;
  CTexture x150_reflectionTex;
  CTexture x1b8_fogVolumeRamp;
  CTexture x220_sphereRamp;
  CGraphicsPalette x288_thermalPalette;
  CRandom16 x2a8_thermalRand;
  rstl::list< CFogVolumeListItem > x2ac_fogVolumes;
  rstl::list< rstl::pair< CVector3f, float > > x2c4_spaceWarps;
  int x2dc_reflectionAge;
  CColor x2e0_primColor;
  CVector3f x2e4_primNormal;
  float x2f0_thermalVisorLevel;
  CColor x2f4_thermalColor;
  uchar x2f8_thermalColdScale;
  CColor x2fc_tevReg1Color;
  rstl::vector< CLight > x300_dynamicLights;
  int x310_phazonSuitMaskCountdown;
  rstl::single_ptr< CTexture > x314_phazonSuitMask;
  bool x318_24_reflectionDirty : 1;
  bool x318_25_drawWireframe : 1;
  bool x318_26_requestRGBA6 : 1;
  bool x318_27_currentRGBA6 : 1;
  bool x318_28_disableFog : 1;
  bool x318_29_thermalVisor : 1;
  bool x318_30_inAreaDraw : 1;
  bool x318_31_persistRGBA6 : 1;

  void GenerateReflectionTex();
  void GenerateFogVolumeRampTex();
  void GenerateSphereRampTex();
  void LoadThermoPalette();

  rstl::list< CCubeRenderer::CAreaListItem >::iterator
  FindStaticGeometry(const rstl::vector< CMetroidModelInstance >* geometry);

  static CCubeRenderer* sRenderer;
};
CHECK_SIZEOF(CCubeRenderer, 0x31C);

extern CCubeRenderer* gpRender;

#endif // _CCUBERENDERER
