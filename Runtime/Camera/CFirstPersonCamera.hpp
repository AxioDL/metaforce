#ifndef __URDE_CFIRSTPERSONCAMERA_HPP__
#define __URDE_CFIRSTPERSONCAMERA_HPP__

#include "CGameCamera.hpp"

namespace urde
{

class CFirstPersonCamera : public CGameCamera
{
    float x188_;
    u8 x18c_;
    zeus::CTransform x190_gunFollowXf;
    float x1c0_;
    TUniqueId x1c4_pitchId;
    union {
        struct
        {
            bool x1c6_24_ : 1;
        };
        u16 _dummy = 0;
    };

    zeus::CVector3f x1c8_;
    float x1d4_ = 0.f;

public:
    CFirstPersonCamera(TUniqueId, const zeus::CTransform& xf, TUniqueId, float,
                       float fov, float nearplane, float farplane, float aspect);

    void Accept(IVisitor& visitor);
    void PreThink(float, CStateManager&);
    void Think(float, CStateManager&);
    void ProcessInput(const CFinalInput&, CStateManager& mgr);
    void Reset(const zeus::CTransform&, CStateManager& mgr);

    void SkipCinematic();
    zeus::CTransform GetGunFollowTransform();
    void UpdateTransform(CStateManager&, float dt);
    void UpdateElevation(CStateManager&);
    void CalculateGunFollowOrientationAndTransform(zeus::CTransform&, zeus::CQuaternion&, float, zeus::CVector3f&);
    void SetScriptPitchId(TUniqueId uid) { x1c4_pitchId = uid; }
};
}

#endif // __URDE_CFIRSTPERSONCAMERA_HPP__
