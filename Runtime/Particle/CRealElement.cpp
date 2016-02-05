#include "CRealElement.hpp"
#include "CParticleGlobals.hpp"

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
            valOut = x18_keys[calcKey];
        }
        else
        {
            int v1 = x10_loopEnd - 1;
            if (v1 < emitterTime)
                calcKey = v1;
            valOut = x18_keys[calcKey];
        }
    }
    else
    {
        int ltPerc = CParticleGlobals::g_particleLifetimePercentTweenInt;
        float ltPercRem = CParticleGlobals::g_particleLifetimePercentTweenIntFloatRem;
        if (ltPerc == 100)
            valOut = x18_keys[100];
        else
            valOut = x18_keys[ltPerc+1] * ltPercRem + (1.0f - ltPercRem) * x18_keys[ltPerc];
    }
    return false;
}

bool CRELifetimeTween::GetValue(int frame, float& valOut) const
{
    float ltFac = frame / CParticleGlobals::g_particleLifetimeFloat;
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
    xc_c->GetValue(frame, v);
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

bool CREClamp::GetValue(int frame, float &valOut) const
{
    float a, b;
    x4_a->GetValue(frame, a);
    x8_b->GetValue(frame, b);
    xc_c->GetValue(frame, valOut);
    if (valOut > b)
        valOut = b;
    if (valOut < a)
        valOut = a;
    return false;
}

}
