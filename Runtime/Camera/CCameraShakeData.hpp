#ifndef __URDE_CCAMERASHAKEDATA_HPP__
#define __URDE_CCAMERASHAKEDATA_HPP__

#include "zeus/CVector3f.hpp"
#include "RetroTypes.hpp"

namespace urde
{
class CRandom16;

class CCameraShakeData
{
    friend class CCameraManager;
    zeus::CVector3f x0_pointA;
    zeus::CVector3f xc_pointB;
    float x18_duration;
    float x1c_curTime = 0.f;
    u32 x20_shakerId = 0;
    zeus::CVector3f x24_position;
    zeus::CVector3f x30_velocity;
    float x3c_cycleTimeLeft;
    bool x40_shakeY;
public:
    CCameraShakeData(float xA, float xB, float yA, float yB,
                     float zA, float zB, float duration, bool shakeY);
    zeus::CVector3f GeneratePoint(float dt, CRandom16& r);
};

}

#endif // __URDE_CCAMERASHAKEDATA_HPP__
