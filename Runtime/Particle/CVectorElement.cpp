#include "CVectorElement.hpp"
#include "CParticleGlobals.hpp"
#include "CRandom16.hpp"
#include <math.h>

namespace Retro
{

CVEKeyframeEmitter::CVEKeyframeEmitter(CInputStream& in)
{
    x4_percent = in.readUint32Big();
    x8_unk1 = in.readUint32Big();
    xc_loop = in.readBool();
    xd_unk2 = in.readBool();
    x10_loopEnd = in.readUint32Big();
    x14_loopStart = in.readUint32Big();

    u32 count = in.readUint32Big();
    x18_keys.reserve(count);
    for (u32 i=0 ; i<count ; ++i)
        x18_keys.push_back(in.readFloatBig());
}

bool CVEKeyframeEmitter::GetValue(int frame, Zeus::CVector3f& valOut) const
{
    if (!x4_percent)
    {
        int emitterTime = CParticleGlobals::g_emitterTimeInt;
        int calcKey = emitterTime;
        if (xc_loop)
        {
            if (emitterTime >= x10_loopEnd)
            {
                int v1 = emitterTime - x14_loopStart;
                int v2 = x10_loopEnd - x14_loopStart;
                calcKey = v1 % v2;
                calcKey += x14_loopStart;
            }
        }
        else
        {
            int v1 = x10_loopEnd - 1;
            if (v1 < emitterTime)
                calcKey = v1;
        }
        valOut = x18_keys[calcKey];
    }
    else
    {
        int ltPerc = CParticleGlobals::g_particleLifetimePercentTweenInt;
        float ltPercRem = CParticleGlobals::g_particleLifetimePercentTweenIntFloatRem;
        if (ltPerc == 100)
            valOut = x18_keys[100];
        else
            valOut = ltPercRem * x18_keys[ltPerc+1] + (1.0f - ltPercRem) * x18_keys[ltPerc];
    }
    return false;
}

CVECone::CVECone(CVectorElement* a, CRealElement* b)
: x4_a(a), x8_b(b)
{
    Zeus::CVector3f av;
    x4_a->GetValue(0, av);
    Zeus::CVector3f avNorm = av.normalized();
    if (avNorm[0] > 0.8)
        xc_vec1 = av.cross(Zeus::CVector3f(0.f, 1.f, 0.f));
    else
        xc_vec1 = av.cross(Zeus::CVector3f(1.f, 0.f, 0.f));
    x18_vec2 = avNorm.cross(xc_vec1);
}

bool CVECone::GetValue(int frame, Zeus::CVector3f& valOut) const
{
    float b;
    x8_b->GetValue(frame, b);
    Zeus::CVector3f av;
    x4_a->GetValue(0, av);
    float b2 = std::min(1.f, b);
    while (true)
    {
        float rand = CRandom16::GetRandomNumber()->Float() - 0.5f;
        float c = 2.f * b2 * rand;


        c = c * c;

    }
}

bool CVEMultiply::GetValue(int frame, Zeus::CVector3f& valOut) const
{
    Zeus::CVector3f a, b;
    x4_a->GetValue(frame, a);
    x8_b->GetValue(frame, b);
    valOut = a * b;
    return false;
}

bool CVERealToVector::GetValue(int frame, Zeus::CVector3f& valOut) const
{
    float a;
    x4_a->GetValue(frame, a);
    valOut = Zeus::CVector3f(a);
    return false;
}

bool CVEPulse::GetValue(int frame, Zeus::CVector3f& valOut) const
{
    int a, b;
    x4_aDuration->GetValue(frame, a);
    x8_bDuration->GetValue(frame, b);
    int cv = std::max(1, a + b + 1);

    if (b >= 1)
    {
        int cv2 = frame % cv;
        if (cv2 >= a)
            x10_bVal->GetValue(frame, valOut);
        else
            xc_aVal->GetValue(frame, valOut);
    }
    else
        xc_aVal->GetValue(frame, valOut);

    return false;
}

bool CVEParticleVelocity::GetValue(int frame, Zeus::CVector3f& valOut) const
{
    /* TODO: Do */
    return false;
}

bool CVESPOS::GetValue(int frame, Zeus::CVector3f& valOut) const
{
    /* TODO: Do */
    return false;
}

bool CVEPLCO::GetValue(int frame, Zeus::CVector3f& valOut) const
{
    /* TODO: Do */
    return false;
}

bool CVEPLOC::GetValue(int frame, Zeus::CVector3f& valOut) const
{
    /* TODO: Do */
    return false;
}

bool CVEPSOR::GetValue(int frame, Zeus::CVector3f& valOut) const
{
    /* TODO: Do */
    return false;
}

bool CVEPSOF::GetValue(int frame, Zeus::CVector3f& valOut) const
{
    /* TODO: Do */
    return false;
}

}
