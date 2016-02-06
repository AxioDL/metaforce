#ifndef __RETRO_CMODVECTORELEMENT_HPP__
#define __RETRO_CMODVECTORELEMENT_HPP__

#include "IElement.hpp"

namespace Retro
{

class CMVEImplosion : public CModVectorElement
{
    std::unique_ptr<CVectorElement> x4_a;
    std::unique_ptr<CRealElement> x8_b;
    std::unique_ptr<CRealElement> xc_c;
    std::unique_ptr<CRealElement> x10_d;
    bool x14_e;
public:
    CMVEImplosion(CVectorElement* a, CRealElement* b, CRealElement* c, CRealElement* d, bool e)
    : x4_a(a), x8_b(b), xc_c(c), x10_d(d), x14_e(e) {}
    bool GetValue(int frame, Zeus::CVector3f& vec1Out, Zeus::CVector3f& vec2Out) const;
};

class CMVEExponentialImplosion : public CModVectorElement
{
    std::unique_ptr<CVectorElement> x4_a;
    std::unique_ptr<CRealElement> x8_b;
    std::unique_ptr<CRealElement> xc_c;
    std::unique_ptr<CRealElement> x10_d;
    bool x14_e;
public:
    CMVEExponentialImplosion(CVectorElement* a, CRealElement* b, CRealElement* c, CRealElement* d, bool e)
    : x4_a(a), x8_b(b), xc_c(c), x10_d(d), x14_e(e) {}
    bool GetValue(int frame, Zeus::CVector3f& vec1Out, Zeus::CVector3f& vec2Out) const;
};

class CMVETimeChain : public CModVectorElement
{
    std::unique_ptr<CModVectorElement> x4_a;
    std::unique_ptr<CModVectorElement> x8_b;
    std::unique_ptr<CIntElement> xc_c;
public:
    CMVETimeChain(CModVectorElement* a, CModVectorElement* b, CIntElement* c)
    : x4_a(a), x8_b(b), xc_c(c) {}
    bool GetValue(int frame, Zeus::CVector3f& vec1Out, Zeus::CVector3f& vec2Out) const;
};

class CMVEBounce : public CModVectorElement
{
    std::unique_ptr<CVectorElement> x4_a;
    std::unique_ptr<CVectorElement> x8_b;
    std::unique_ptr<CRealElement> xc_c;
    std::unique_ptr<CRealElement> x10_d;
    bool x14_e;
public:
    CMVEBounce(CVectorElement* a, CVectorElement* b, CRealElement* c, CRealElement* d, bool e)
    : x4_a(a), x8_b(b), xc_c(c), x10_d(d), x14_e(e) {}
    bool GetValue(int frame, Zeus::CVector3f& vec1Out, Zeus::CVector3f& vec2Out) const;
};

class CMVEConstant : public CModVectorElement
{
    std::unique_ptr<CRealElement> x4_a;
    std::unique_ptr<CRealElement> x8_b;
    std::unique_ptr<CRealElement> xc_c;
public:
    CMVEConstant(CRealElement* a, CRealElement* b, CRealElement* c)
    : x4_a(a), x8_b(b), xc_c(c) {}
    bool GetValue(int frame, Zeus::CVector3f& vec1Out, Zeus::CVector3f& vec2Out) const;
};

class CMVEFastConstant : public CModVectorElement
{
    Zeus::CVector3f x4_val;
public:
    CMVEFastConstant(float a, float b, float c)
    : x4_val(a, b, c) {}
    bool GetValue(int frame, Zeus::CVector3f& vec1Out, Zeus::CVector3f& vec2Out) const;
};

class CMVEGravity : public CModVectorElement
{
    std::unique_ptr<CVectorElement> x4_a;
public:
    CMVEGravity(CVectorElement* a)
    : x4_a(a) {}
    bool GetValue(int frame, Zeus::CVector3f& vec1Out, Zeus::CVector3f& vec2Out) const;
};

class CMVEExplode : public CModVectorElement
{
    std::unique_ptr<CRealElement> x4_a;
    std::unique_ptr<CRealElement> x8_b;
public:
    CMVEExplode(CRealElement* a, CRealElement* b)
    : x4_a(a), x8_b(b) {}
    bool GetValue(int frame, Zeus::CVector3f& vec1Out, Zeus::CVector3f& vec2Out) const;
};

class CMVESetPosition : public CModVectorElement
{
    std::unique_ptr<CVectorElement> x4_a;
public:
    CMVESetPosition(CVectorElement* a)
    : x4_a(a) {}
    bool GetValue(int frame, Zeus::CVector3f& vec1Out, Zeus::CVector3f& vec2Out) const;
};

class CMVELinearImplosion : public CModVectorElement
{
    std::unique_ptr<CVectorElement> x4_a;
    std::unique_ptr<CRealElement> x8_b;
    std::unique_ptr<CRealElement> xc_c;
    std::unique_ptr<CRealElement> x10_d;
    bool x14_e;
public:
    CMVELinearImplosion(CVectorElement* a, CRealElement* b, CRealElement* c, CRealElement* d, bool e)
    : x4_a(a), x8_b(b), xc_c(c), x10_d(d), x14_e(e) {}
    bool GetValue(int frame, Zeus::CVector3f& vec1Out, Zeus::CVector3f& vec2Out) const;
};

class CMVEPulse : public CModVectorElement
{
    std::unique_ptr<CIntElement> x4_a;
    std::unique_ptr<CIntElement> x8_b;
    std::unique_ptr<CModVectorElement> xc_c;
    std::unique_ptr<CModVectorElement> x10_d;
public:
    CMVEPulse(CIntElement* a, CIntElement* b, CModVectorElement* c, CModVectorElement* d)
    : x4_a(a), x8_b(b), xc_c(c), x10_d(d) {}
    bool GetValue(int frame, Zeus::CVector3f& vec1Out, Zeus::CVector3f& vec2Out) const;
};

class CMVEWind : public CModVectorElement
{
    std::unique_ptr<CVectorElement> x4_a;
    std::unique_ptr<CRealElement> x8_b;
public:
    CMVEWind(CVectorElement* a, CRealElement* b)
    : x4_a(a), x8_b(b) {}
    bool GetValue(int frame, Zeus::CVector3f& vec1Out, Zeus::CVector3f& vec2Out) const;
};

class CMVESwirl : public CModVectorElement
{
    std::unique_ptr<CVectorElement> x4_a;
    std::unique_ptr<CVectorElement> x8_b;
    std::unique_ptr<CRealElement> xc_c;
    std::unique_ptr<CRealElement> x10_d;
public:
    CMVESwirl(CVectorElement* a, CVectorElement* b, CRealElement* c, CRealElement* d)
    : x4_a(a), x8_b(b), xc_c(c), x10_d(d) {}
    bool GetValue(int frame, Zeus::CVector3f& vec1Out, Zeus::CVector3f& vec2Out) const;
};

}

#endif // __RETRO_CMODVECTORELEMENT_HPP__
