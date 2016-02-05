#ifndef __RETRO_CINTELEMENT_HPP__
#define __RETRO_CINTELEMENT_HPP__

#include "IElement.hpp"

namespace Retro
{

class CIEKeyframeEmitter : public CIntElement
{
public:
    bool GetValue(int frame, int& valOut) const;
};

class CIEDeath : public CIntElement
{
public:
    bool GetValue(int frame, int& valOut) const;
};

class CIEClamp : public CIntElement
{
public:
    bool GetValue(int frame, int& valOut) const;
};

class CIETimeChain : public CIntElement
{
public:
    bool GetValue(int frame, int& valOut) const;
};

class CIEAdd : public CIntElement
{
public:
    bool GetValue(int frame, int& valOut) const;
};

class CIEConstant : public CIntElement
{
public:
    bool GetValue(int frame, int& valOut) const;
};

class CIEImpulse : public CIntElement
{
public:
    bool GetValue(int frame, int& valOut) const;
};

class CIELifetimePercent : public CIntElement
{
public:
    bool GetValue(int frame, int& valOut) const;
};

class CIEInitialRandom : public CIntElement
{
public:
    bool GetValue(int frame, int& valOut) const;
};

class CIEPulse : public CIntElement
{
public:
    bool GetValue(int frame, int& valOut) const;
};

class CIEMultiply : public CIntElement
{
public:
    bool GetValue(int frame, int& valOut) const;
};

class CIESampleAndHold : public CIntElement
{
public:
    bool GetValue(int frame, int& valOut) const;
};

class CIERandom : public CIntElement
{
public:
    bool GetValue(int frame, int& valOut) const;
};

class CIETimeScale : public CIntElement
{
public:
    bool GetValue(int frame, int& valOut) const;
};

class CIEGTCP : public CIntElement
{
public:
    bool GetValue(int frame, int& valOut) const;
};

class CIEModulo : public CIntElement
{
public:
    bool GetValue(int frame, int& valOut) const;
};

class CIESubtract : public CIntElement
{
public:
    bool GetValue(int frame, int& valOut) const;
};

}

#endif // __RETRO_CINTELEMENT_HPP__
