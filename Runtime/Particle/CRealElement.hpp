#ifndef __RETRO_CREALELEMENT_HPP__
#define __RETRO_CREALELEMENT_HPP__

#include "IElement.hpp"

namespace Retro
{

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
    std::unique_ptr<CIntElement> xc_c;
public:
    CRETimeChain(CRealElement* a, CRealElement* b, CIntElement* c)
    : x4_a(a), x8_b(b), xc_c(c) {}
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
    std::unique_ptr<CRealElement> x4_a;
    std::unique_ptr<CRealElement> x8_b;
    std::unique_ptr<CRealElement> xc_c;
public:
    CREClamp(CRealElement* a, CRealElement* b, CRealElement* c)
    : x4_a(a), x8_b(b), xc_c(c) {}
    bool GetValue(int frame, float& valOut) const;
};

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

class CREInitialRandom : public CRealElement
{
public:
    bool GetValue(int frame, float& valOut) const;
    bool IsConstant() const {return true;}
};

class CRERandom : public CRealElement
{
public:
    bool GetValue(int frame, float& valOut) const;
};

class CREMultiply : public CRealElement
{
public:
    bool GetValue(int frame, float& valOut) const;
};

class CREPulse : public CRealElement
{
public:
    bool GetValue(int frame, float& valOut) const;
};

class CRETimeScale : public CRealElement
{
public:
    bool GetValue(int frame, float& valOut) const;
};

class CRELifetimePercent : public CRealElement
{
public:
    bool GetValue(int frame, float& valOut) const;
};

class CRESineWave : public CRealElement
{
public:
    bool GetValue(int frame, float& valOut) const;
};

class CREISWT : public CRealElement
{
public:
    bool GetValue(int frame, float& valOut) const;
};

class CRECompareLessThan : public CRealElement
{
public:
    bool GetValue(int frame, float& valOut) const;
};

class CRECompareEquals : public CRealElement
{
public:
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

class CREPSOF : public CRealElement
{
public:
    bool GetValue(int frame, float& valOut) const;
};

class CRESubtract : public CRealElement
{
public:
    bool GetValue(int frame, float& valOut) const;
};

class CREVectorMagnitude : public CRealElement
{
public:
    bool GetValue(int frame, float& valOut) const;
};

class CREVectorXToReal : public CRealElement
{
public:
    bool GetValue(int frame, float& valOut) const;
};

class CREVectorYToReal : public CRealElement
{
public:
    bool GetValue(int frame, float& valOut) const;
};

class CREVectorZToReal : public CRealElement
{
public:
    bool GetValue(int frame, float& valOut) const;
};

class CRECEXT : public CRealElement
{
public:
    bool GetValue(int frame, float& valOut) const;
};

class CREITRL : public CRealElement
{
public:
    bool GetValue(int frame, float& valOut) const;
};

}

#endif // __RETRO_CREALELEMENT_HPP__
