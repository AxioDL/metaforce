#ifndef _CGAMECAMERA
#define _CGAMECAMERA

#include "types.h"

#include "MetroidPrime/CActor.hpp"

#include "Kyoto/Math/CMatrix4f.hpp"
#include "Kyoto/Math/CTransform4f.hpp"

class CFinalInput;

class CGameCamera : public CActor {
public:
  CGameCamera(const TUniqueId uid, const bool active, const rstl::string& name,
              const CEntityInfo& info, const CTransform4f& xf, const float fov, const float nearZ,
              const float farZ, const float aspect, const TUniqueId watchedId,
              const bool disableInput, const int controllerIdx);

  // CEntity
  ~CGameCamera() override;
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;
  void SetActive(const bool active) override;

  // CGameCamera
  virtual void ProcessInput(const CFinalInput&, CStateManager& mgr) = 0;
  virtual void Reset(const CTransform4f&, CStateManager& mgr) = 0;

  const CMatrix4f& GetPerspectiveMatrix() const;
  CVector3f ConvertToScreenSpace(const CVector3f& vec) const;
  CTransform4f ValidateCameraTransform(const CTransform4f& newXf, const CTransform4f& oldXf);
  // ShouldTryRender__11CGameCameraCFv
  void SkipFovInterpolation();

  TUniqueId GetWatchedObject() const { return mWatchedObject; }
  float GetFov() const { return mCurrentFov; }
  void SetFov(float fov) {
    mCurrentFov = fov;
    mPerspDirty = true;
  }
  float GetNearClipDistance() const { return mZnear; }
  float GetFarClipDistance() const { return mZfar; }
  void InterpolateFOV(float start, float fov, float time, float delayTime);
  void UpdatePerspective(float dt);
  int GetControllerNumber() const { return mControllerIdx; }
  const bool DisablesInput() const { return mDisablesInput; }
#if defined(TARGET_PC)
  float GetAspectRatio() const;
#else
  float GetAspectRatio() const { return mAspect; }
#endif

protected:
  TUniqueId mWatchedObject;
  mutable CMatrix4f mPerspectiveMatrix;
  CTransform4f mOrigXf;
  float mCurrentFov;
  float mZnear;
  float mZfar;
  float mAspect;
  int mControllerIdx;
  mutable bool mPerspDirty : 1;
  bool mDisablesInput : 1;
  float mDelayTime;
  float mPerspInterpRemTime;
  float mPerspInterpDur;
  float mPerspInterpStartFov;
  float mPerspInterpEndFov;
#if defined(TARGET_PC)
  mutable float mCachedAspect = 0.f;
#endif
};
CHECK_SIZEOF(CGameCamera, (VERSION >= VERSION_GM8E_02 ? 0x198 : 0x188))

#endif // _CGAMECAMERA
