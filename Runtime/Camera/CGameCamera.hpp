#pragma once

#include "Runtime/RetroTypes.hpp"
#include "Runtime/World/CActor.hpp"

#include <zeus/CMatrix4f.hpp>
#include <zeus/CTransform.hpp>

namespace urde {
struct CFinalInput;

class CGameCamera : public CActor {
  friend class CCameraManager;
  friend class CStateManager;

protected:
  TUniqueId xe8_watchedObject;
  zeus::CMatrix4f xec_perspectiveMatrix;
  zeus::CTransform x12c_origXf;
  float x15c_currentFov;
  float x160_znear;
  float x164_zfar;
  float x168_aspect;
  u32 x16c_controllerIdx;
  bool x170_24_perspDirty : 1 = true;
  bool x170_25_disablesInput : 1;
  float x174_delayTime = 0.f;
  float x178_perspInterpRemTime = 0.f;
  float x17c_perspInterpDur = 0.f;
  float x180_perspInterpStartFov;
  float x184_perspInterpEndFov;

public:
  CGameCamera(TUniqueId, bool active, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
              float fov, float nearz, float farz, float aspect, TUniqueId watchedId, bool disableInput,
              u32 controllerIdx);

  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
  void SetActive(bool active) override;
  virtual void ProcessInput(const CFinalInput&, CStateManager& mgr) = 0;
  virtual void Reset(const zeus::CTransform&, CStateManager& mgr) = 0;

  zeus::CMatrix4f GetPerspectiveMatrix() const;
  zeus::CVector3f ConvertToScreenSpace(const zeus::CVector3f&) const;
  zeus::CTransform ValidateCameraTransform(const zeus::CTransform&, const zeus::CTransform&) const;
  float GetNearClipDistance() const { return x160_znear; }
  float GetFarClipDistance() const { return x164_zfar; }
  float GetAspectRatio() const { return x168_aspect; }
  TUniqueId GetWatchedObject() const { return xe8_watchedObject; }
  float GetFov() const { return x15c_currentFov; }
  void GetControllerNumber() const;
  bool DisablesInput() const;
  void UpdatePerspective(float);
  void SetFovInterpolation(float start, float end, float time, float delayTime);
  void SkipFovInterpolation();
};
} // namespace urde
