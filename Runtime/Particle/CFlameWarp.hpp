#ifndef __PSHAG_CFLAMEWARP_HPP__
#define __PSHAG_CFLAMEWARP_HPP__

#include "CWarp.hpp"

namespace pshag
{
class CFlameWarp : public CWarp
{
    Zeus::CVector3f x4;
    Zeus::CVector3f xc;
    float x1c;
    float x20;
    int x24;
    bool x28_activated : 1;

public:
    CFlameWarp(float a, const Zeus::CVector3f& b)
        : x4(b), x1c(0.0), x20(a * a), x24(0)
    {
        x28_activated = false;
    }

    ~CFlameWarp() {}

    bool UpdateWarp() { return x28_activated; }
    void ModifyParticles(int, int, int *,
                         Zeus::CVector3f*,
                         Zeus::CVector3f*,
                         Zeus::CVector3f*,
                         Zeus::CColor*,
                         float*, float*) {}
    void Activate(bool) { x28_activated = true; }
    bool IsActivated() { return x28_activated; }
    FourCC Get4CharID() { return FOURCC('FWRP'); }
};
}

#endif // __PSHAG_CFLAMEWARP_HPP__
