#pragma once

#include "CFluidPlane.hpp"
#include "Graphics/Shaders/CFluidPlaneShader.hpp"

namespace urde {
class CFluidPlaneDoor final : public CFluidPlane {
  float xa0_tileSize;
  int xa4_tileSubdivisions;
  float xa8_rippleResolution;

  CFluidPlaneShader::RenderSetupInfo RenderSetup(const CStateManager& mgr, float alpha, const zeus::CTransform& xf,
                                                 const zeus::CAABox& aabb, bool noNormals) const;

public:
  CFluidPlaneDoor(CAssetId patternTex1, CAssetId patternTex2, CAssetId colorTex, float tileSize, u32 tileSubdivisions,
                  EFluidType fluidType, float alpha, const CFluidUVMotion& uvMotion);
  void AddRipple(float mag, TUniqueId rippler, const zeus::CVector3f& center, CScriptWater& water, CStateManager& mgr) {
  }
  void AddRipple(float intensity, TUniqueId rippler, const zeus::CVector3f& center, const zeus::CVector3f& velocity,
                 const CScriptWater& water, CStateManager& mgr, const zeus::CVector3f& upVec) {}
  void AddRipple(const CRipple& ripple, const CScriptWater& water, CStateManager& mgr) {}

  void Render(const CStateManager& mgr, float alpha, const zeus::CAABox& aabb, const zeus::CTransform& xf,
              const zeus::CTransform& areaXf, bool noNormals, const zeus::CFrustum& frustum,
              const std::optional<CRippleManager>& rippleManager, TUniqueId waterId,
              const bool* gridFlags, u32 gridDimX, u32 gridDimY, const zeus::CVector3f& areaCenter) const;
};

} // namespace urde
