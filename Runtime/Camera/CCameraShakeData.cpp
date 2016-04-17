#include "CCameraShakeData.hpp"
#include "CRandom16.hpp"

namespace urde
{

CCameraShakeData::CCameraShakeData(float xA, float xB, float yA, float yB,
                                   float zA, float zB, float duration, bool shakeY)
: x0_pointA(xA, yA, zA), xc_pointB(xB, yB, zB), x18_duration(duration), x40_shakeY(shakeY) {}

zeus::CVector3f CCameraShakeData::GeneratePoint(float dt, CRandom16& r)
{
    x3c_cycleTimeLeft -= dt;
    if (x3c_cycleTimeLeft <= 0.f)
    {
        x3c_cycleTimeLeft = r.Range(1.f / 60.f, 0.1f);
        float zVal = r.Range(-1.f, 1.f);
        float yVal = 0.f;
        if (x40_shakeY)
            yVal = r.Range(-1.f, 1.f);
        float xVal = r.Range(-1.f, 1.f);
        zeus::CVector3f shakeVec(xVal, yVal, zVal);
        if (!shakeVec.canBeNormalized())
            shakeVec = {0.f, 0.f, 1.f};
        else
            shakeVec.normalize();
        x30_velocity = (shakeVec - x24_position) / x3c_cycleTimeLeft;
    }

    x24_position += x30_velocity * dt;
    float interp = zeus::clamp(0.f, 1.f - (x18_duration - x1c_curTime) / x18_duration, 1.f);

    x1c_curTime += dt;
    return x24_position * zeus::CVector3f::lerp(x0_pointA, xc_pointB, interp);
}

}
