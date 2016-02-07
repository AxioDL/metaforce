#include "CIntElement.hpp"
#include "CParticleGlobals.hpp"
#include "CRandom16.hpp"

namespace Retro
{

CIEKeyframeEmitter::CIEKeyframeEmitter(CInputStream& in)
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
        x18_keys.push_back(in.readInt32Big());
}

bool CIEKeyframeEmitter::GetValue(int frame, int& valOut) const
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

bool CIEDeath::GetValue(int frame, int &valOut) const
{
    x4_a->GetValue(frame, valOut);
    int b;
    x8_b->GetValue(frame, b);
    /* Not 100% sure about this, originally some kinda branchless comparison */
    return frame > b;
}

bool CIEClamp::GetValue(int frame, int& valOut) const
{
    int a, b;
    x4_min->GetValue(frame, a);
    x8_max->GetValue(frame, b);
    xc_val->GetValue(frame, valOut);
    if (valOut > b)
        valOut = b;
    if (valOut < a)
        valOut = a;
    return false;
}

bool CIETimeChain::GetValue(int frame, int& valOut) const
{
    int v;
    xc_swFrame->GetValue(frame, v);
    if (frame >= v)
        return x8_b->GetValue(frame, valOut);
    else
        return x4_a->GetValue(frame, valOut);
}

bool CIEAdd::GetValue(int frame, int& valOut) const
{
    int a, b;
    x4_a->GetValue(frame, a);
    x8_b->GetValue(frame, b);
    valOut = a + b;
    return false;
}

bool CIEConstant::GetValue(int frame, int& valOut) const
{
    valOut = x4_val;
    return false;
}

bool CIEImpulse::GetValue(int frame, int& valOut) const
{
    if (frame == 0)
        x4_a->GetValue(frame, valOut);
    else
        valOut = 0;
    return false;
}

bool CIELifetimePercent::GetValue(int frame, int& valOut) const
{
    int a;
    x4_percentVal->GetValue(frame, a);
    a = std::max(0, a);
    valOut = (a / 100.0f) * CParticleGlobals::g_particleLifetimeFloat;
    return false;
}

bool CIEInitialRandom::GetValue(int frame, int& valOut) const
{
    if (frame == 0)
    {
        int a, b;
        x4_a->GetValue(frame, a);
        x8_b->GetValue(frame, b);
        valOut = CRandom16::GetRandomNumber()->Range(a, b);
    }
    return false;
}

bool CIEPulse::GetValue(int frame, int& valOut) const
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


bool CIEMultiply::GetValue(int frame, int& valOut) const
{
    int a, b;
    x4_a->GetValue(frame, a);
    x8_b->GetValue(frame, b);
    valOut = a * b;
    return false;
}

bool CIESampleAndHold::GetValue(int frame, int& valOut) const
{
    if (x8_nextSampleFrame < frame)
    {
        int b, c;
        xc_waitFramesMin->GetValue(frame, b);
        x10_waitFramesMax->GetValue(frame, c);
        /* const-correctness, who needs it? */
        ((CIESampleAndHold*)this)->x8_nextSampleFrame = CRandom16::GetRandomNumber()->Range(b, c) + frame;
        x4_sampleSource->GetValue(frame, valOut);
        ((CIESampleAndHold*)this)->x14_holdVal = valOut;
    }
    else
        valOut = x14_holdVal;
    return false;
}

bool CIERandom::GetValue(int frame, int& valOut) const
{
    int a, b;
    x4_min->GetValue(frame, a);
    x8_max->GetValue(frame, b);
    if (frame > 0)
        valOut = CRandom16::GetRandomNumber()->Range(a, b);
    else
        valOut = CRandom16::GetRandomNumber()->Next();
    return false;
}

bool CIETimeScale::GetValue(int frame, int& valOut) const
{
    float a;
    x4_a->GetValue(frame, a);
    valOut = float(frame) * a;
    return false;
}

bool CIEGTCP::GetValue(int frame, int& valOut) const
{
    /* TODO: Do */
    return false;
}

bool CIEModulo::GetValue(int frame, int& valOut) const
{
    int a, b;
    x4_a->GetValue(frame, a);
    x8_b->GetValue(frame, b);
    if (b != 0)
        valOut = a % b;
    else
        valOut = a;
    return false;
}

bool CIESubtract::GetValue(int frame, int& valOut) const
{
    int a, b;
    x4_a->GetValue(frame, a);
    x8_b->GetValue(frame, b);
    valOut = a - b;
    return false;
}

}
