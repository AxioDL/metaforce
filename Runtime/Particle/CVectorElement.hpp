#ifndef __RETRO_CVECTORELEMENT_HPP__
#define __RETRO_CVECTORELEMENT_HPP__

#include "IElement.hpp"

namespace Retro
{

class CVEKeyframeEmitter : public CVectorElement
{
    u32 x4_percent;
    u32 x8_unk1;
    bool xc_loop;
    bool xd_unk2;
    u32 x10_loopEnd;
    u32 x14_loopStart;
    std::vector<Zeus::CVector3f> x18_keys;
public:
    CVEKeyframeEmitter(CInputStream& in);
    bool GetValue(int frame, Zeus::CVector3f& valOut) const;
};

class CVECone : public CVectorElement
{
    std::unique_ptr<CVectorElement> x4_a;
    std::unique_ptr<CRealElement> x8_b;
    Zeus::CVector3f xc_vec1;
    Zeus::CVector3f x18_vec2;
public:
    CVECone(CVectorElement* a, CRealElement* b);
    bool GetValue(int frame, Zeus::CVector3f& valOut) const;
};

class CVETimeChain : public CVectorElement
{
    std::unique_ptr<CVectorElement> x4_a;
    std::unique_ptr<CVectorElement> x8_b;
    std::unique_ptr<CIntElement> xc_c;
public:
    CVETimeChain(CVectorElement* a, CVectorElement* b, CIntElement* c)
    : x4_a(a), x8_b(b), xc_c(c) {}
    bool GetValue(int frame, Zeus::CVector3f& valOut) const;
};

class CVEAngleCone : public CVectorElement
{
    std::unique_ptr<CRealElement> x4_a;
    std::unique_ptr<CRealElement> x8_b;
    std::unique_ptr<CRealElement> xc_c;
    std::unique_ptr<CRealElement> x10_d;
    std::unique_ptr<CRealElement> x14_e;
public:
    CVEAngleCone(CRealElement* a, CRealElement* b, CRealElement* c, CRealElement* d, CRealElement* e)
    : x4_a(a), x8_b(b), xc_c(c), x10_d(d), x14_e(e) {}
    bool GetValue(int frame, Zeus::CVector3f& valOut) const;
};

class CVEAdd : public CVectorElement
{
    std::unique_ptr<CVectorElement> x4_a;
    std::unique_ptr<CVectorElement> x8_b;
public:
    CVEAdd(CVectorElement* a, CVectorElement* b)
    : x4_a(a), x8_b(b) {}
    bool GetValue(int frame, Zeus::CVector3f& valOut) const;
};

class CVECircleCluster : public CVectorElement
{
    std::unique_ptr<CVectorElement> x4_a;
    std::unique_ptr<CVectorElement> x8_b;
    std::unique_ptr<CIntElement> xc_c;
    std::unique_ptr<CRealElement> x10_d;
public:
    CVECircleCluster(CVectorElement* a, CVectorElement* b, CIntElement* c, CRealElement* d)
    : x4_a(a), x8_b(b), xc_c(c), x10_d(d) {}
    bool GetValue(int frame, Zeus::CVector3f& valOut) const;
};

class CVEConstant : public CVectorElement
{
    std::unique_ptr<CRealElement> x4_a;
    std::unique_ptr<CRealElement> x8_b;
    std::unique_ptr<CRealElement> xc_c;
public:
    CVEConstant(CRealElement* a, CRealElement* b, CRealElement* c)
    : x4_a(a), x8_b(b), xc_c(c) {}
    bool GetValue(int frame, Zeus::CVector3f& valOut) const;
};

class CVEFastConstant : public CVectorElement
{
    Zeus::CVector3f x4_val;
public:
    CVEFastConstant(float a, float b, float c) : x4_val(a, b, c) {}
    bool GetValue(int frame, Zeus::CVector3f& valOut) const;
    bool IsFastConstant() const {return true;}
};

class CVECircle : public CVectorElement
{
    std::unique_ptr<CVectorElement> x4_a;
    std::unique_ptr<CVectorElement> x8_b;
    std::unique_ptr<CRealElement> xc_c;
    std::unique_ptr<CRealElement> x10_d;
    std::unique_ptr<CRealElement> x14_e;
public:
    CVECircle(CVectorElement* a, CVectorElement* b, CRealElement* c, CRealElement* d, CRealElement* e)
    : x4_a(a), x8_b(b), xc_c(c), x10_d(d), x14_e(e) {}
    bool GetValue(int frame, Zeus::CVector3f& valOut) const;
};

class CVEMultiply : public CVectorElement
{
    std::unique_ptr<CVectorElement> x4_a;
    std::unique_ptr<CVectorElement> x8_b;
public:
    CVEMultiply(CVectorElement* a, CVectorElement* b)
    : x4_a(a), x8_b(b) {}
    bool GetValue(int frame, Zeus::CVector3f& valOut) const;
};

class CVERealToVector : public CVectorElement
{
    std::unique_ptr<CRealElement> x4_a;
public:
    CVERealToVector(CRealElement* a)
    : x4_a(a) {}
    bool GetValue(int frame, Zeus::CVector3f& valOut) const;
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
    bool GetValue(int frame, Zeus::CVector3f& valOut) const;
};

class CVEParticleVelocity : public CVectorElement
{
public:
    bool GetValue(int frame, Zeus::CVector3f& valOut) const;
};

class CVESPOS : public CVectorElement
{
    std::unique_ptr<CVectorElement> x4_a;
public:
    CVESPOS(CVectorElement* a)
    : x4_a(a) {}
    bool GetValue(int frame, Zeus::CVector3f& valOut) const;
};

class CVEPLCO : public CVectorElement
{
public:
    bool GetValue(int frame, Zeus::CVector3f& valOut) const;
};

class CVEPLOC : public CVectorElement
{
public:
    bool GetValue(int frame, Zeus::CVector3f& valOut) const;
};

class CVEPSOR : public CVectorElement
{
public:
    bool GetValue(int frame, Zeus::CVector3f& valOut) const;
};

class CVEPSOF : public CVectorElement
{
public:
    bool GetValue(int frame, Zeus::CVector3f& valOut) const;
};

}

#endif // __RETRO_CVECTORELEMENT_HPP__
