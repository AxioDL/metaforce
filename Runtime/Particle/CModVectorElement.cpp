#include "CModVectorElement.hpp"
#include "CParticleGlobals.hpp"
#include "CRandom16.hpp"
#include <math.h>

namespace Retro
{

bool CMVEImplosion::GetValue(int frame, Zeus::CVector3f& pVel, Zeus::CVector3f& pPos) const
{
    Zeus::CVector3f av;
    x4_implPoint->GetValue(frame, av);

    Zeus::CVector3f dv = av - pPos;
    float dvm = dv.magnitude();

    float c;
    xc_maxMag->GetValue(frame, c);
    if (dvm > c)
        return false;

    float d;
    x10_minMag->GetValue(frame, d);
    if (x14_enableMinMag && dvm < d)
        return false;

    if (0.f == dvm)
        return false;

    float b;
    x8_magScale->GetValue(frame, b);
    pVel += Zeus::CVector3f(b / dvm) * dv;
    return false;
}

bool CMVEExponentialImplosion::GetValue(int frame, Zeus::CVector3f& pVel, Zeus::CVector3f& pPos) const
{
    Zeus::CVector3f av;
    x4_implPoint->GetValue(frame, av);

    Zeus::CVector3f dv = av - pPos;
    float dvm = dv.magnitude();

    float c;
    xc_maxMag->GetValue(frame, c);
    if (dvm > c)
        return false;

    float d;
    x10_minMag->GetValue(frame, d);
    if (x14_enableMinMag && dvm < d)
        return false;

    if (0.f == dvm)
        return false;

    float b;
    x8_magScale->GetValue(frame, b);
    pVel += Zeus::CVector3f(b) * dv;
    return false;
}

bool CMVELinearImplosion::GetValue(int frame, Zeus::CVector3f& pVel, Zeus::CVector3f& pPos) const
{
    Zeus::CVector3f av;
    x4_implPoint->GetValue(frame, av);

    Zeus::CVector3f dv = av - pPos;
    float dvm = dv.magnitude();

    float c;
    xc_maxMag->GetValue(frame, c);
    if (dvm > c)
        return false;

    float d;
    x10_minMag->GetValue(frame, d);
    if (x14_enableMinMag && dvm < d)
        return false;

    if (0.f == dvm)
        return false;

    float b;
    x8_magScale->GetValue(frame, b);
    pVel = Zeus::CVector3f(b / dvm) * dv;
    return false;
}

bool CMVETimeChain::GetValue(int frame, Zeus::CVector3f& pVel, Zeus::CVector3f& pPos) const
{
    int v;
    xc_swFrame->GetValue(frame, v);
    if (frame >= v)
        return x8_b->GetValue(frame, pVel, pPos);
    else
        return x4_a->GetValue(frame, pVel, pPos);
}

}
