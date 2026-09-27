#ifndef _CFLUIDPLANEDOOR
#define _CFLUIDPLANEDOOR

#include "MetroidPrime/CFluidPlaneManager.hpp"

class CFluidPlaneDoor : public CFluidPlane {
public:
  CFluidPlaneDoor(CAssetId texPattern1, CAssetId texPattern2, CAssetId texColor, float tileSize,
                  uint tileSubdivisions, EFluidType type, float alpha,
                  const CFluidUVMotion& uvMotion);
  ~CFluidPlaneDoor() override;

  void RenderSetup(const CStateManager& mgr, float alpha, const CTransform4f& xf,
                   const CAABox& bounds, bool noNormals) const;
  void AddRipple(const float mag, const TUniqueId rippler, const CVector3f& center,
                 const CScriptWater& water, CStateManager& mgr) override {}
  void AddRipple(float intensity, TUniqueId rippler, const CVector3f& center,
                 const CVector3f& velocity, const CScriptWater& water, CStateManager& mgr,
                 const CVector3f& upVec) override {}
  void AddRipple(const CRipple& ripple, const CScriptWater& water, CStateManager& mgr) override {}
  void Render(const CStateManager& mgr, float alpha, const CAABox& aabb,
              const CTransform4f& xf, const CTransform4f& areaXf, bool noNormals,
              const CFrustumPlanes& frustum,
              const rstl::optional_object< CRippleManager >& rippleManager,
              TUniqueId waterId, const char* gridFlags, int gridDimX, int gridDimY,
              const CVector3f& areaCenter) const override;

  void Update();

  void RenderCleanup() const;

private:
  float mTileSize;
  int mTileSubdivisions;
  float mRippleResolution;
};

#endif // _CFLUIDPLANEDOOR
