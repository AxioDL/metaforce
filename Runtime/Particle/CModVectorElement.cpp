#include "CModVectorElement.hpp"
#include "CParticleGlobals.hpp"
#include "CRandom16.hpp"
#include <math.h>

namespace pshag
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

CMVEBounce::CMVEBounce(CVectorElement *a, CVectorElement *b, CRealElement *c, CRealElement *d, bool f)
    : x4_a(a), x8_b(b), xc_c(c), x10_d(d), x14_e(false), x15_f(f), x24_j(0.0)
{
    if (x4_a && x8_b && x4_a->IsFastConstant() && x8_b->IsFastConstant())
    {
        x14_e = true;
        x8_b->GetValue(0, x18_g);

        if (x18_g.magSquared() > 0.0)
            x18_g.normalize();
        Zeus::CVector3f a;
        x4_a->GetValue(0, a);
        x24_j = x18_g.dot(a);
    }
}

bool CMVEBounce::GetValue(int frame, Zeus::CVector3f &pVel, Zeus::CVector3f &pPos) const
{
    if (!x14_e)
    {
        x8_b->GetValue(frame, ((Zeus::CVector3f&)x18_g));
        ((Zeus::CVector3f&)x18_g).normalize();

        Zeus::CVector3f a;
        x4_a->GetValue(frame, a);

        (float&)(x24_j) = x18_g.dot(a);
    }

    float dot = x18_g.dot(pPos);
    if ((dot - x24_j) <= 0.0f)
    {
        if (!x15_f)
            return true;
    }
    else
        return false;

    if (pVel.magSquared() > 0.0f)
        return false;

    Zeus::CVector3f delta = pPos - pVel;

    return false;
}

}
