#ifndef __PSHAG_CEMITTERELEMENT_HPP__
#define __PSHAG_CEMITTERELEMENT_HPP__

#include "IElement.hpp"

/* Documentation at: http://www.metroid2002.com/retromodding/wiki/Particle_Script#Emitter_Elements */

namespace urde
{

class CEESimpleEmitter : public CEmitterElement
{
    std::unique_ptr<CVectorElement> x4_loc;
    std::unique_ptr<CVectorElement> x8_vec;
public:
    CEESimpleEmitter(CVectorElement* a, CVectorElement* b)
    : x4_loc(a), x8_vec(b) {}
    bool GetValue(int frame, zeus::CVector3f& pPos, zeus::CVector3f& pVel) const;
};

class CVESphere : public CEmitterElement
{
    std::unique_ptr<CVectorElement> x4_sphereOrigin;
    std::unique_ptr<CRealElement> x8_sphereRadius;
    std::unique_ptr<CRealElement> xc_velocityMag;
public:
    CVESphere(CVectorElement* a, CRealElement* b, CRealElement* c)
    : x4_sphereOrigin(a), x8_sphereRadius(b), xc_velocityMag(c) {}
    bool GetValue(int frame, zeus::CVector3f& pPos, zeus::CVector3f& pVel) const;
};

class CVEAngleSphere : public CEmitterElement
{
    std::unique_ptr<CVectorElement> x4_sphereOrigin;
    std::unique_ptr<CRealElement> x8_sphereRadius;
    std::unique_ptr<CRealElement> xc_velocityMag;
    std::unique_ptr<CRealElement> x10_angleXBias;
    std::unique_ptr<CRealElement> x14_angleYBias;
    std::unique_ptr<CRealElement> x18_angleXRange;
    std::unique_ptr<CRealElement> x1c_angleYRange;
public:
    CVEAngleSphere(CVectorElement* a, CRealElement* b, CRealElement* c, CRealElement* d,
                   CRealElement* e, CRealElement* f, CRealElement* g)
    : x4_sphereOrigin(a), x8_sphereRadius(b), xc_velocityMag(c), x10_angleXBias(d), x14_angleYBias(e), x18_angleXRange(f), x1c_angleYRange(g) {}
    bool GetValue(int frame, zeus::CVector3f& pPos, zeus::CVector3f& pVel) const;
};

}

#endif // __PSHAG_CEMITTERELEMENT_HPP__
