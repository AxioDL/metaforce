#include "CRealElement.hpp"
#include "CParticleGlobals.hpp"
#include "CRandom16.hpp"
#include <math.h>

namespace Retro
{

CREKeyframeEmitter::CREKeyframeEmitter(CInputStream& in)
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

bool CREKeyframeEmitter::GetValue(int frame, float& valOut) const
{
    if (!x4_percent)
    {
        int emitterTime = CParticleGlobals::g_EmitterTime;
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
        int ltPerc = CParticleGlobals::g_ParticleLifetimePercentage;
        float ltPercRem = CParticleGlobals::g_ParticleLifetimePercentageRemainder;
        if (ltPerc == 100)
            valOut = x18_keys[100];
        else
            valOut = ltPercRem * x18_keys[ltPerc+1] + (1.0f - ltPercRem) * x18_keys[ltPerc];
    }
    return false;
}

bool CRELifetimeTween::GetValue(int frame, float& valOut) const
{
    float ltFac = frame / CParticleGlobals::g_ParticleLifetimeReal;
    float a, b;
    x4_a->GetValue(frame, a);
    x8_b->GetValue(frame, b);
    valOut = b * ltFac + (1.0f - ltFac) * a;
    return false;
}

bool CREConstant::GetValue(int frame, float& valOut) const
{
    valOut = x4_val;
    return false;
}

bool CRETimeChain::GetValue(int frame, float& valOut) const
{
    int v;
    xc_swFrame->GetValue(frame, v);
    if (frame >= v)
        return x8_b->GetValue(frame, valOut);
    else
        return x4_a->GetValue(frame, valOut);
}

bool CREAdd::GetValue(int frame, float& valOut) const
{
    float a, b;
    x4_a->GetValue(frame, a);
    x8_b->GetValue(frame, b);
    valOut = a + b;
    return false;
}

bool CREClamp::GetValue(int frame,float& valOut) const
{
    float a, b;
    x4_min->GetValue(frame, a);
    x8_max->GetValue(frame, b);
    xc_val->GetValue(frame, valOut);
    if (valOut > b)
        valOut = b;
    if (valOut < a)
        valOut = a;
    return false;
}

bool CRERandom::GetValue(int frame, float& valOut) const
{
    float a, b;
    x4_min->GetValue(frame, a);
    x8_max->GetValue(frame, b);
    float rand = CRandom16::GetRandomNumber()->Float();
    valOut = b * rand + a * (1.0f - rand);
    return false;
}

bool CREDotProduct::GetValue(int frame, float& valOut) const
{
    Zeus::CVector3f a, b;
    x4_a->GetValue(frame, a);
    x8_b->GetValue(frame, b);
    valOut = a.dot(b);
    return false;
}


bool CREMultiply::GetValue(int frame, float& valOut) const
{
    float a, b;
    x4_a->GetValue(frame, a);
    x8_b->GetValue(frame, b);
    valOut = a * b;
    return false;
}

bool CREPulse::GetValue(int frame, float& valOut) const
{
    int a, b;
    x4_aDuration->GetValue(frame, a);
    x8_bDuration->GetValue(frame, b);
    int cv = std::max(1, a + b + 1);

    if (b >= 1)
    {
        int cv2 = frame % cv;
        if (cv2 >= a)
            x10_valB->GetValue(frame, valOut);
        else
            xc_valA->GetValue(frame, valOut);
    }
    else
        xc_valA->GetValue(frame, valOut);

    return false;
}

bool CRETimeScale::GetValue(int frame, float& valOut) const
{
    float a;
    x4_a->GetValue(frame, a);
    valOut = float(frame) * a;
    return false;
}

bool CRELifetimePercent::GetValue(int frame, float& valOut) const
{
    float a;
    x4_percentVal->GetValue(frame, a);
    a = std::max(0.0f, a);
    valOut = (a / 100.0f) * CParticleGlobals::g_ParticleLifetimeReal;
    return false;
}

bool CRESineWave::GetValue(int frame, float& valOut) const
{
    float a, b, c;
    x4_magnitude->GetValue(frame, a);
    x8_linearFrame->GetValue(frame, b);
    xc_constantFrame->GetValue(frame, c);
    valOut = sinf((frame * b + c) * M_PI / 180.f) * a;
    return false;
}

bool CREISWT::GetValue(int frame, float& valOut) const
{
    if (frame == 0)
        x4_a->GetValue(frame, valOut);
    else
        x8_b->GetValue(frame, valOut);
    return false;
}

bool CRECompareLessThan::GetValue(int frame, float& valOut) const
{
    float a, b;
    x4_a->GetValue(frame, a);
    x8_b->GetValue(frame, b);
    if (a < b)
        xc_c->GetValue(frame, valOut);
    else
        x10_d->GetValue(frame, valOut);
    return false;
}

bool CRECompareEquals::GetValue(int frame, float& valOut) const
{
    float a, b;
    x4_a->GetValue(frame, a);
    x8_b->GetValue(frame, b);
    if (fabsf(a-b) < 0.00001)
        xc_c->GetValue(frame, valOut);
    else
        x10_d->GetValue(frame, valOut);
    return false;
}

bool CREParticleAccessParam1::GetValue(int frame, float& valOut) const
{
    valOut = CParticleGlobals::g_papValues[0];
    return false;
}

bool CREParticleAccessParam2::GetValue(int frame, float& valOut) const
{
    valOut = CParticleGlobals::g_papValues[1];
    return false;
}

bool CREParticleAccessParam3::GetValue(int frame, float& valOut) const
{
    valOut = CParticleGlobals::g_papValues[2];
    return false;
}

bool CREParticleAccessParam4::GetValue(int frame, float& valOut) const
{
    valOut = CParticleGlobals::g_papValues[3];
    return false;
}

bool CREParticleAccessParam5::GetValue(int frame, float& valOut) const
{
    valOut = CParticleGlobals::g_papValues[4];
    return false;
}

bool CREParticleAccessParam6::GetValue(int frame, float& valOut) const
{
    valOut = CParticleGlobals::g_papValues[5];
    return false;
}

bool CREParticleAccessParam7::GetValue(int frame, float& valOut) const
{
    valOut = CParticleGlobals::g_papValues[6];
    return false;
}

bool CREParticleAccessParam8::GetValue(int frame, float& valOut) const
{
    valOut = CParticleGlobals::g_papValues[7];
    return false;
}

bool CREPSLL::GetValue(int frame, float& valOut) const
{
    valOut = CParticleGlobals::g_particleMetrics->x2c_psll;
    return false;
}

bool CREPRLW::GetValue(int frame, float& valOut) const
{
    valOut = CParticleGlobals::g_particleMetrics->x30_prlw;
    return false;
}

bool CRESubtract::GetValue(int frame, float& valOut) const
{
    float a, b;
    x4_a->GetValue(frame, a);
    x8_b->GetValue(frame, b);
    valOut = a - b;
    return false;
}

bool CREVectorMagnitude::GetValue(int frame, float& valOut) const
{
    Zeus::CVector3f a;
    x4_a->GetValue(frame, a);
    valOut = a.magnitude();
    return false;
}

bool CREVectorXToReal::GetValue(int frame, float& valOut) const
{
    Zeus::CVector3f a;
    x4_a->GetValue(frame, a);
    valOut = a[0];
    return false;
}

bool CREVectorYToReal::GetValue(int frame, float& valOut) const
{
    Zeus::CVector3f a;
    x4_a->GetValue(frame, a);
    valOut = a[1];
    return false;
}

bool CREVectorZToReal::GetValue(int frame, float& valOut) const
{
    Zeus::CVector3f a;
    x4_a->GetValue(frame, a);
    valOut = a[2];
    return false;
}

bool CRECEXT::GetValue(int frame, float& valOut) const
{
    int a;
    x4_a->GetValue(frame, a);
    int cv = std::max(0, a);
    /* TODO: Figure out how value table is generated/stored in 0-00 */
    return false;
}

bool CREIntTimesReal::GetValue(int frame, float& valOut) const
{
    int a;
    x4_a->GetValue(frame, a);
    float b;
    x8_b->GetValue(frame, b);
    valOut = float(a) * b;
    return false;
}

bool CREConstantRange::GetValue(int frame, float& valOut) const
{
    float val, min, max;
    x4_val->GetValue(frame, val);
    x8_min->GetValue(frame, min);
    xc_max->GetValue(frame, max);

    if (val > min && val < max)
        x10_inRange->GetValue(Frame, valOut);
    else
        x14_outOfRange->GetValue(frame, valOut);

    return false;
}

bool CREGetComponentRed::GetValue(int frame, float& valOut) const
{
    Zeus::CColor a = Zeus::CColor::skBlack;
    x4_a->GetValue(frame, a);
    valOut = a.r;
    return false;
}

bool CREGetComponentGreen::GetValue(int frame, float& valOut) const
{
    Zeus::CColor a = Zeus::CColor::skBlack;
    x4_a->GetValue(frame, a);
    valOut = a.g;
    return false;
}

bool CREGetComponentBlue::GetValue(int frame, float& valOut) const
{
    Zeus::CColor a = Zeus::CColor::skBlack;
    x4_a->GetValue(frame, a);
    valOut = a.b;
    return false;
}

bool CREGetComponentAlpha::GetValue(int frame, float& valOut) const
{
    Zeus::CColor a = Zeus::CColor::skBlack;
    x4_a->GetValue(frame, a);
    valOut = a.a;
    return false;
}

}
