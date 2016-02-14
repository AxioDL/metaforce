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

bool CMVEBounce::GetValue(int frame, Zeus::CVector3f& pVel, Zeus::CVector3f& pPos) const
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

bool CMVEConstant::GetValue(int frame, Zeus::CVector3f& pVel, Zeus::CVector3f& pPos) const
{
    x4_x->GetValue(frame, pVel.x);
    x8_y->GetValue(frame, pVel.y);
    xc_z->GetValue(frame, pVel.z);
    return false;
}

bool CMVEFastConstant::GetValue(int /*frame*/, Zeus::CVector3f& pVel, Zeus::CVector3f& /*pPos*/) const
{
    pVel = x4_val;
    return false;
}

bool CMVEGravity::GetValue(int frame, Zeus::CVector3f& pVel, Zeus::CVector3f& /*pPos*/) const
{
    Zeus::CVector3f grav;
    x4_a->GetValue(frame, grav);
    pVel += grav;
    return false;
}

bool CMVEExplode::GetValue(int frame, Zeus::CVector3f& pVel, Zeus::CVector3f& /*pPos*/) const
{
    if (frame == 0)
    {
        float b;
        x8_b->GetValue(frame, b);
        pVel *= b;
    }
    else
    {
        CRandom16* rand = CRandom16::GetRandomNumber();
        Zeus::CVector3f vec;
        do
        {
            vec = {rand->Float() - 0.5f, rand->Float() - 0.5f, rand->Float() - 0.5f};
        }
        while (vec.magSquared() > 1.0);

        vec.normalize();
        float a;
        x4_a->GetValue(frame, a);
        pVel = vec * a;
    }

    return false;
}

bool CMVESetPosition::GetValue(int frame, Zeus::CVector3f& /*pVel*/, Zeus::CVector3f& pPos) const
{
    x4_a->GetValue(frame, pPos);
    return false;
}

bool CMVEPulse::GetValue(int frame, Zeus::CVector3f& pVel, Zeus::CVector3f& pPos) const
{
    int a, b;
    x4_aDuration->GetValue(frame, a);
    x8_bDuration->GetValue(frame, b);
    int cv = std::max(1, a + b + 1);

    if (b >= 1)
    {
        int cv2 = frame % cv;
        if (cv2 >= a)
            x10_bVal->GetValue(frame, pVel, pPos);
        else
            xc_aVal->GetValue(frame, pVel, pPos);
    }
    else
        xc_aVal->GetValue(frame, pVel, pPos);

    return false;
}

bool CMVEWind::GetValue(int frame, Zeus::CVector3f& pVel, Zeus::CVector3f& /*pPos*/) const
{
    Zeus::CVector3f direction;
    x4_direction->GetValue(frame, direction);
    float speed;
    x8_speed->GetValue(frame, speed);
    pVel += (direction - pVel) * speed;
    return false;
}

bool CMVESwirl::GetValue(int frame, Zeus::CVector3f& pVel, Zeus::CVector3f& pPos) const
{
    Zeus::CVector3f a, b;
    x4_a->GetValue(frame, a);
    x8_b->GetValue(frame, b);
    const Zeus::CVector3f diff = a - pPos;
    float x = (diff.x - (((diff.z * ((diff.x * (diff.y * b.y)) + b.x)) + b.z) * b.x));
    float y = (diff.y - (((diff.z * ((diff.x * (diff.y * b.y)) + b.x)) + b.z) * b.y));
    float z = (diff.z - (((diff.z * ((diff.x * (diff.y * b.y)) + b.x)) + b.z) * b.z));
    float c = 0.0f, d = 0.0f;
    xc_c->GetValue(frame, c);
    x10_d->GetValue(frame, d);

    const float f9 = (b.z * ((b.x * (b.y * pVel.y)) + pVel.x)) + pVel.x;
    pVel.x = (c * ((f9 * b.x) + (d * ((b.y * (y * b.z)) - z)))) + ((1.0 - c) * pVel.x);
    pVel.y = (c * ((f9 * b.y) + (d * ((b.z * x) - (z * b.x))))) + ((1.0 - c) * pVel.y);
    pVel.z = (c * ((f9 * b.z) + (d * ((b.x * (x * b.y)) - y)))) + ((1.0 - c) * pVel.x);
    return false;
}


}
