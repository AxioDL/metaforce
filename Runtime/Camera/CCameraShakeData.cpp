#include "CCameraShakeData.hpp"
#include "CRandom16.hpp"
#include "World/ScriptLoader.hpp"

namespace urde
{

SCameraShakePoint SCameraShakePoint::LoadCameraShakePoint(CInputStream& in)
{
    u32 flags = ScriptLoader::LoadParameterFlags(in);
    float f1 = in.readFloatBig();
    float f2 = in.readFloatBig();
    float f3 = in.readFloatBig();
    float f4 = in.readFloatBig();
    return {flags, f1, f2, f3, f4};
}

CCameraShakerComponent CCameraShakerComponent::LoadNewCameraShakerComponent(CInputStream& in)
{
    u32 flags = ScriptLoader::LoadParameterFlags(in);
    SCameraShakePoint sp1 = SCameraShakePoint::LoadCameraShakePoint(in);
    SCameraShakePoint sp2 = SCameraShakePoint::LoadCameraShakePoint(in);
    return {flags, sp1, sp2};
}

CCameraShakeData::CCameraShakeData(float f1, float f2, u32 w1, const zeus::CVector3f& v1,
                                   const CCameraShakerComponent& shaker1, const CCameraShakerComponent& shaker2,
                                   const CCameraShakerComponent& shaker3)
: x0_duration(f1), x8_shaker1(shaker1), x44_shaker2(shaker2), x80_shaker3(shaker3), xc0_flags(w1), xc4_sfxPos(v1), xd0_f2(f2)
{}

CCameraShakeData::CCameraShakeData(float f1, float f2)
: CCameraShakeData(f1, 100.f, 0, zeus::CVector3f::skZero, CCameraShakerComponent{}, CCameraShakerComponent{},
                   CCameraShakerComponent{1,
                   SCameraShakePoint{0, 0.25f * f1, 0.f, 0.75f * f1, f2},
                   SCameraShakePoint{1, 0.f, 0.f, 0.5f * f1, 2.f}})
{}

float CCameraShakeData::GetSomething() const
{
    float ret = 0.f;
    if (x8_shaker1.x4_w1)
        ret = x8_shaker1.x8_sp1.GetSomething();
    if (x44_shaker2.x4_w1)
        ret = std::max(ret, x44_shaker2.x8_sp1.GetSomething());
    if (x80_shaker3.x4_w1)
        ret = std::max(ret, x80_shaker3.x8_sp1.GetSomething());
    return ret;
}

float CCameraShakeData::GetSomething2() const
{
    float ret = 0.f;
    if (x8_shaker1.x4_w1)
        ret = x8_shaker1.x20_sp2.GetSomething();
    if (x44_shaker2.x4_w1)
        ret = std::max(ret, x44_shaker2.x20_sp2.GetSomething());
    if (x80_shaker3.x4_w1)
        ret = std::max(ret, x80_shaker3.x20_sp2.GetSomething());
    return ret;
}

#if 0
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
#endif

CCameraShakeData CCameraShakeData::LoadCameraShakeData(CInputStream& in)
{
    float f1 = in.readFloatBig();
    float f2 = in.readFloatBig();
    float f3 = in.readFloatBig();
    float f4 = in.readFloatBig();
    float f5 = in.readFloatBig();
    float f6 = in.readFloatBig();
    float duration = in.readFloatBig();

    SCameraShakePoint sp1(0, 0.f, 0.f, duration, 2.f * f1);
    SCameraShakePoint sp2(0, 0.f, 0.f, duration, 2.f * f3);
    SCameraShakePoint sp3(0, 0.f, 0.f, duration, 2.f * f5);
    SCameraShakePoint sp4(0, 0.f, 0.f, 0.5f * duration, 3.f);
    SCameraShakePoint sp5(0, 0.f, 0.f, 0.5f * duration, 0.f);
    SCameraShakePoint sp6(0, 0.f, 0.f, 0.5f * duration, 3.f);

    CCameraShakerComponent shaker1(1, sp1, sp4);
    CCameraShakerComponent shaker2;
    CCameraShakerComponent shaker3(1, sp3, sp6);

    return {duration, 100.f, 0, zeus::CVector3f::skZero, shaker1, shaker2, shaker3};
}

}
