#include "CEmitterElement.hpp"
#include "CRandom16.hpp"

/* Documentation at: http://www.metroid2002.com/retromodding/wiki/Particle_Script#Emitter_Elements */

namespace pshag
{

bool CEESimpleEmitter::GetValue(int frame, Zeus::CVector3f& pPos, Zeus::CVector3f& pVel) const
{
    x4_loc->GetValue(frame, pPos);

    if (x8_vec)
        x8_vec->GetValue(frame, pVel);
    else
        pVel = Zeus::CVector3f();

    return false;
}

bool CVESphere::GetValue(int frame, Zeus::CVector3f& pPos, Zeus::CVector3f& pVel) const
{
    Zeus::CVector3f a;
    x4_a->GetValue(frame, a);
    float b;
    x8_b->GetValue(frame, b);
    CRandom16* rand = CRandom16::GetRandomNumber();
    int rand1 = rand->Range(-100, 100);
    int rand2 = rand->Range(-100, 100);
    int rand3 = rand->Range(-100, 100);

    Zeus::CVector3f normVec1 = Zeus::CVector3f(0.0099999998f * float(rand3),
                                               0.0099999998f * float(rand2),
                                               0.0099999998f * float(rand1));
    if (normVec1.canBeNormalized())
        normVec1.normalize();

    pPos = b * normVec1 + a;

    Zeus::CVector3f normVec2 = (pPos - a);
    if (normVec2.canBeNormalized())
        normVec2.normalize();

    float c;
    xc_c->GetValue(frame, c);
    pVel = c * normVec2;

    return false;
}

bool CVEAngleSphere::GetValue(int frame, Zeus::CVector3f& pPos, Zeus::CVector3f& pVel) const
{
    Zeus::CVector3f a;
    x4_a->GetValue(frame, a);

    float b, d, e, f, g;
    x8_b->GetValue(frame,  b);
    x10_d->GetValue(frame, d);
    x14_e->GetValue(frame, e);
    x18_f->GetValue(frame, f);
    x1c_g->GetValue(frame, g);
    CRandom16* rand = CRandom16::GetRandomNumber();
    d = (d + ((0.5f * (f * rand->Float())) - f)) * M_PI / 180.f;
    e = (e + ((0.5f * (f * rand->Float())) - f)) * M_PI / 180.f;

    float cosD = Zeus::Math::fastCosR(d);
    pPos.x = a.x + (b * (-Zeus::Math::fastSinR(e) * cosD));
    pPos.y = a.y + (b * Zeus::Math::fastSinR(d));
    pPos.z = a.z + (b * (cosD * cosD));
    Zeus::CVector3f normVec = (pPos - a).normalized();

    float c;
    xc_c->GetValue(frame, c);
    pVel = c * normVec;
    return false;
}

}
