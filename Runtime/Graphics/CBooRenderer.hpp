#ifndef __URDE_CBOORENDERER_HPP__
#define __URDE_CBOORENDERER_HPP__

#include "IRenderer.hpp"

namespace urde
{
class IObjectStore;
class CMemorySys;
class IFactory;

class CBooRenderer : public IRenderer
{
public:
    CBooRenderer(IObjectStore&, IFactory&)
    {
    }

    void AddStaticGeometry(const std::vector<CMetroidModelInstance>&, const CAreaOctTree*, int);
    void RemoveStaticGeometry(const std::vector<CMetroidModelInstance>&);
    void DrawUnsortedGeometry(const std::vector<CLight>&, int, unsigned int, unsigned int);
    void DrawSortedGeometry(const std::vector<CLight>&, int, unsigned int, unsigned int);
    void DrawStaticGeometry(const std::vector<CLight>&, int, unsigned int, unsigned int);
    void PostRenderFogs();
    void AddParticleGen(const CElementGen&);
    void AddPlaneObject(const void*, const zeus::CAABox&, const zeus::CPlane&, int);
    void AddDrawable(void const *, const zeus::CVector3f&, const zeus::CAABox&, int, EDrawableSorting);
    void SetDrawableCallback(TDrawableCallback, const void*);
    void SetWorldViewpoint(const zeus::CTransform&);
    void SetPerspectiveFovScalar(float);
    void SetPerspective(float, float, float, float, float);
    void SetPerspective(float, float, float, float);
    void SetViewportOrtho(bool, float, float);
    void SetClippingPlanes(const zeus::CFrustum&);
    void SetViewport(int, int, int, int);
    void SetDepthReadWrite(bool, bool);
    void SetBlendMode_AdditiveAlpha();
    void SetBlendMode_AlphaBlended();
    void SetBlendMode_NoColorWrite();
    void SetBlendMode_ColorMultiply();
    void SetBlendMode_InvertDst();
    void SetBlendMode_InvertSrc();
    void SetBlendMode_Replace();
    void SetBlendMode_AdditiveDestColor();
    void SetDebugOption(EDebugOption, int);
    void BeginScene();
    void EndScene();
    void BeginPrimitive(EPrimitiveType, int);
    void BeginLines(int);
    void BeginLineStrip(int);
    void BeginTriangles(int);
    void BeginTriangleStrip(int);
    void BeginTriangleFan(int);
    void PrimVertex(const zeus::CVector3f&);
    void PrimNormal(const zeus::CVector3f&);
    void PrimColor(float, float, float, float);
    void PrimColor(const zeus::CColor&);
    void EndPrimitive();
    void SetAmbientColor(const zeus::CColor&);
    void SetStaticWorldAmbientColor(const zeus::CColor&);
    void DrawString(const char*, int, int);
    u32 GetFPS();
    void CacheReflection(TReflectionCallback, void*, bool);
    void DrawSpaceWarp(const zeus::CVector3f&, float);
    void DrawThermalModel(const CModel&, const zeus::CColor&, const zeus::CColor&, const float*, const float*);
    void DrawXRayOutline(const CModel&, const float*, const float*);
    void SetWireframeFlags(int);
    void SetWorldFog(ERglFogMode, float, float, const zeus::CColor&);
    void RenderFogVolume(const zeus::CColor&, const zeus::CAABox&, const TLockedToken<CModel>*, const CSkinnedModel*);
    void SetThermal(bool, float, const zeus::CColor&);
    void DoThermalBlendCold();
    void DoThermalBlendHot();
    u32 GetStaticWorldDataSize();
};

}

#endif // __URDE_CBOORENDERER_HPP__
