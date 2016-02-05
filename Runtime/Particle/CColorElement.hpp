#ifndef __RETRO_CCOLORELEMENT_HPP__
#define __RETRO_CCOLORELEMENT_HPP__

#include "IElement.hpp"

namespace Retro
{

class CCEKeyframeEmitter : public CColorElement
{
public:
    bool GetValue(int frame, Zeus::CColor& colorOut) const;
};

class CCEConstant : public CColorElement
{
public:
    bool GetValue(int frame, Zeus::CColor& colorOut) const;
};

class CCEFastConstant : public CColorElement
{
public:
    bool GetValue(int frame, Zeus::CColor& colorOut) const;
};

class CCETimeChain : public CColorElement
{
public:
    bool GetValue(int frame, Zeus::CColor& colorOut) const;
};

class CCEFadeEnd : public CColorElement
{
public:
    bool GetValue(int frame, Zeus::CColor& colorOut) const;
};

class CCEFade : public CColorElement
{
public:
    bool GetValue(int frame, Zeus::CColor& colorOut) const;
};

class CCEPulse : public CColorElement
{
public:
    bool GetValue(int frame, Zeus::CColor& colorOut) const;
};

}

#endif // __RETRO_CCOLORELEMENT_HPP__
