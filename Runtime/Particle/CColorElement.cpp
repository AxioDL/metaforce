#include "CColorElement.hpp"
#include "CElementGen.hpp"
#include "CParticleGlobals.hpp"
#include "CRandom16.hpp"
#include <math.h>

/* Documentation at: http://www.metroid2002.com/retromodding/wiki/Particle_Script#Color_Elements */

namespace pshag
{

CCEKeyframeEmitter::CCEKeyframeEmitter(CInputStream& in)
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
        x18_keys.push_back(in.readVec4fBig());
}

bool CCEKeyframeEmitter::GetValue(int frame, Zeus::CColor& valOut) const
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

bool CCEConstant::GetValue(int frame, Zeus::CColor& valOut) const
{
    float a, b, c, d;
    x4_a->GetValue(frame, a);
    x8_b->GetValue(frame, b);
    xc_c->GetValue(frame, c);
    x10_d->GetValue(frame, d);
    valOut = Zeus::CColor(a, b, c, d);
    return false;
}

bool CCEFastConstant::GetValue(int frame, Zeus::CColor& valOut) const
{
    valOut = x4_val;
    return false;
}

bool CCETimeChain::GetValue(int frame, Zeus::CColor& valOut) const
{
    int v;
    xc_swFrame->GetValue(frame, v);
    if (frame >= v)
        return x8_b->GetValue(frame, valOut);
    else
        return x4_a->GetValue(frame, valOut);
}

bool CCEFadeEnd::GetValue(int frame, Zeus::CColor& valOut) const
{
    float c;
    xc_startFrame->GetValue(frame, c);

    if (frame < c)
    {
        x4_a->GetValue(frame, valOut);
        return false;
    }

    float d;
    x10_endFrame->GetValue(frame, d);

    Zeus::CColor colA;
    Zeus::CColor colB;
    x4_a->GetValue(frame, colA);
    x8_b->GetValue(frame, colB);

    float t = (frame - c) / (d - c);
    valOut = Zeus::CColor::lerp(colA, colB, t);
    return false;
}

bool CCEFade::GetValue(int frame, Zeus::CColor& valOut) const
{
    float c;
    xc_startFrame->GetValue(frame, c);

    float t = frame / c;
    if (t > 1.f)
    {
        x8_b->GetValue(frame, valOut);
        return false;
    }

    Zeus::CColor colA;
    Zeus::CColor colB;
    x4_a->GetValue(frame, colA);
    x8_b->GetValue(frame, colB);

    valOut = Zeus::CColor::lerp(colA, colB, t);
    return false;
}

bool CCEPulse::GetValue(int frame, Zeus::CColor& valOut) const
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

bool CCEParticleColor::GetValue(int /*frame*/, Zeus::CColor& colorOut) const
{
    colorOut = CElementGen::g_currentParticle->x34_color;
    return false;
}

}
