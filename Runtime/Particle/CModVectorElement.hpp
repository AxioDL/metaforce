#ifndef __RETRO_CMODVECTORELEMENT_HPP__
#define __RETRO_CMODVECTORELEMENT_HPP__

#include "IElement.hpp"

namespace Retro
{

class CMVEImplosion : public CModVectorElement
{
public:
    bool GetValue(int frame, Zeus::CVector3f& vec1Out, Zeus::CVector3f& vec2Out) const=0;
};

class CMVEExponentialImplosion : public CModVectorElement
{
public:
    bool GetValue(int frame, Zeus::CVector3f& vec1Out, Zeus::CVector3f& vec2Out) const=0;
};

class CMVETimeChain : public CModVectorElement
{
public:
    bool GetValue(int frame, Zeus::CVector3f& vec1Out, Zeus::CVector3f& vec2Out) const=0;
};

class CMVEBounce : public CModVectorElement
{
public:
    bool GetValue(int frame, Zeus::CVector3f& vec1Out, Zeus::CVector3f& vec2Out) const=0;
};

class CMVEConstant : public CModVectorElement
{
public:
    bool GetValue(int frame, Zeus::CVector3f& vec1Out, Zeus::CVector3f& vec2Out) const=0;
};

class CMVEFastConstant : public CModVectorElement
{
public:
    bool GetValue(int frame, Zeus::CVector3f& vec1Out, Zeus::CVector3f& vec2Out) const=0;
};

class CMVEGravity : public CModVectorElement
{
public:
    bool GetValue(int frame, Zeus::CVector3f& vec1Out, Zeus::CVector3f& vec2Out) const=0;
};

class CMVEExplode : public CModVectorElement
{
public:
    bool GetValue(int frame, Zeus::CVector3f& vec1Out, Zeus::CVector3f& vec2Out) const=0;
};

class CMVESetPosition : public CModVectorElement
{
public:
    bool GetValue(int frame, Zeus::CVector3f& vec1Out, Zeus::CVector3f& vec2Out) const=0;
};

class CMVELinearImplosion : public CModVectorElement
{
public:
    bool GetValue(int frame, Zeus::CVector3f& vec1Out, Zeus::CVector3f& vec2Out) const=0;
};

class CMVEPulse : public CModVectorElement
{
public:
    bool GetValue(int frame, Zeus::CVector3f& vec1Out, Zeus::CVector3f& vec2Out) const=0;
};

class CMVEWind : public CModVectorElement
{
public:
    bool GetValue(int frame, Zeus::CVector3f& vec1Out, Zeus::CVector3f& vec2Out) const=0;
};

class CMVESwirl : public CModVectorElement
{
public:
    bool GetValue(int frame, Zeus::CVector3f& vec1Out, Zeus::CVector3f& vec2Out) const=0;
};

}

#endif // __RETRO_CMODVECTORELEMENT_HPP__
