#ifndef _CINTERPOLATIONCAMERA
#define _CINTERPOLATIONCAMERA

#include "MetroidPrime/Cameras/CGameCamera.hpp"

class CInterpolationCamera : public CGameCamera {
public:
  ~CInterpolationCamera() override;
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) override;
  void Think(float dt, CStateManager& mgr) override;
  void Render(const CStateManager& mgr) const override;

  // CGameCamera
  void ProcessInput(const CFinalInput&, CStateManager& mgr) override;
  void Reset(const CTransform4f&, CStateManager& mgr) override;

  CInterpolationCamera(TUniqueId uid, const CTransform4f& xf);

  void SetInterpolation(const CTransform4f& xf, CVector3f lookPos, float maxTime,
                        float positionSpeed, float rotationSpeed, TUniqueId targetId,
                        bool sinusoidal, CStateManager& mgr);

private:
  void EndInterpolation(CStateManager& mgr);
  bool InterpolateSinusoidal(CTransform4f& xf, const CVector3f& targetOrigin, CVector3f lookPos,
                             float maxTime, float curTime);
  const bool InterpolateWithDistance(CTransform4f& xf, const CVector3f& targetOrigin,
                                     const CVector3f& lookPos, float positionSpeed,
                                     float rotationSpeed, float dt, float maxTime, float curTime);

  TUniqueId mTargetId;
  float mTime;
  float mMaxTime;
  CTransform4f mStartTransform;
  CVector3f mLookPos;
  float mPositionSpeed;
  float mRotationSpeed;
  bool mSinusoidal : 1;
  float mCloseInAngle;
};
CHECK_SIZEOF(CInterpolationCamera, (VERSION >= VERSION_GM8E_02 ? 0x1f0 : 0x1e0))

#endif // _CINTERPOLATIONCAMERA
