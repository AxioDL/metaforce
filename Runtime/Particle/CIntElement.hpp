#ifndef __RETRO_CINTELEMENT_HPP__
#define __RETRO_CINTELEMENT_HPP__

#include "IElement.hpp"

namespace Retro
{

class CIEKeyframeEmitter : public CIntElement
{
    u32 x4_percent;
    u32 x8_unk1;
    bool xc_loop;
    bool xd_unk2;
    u32 x10_loopEnd;
    u32 x14_loopStart;
    std::vector<int> x18_keys;
public:
    CIEKeyframeEmitter(CInputStream& in);
    bool GetValue(int frame, int& valOut) const;
};

class CIEDeath : public CIntElement
{
    std::unique_ptr<CIntElement> x4_a;
    std::unique_ptr<CIntElement> x8_b;
public:
    CIEDeath(CIntElement* a, CIntElement* b)
    : x4_a(a), x8_b(b) {}
    bool GetValue(int frame, int& valOut) const;
};

class CIEClamp : public CIntElement
{
    std::unique_ptr<CIntElement> x4_min;
    std::unique_ptr<CIntElement> x8_max;
    std::unique_ptr<CIntElement> xc_val;
public:
    CIEClamp(CIntElement* a, CIntElement* b, CIntElement* c)
    : x4_min(a), x8_max(b), xc_val(c) {}
    bool GetValue(int frame, int& valOut) const;
};

class CIETimeChain : public CIntElement
{
    std::unique_ptr<CIntElement> x4_a;
    std::unique_ptr<CIntElement> x8_b;
    std::unique_ptr<CIntElement> xc_c;
public:
    CIETimeChain(CIntElement* a, CIntElement* b, CIntElement* c)
    : x4_a(a), x8_b(b), xc_c(c) {}
    bool GetValue(int frame, int& valOut) const;
};

class CIEAdd : public CIntElement
{
    std::unique_ptr<CIntElement> x4_a;
    std::unique_ptr<CIntElement> x8_b;
public:
    CIEAdd(CIntElement* a, CIntElement* b)
    : x4_a(a), x8_b(b) {}
    bool GetValue(int frame, int& valOut) const;
};

class CIEConstant : public CIntElement
{
    int x4_val;
public:
    CIEConstant(int val) : x4_val(val) {}
    bool GetValue(int frame, int& valOut) const;
};

class CIEImpulse : public CIntElement
{
    std::unique_ptr<CIntElement> x4_a;
public:
    CIEImpulse(CIntElement* a)
    : x4_a(a) {}
    bool GetValue(int frame, int& valOut) const;
};

class CIELifetimePercent : public CIntElement
{
    std::unique_ptr<CIntElement> x4_a;
public:
    CIELifetimePercent(CIntElement* a)
    : x4_a(a) {}
    bool GetValue(int frame, int& valOut) const;
};

class CIEInitialRandom : public CIntElement
{
    std::unique_ptr<CIntElement> x4_a;
    std::unique_ptr<CIntElement> x8_b;
public:
    CIEInitialRandom(CIntElement* a, CIntElement* b)
    : x4_a(a), x8_b(b) {}
    bool GetValue(int frame, int& valOut) const;
};

class CIEPulse : public CIntElement
{
    std::unique_ptr<CIntElement> x4_a;
    std::unique_ptr<CIntElement> x8_b;
    std::unique_ptr<CIntElement> xc_c;
    std::unique_ptr<CIntElement> x10_d;
public:
    CIEPulse(CIntElement* a, CIntElement* b, CIntElement* c, CIntElement* d)
    : x4_a(a), x8_b(b), xc_c(c), x10_d(d) {}
    bool GetValue(int frame, int& valOut) const;
};

class CIEMultiply : public CIntElement
{
    std::unique_ptr<CIntElement> x4_a;
    std::unique_ptr<CIntElement> x8_b;
public:
    CIEMultiply(CIntElement* a, CIntElement* b)
    : x4_a(a), x8_b(b) {}
    bool GetValue(int frame, int& valOut) const;
};

class CIESampleAndHold : public CIntElement
{
    std::unique_ptr<CIntElement> x4_a;
    std::unique_ptr<CIntElement> x8_b;
    std::unique_ptr<CIntElement> xc_c;
public:
    CIESampleAndHold(CIntElement* a, CIntElement* b, CIntElement* c)
    : x4_a(a), x8_b(b), xc_c(c) {}
    bool GetValue(int frame, int& valOut) const;
};

class CIERandom : public CIntElement
{
    std::unique_ptr<CIntElement> x4_a;
    std::unique_ptr<CIntElement> x8_b;
public:
    CIERandom(CIntElement* a, CIntElement* b)
    : x4_a(a), x8_b(b) {}
    bool GetValue(int frame, int& valOut) const;
};

class CIETimeScale : public CIntElement
{
    std::unique_ptr<CRealElement> x4_a;
public:
    CIETimeScale(CRealElement* a)
    : x4_a(a) {}
    bool GetValue(int frame, int& valOut) const;
};

class CIEGTCP : public CIntElement
{
public:
    bool GetValue(int frame, int& valOut) const;
};

class CIEModulo : public CIntElement
{
    std::unique_ptr<CIntElement> x4_a;
    std::unique_ptr<CIntElement> x8_b;
public:
    CIEModulo(CIntElement* a, CIntElement* b)
    : x4_a(a), x8_b(b) {}
    bool GetValue(int frame, int& valOut) const;
};

class CIESubtract : public CIntElement
{
    std::unique_ptr<CIntElement> x4_a;
    std::unique_ptr<CIntElement> x8_b;
public:
    CIESubtract(CIntElement* a, CIntElement* b)
    : x4_a(a), x8_b(b) {}
    bool GetValue(int frame, int& valOut) const;
};

}

#endif // __RETRO_CINTELEMENT_HPP__
