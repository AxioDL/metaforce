#ifndef __URDE_CGAMECAMERA_HPP__
#define __URDE_CGAMECAMERA_HPP__

#include "World/CActor.hpp"
#include "zeus/CTransform.hpp"

namespace urde
{
class CFinalInput;

class CGameCamera : public CActor
{
    friend class CStateManager;
    friend class CCameraManager;

protected:
    TUniqueId xe8_watchedObject;
    zeus::CMatrix4f xec_perspectiveMatrix;
    zeus::CTransform x12c_;
    float x15c_currentFov;
    float x160_znear;
    float x164_zfar;
    float x168_aspect;
    u32 x16c_controllerIdx;
    bool x170_24_perspDirty : 1;
    bool x170_25_disablesInput : 1;
    float x174_ = 0.f;
    float x178_ = 0.f;
    float x17c_ = 0.f;
    float x180_;
    float x184_fov;
public:
    CGameCamera(TUniqueId, bool active, const std::string& name, const CEntityInfo& info, const zeus::CTransform& xf,
                float fov, float nearz, float farz, float aspect, TUniqueId watchedId, bool disableInput,
                u32 controllerIdx);

    void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);
    void SetActive(bool active);
    virtual void ProcessInput(const CFinalInput&, CStateManager& mgr) = 0;
    virtual void Reset(const zeus::CTransform&, CStateManager& mgr) = 0;

    const zeus::CTransform& GetTransform() const { return x34_transform; }
    zeus::CMatrix4f GetPerspectiveMatrix() const;
    zeus::CVector3f ConvertToScreenSpace(const zeus::CVector3f&) const;
    zeus::CTransform ValidateCameraTransform(const zeus::CTransform&, const zeus::CTransform&);
    float GetNearClipDistance() const { return x160_znear; }
    float GetFarClipDistance() const { return x164_zfar; }
    float GetAspectRatio() const { return x168_aspect; }
    TUniqueId GetWatchedObject() const { return xe8_watchedObject; }
    float GetFov() const { return x15c_currentFov; }
    void GetControllerNumber() const;
    bool DisablesInput() const;
    void UpdatePerspective(float);
    void SetFovInterpolation(float start, float end, float time, float f4);
    void SkipFovInterpolation();
};
}

#endif // __URDE_CGAMECAMERA_HPP__
