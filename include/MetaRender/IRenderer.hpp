#ifndef _IRENDERER
#define _IRENDERER

#include "types.h"

#include <Kyoto/Graphics/CGraphics.hpp>
#include <Kyoto/Graphics/ModelTypes.hpp>
#include <dolphin/gx/GXEnum.h>

#include "rstl/pair.hpp"
#include "rstl/vector.hpp"

#include "Kyoto/TToken.hpp"

class CAABox;
class CAreaRenderOctTree;
class CColor;
class CFrustumPlanes;
class CLight;
class CMemorySys;
class CMetroidModelInstance;
class CModel;
class CModelFlags;
class COsContext;
class CParticleGen;
class CPlane;
class CPVSVisSet;
class CResFactory;
class CSkinnedModel;
class CTexture;
class CTransform4f;
class CVector2f;
class CVector3f;
class IObjectStore;

class IRenderer {
public:
  typedef void (*TDrawableCallback)(const void*, const void*, int);

  enum EDrawableSorting {
    kDS_SortedCallback,
    kDS_UnsortedCallback,
  };

  enum EPrimitiveType {
    kPT_Quads = GX_QUADS,
    kPT_Triangles = GX_TRIANGLES,
    kPT_TriangleStrip = GX_TRIANGLESTRIP,
    kPT_TriangleFan = GX_TRIANGLEFAN,
    kPT_Lines = GX_LINES,
    kPT_LineStrip = GX_LINESTRIP,
    kPT_Points = GX_POINTS,
  };

  enum EDebugOption {
    kDO_Invalid = -1,
    kDO_PVSMode,
    kDO_PVSState,
    kDO_FogDisabled,
  };

  virtual ~IRenderer() = 0;

  virtual void AddStaticGeometry(const rstl::vector< CMetroidModelInstance >* geometry,
                                 const CAreaRenderOctTree* octTree, int areaIdx) = 0;
  virtual void EnablePVS(const CPVSVisSet* set, int areaIdx) = 0;
  virtual void DisablePVS() = 0;
  virtual void RemoveStaticGeometry(const rstl::vector< CMetroidModelInstance >* geometry) = 0;
  virtual void DrawUnsortedGeometry(int areaIdx, uint mask, uint targetMask) = 0;
  virtual void DrawSortedGeometry(int areaIdx, uint mask, uint targetMask) = 0;
  virtual void DrawStaticGeometry(int areaIdx, uint mask, uint targetMask) = 0;
  virtual void DrawAreaGeometry(int areaIdx, uint mask, uint targetMask) = 0;
  virtual void PostRenderFogs() = 0;
  virtual void SetModelMatrix(const CTransform4f& xf) = 0;
  virtual void AddParticleGen(const CParticleGen& gen) = 0;
  virtual void AddParticleGen(const CParticleGen& gen, const CVector3f&, const CAABox&) = 0;
  virtual void AddPlaneObject(const void* obj, const CAABox& aabb, const CPlane& plane,
                              int type) = 0;
  virtual void AddDrawable(const void* obj, const CVector3f& pos, const CAABox& bounds, int mode,
                           IRenderer::EDrawableSorting sorting) = 0;
  virtual void SetDrawableCallback(TDrawableCallback cb, const void* ctx) = 0;
  virtual void SetWorldViewpoint(const CTransform4f& xf) = 0;
  virtual void SetPerspective(float, float, float, float, float) = 0;
  virtual void SetPerspective(float, float, float, float) = 0;
  virtual rstl::pair< CVector2f, CVector2f > SetViewportOrtho(bool centered, float znear,
                                                              float zfar) = 0;
  virtual void SetClippingPlanes(const CFrustumPlanes&) = 0;
  virtual void SetViewport(int left, int right, int width, int height) = 0;
  virtual void SetDepthReadWrite(const bool read, const bool update) = 0;
  virtual void SetBlendMode_AdditiveAlpha() = 0;
  virtual void SetBlendMode_AlphaBlended() = 0;
  virtual void SetBlendMode_NoColorWrite() = 0;
  virtual void SetBlendMode_ColorMultiply() = 0;
  virtual void SetBlendMode_InvertDst() = 0;
  virtual void SetBlendMode_InvertSrc() = 0;
  virtual void SetBlendMode_Replace() = 0;
  virtual void SetBlendMode_AdditiveDestColor() = 0;

  virtual void SetDebugOption(IRenderer::EDebugOption option, int value) = 0;
  virtual void BeginScene() = 0;
  virtual void EndScene() = 0;
  virtual void BeginPrimitive(IRenderer::EPrimitiveType prim, int count) = 0;
  virtual void BeginLines(int nverts) = 0;
  virtual void BeginLineStrip(int nverts) = 0;
  virtual void BeginTriangles(int nverts) = 0;
  virtual void BeginTriangleStrip(int nverts) = 0;
  virtual void BeginTriangleFan(int nverts) = 0;
  virtual void PrimVertex(const CVector3f& vtx) = 0;
  virtual void PrimNormal(const CVector3f& nrm) = 0;
  virtual void PrimColor(float r, float g, float b, float a) = 0;
  virtual void PrimColor(const CColor& color) = 0;
  virtual void EndPrimitive() = 0;
  virtual void SetAmbientColor(const CColor& color) = 0;
  virtual void DrawString(const char*, int, int) = 0;
  virtual float GetFPS() = 0;
  virtual void CacheReflection(void (*)(void*, const CVector3f&), void*, bool) = 0;
  virtual void DrawSpaceWarp(const CVector3f&, float) = 0;
  virtual void DrawThermalModel(const CModel&, const CColor&, const CColor&, TModelPositions,
                                TModelNormals, const CModelFlags&) = 0;
  virtual void DrawModelDisintegrate(const CModel&, const CTexture&, const CColor&, TModelPositions,
                                     TModelNormals, float) = 0;
  virtual void DrawModelFlat(const CModel&, const CModelFlags&, const bool, TModelPositions,
                             TModelNormals) = 0;
  virtual void SetWireframeFlags(int) = 0;
  virtual void SetWorldFog(ERglFogMode mode, float startz, float endz, const CColor& color) = 0;
  virtual void RenderFogVolume(const CColor&, const CAABox&, const TLockedToken< CModel >*,
                               const CSkinnedModel*) = 0;
  virtual void SetThermal(bool, float, const CColor&) = 0;
  virtual void SetThermalColdScale(float) = 0;
  virtual void DoThermalBlendCold() = 0;
  virtual void DoThermalBlendHot() = 0;
  virtual int GetStaticWorldDataSize() = 0;
  virtual void SetGXRegister1Color(const CColor&) = 0;
  virtual void SetWorldLightFadeLevel(float) = 0;
  virtual CAABox GetAreaModelBounds(int areaIdx, int modelIdx) = 0;
  virtual void PrepareDynamicLights(const rstl::vector< CLight >& lights) = 0;
};

inline IRenderer::~IRenderer() {}

namespace Renderer {
#if defined(TARGET_PC)
IRenderer* AllocateRenderer(IObjectStore&, CResFactory&);
#else
IRenderer* AllocateRenderer(IObjectStore&, COsContext&, CMemorySys&, CResFactory&);
#endif
}; // namespace Renderer

#endif // _IRENDERER
