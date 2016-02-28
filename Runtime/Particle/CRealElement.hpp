#ifndef __PSHAG_CREALELEMENT_HPP__
#define __PSHAG_CREALELEMENT_HPP__

#include "IElement.hpp"

/* Documentation at: http://www.metroid2002.com/retromodding/wiki/Particle_Script#Real_Elements */

namespace pshag
{

class CREKeyframeEmitter : public CRealElement
{
    u32 x4_percent;
    u32 x8_unk1;
    bool xc_loop;
    bool xd_unk2;
    u32 x10_loopEnd;
    u32 x14_loopStart;
    std::vector<float> x18_keys;
public:
    CREKeyframeEmitter(CInputStream& in);
    bool GetValue(int frame, float& valOut) const;
};

class CRELifetimeTween : public CRealElement
{
    std::unique_ptr<CRealElement> x4_a;
    std::unique_ptr<CRealElement> x8_b;
public:
    CRELifetimeTween(CRealElement* a, CRealElement* b)
    : x4_a(a), x8_b(b) {}
    bool GetValue(int frame, float& valOut) const;
};

class CREConstant : public CRealElement
{
    float x4_val;
public:
    CREConstant(float val) : x4_val(val) {}
    bool GetValue(int frame, float& valOut) const;
    bool IsConstant() const {return true;}
};

class CRETimeChain : public CRealElement
{
    std::unique_ptr<CRealElement> x4_a;
    std::unique_ptr<CRealElement> x8_b;
    std::unique_ptr<CIntElement> xc_swFrame;
public:
    CRETimeChain(CRealElement* a, CRealElement* b, CIntElement* c)
    : x4_a(a), x8_b(b), xc_swFrame(c) {}
    bool GetValue(int frame, float& valOut) const;
};

class CREAdd : public CRealElement
{
    std::unique_ptr<CRealElement> x4_a;
    std::unique_ptr<CRealElement> x8_b;
public:
    CREAdd(CRealElement* a, CRealElement* b)
    : x4_a(a), x8_b(b) {}
    bool GetValue(int frame, float& valOut) const;
};

class CREClamp : public CRealElement
{
    std::unique_ptr<CRealElement> x4_min;
    std::unique_ptr<CRealElement> x8_max;
    std::unique_ptr<CRealElement> xc_val;
public:
    CREClamp(CRealElement* a, CRealElement* b, CRealElement* c)
    : x4_min(a), x8_max(b), xc_val(c) {}
    bool GetValue(int frame, float& valOut) const;
};

class CREInitialRandom : public CRealElement
{
    std::unique_ptr<CRealElement> x4_min;
    std::unique_ptr<CRealElement> x8_max;
public:
    CREInitialRandom(CRealElement* a, CRealElement* b)
    : x4_min(a), x8_max(b) {}
    bool GetValue(int frame, float& valOut) const;
    bool IsConstant() const {return true;}
};

class CRERandom : public CRealElement
{
    std::unique_ptr<CRealElement> x4_min;
    std::unique_ptr<CRealElement> x8_max;
public:
    CRERandom(CRealElement* a, CRealElement* b)
    : x4_min(a), x8_max(b) {}
    bool GetValue(int frame, float& valOut) const;
};

class CREDotProduct : public CRealElement
{
    std::unique_ptr<CVectorElement> x4_a;
    std::unique_ptr<CVectorElement> x8_b;
public:
    CREDotProduct(CVectorElement* a, CVectorElement* b)
        : x4_a(a), x8_b(b) {}
    bool GetValue(int frame, float& valOut) const;
};

class CREMultiply : public CRealElement
{
    std::unique_ptr<CRealElement> x4_a;
    std::unique_ptr<CRealElement> x8_b;
public:
    CREMultiply(CRealElement* a, CRealElement* b)
    : x4_a(a), x8_b(b) {}
    bool GetValue(int frame, float& valOut) const;
};

class CREPulse : public CRealElement
{
    std::unique_ptr<CIntElement> x4_aDuration;
    std::unique_ptr<CIntElement> x8_bDuration;
    std::unique_ptr<CRealElement> xc_valA;
    std::unique_ptr<CRealElement> x10_valB;
public:
    CREPulse(CIntElement* a, CIntElement* b, CRealElement* c, CRealElement* d)
    : x4_aDuration(a), x8_bDuration(b), xc_valA(c), x10_valB(d) {}
    bool GetValue(int frame, float& valOut) const;
};

class CRETimeScale : public CRealElement
{
    std::unique_ptr<CRealElement> x4_a;
public:
    CRETimeScale(CRealElement* a)
    : x4_a(a) {}
    bool GetValue(int frame, float& valOut) const;
};

class CRELifetimePercent : public CRealElement
{
    std::unique_ptr<CRealElement> x4_percentVal;
public:
    CRELifetimePercent(CRealElement* a)
    : x4_percentVal(a) {}
    bool GetValue(int frame, float& valOut) const;
};

class CRESineWave : public CRealElement
{
    std::unique_ptr<CRealElement> x4_magnitude;
    std::unique_ptr<CRealElement> x8_linearFrame;
    std::unique_ptr<CRealElement> xc_constantFrame;
public:
    CRESineWave(CRealElement* a, CRealElement* b, CRealElement* c)
    : x4_magnitude(a), x8_linearFrame(b), xc_constantFrame(c) {}
    bool GetValue(int frame, float& valOut) const;
};

class CREISWT : public CRealElement
{
    std::unique_ptr<CRealElement> x4_a;
    std::unique_ptr<CRealElement> x8_b;
public:
    CREISWT(CRealElement* a, CRealElement* b)
    : x4_a(a), x8_b(b) {}
    bool GetValue(int frame, float& valOut) const;
};

class CRECompareLessThan : public CRealElement
{
    std::unique_ptr<CRealElement> x4_a;
    std::unique_ptr<CRealElement> x8_b;
    std::unique_ptr<CRealElement> xc_c;
    std::unique_ptr<CRealElement> x10_d;
public:
    CRECompareLessThan(CRealElement* a, CRealElement* b, CRealElement* c, CRealElement* d)
    : x4_a(a), x8_b(b), xc_c(c), x10_d(d) {}
    bool GetValue(int frame, float& valOut) const;
};

class CRECompareEquals : public CRealElement
{
    std::unique_ptr<CRealElement> x4_a;
    std::unique_ptr<CRealElement> x8_b;
    std::unique_ptr<CRealElement> xc_c;
    std::unique_ptr<CRealElement> x10_d;
public:
    CRECompareEquals(CRealElement* a, CRealElement* b, CRealElement* c, CRealElement* d)
    : x4_a(a), x8_b(b), xc_c(c), x10_d(d) {}
    bool GetValue(int frame, float& valOut) const;
};

class CREParticleAccessParam1 : public CRealElement
{
public:
    bool GetValue(int frame, float& valOut) const;
};

class CREParticleAccessParam2 : public CRealElement
{
public:
    bool GetValue(int frame, float& valOut) const;
};

class CREParticleAccessParam3 : public CRealElement
{
public:
    bool GetValue(int frame, float& valOut) const;
};

class CREParticleAccessParam4 : public CRealElement
{
public:
    bool GetValue(int frame, float& valOut) const;
};

class CREParticleAccessParam5 : public CRealElement
{
public:
    bool GetValue(int frame, float& valOut) const;
};

class CREParticleAccessParam6 : public CRealElement
{
public:
    bool GetValue(int frame, float& valOut) const;
};

class CREParticleAccessParam7 : public CRealElement
{
public:
    bool GetValue(int frame, float& valOut) const;
};

class CREParticleAccessParam8 : public CRealElement
{
public:
    bool GetValue(int frame, float& valOut) const;
};

class CREPSLL : public CRealElement
{
public:
    bool GetValue(int frame, float& valOut) const;
};

class CREPRLW : public CRealElement
{
public:
    bool GetValue(int frame, float& valOut) const;
};

class CRESubtract : public CRealElement
{
    std::unique_ptr<CRealElement> x4_a;
    std::unique_ptr<CRealElement> x8_b;
public:
    CRESubtract(CRealElement* a, CRealElement* b)
    : x4_a(a), x8_b(b) {}
    bool GetValue(int frame, float& valOut) const;
};

class CREVectorMagnitude : public CRealElement
{
    std::unique_ptr<CVectorElement> x4_a;
public:
    CREVectorMagnitude(CVectorElement* a)
    : x4_a(a) {}
    bool GetValue(int frame, float& valOut) const;
};

class CREVectorXToReal : public CRealElement
{
    std::unique_ptr<CVectorElement> x4_a;
public:
    CREVectorXToReal(CVectorElement* a)
    : x4_a(a) {}
    bool GetValue(int frame, float& valOut) const;
};

class CREVectorYToReal : public CRealElement
{
    std::unique_ptr<CVectorElement> x4_a;
public:
    CREVectorYToReal(CVectorElement* a)
    : x4_a(a) {}
    bool GetValue(int frame, float& valOut) const;
};

class CREVectorZToReal : public CRealElement
{
    std::unique_ptr<CVectorElement> x4_a;
public:
    CREVectorZToReal(CVectorElement* a)
    : x4_a(a) {}
    bool GetValue(int frame, float& valOut) const;
};

class CRECEXT : public CRealElement
{
    std::unique_ptr<CIntElement> x4_a;
public:
    CRECEXT(CIntElement* a)
    : x4_a(a) {}
    bool GetValue(int frame, float& valOut) const;
};

class CREIntTimesReal : public CRealElement
{
    std::unique_ptr<CIntElement> x4_a;
    std::unique_ptr<CRealElement> x8_b;
public:
    CREIntTimesReal(CIntElement* a, CRealElement* b)
    : x4_a(a), x8_b(b) {}
    bool GetValue(int frame, float& valOut) const;
};

class CREConstantRange : public CRealElement
{
    std::unique_ptr<CRealElement> x4_val;
    std::unique_ptr<CRealElement> x8_min;
    std::unique_ptr<CRealElement> xc_max;
    std::unique_ptr<CRealElement> x10_inRange;
    std::unique_ptr<CRealElement> x14_outOfRange;
public:
    CREConstantRange(CRealElement* a, CRealElement* b, CRealElement* c, CRealElement* d, CRealElement* e)
        : x4_val(a), x8_min(b), xc_max(c), x10_inRange(d), x14_outOfRange(e) {}

    bool GetValue(int frame, float& valOut) const;
};

class CREGetComponentRed : public CRealElement
{
    std::unique_ptr<CColorElement> x4_a;
public:
    CREGetComponentRed(CColorElement* a)
        : x4_a(a) {}

    bool GetValue(int frame, float& valOut) const;
};

class CREGetComponentGreen : public CRealElement
{
    std::unique_ptr<CColorElement> x4_a;
public:
    CREGetComponentGreen(CColorElement* a)
        : x4_a(a) {}

    bool GetValue(int frame, float& valOut) const;
};

class CREGetComponentBlue : public CRealElement
{
    std::unique_ptr<CColorElement> x4_a;
public:
    CREGetComponentBlue(CColorElement* a)
        : x4_a(a) {}

    bool GetValue(int frame, float& valOut) const;
};

class CREGetComponentAlpha : public CRealElement
{
    std::unique_ptr<CColorElement> x4_a;
public:
    CREGetComponentAlpha(CColorElement* a)
        : x4_a(a) {}

    bool GetValue(int frame, float& valOut) const;
};
}

#endif // __PSHAG_CREALELEMENT_HPP__
