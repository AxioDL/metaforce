#pragma once

#include "IElement.hpp"

/* Documentation at: http://www.metroid2002.com/retromodding/wiki/Particle_Script#Color_Elements */

namespace urde
{

class CCEKeyframeEmitter : public CColorElement
{
    u32 x4_percent;
    u32 x8_unk1;
    bool xc_loop;
    bool xd_unk2;
    s32 x10_loopEnd;
    s32 x14_loopStart;
    std::vector<zeus::CColor> x18_keys;
public:
    CCEKeyframeEmitter(CInputStream& in);
    bool GetValue(int frame, zeus::CColor& colorOut) const;
};

class CCEConstant : public CColorElement
{
    std::unique_ptr<CRealElement> x4_r;
    std::unique_ptr<CRealElement> x8_g;
    std::unique_ptr<CRealElement> xc_b;
    std::unique_ptr<CRealElement> x10_a;
public:
    CCEConstant(std::unique_ptr<CRealElement>&& a, std::unique_ptr<CRealElement>&& b,
                std::unique_ptr<CRealElement>&& c, std::unique_ptr<CRealElement>&& d)
    : x4_r(std::move(a)), x8_g(std::move(b)), xc_b(std::move(c)), x10_a(std::move(d)) {}
    bool GetValue(int frame, zeus::CColor& colorOut) const;
};

class CCEFastConstant : public CColorElement
{
    zeus::CColor x4_val;
public:
    CCEFastConstant(float a, float b, float c, float d)
    : x4_val(a, b, c, d) {}
    bool GetValue(int frame, zeus::CColor& colorOut) const;
};

class CCETimeChain : public CColorElement
{
    std::unique_ptr<CColorElement> x4_a;
    std::unique_ptr<CColorElement> x8_b;
    std::unique_ptr<CIntElement> xc_swFrame;
public:
    CCETimeChain(std::unique_ptr<CColorElement>&& a, std::unique_ptr<CColorElement>&& b,
                 std::unique_ptr<CIntElement>&& c)
    : x4_a(std::move(a)), x8_b(std::move(b)), xc_swFrame(std::move(c)) {}
    bool GetValue(int frame, zeus::CColor& colorOut) const;
};

class CCEFadeEnd : public CColorElement
{
    std::unique_ptr<CColorElement> x4_a;
    std::unique_ptr<CColorElement> x8_b;
    std::unique_ptr<CRealElement> xc_startFrame;
    std::unique_ptr<CRealElement> x10_endFrame;
public:
    CCEFadeEnd(std::unique_ptr<CColorElement>&& a, std::unique_ptr<CColorElement>&& b,
               std::unique_ptr<CRealElement>&& c, std::unique_ptr<CRealElement>&& d)
    : x4_a(std::move(a)), x8_b(std::move(b)), xc_startFrame(std::move(c)), x10_endFrame(std::move(d)) {}
    bool GetValue(int frame, zeus::CColor& colorOut) const;
};

class CCEFade : public CColorElement
{
    std::unique_ptr<CColorElement> x4_a;
    std::unique_ptr<CColorElement> x8_b;
    std::unique_ptr<CRealElement> xc_endFrame;
public:
    CCEFade(std::unique_ptr<CColorElement>&& a, std::unique_ptr<CColorElement>&& b,
            std::unique_ptr<CRealElement>&& c)
    : x4_a(std::move(a)), x8_b(std::move(b)), xc_endFrame(std::move(c)) {}
    bool GetValue(int frame, zeus::CColor& colorOut) const;
};

class CCEPulse : public CColorElement
{
    std::unique_ptr<CIntElement> x4_aDuration;
    std::unique_ptr<CIntElement> x8_bDuration;
    std::unique_ptr<CColorElement> xc_aVal;
    std::unique_ptr<CColorElement> x10_bVal;
public:
    CCEPulse(std::unique_ptr<CIntElement>&& a, std::unique_ptr<CIntElement>&& b,
             std::unique_ptr<CColorElement>&& c, std::unique_ptr<CColorElement>&& d)
    : x4_aDuration(std::move(a)), x8_bDuration(std::move(b)), xc_aVal(std::move(c)), x10_bVal(std::move(d)) {}
    bool GetValue(int frame, zeus::CColor& colorOut) const;
};

class CCEParticleColor : public CColorElement
{
public:
    bool GetValue(int frame, zeus::CColor& colorOut) const;
};
}

