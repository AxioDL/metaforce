#include "CVectorElement.hpp"
#include "CParticleGlobals.hpp"
#include "CRandom16.hpp"
#include "CParticleGen.hpp"
#include <math.h>

namespace pshag
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
        x18_keys.push_back(in.readVec3fBig());
}

bool CVEKeyframeEmitter::GetValue(int frame, Zeus::CVector3f& valOut) const
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

CVECone::CVECone(CVectorElement* a, CRealElement* b)
: x4_direction(a), x8_magnitude(b)
{
    Zeus::CVector3f av;
    x4_direction->GetValue(0, av);
    av.normalize();
    if (av[0] > 0.8)
        xc_xVec = av.cross(Zeus::CVector3f(0.f, 1.f, 0.f));
    else
        xc_xVec = av.cross(Zeus::CVector3f(1.f, 0.f, 0.f));
    x18_yVec = av.cross(xc_xVec);
}

bool CVECone::GetValue(int frame, Zeus::CVector3f& valOut) const
{
    float b;
    x8_magnitude->GetValue(frame, b);
    Zeus::CVector3f dir;
    x4_direction->GetValue(frame, dir);
    float b2 = std::min(1.f, b);

    float randX, randY;
    do
    {
        float rand1 = CRandom16::GetRandomNumber()->Float() - 0.5f;
        randX = 2.f * b2 * rand1;
        float rand2 = CRandom16::GetRandomNumber()->Float() - 0.5f;
        randY = 2.f * b2 * rand2;
    } while (randX * randX + randY * randY > 1.f);

    valOut = xc_xVec * randX + x18_yVec * randY + dir;
    return false;
}

bool CVETimeChain::GetValue(int frame, Zeus::CVector3f& valOut) const
{
    int v;
    xc_swFrame->GetValue(frame, v);
    if (frame >= v)
        return x8_b->GetValue(frame, valOut);
    else
        return x4_a->GetValue(frame, valOut);
}

bool CVEAngleCone::GetValue(int frame, Zeus::CVector3f& valOut) const
{
    float xc, yc, xr, yr;
    x4_angleXConstant->GetValue(frame, xc);
    x8_angleYConstant->GetValue(frame, yc);
    xc_angleXRange->GetValue(frame, xr);
    x10_angleYRange->GetValue(frame, yr);

    float xtmp = CRandom16::GetRandomNumber()->Float() * xr;
    float xang = (0.5f * xr - xtmp + xc) * M_PI / 180.f;

    float ytmp = CRandom16::GetRandomNumber()->Float() * yr;
    float yang = (0.5f * yr - ytmp + yc) * M_PI / 180.f;

    float mag;
    x14_magnitude->GetValue(frame, mag);

    /* This takes a +Z vector and rotates it around X and Y axis (like a rotation matrix would) */
    valOut = Zeus::CVector3f(cosf(xang) * -sinf(yang), sinf(xang), cosf(xang) * cosf(yang)) * Zeus::CVector3f(mag);
    return false;
}

bool CVEAdd::GetValue(int frame, Zeus::CVector3f& valOut) const
{
    Zeus::CVector3f a, b;
    x4_a->GetValue(frame, a);
    x8_b->GetValue(frame, b);
    valOut = a + b;
    return false;
}

CVECircleCluster::CVECircleCluster(CVectorElement* a, CVectorElement* b, CIntElement* c, CRealElement* d)
: x4_a(a), x24_magnitude(d)
{
    int cv;
    c->GetValue(0, cv);
    x20_deltaAngle = 360.f / float(cv) * M_PI / 180.f;

    Zeus::CVector3f bv;
    b->GetValue(0, bv);
    bv.normalize();
    if (bv[0] > 0.8)
        x8_xVec = bv.cross(Zeus::CVector3f(0.f, 1.f, 0.f));
    else
        x8_xVec = bv.cross(Zeus::CVector3f(1.f, 0.f, 0.f));
    x14_yVec = bv.cross(x8_xVec);

    delete b;
    delete c;
}

bool CVECircleCluster::GetValue(int frame, Zeus::CVector3f& valOut) const
{
    Zeus::CVector3f av;
    x4_a->GetValue(frame, av);

    float curAngle = frame * x20_deltaAngle;
    Zeus::CVector3f x = x8_xVec * cosf(curAngle);
    Zeus::CVector3f y = x14_yVec * sinf(curAngle);
    Zeus::CVector3f tv = x + y + av;

    float dv;
    x24_magnitude->GetValue(frame, dv);

    Zeus::CVector3f magVec(dv * tv.magnitude());
    Zeus::CVector3f rv = magVec * Zeus::CVector3f(CRandom16::GetRandomNumber()->Float(),
                                                  CRandom16::GetRandomNumber()->Float(),
                                                  CRandom16::GetRandomNumber()->Float());

    valOut = tv + rv;
    return false;
}

bool CVEConstant::GetValue(int frame, Zeus::CVector3f& valOut) const
{
    float a, b, c;
    x4_a->GetValue(frame, a);
    x8_b->GetValue(frame, b);
    xc_c->GetValue(frame, c);
    valOut = Zeus::CVector3f(a, b, c);
    return false;
}

bool CVEFastConstant::GetValue(int frame, Zeus::CVector3f& valOut) const
{
    valOut = x4_val;
    return false;
}

CVECircle::CVECircle(CVectorElement* a, CVectorElement* b, CRealElement* c, CRealElement* d, CRealElement* e)
: x4_direction(a), x20_angleConstant(c), x24_angleLinear(d), x28_magnitude(e)
{
    Zeus::CVector3f bv;
    b->GetValue(0, bv);
    bv.normalize();
    if (bv[0] > 0.8)
        x8_xVec = bv.cross(Zeus::CVector3f(0.f, 1.f, 0.f));
    else
        x8_xVec = bv.cross(Zeus::CVector3f(1.f, 0.f, 0.f));
    x14_yVec = bv.cross(x8_xVec);
    delete b;
}

bool CVECircle::GetValue(int frame, Zeus::CVector3f& valOut) const
{
    float c, d, e;
    x20_angleConstant->GetValue(frame, c);
    x24_angleLinear->GetValue(frame, d);
    x28_magnitude->GetValue(frame, e);

    float curAngle = (d * frame + c) * M_PI / 180.f;

    Zeus::CVector3f av;
    x4_direction->GetValue(frame, av);

    Zeus::CVector3f x = x8_xVec * e * cosf(curAngle);
    Zeus::CVector3f y = x14_yVec * e * sinf(curAngle);

    valOut = x + y + av;
    return false;
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

bool CVEParticleVelocity::GetValue(int /*frame*/, Zeus::CVector3f& valOut) const
{
    valOut = CParticleGlobals::g_particleMetrics->x1c_pvel;
    return false;
}

bool CVEPLCO::GetValue(int /*frame*/, Zeus::CVector3f& valOut) const
{
    valOut = CParticleGlobals::g_particleMetrics->x10_plco;
    return false;
}

bool CVEPLOC::GetValue(int /*frame*/, Zeus::CVector3f& valOut) const
{
    valOut = CParticleGlobals::g_particleMetrics->x0_ploc;
    return false;
}

bool CVEPSOF::GetValue(int /*frame*/, Zeus::CVector3f& valOut) const
{
    Zeus::CTransform trans= CParticleGlobals::g_currentParticleSystem->x4_system->GetOrientation();
    valOut.x = trans.m_basis[0].y;
    valOut.y = trans.m_basis[1].z;
    valOut.z = trans.m_origin.x;
    return false;
}

bool CVEPSOU::GetValue(int /*frame*/, Zeus::CVector3f& valOut) const
{
    Zeus::CTransform trans= CParticleGlobals::g_currentParticleSystem->x4_system->GetOrientation();
    valOut.x = trans.m_basis[0].z;
    valOut.y = trans.m_basis[1].x;
    valOut.z = trans.m_origin.y;
    return false;
}

bool CVEPSOR::GetValue(int /*frame*/, Zeus::CVector3f& valOut) const
{
    Zeus::CTransform trans= CParticleGlobals::g_currentParticleSystem->x4_system->GetOrientation();
    valOut.x = trans.m_basis[0].x;
    valOut.y = trans.m_basis[1].y;
    valOut.z = trans.m_basis[2].z;
    return false;
}

bool CVEPSTR::GetValue(int /*frame*/, Zeus::CVector3f& valOut) const
{
    valOut = CParticleGlobals::g_currentParticleSystem->x4_system->GetTranslation();
    return false;
}

bool CVESubtract::GetValue(int frame, Zeus::CVector3f& valOut) const
{
    Zeus::CVector3f a, b;
    x4_a->GetValue(frame, a);
    x8_b->GetValue(frame, b);
    valOut = a - b;
    return false;
}

bool CVEColorToVector::GetValue(int frame, Zeus::CVector3f &valOut) const
{
    Zeus::CColor val = {0.0, 0.0, 0.0, 1.0};
    x4_a->GetValue(frame, val);
    valOut = Zeus::CVector3f{val.r, val.g, val.b};
    return false;
}

}
