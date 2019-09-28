#pragma once

#include "Runtime/Camera/CCameraSpline.hpp"
#include "Runtime/Camera/CGameCamera.hpp"

namespace urde {

class CPathCamera : public CGameCamera {
public:
  enum class EInitialSplinePosition { BallCamBasis, Negative, Positive, ClampBasis };

private:
  CCameraSpline x188_spline;
  float x1d4_pos = 0.f;
  float x1d8_time = 0.f;
  float x1dc_lengthExtent;
  float x1e0_filterMag;
  float x1e4_filterProportion;
  EInitialSplinePosition x1e8_initPos;
  u32 x1ec_flags;
  float x1f0_minEaseDist;
  float x1f4_maxEaseDist;

public:
  CPathCamera(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf, bool active,
              float lengthExtent, float filterMag, float filterProportion, float minEaseDist, float maxEaseDist,
              u32 flags, EInitialSplinePosition initPos);

  void Accept(IVisitor&) override;
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
  void Think(float, CStateManager&) override;
  void Render(const CStateManager&) const override {}
  void ProcessInput(const CFinalInput&, CStateManager& mgr) override;
  void Reset(const zeus::CTransform&, CStateManager& mgr) override;
  zeus::CTransform MoveAlongSpline(float, CStateManager&);
  void ClampToClosedDoor(CStateManager&);
};

} // namespace urde
