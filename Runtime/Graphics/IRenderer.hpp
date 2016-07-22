#ifndef __URDE_IRENDERER_HPP__
#define __URDE_IRENDERER_HPP__

#include <vector>
#include "../RetroTypes.hpp"
#include "../CToken.hpp"
#include "zeus/CAABox.hpp"
#include "zeus/CPlane.hpp"
#include "zeus/CFrustum.hpp"
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
    using TDrawableCallback = std::function<void(const void*, const void*, int)>;
    using TReflectionCallback = std::function<void(void*, const zeus::CVector3f&)>;

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

    virtual ~IRenderer() = default;
    virtual void AddStaticGeometry(const std::vector<CMetroidModelInstance>&, const CAreaOctTree*, int)=0;
    virtual void RemoveStaticGeometry(const std::vector<CMetroidModelInstance>&)=0;
    virtual void DrawUnsortedGeometry(const std::vector<CLight>&, int, unsigned int, unsigned int)=0;
    virtual void DrawSortedGeometry(const std::vector<CLight>&, int, unsigned int, unsigned int)=0;
    virtual void DrawStaticGeometry(const std::vector<CLight>&, int, unsigned int, unsigned int)=0;
    virtual void PostRenderFogs()=0;
    virtual void AddParticleGen(const CElementGen&)=0;
    virtual void AddPlaneObject(const void*, const zeus::CAABox&, const zeus::CPlane&, int)=0;
    virtual void AddDrawable(void const *, const zeus::CVector3f&, const zeus::CAABox&, int, EDrawableSorting)=0;
    virtual void SetDrawableCallback(TDrawableCallback, const void*)=0;
    virtual void SetWorldViewpoint(const zeus::CTransform&)=0;
    virtual void SetPerspectiveFovScalar(float)=0;
    virtual void SetPerspective(float, float, float, float, float)=0;
    virtual void SetPerspective(float, float, float, float)=0;
    virtual void SetViewportOrtho(bool, float, float)=0;
    virtual void SetClippingPlanes(const zeus::CFrustum&)=0;
    virtual void SetViewport(int, int, int, int)=0;
    virtual void SetDepthReadWrite(bool, bool)=0;
    virtual void SetBlendMode_AdditiveAlpha()=0;
    virtual void SetBlendMode_AlphaBlended()=0;
    virtual void SetBlendMode_NoColorWrite()=0;
    virtual void SetBlendMode_ColorMultiply()=0;
    virtual void SetBlendMode_InvertDst()=0;
    virtual void SetBlendMode_InvertSrc()=0;
    virtual void SetBlendMode_Replace()=0;
    virtual void SetBlendMode_AdditiveDestColor()=0;
    virtual void SetDebugOption(EDebugOption, int)=0;
    virtual void BeginScene()=0;
    virtual void EndScene()=0;
    virtual void BeginPrimitive(EPrimitiveType, int)=0;
    virtual void BeginLines(int)=0;
    virtual void BeginLineStrip(int)=0;
    virtual void BeginTriangles(int)=0;
    virtual void BeginTriangleStrip(int)=0;
    virtual void BeginTriangleFan(int)=0;
    virtual void PrimVertex(const zeus::CVector3f&)=0;
    virtual void PrimNormal(const zeus::CVector3f&)=0;
    virtual void PrimColor(float, float, float, float)=0;
    virtual void PrimColor(const zeus::CColor&)=0;
    virtual void EndPrimitive()=0;
    virtual void SetAmbientColor(const zeus::CColor&)=0;
    virtual void SetStaticWorldAmbientColor(const zeus::CColor&)=0;
    virtual void DrawString(const char*, int, int)=0;
    virtual u32 GetFPS()=0;
    virtual void CacheReflection(TReflectionCallback, void*, bool)=0;
    virtual void DrawSpaceWarp(const zeus::CVector3f&, float)=0;
    virtual void DrawThermalModel(const CModel&, const zeus::CColor&, const zeus::CColor&, const float*, const float*)=0;
    virtual void DrawXRayOutline(const CModel&, const float*, const float*)=0;
    virtual void SetWireframeFlags(int)=0;
    virtual void SetWorldFog(ERglFogMode, float, float, const zeus::CColor&)=0;
    virtual void RenderFogVolume(const zeus::CColor&, const zeus::CAABox&, const TLockedToken<CModel>*, const CSkinnedModel*)=0;
    virtual void SetThermal(bool, float, const zeus::CColor&)=0;
    virtual void DoThermalBlendCold()=0;
    virtual void DoThermalBlendHot()=0;
    virtual u32 GetStaticWorldDataSize()=0;
};

}

#endif // __URDE_IRENDERER_HPP__
