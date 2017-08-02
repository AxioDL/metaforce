#ifndef __URDE_CFIRSTPERSONCAMERA_HPP__
#define __URDE_CFIRSTPERSONCAMERA_HPP__

#include "CGameCamera.hpp"

namespace urde
{

class CFirstPersonCamera : public CGameCamera
{
    float x188_;
    bool x18c_ = false;
    zeus::CTransform x190_gunFollowXf;
    float x1c0_ = 0.f;
    TUniqueId x1c4_pitchId = kInvalidUniqueId;
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
    const zeus::CTransform& GetGunFollowTransform() const { return x190_gunFollowXf; }
    void UpdateTransform(CStateManager&, float dt);
    void UpdateElevation(CStateManager&);
    void CalculateGunFollowOrientationAndTransform(zeus::CTransform&, zeus::CQuaternion&, float, zeus::CVector3f&);
    void SetScriptPitchId(TUniqueId uid) { x1c4_pitchId = uid; }
    void SetX18C(bool v) { x18c_ = v; }
    void SetX1C6_24(bool v) { x1c6_24_ = v; }
};
}

#endif // __URDE_CFIRSTPERSONCAMERA_HPP__
