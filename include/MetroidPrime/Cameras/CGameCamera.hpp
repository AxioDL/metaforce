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

  TUniqueId GetWatchedObject() const { return xe8_watchedObject; }
  float GetFov() const { return x15c_currentFov; }
  void SetFov(float fov) {
    x15c_currentFov = fov;
    x170_24_perspDirty = true;
  }
  float GetNearClipDistance() const { return x160_znear; }
  float GetFarClipDistance() const { return x164_zfar; }
  void InterpolateFOV(float start, float fov, float time, float delayTime);
  void UpdatePerspective(float dt);
  int GetControllerNumber() const { return x16c_controllerIdx; }
  const bool DisablesInput() const { return x170_25_disablesInput; }
  float GetAspectRatio() const { return x168_aspect; }

protected:
  TUniqueId xe8_watchedObject;
  mutable CMatrix4f xec_perspectiveMatrix;
  CTransform4f x12c_origXf;
  float x15c_currentFov;
  float x160_znear;
  float x164_zfar;
  float x168_aspect;
  int x16c_controllerIdx;
  mutable bool x170_24_perspDirty : 1;
  bool x170_25_disablesInput : 1;
  float x174_delayTime;
  float x178_perspInterpRemTime;
  float x17c_perspInterpDur;
  float x180_perspInterpStartFov;
  float x184_perspInterpEndFov;
};
CHECK_SIZEOF(CGameCamera, (VERSION >= VERSION_GM8P_00 ? 0x198 : 0x188))

#endif // _CGAMECAMERA
