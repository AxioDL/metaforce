#ifndef __URDE_IRENDERER_HPP__
#define __URDE_IRENDERER_HPP__

#include <vector>
#include "../RetroTypes.hpp"
#include "../CToken.hpp"
#include "zeus/CAABox.hpp"
#include "zeus/CPlane.hpp"
#include "CFrustum.hpp"
#include "zeus/CColor.hpp"

namespace urde
{
class CMetroidModelInstance;
class CLight;
class CAreaOctTree;
class CElementGen;
class CModel;
class CSkinnedModel;

class IRenderer
{
public:
    typedef void(*TDrawableCallback)(const void*, const void*, int);
    typedef void(*TReflectionCallback)(void*, const CVector3f&);
    enum class EDrawableSorting
    {
    };
    enum class EDebugOption
    {
    };
    enum class EPrimitiveType
    {
    };
    enum class ERglFogMode
    {
    };

    virtual void AddStaticGeometry(const std::vector<CMetroidModelInstance>&, const CAreaOctTree*, int);
    virtual void RemoveStaticGeometry(const std::vector<CMetroidModelInstance>&);
    virtual void DrawUnsortedGeometry(const std::vector<CLight>&, int, unsigned int, unsigned int);
    virtual void DrawSortedGeometry(const std::vector<CLight>&, int, unsigned int, unsigned int);
    virtual void DrawStaticGeometry(const std::vector<CLight>&, int, unsigned int, unsigned int);
    virtual void PostRenderFogs();
    virtual void AddParticleGen(const CElementGen&);
    virtual void AddPlaneObject(const void*, const CAABox&, const CPlane&, int);
    virtual void AddDrawable(void const *, const CVector3f&, const CAABox&, int, EDrawableSorting);
    virtual void SetDrawableCallback(TDrawableCallback, const void*);
    virtual void SetWorldViewpoint(const CTransform&);
    virtual void SetPerspectiveFovScalar(float);
    virtual void SetPerspective(float, float, float, float, float);
    virtual void SetPerspective(float, float, float, float);
    virtual void SetViewportOrtho(bool, float, float);
    virtual void SetClippingPlanes(const CFrustum&);
    virtual void SetViewport(int, int, int, int);
    virtual void SetDepthReadWrite(bool, bool);
    virtual void SetBlendMode_AdditiveAlpha();
    virtual void SetBlendMode_AlphaBlended();
    virtual void SetBlendMode_NoColorWrite();
    virtual void SetBlendMode_ColorMultiply();
    virtual void SetBlendMode_InvertDst();
    virtual void SetBlendMode_InvertSrc();
    virtual void SetBlendMode_Replace();
    virtual void SetBlendMode_AdditiveDestColor();
    virtual void SetDebugOption(EDebugOption, int);
    virtual void BeginScene();
    virtual void EndScene();
    virtual void BeginPrimitive(EPrimitiveType, int);
    virtual void BeginLines(int);
    virtual void BeginLineStrip(int);
    virtual void BeginTriangles(int);
    virtual void BeginTriangleStrip(int);
    virtual void BeginTriangleFan(int);
    virtual void PrimVertex(const CVector3f&);
    virtual void PrimNormal(const CVector3f&);
    virtual void PrimColor(float, float, float, float);
    virtual void PrimColor(const CColor&);
    virtual void EndPrimitive();
    virtual void SetAmbientColor(const CColor&);
    virtual void SetStaticWorldAmbientColor(const CColor&);
    virtual void DrawString(const char*, int, int);
    virtual u32 GetFPS();
    virtual void CacheReflection(TReflectionCallback, void*, bool);
    virtual void DrawSpaceWarp(const CVector3f&, float);
    virtual void DrawThermalModel(const CModel&, const CColor&, const CColor&, const float*, const float*);
    virtual void DrawXRayOutline(const CModel&, const float*, const float*);
    virtual void SetWireframeFlags(int);
    virtual void SetWorldFog(ERglFogMode, float, float, const CColor&);
    virtual void RenderFogVolume(const CColor&, const CAABox&, const TLockedToken<CModel>*, const CSkinnedModel*);
    virtual void SetThermal(bool, float, const CColor&);
    virtual void DoThermalBlendCold();
    virtual void DoThermalBlendHot();
    virtual u32 GetStaticWorldDataSize();
};

}

#endif // __URDE_IRENDERER_HPP__
