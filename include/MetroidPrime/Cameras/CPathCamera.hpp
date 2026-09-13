#ifndef _CPATHCAMERA
#define _CPATHCAMERA

#include "MetroidPrime/Cameras/CCameraSpline.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"

class CPathCamera : public CGameCamera {
public:
  enum EInitialSplinePosition {
    kISP_BallCamBasis,
    kISP_Negative,
    kISP_Positive,
    kISP_ClampBasis,
  };

  CPathCamera(const TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
              const CTransform4f& xf, bool active, const float lengthExtent, const float filterMag,
              const float filterProportion, const float minEaseDist, const float maxEaseDist, const uint flags,
              const EInitialSplinePosition initPos);
  ~CPathCamera() override;

  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) override;
  void Think(float dt, CStateManager& mgr) override;
  void Render(const CStateManager& mgr) const override;
  void ProcessInput(const CFinalInput& input, CStateManager& mgr) override;
  void Reset(const CTransform4f& xf, CStateManager& mgr) override;
  CTransform4f MoveAlongSpline(float dt, CStateManager& mgr);
  void AvoidDoorCollisions(CStateManager& mgr);

private:
  CCameraSpline mSpline;
  float mPos;
  float mTime;
  float mLengthExtent;
  float mFilterMag;
  float mFilterProportions;
  EInitialSplinePosition mInitialPosition;
  uint mFlags;
  float mMinEaseDist;
  float mMaxEaseDist;
};
CHECK_SIZEOF(CPathCamera, 0x1f8)

#endif // _CPATHCAMERA
