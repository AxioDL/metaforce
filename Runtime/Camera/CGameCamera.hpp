#ifndef __URDE_CGAMECAMERA_HPP__
#define __URDE_CGAMECAMERA_HPP__

#include "World/CActor.hpp"
#include "zeus/CTransform.hpp"

namespace urde
{
class CFinalInput;

class CGameCamera : public CActor
{
    TUniqueId xe8_watchedObject;
    zeus::CMatrix4f xec_perspectiveMatrix;
    zeus::CTransform x12c_;
    float x15c_fov;
    float x160_znear;
    float x164_zfar;
    float x168_aspect;
    u32 x16c_;
    union {
        struct
        {
            bool x170_24_perspDirty : 1;
            bool x170_25_disablesInput : 1;
        };
        u32 _dummy = 0;
    };
    float x174_ = 0.f;
    float x178_ = 0.f;
    float x17c_ = 0.f;
    float x180_;
    float x184_;
public:
    CGameCamera(TUniqueId, bool active, const std::string& name, const CEntityInfo& info, const zeus::CTransform& xf,
                float fov, float nearz, float farz, float aspect, TUniqueId, bool, u32);

    void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);
    void SetActive(bool active);
    virtual void ProcessInput(const CFinalInput&, CStateManager& mgr) = 0;
    virtual void Reset(const zeus::CTransform&, CStateManager& mgr) = 0;

    const zeus::CTransform& GetTransform() const { return x34_transform; }
    zeus::CMatrix4f GetPerspectiveMatrix() const;
    zeus::CVector3f ConvertToScreenSpace(const zeus::CVector3f&) const;
    zeus::CTransform ValidateCameraTransform(const zeus::CTransform&, const zeus::CTransform&);
    float GetNearClipDistance() const;
    float GetFarClipDistance() const;
    float GetAspectRatio() const;
    TUniqueId GetWatchedObject() const;
    float GetFov() const;
    void SetFov(float);
    void GetControllerNumber() const;
    bool DisablesInput() const;

};
}

#endif // __URDE_CGAMECAMERA_HPP__
