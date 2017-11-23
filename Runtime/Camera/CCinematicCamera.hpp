#ifndef __URDE_CCINEMATICCAMERA_HPP__
#define __URDE_CCINEMATICCAMERA_HPP__

#include "CGameCamera.hpp"

namespace urde
{

class CCinematicCamera : public CGameCamera
{
    std::vector<zeus::CVector3f> x188_;
    std::vector<zeus::CQuaternion> x198_;
    std::vector<TUniqueId> x1a8_;
    std::vector<zeus::CVector3f> x1b8_;
    std::vector<TUniqueId> x1c8_;
    std::vector<float> x1d8_;
    float x1e8_duration;
    float x1ec_ = 0.f;
    float x1f0_origFovy;
    u32 x1f4_ = 0;
    u32 x1f8_ = 0;
    zeus::CQuaternion x1fc_;
    TUniqueId x20c_lookAtId = kInvalidUniqueId;
    zeus::CVector3f x210_;
    u32 x21c_flags;
    bool x220_24_;
public:
    CCinematicCamera(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                     const zeus::CTransform& xf, bool active, float shotDuration,
                     float fovy, float znear, float zfar, float aspect, u32 flags);

    void Accept(IVisitor& visitor);
    void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr);
    void ProcessInput(const CFinalInput&, CStateManager& mgr);
    void Reset(const zeus::CTransform&, CStateManager& mgr);
    u32 GetFlags() const { return x21c_flags; }
    void WasDeactivated(CStateManager& mgr);
    void CalculateWaypoints(CStateManager& mgr);
    void SendArrivedMsg(TUniqueId reciever, CStateManager& mgr);
};

}

#endif // __URDE_CCINEMATICCAMERA_HPP__
