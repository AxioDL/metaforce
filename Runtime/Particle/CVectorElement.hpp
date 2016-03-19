#ifndef __PSHAG_CVECTORELEMENT_HPP__
#define __PSHAG_CVECTORELEMENT_HPP__

#include "IElement.hpp"

/* Documentation at: http://www.metroid2002.com/retromodding/wiki/Particle_Script#Vector_Elements */

namespace urde
{

class CVEKeyframeEmitter : public CVectorElement
{
    u32 x4_percent;
    u32 x8_unk1;
    bool xc_loop;
    bool xd_unk2;
    u32 x10_loopEnd;
    u32 x14_loopStart;
    std::vector<zeus::CVector3f> x18_keys;
public:
    CVEKeyframeEmitter(CInputStream& in);
    bool GetValue(int frame, zeus::CVector3f& valOut) const;
};

class CVECone : public CVectorElement
{
    std::unique_ptr<CVectorElement> x4_direction;
    std::unique_ptr<CRealElement> x8_magnitude;
    zeus::CVector3f xc_xVec;
    zeus::CVector3f x18_yVec;
public:
    CVECone(CVectorElement* a, CRealElement* b);
    bool GetValue(int frame, zeus::CVector3f& valOut) const;
};

class CVETimeChain : public CVectorElement
{
    std::unique_ptr<CVectorElement> x4_a;
    std::unique_ptr<CVectorElement> x8_b;
    std::unique_ptr<CIntElement> xc_swFrame;
public:
    CVETimeChain(CVectorElement* a, CVectorElement* b, CIntElement* c)
    : x4_a(a), x8_b(b), xc_swFrame(c) {}
    bool GetValue(int frame, zeus::CVector3f& valOut) const;
};

class CVEAngleCone : public CVectorElement
{
    std::unique_ptr<CRealElement> x4_angleXConstant;
    std::unique_ptr<CRealElement> x8_angleYConstant;
    std::unique_ptr<CRealElement> xc_angleXRange;
    std::unique_ptr<CRealElement> x10_angleYRange;
    std::unique_ptr<CRealElement> x14_magnitude;
public:
    CVEAngleCone(CRealElement* a, CRealElement* b, CRealElement* c, CRealElement* d, CRealElement* e)
    : x4_angleXConstant(a), x8_angleYConstant(b), xc_angleXRange(c), x10_angleYRange(d), x14_magnitude(e) {}
    bool GetValue(int frame, zeus::CVector3f& valOut) const;
};

class CVEAdd : public CVectorElement
{
    std::unique_ptr<CVectorElement> x4_a;
    std::unique_ptr<CVectorElement> x8_b;
public:
    CVEAdd(CVectorElement* a, CVectorElement* b)
    : x4_a(a), x8_b(b) {}
    bool GetValue(int frame, zeus::CVector3f& valOut) const;
};

class CVECircleCluster : public CVectorElement
{
    std::unique_ptr<CVectorElement> x4_a;
    zeus::CVector3f x8_xVec;
    zeus::CVector3f x14_yVec;
    float x20_deltaAngle;
    std::unique_ptr<CRealElement> x24_magnitude;
public:
    CVECircleCluster(CVectorElement* a, CVectorElement* b, CIntElement* c, CRealElement* d);
    bool GetValue(int frame, zeus::CVector3f& valOut) const;
};

class CVEConstant : public CVectorElement
{
    std::unique_ptr<CRealElement> x4_a;
    std::unique_ptr<CRealElement> x8_b;
    std::unique_ptr<CRealElement> xc_c;
public:
    CVEConstant(CRealElement* a, CRealElement* b, CRealElement* c)
    : x4_a(a), x8_b(b), xc_c(c) {}
    bool GetValue(int frame, zeus::CVector3f& valOut) const;
};

class CVEFastConstant : public CVectorElement
{
    zeus::CVector3f x4_val;
public:
    CVEFastConstant(float a, float b, float c) : x4_val(a, b, c) {}
    bool GetValue(int frame, zeus::CVector3f& valOut) const;
    bool IsFastConstant() const {return true;}
};

class CVECircle : public CVectorElement
{
    std::unique_ptr<CVectorElement> x4_direction;
    zeus::CVector3f x8_xVec;
    zeus::CVector3f x14_yVec;
    std::unique_ptr<CRealElement> x20_angleConstant;
    std::unique_ptr<CRealElement> x24_angleLinear;
    std::unique_ptr<CRealElement> x28_radius;
public:
    CVECircle(CVectorElement* a, CVectorElement* b, CRealElement* c, CRealElement* d, CRealElement* e);
    bool GetValue(int frame, zeus::CVector3f& valOut) const;
};

class CVEMultiply : public CVectorElement
{
    std::unique_ptr<CVectorElement> x4_a;
    std::unique_ptr<CVectorElement> x8_b;
public:
    CVEMultiply(CVectorElement* a, CVectorElement* b)
    : x4_a(a), x8_b(b) {}
    bool GetValue(int frame, zeus::CVector3f& valOut) const;
};

class CVERealToVector : public CVectorElement
{
    std::unique_ptr<CRealElement> x4_a;
public:
    CVERealToVector(CRealElement* a)
    : x4_a(a) {}
    bool GetValue(int frame, zeus::CVector3f& valOut) const;
};

class CVEPulse : public CVectorElement
{
    std::unique_ptr<CIntElement> x4_aDuration;
    std::unique_ptr<CIntElement> x8_bDuration;
    std::unique_ptr<CVectorElement> xc_aVal;
    std::unique_ptr<CVectorElement> x10_bVal;
public:
    CVEPulse(CIntElement* a, CIntElement* b, CVectorElement* c, CVectorElement* d)
    : x4_aDuration(a), x8_bDuration(b), xc_aVal(c), x10_bVal(d) {}
    bool GetValue(int frame, zeus::CVector3f& valOut) const;
};

class CVEParticleVelocity : public CVectorElement
{
public:
    bool GetValue(int frame, zeus::CVector3f& valOut) const;
};

class CVEParticleColor : public CVectorElement
{
public:
    bool GetValue(int frame, zeus::CVector3f& valOut) const;
};

class CVEParticleLocation : public CVectorElement
{
public:
    bool GetValue(int frame, zeus::CVector3f& valOut) const;
};

class CVEParticleSystemOrientationFront : public CVectorElement
{
public:
    bool GetValue(int frame, zeus::CVector3f& valOut) const;
};

class CVEParticleSystemOrientationUp : public CVectorElement
{
public:
    bool GetValue(int frame, zeus::CVector3f& valOut) const;
};

class CVEParticleSystemOrientationRight : public CVectorElement
{
public:
    bool GetValue(int frame, zeus::CVector3f& valOut) const;
};

class CVEParticleSystemTranslation : public CVectorElement
{
public:
    bool GetValue(int frame, zeus::CVector3f& valOut) const;
};

class CVESubtract : public CVectorElement
{
    std::unique_ptr<CVectorElement> x4_a;
    std::unique_ptr<CVectorElement> x8_b;
public:
    CVESubtract(CVectorElement* a, CVectorElement* b)
        : x4_a(a), x8_b(b)
    {}
    bool GetValue(int frame, zeus::CVector3f& valOut) const;
};

class CVEColorToVector : public CVectorElement
{
    std::unique_ptr<CColorElement> x4_a;
public:
    CVEColorToVector(CColorElement* a)
        : x4_a(a) {}

    bool GetValue(int frame, zeus::CVector3f& valOut) const;
};

}

#endif // __PSHAG_CVECTORELEMENT_HPP__
