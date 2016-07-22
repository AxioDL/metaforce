#include "CBooRenderer.hpp"

namespace urde
{

CBooRenderer::CBooRenderer(IObjectStore& store, IFactory& resFac)
: x8_factory(resFac), xc_store(store)
{
}

void CBooRenderer::AddStaticGeometry(const std::vector<CMetroidModelInstance>&, const CAreaOctTree*, int)
{
}

void CBooRenderer::RemoveStaticGeometry(const std::vector<CMetroidModelInstance>&)
{
}

void CBooRenderer::DrawUnsortedGeometry(const std::vector<CLight>&, int, unsigned int, unsigned int)
{
}

void CBooRenderer::DrawSortedGeometry(const std::vector<CLight>&, int, unsigned int, unsigned int)
{
}

void CBooRenderer::DrawStaticGeometry(const std::vector<CLight>&, int, unsigned int, unsigned int)
{
}

void CBooRenderer::PostRenderFogs()
{
}

void CBooRenderer::AddParticleGen(const CElementGen&)
{
}

void CBooRenderer::AddPlaneObject(const void*, const zeus::CAABox&, const zeus::CPlane&, int)
{
}

void CBooRenderer::AddDrawable(const void*, const zeus::CVector3f&, const zeus::CAABox&, int, EDrawableSorting)
{
}

void CBooRenderer::SetDrawableCallback(TDrawableCallback, const void*)
{
}

void CBooRenderer::SetWorldViewpoint(const zeus::CTransform&)
{
}

void CBooRenderer::SetPerspectiveFovScalar(float)
{
}

void CBooRenderer::SetPerspective(float, float, float, float, float)
{
}

void CBooRenderer::SetPerspective(float, float, float, float)
{
}

void CBooRenderer::SetViewportOrtho(bool, float, float)
{
}

void CBooRenderer::SetClippingPlanes(const zeus::CFrustum&)
{
}

void CBooRenderer::SetViewport(int, int, int, int)
{
}

void CBooRenderer::SetDepthReadWrite(bool, bool)
{
}

void CBooRenderer::SetBlendMode_AdditiveAlpha()
{
}

void CBooRenderer::SetBlendMode_AlphaBlended()
{
}

void CBooRenderer::SetBlendMode_NoColorWrite()
{
}

void CBooRenderer::SetBlendMode_ColorMultiply()
{
}

void CBooRenderer::SetBlendMode_InvertDst()
{
}

void CBooRenderer::SetBlendMode_InvertSrc()
{
}

void CBooRenderer::SetBlendMode_Replace()
{
}

void CBooRenderer::SetBlendMode_AdditiveDestColor()
{
}

void CBooRenderer::SetDebugOption(EDebugOption, int)
{
}

void CBooRenderer::BeginScene()
{
}

void CBooRenderer::EndScene()
{
}

void CBooRenderer::BeginPrimitive(EPrimitiveType, int)
{
}

void CBooRenderer::BeginLines(int)
{
}

void CBooRenderer::BeginLineStrip(int)
{
}

void CBooRenderer::BeginTriangles(int)
{
}

void CBooRenderer::BeginTriangleStrip(int)
{
}

void CBooRenderer::BeginTriangleFan(int)
{
}

void CBooRenderer::PrimVertex(const zeus::CVector3f&)
{
}

void CBooRenderer::PrimNormal(const zeus::CVector3f&)
{
}

void CBooRenderer::PrimColor(float, float, float, float)
{
}

void CBooRenderer::PrimColor(const zeus::CColor&)
{
}

void CBooRenderer::EndPrimitive()
{
}

void CBooRenderer::SetAmbientColor(const zeus::CColor&)
{
}

void CBooRenderer::SetStaticWorldAmbientColor(const zeus::CColor&)
{
}

void CBooRenderer::DrawString(const char*, int, int)
{
}

u32 CBooRenderer::GetFPS()
{
}

void CBooRenderer::CacheReflection(TReflectionCallback, void*, bool)
{
}

void CBooRenderer::DrawSpaceWarp(const zeus::CVector3f&, float)
{
}

void CBooRenderer::DrawThermalModel(const CModel&, const zeus::CColor&, const zeus::CColor&, const float*, const float*)
{
}

void CBooRenderer::DrawXRayOutline(const CModel&, const float*, const float*)
{
}

void CBooRenderer::SetWireframeFlags(int)
{
}

void CBooRenderer::SetWorldFog(ERglFogMode, float, float, const zeus::CColor&)
{
}

void CBooRenderer::RenderFogVolume(const zeus::CColor&, const zeus::CAABox&, const TLockedToken<CModel>*, const CSkinnedModel*)
{
}

void CBooRenderer::SetThermal(bool, float, const zeus::CColor&)
{
}

void CBooRenderer::DoThermalBlendCold()
{
}

void CBooRenderer::DoThermalBlendHot()
{
}

u32 CBooRenderer::GetStaticWorldDataSize()
{
}

}
