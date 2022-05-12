#pragma once

#include <functional>
#include <vector>

#include "Runtime/CToken.hpp"
#include "Runtime/Graphics/CGraphics.hpp"
#include "Runtime/Graphics/CModel.hpp"
#include "Runtime/RetroTypes.hpp"

#include <zeus/CAABox.hpp>
#include <zeus/CColor.hpp>
#include <zeus/CFrustum.hpp>
#include <zeus/CPlane.hpp>
#include <zeus/CRectangle.hpp>

namespace metaforce {
class CAreaOctTree;
class CLight;
class CMetroidModelInstance;
class CPVSVisSet;
class CParticleGen;
class CSkinnedModel;
struct CAreaRenderOctTree;
struct CModelFlags;
struct SShader;

class IRenderer {
public:
  using TDrawableCallback = void (*)(void*, void*, int);
  using TReflectionCallback = std::function<void(void*, const zeus::CVector3f&)>;

  enum class EDrawableSorting { SortedCallback, UnsortedCallback };
  enum class EDebugOption { Invalid = -1, PVSMode, PVSState, FogDisabled };
  enum class EPrimitiveType {
    Triangles = GX::TRIANGLES,
    TriangleFan = GX::TRIANGLEFAN,
    TriangleStrip = GX::TRIANGLESTRIP,
    Lines = GX::LINES,
    LineStrip = GX::LINESTRIP,
  };

  virtual ~IRenderer() = default;
  virtual void AddStaticGeometry(const std::vector<CMetroidModelInstance>* geometry, const CAreaRenderOctTree* octTree,
                                 s32 areaIdx) = 0;
  virtual void EnablePVS(const CPVSVisSet& set, u32 areaIdx) = 0;
  virtual void DisablePVS() = 0;
  virtual void RemoveStaticGeometry(const std::vector<CMetroidModelInstance>* geometry) = 0;
  virtual void DrawUnsortedGeometry(s32 areaIdx, s32 mask, s32 targetMask) = 0;
  virtual void DrawSortedGeometry(s32 areaIdx, s32 mask, s32 targetMask) = 0;
  virtual void DrawStaticGeometry(s32 areaIdx, s32 mask, s32 targetMask) = 0;
  virtual void DrawAreaGeometry(s32 areaIdx, s32 mask, s32 targetMask) = 0;
  virtual void PostRenderFogs() = 0;
  virtual void SetModelMatrix(const zeus::CTransform& xf) = 0;
  virtual void AddParticleGen(CParticleGen& gen) = 0;
  virtual void AddParticleGen(CParticleGen& gen, const zeus::CVector3f& pos, const zeus::CAABox& bounds) = 0;
  virtual void AddPlaneObject(void* obj, const zeus::CAABox& aabb, const zeus::CPlane& plane, s32 type) = 0;
  virtual void AddDrawable(void* obj, const zeus::CVector3f& pos, const zeus::CAABox& aabb, s32 mode,
                           EDrawableSorting sorting) = 0;
  virtual void SetDrawableCallback(TDrawableCallback cb, void* ctx) = 0;
  virtual void SetWorldViewpoint(const zeus::CTransform& xf) = 0;
  virtual void SetPerspective(float fovy, float width, float height, float znear, float zfar) = 0;
  virtual void SetPerspective(float fovy, float aspect, float znear, float zfar) = 0;
  virtual std::pair<zeus::CVector2f, zeus::CVector2f> SetViewportOrtho(bool centered, float znear, float zfar) = 0;
  virtual void SetClippingPlanes(const zeus::CFrustum& frustum) = 0;
  virtual void SetViewport(s32 left, s32 bottom, s32 width, s32 height) = 0;
  virtual void SetDepthReadWrite(bool, bool) = 0;
  virtual void SetBlendMode_AdditiveAlpha() = 0;
  virtual void SetBlendMode_AlphaBlended() = 0;
  virtual void SetBlendMode_NoColorWrite() = 0;
  virtual void SetBlendMode_ColorMultiply() = 0;
  virtual void SetBlendMode_InvertDst() = 0;
  virtual void SetBlendMode_InvertSrc() = 0;
  virtual void SetBlendMode_Replace() = 0;
  virtual void SetBlendMode_AdditiveDestColor() = 0;
  virtual void SetDebugOption(EDebugOption, s32) = 0;
  virtual void BeginScene() = 0;
  virtual void EndScene() = 0;
  virtual void BeginPrimitive(EPrimitiveType, s32) = 0;
  virtual void BeginLines(s32) = 0;
  virtual void BeginLineStrip(s32) = 0;
  virtual void BeginTriangles(s32) = 0;
  virtual void BeginTriangleStrip(s32) = 0;
  virtual void BeginTriangleFan(s32) = 0;
  virtual void PrimVertex(const zeus::CVector3f&) = 0;
  virtual void PrimNormal(const zeus::CVector3f&) = 0;
  virtual void PrimColor(float, float, float, float) = 0;
  virtual void PrimColor(const zeus::CColor&) = 0;
  virtual void EndPrimitive() = 0;
  virtual void SetAmbientColor(const zeus::CColor& color) = 0;
  virtual void DrawString(const char* string, int, int) = 0;
  virtual u32 GetFPS() = 0;
  virtual void CacheReflection(TReflectionCallback cb, void* ctx, bool clearAfter) = 0;
  virtual void DrawSpaceWarp(const zeus::CVector3f& pt, float strength) = 0;
  virtual void DrawThermalModel(CModel& model, const zeus::CColor& multCol, const zeus::CColor& addCol,
                                TConstVectorRef positions, TConstVectorRef normals, const CModelFlags& flags) = 0;
  virtual void DrawModelDisintegrate(CModel& model, CTexture& tex, const zeus::CColor& color, TConstVectorRef positions,
                                     TConstVectorRef normals, float t) = 0;
  virtual void DrawModelFlat(CModel& model, const CModelFlags& flags, bool unsortedOnly, TConstVectorRef positions,
                             TConstVectorRef normals) = 0;
  virtual void SetWireframeFlags(s32 flags) = 0;
  virtual void SetWorldFog(ERglFogMode mode, float startz, float endz, const zeus::CColor& color) = 0;
  virtual void RenderFogVolume(const zeus::CColor& color, const zeus::CAABox& aabb, const TLockedToken<CModel>* model,
                               const CSkinnedModel* sModel) = 0;
  virtual void SetThermal(bool thermal, float level, const zeus::CColor& color) = 0;
  virtual void SetThermalColdScale(float scale) = 0;
  virtual void DoThermalBlendCold() = 0;
  virtual void DoThermalBlendHot() = 0;
  virtual u32 GetStaticWorldDataSize() = 0;
  virtual void SetGXRegister1Color(const zeus::CColor& color) = 0;
  virtual void SetWorldLightFadeLevel(float level) = 0;
  // Something
  virtual void PrepareDynamicLights(const std::vector<CLight>& lights) = 0;
};

} // namespace metaforce
