#include "CModVectorElement.hpp"
#include "CParticleGlobals.hpp"
#include "CRandom16.hpp"
#include "zeus/Math.hpp"

/* Documentation at: http://www.metroid2002.com/retromodding/wiki/Particle_Script#Mod_Vector_Elements */

namespace urde
{

bool CMVEImplosion::GetValue(int frame, zeus::CVector3f& pVel, zeus::CVector3f& pPos) const
{
    zeus::CVector3f av;
    x4_implPoint->GetValue(frame, av);

    zeus::CVector3f dv = av - pPos;
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
    pVel += zeus::CVector3f(b / dvm) * dv;
    return false;
}

bool CMVEExponentialImplosion::GetValue(int frame, zeus::CVector3f& pVel, zeus::CVector3f& pPos) const
{
    zeus::CVector3f av;
    x4_implPoint->GetValue(frame, av);

    zeus::CVector3f dv = av - pPos;
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
    pVel += zeus::CVector3f(b) * dv;
    return false;
}

bool CMVELinearImplosion::GetValue(int frame, zeus::CVector3f& pVel, zeus::CVector3f& pPos) const
{
    zeus::CVector3f av;
    x4_implPoint->GetValue(frame, av);

    zeus::CVector3f dv = av - pPos;
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
    pVel = zeus::CVector3f(b / dvm) * dv;
    return false;
}

bool CMVETimeChain::GetValue(int frame, zeus::CVector3f& pVel, zeus::CVector3f& pPos) const
{
    int v;
    xc_swFrame->GetValue(frame, v);
    if (frame >= v)
        return x8_b->GetValue(frame, pVel, pPos);
    else
        return x4_a->GetValue(frame, pVel, pPos);
}

CMVEBounce::CMVEBounce(CVectorElement* a, CVectorElement* b, CRealElement* c, CRealElement* d, bool f)
: x4_planePoint(a), x8_planeNormal(b), xc_friction(c), x10_restitution(d), x14_planePrecomputed(false), x15_dieOnPenetrate(f), x24_planeD(0.0)
{
    if (x4_planePoint && x8_planeNormal && x4_planePoint->IsFastConstant() && x8_planeNormal->IsFastConstant())
    {
        /* Precompute Hesse normal form of plane (for penetration testing)
         * https://en.wikipedia.org/wiki/Hesse_normal_form */
        x14_planePrecomputed = true;
        x8_planeNormal->GetValue(0, x18_planeValidatedNormal);

        if (x18_planeValidatedNormal.magSquared() > 0.0)
            x18_planeValidatedNormal.normalize();
        zeus::CVector3f a;
        x4_planePoint->GetValue(0, a);
        x24_planeD = x18_planeValidatedNormal.dot(a);
    }
}

bool CMVEBounce::GetValue(int frame, zeus::CVector3f& pVel, zeus::CVector3f& pPos) const
{
    if (!x14_planePrecomputed)
    {
        /* Compute Hesse normal form of plane (for penetration testing) */
        x8_planeNormal->GetValue(frame, ((zeus::CVector3f&)x18_planeValidatedNormal));
        ((zeus::CVector3f&)x18_planeValidatedNormal).normalize();

        zeus::CVector3f a;
        x4_planePoint->GetValue(frame, a);

        (float&)(x24_planeD) = x18_planeValidatedNormal.dot(a);
    }

    float dot = x18_planeValidatedNormal.dot(pPos);
    if ((dot - x24_planeD) <= 0.0f)
    {
        if (x15_dieOnPenetrate)
            return true;
    }
    else
        return false;

    /* Deflection event */

    if (pVel.magSquared() > 0.0f)
        return false;

    zeus::CVector3f delta = pPos - pVel;
    pPos += zeus::CVector3f{(-((((delta.z * ((delta.x * (delta.y * x18_planeValidatedNormal.y))
                                             + ((pVel.x * (x18_planeValidatedNormal.y * pVel.y)) + x18_planeValidatedNormal.x))) + x18_planeValidatedNormal.z) - x24_planeD)) /
                             ((pVel.z * ((pVel.x * (x18_planeValidatedNormal.y * pVel.y)) + x18_planeValidatedNormal.x)) + x18_planeValidatedNormal.z)) - (
                (x18_planeValidatedNormal.z * ((x18_planeValidatedNormal.x * (x18_planeValidatedNormal.y * pVel.y)) + pVel.x)) +  pVel.z)} * pVel;

    float d = 0.0f;
    x10_restitution->GetValue(frame, d);
    pVel -= d * pVel;

    float c = 0.0f;
    xc_friction->GetValue(frame, c);
    pVel -= zeus::CVector3f{(1.0f + c) * ((x18_planeValidatedNormal.z * (x18_planeValidatedNormal.x * (x18_planeValidatedNormal.y * pVel.y)) + pVel.x) + pVel.x)} * x18_planeValidatedNormal;
    return false;
}

bool CMVEConstant::GetValue(int frame, zeus::CVector3f& pVel, zeus::CVector3f& /*pPos*/) const
{
    x4_x->GetValue(frame, pVel.x);
    x8_y->GetValue(frame, pVel.y);
    xc_z->GetValue(frame, pVel.z);
    return false;
}

bool CMVEFastConstant::GetValue(int /*frame*/, zeus::CVector3f& pVel, zeus::CVector3f& /*pPos*/) const
{
    pVel = x4_val;
    return false;
}

bool CMVEGravity::GetValue(int frame, zeus::CVector3f& pVel, zeus::CVector3f& /*pPos*/) const
{
    zeus::CVector3f grav;
    x4_a->GetValue(frame, grav);
    pVel += grav;
    return false;
}

bool CMVEExplode::GetValue(int frame, zeus::CVector3f& pVel, zeus::CVector3f& /*pPos*/) const
{
    if (frame == 0)
    {
        CRandom16* rand = CRandom16::GetRandomNumber();
        zeus::CVector3f vec = {rand->Float() - 0.5f, rand->Float() - 0.5f, rand->Float() - 0.5f};
        vec.normalize();
        float a;
        x4_a->GetValue(frame, a);
        pVel = vec * a;
    }
    else
    {
        float b;
        x8_b->GetValue(frame, b);
        pVel *= zeus::CVector3f(b);
    }

    return false;
}

bool CMVESetPosition::GetValue(int frame, zeus::CVector3f& /*pVel*/, zeus::CVector3f& pPos) const
{
    x4_a->GetValue(frame, pPos);
    return false;
}

bool CMVEPulse::GetValue(int frame, zeus::CVector3f& pVel, zeus::CVector3f& pPos) const
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

bool CMVEWind::GetValue(int frame, zeus::CVector3f& pVel, zeus::CVector3f& /*pPos*/) const
{
    zeus::CVector3f wVel;
    x4_velocity->GetValue(frame, wVel);
    float factor;
    x8_factor->GetValue(frame, factor);
    pVel += (wVel - pVel) * factor;
    return false;
}

bool CMVESwirl::GetValue(int frame, zeus::CVector3f& pVel, zeus::CVector3f& pPos) const
{
    zeus::CVector3f a, b;
    x4_helixPoint->GetValue(frame, a);
    x8_curveBinormal->GetValue(frame, b);

    /* Compute Frenet–Serret normal
     * https://en.wikipedia.org/wiki/Frenet–Serret_formulas */
    const zeus::CVector3f diff = a - pPos;
    float x = (diff.x - (((diff.z * ((diff.x * (diff.y * b.y)) + b.x)) + b.z) * b.x));
    float y = (diff.y - (((diff.z * ((diff.x * (diff.y * b.y)) + b.x)) + b.z) * b.y));
    float z = (diff.z - (((diff.z * ((diff.x * (diff.y * b.y)) + b.x)) + b.z) * b.z));
    float c = 0.0f, d = 0.0f;
    xc_targetRadius->GetValue(frame, c);
    x10_tangentialVelocity->GetValue(frame, d);

    /* Integrate tangential velocity by crossing particle normal with binormal,
     * also "homing" towards the target radius */
    const float f9 = (b.z * ((b.x * (b.y * pVel.y)) + pVel.x)) + pVel.x;
    pVel.x = (c * ((f9 * b.x) + (d * ((b.y * (y * b.z)) - z)))) + ((1.0 - c) * pVel.x);
    pVel.y = (c * ((f9 * b.y) + (d * ((b.z * x) - (z * b.x))))) + ((1.0 - c) * pVel.y);
    pVel.z = (c * ((f9 * b.z) + (d * ((b.x * (x * b.y)) - y)))) + ((1.0 - c) * pVel.x);
    return false;
}


}
