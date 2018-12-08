#pragma once

#include "CGameCamera.hpp"
#include "CCameraSpline.hpp"

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

  void Accept(IVisitor&);
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);
  void Think(float, CStateManager&);
  void Render(const CStateManager&) const {}
  void ProcessInput(const CFinalInput&, CStateManager& mgr);
  void Reset(const zeus::CTransform&, CStateManager& mgr);
  zeus::CTransform MoveAlongSpline(float, CStateManager&);
  void ClampToClosedDoor(CStateManager&);
};

} // namespace urde
