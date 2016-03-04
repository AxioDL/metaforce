#ifndef __PSHAG_CFLAMEWARP_HPP__
#define __PSHAG_CFLAMEWARP_HPP__

#include "CWarp.hpp"

namespace urde
{
class CFlameWarp : public CWarp
{
    zeus::CVector3f x4;
    zeus::CVector3f xc;
    float x1c;
    float x20;
    int x24;
    bool x28_activated : 1;

public:
    CFlameWarp(float a, const zeus::CVector3f& b)
        : x4(b), x1c(0.0), x20(a * a), x24(0)
    {
        x28_activated = false;
    }

    ~CFlameWarp() {}

    bool UpdateWarp() { return x28_activated; }
    void ModifyParticles(int, int, int *,
                         zeus::CVector3f*,
                         zeus::CVector3f*,
                         zeus::CVector3f*,
                         zeus::CColor*,
                         float*, float*) {}
    void Activate(bool val) { x28_activated = val; }
    bool IsActivated() { return x28_activated; }
    FourCC Get4CharID() { return FOURCC('FWRP'); }
};
}

#endif // __PSHAG_CFLAMEWARP_HPP__
