#ifndef __PSHAG_IELEMENT_HPP__
#define __PSHAG_IELEMENT_HPP__

#include <memory>
#include "GCNTypes.hpp"
#include "CVector3f.hpp"
#include "CColor.hpp"
#include "IOStreams.hpp"

namespace pshag
{

class IElement
{
public:
    virtual ~IElement() = default;
};

class CRealElement : public IElement
{
public:
    virtual bool GetValue(int frame, float& valOut) const=0;
    virtual bool IsConstant() const {return false;}
};

class CIntElement : public IElement
{
public:
    virtual bool GetValue(int frame, int& valOut) const=0;
};

class CVectorElement : public IElement
{
public:
    virtual bool GetValue(int frame, Zeus::CVector3f& valOut) const=0;
    virtual bool IsFastConstant() const {return false;}
};

class CModVectorElement : public IElement
{
public:
    virtual bool GetValue(int frame, Zeus::CVector3f& pVel, Zeus::CVector3f& pPos) const=0;
};

class CColorElement : public IElement
{
public:
    virtual bool GetValue(int frame, Zeus::CColor& colorOut) const=0;
};

class CEmitterElement : public IElement
{
public:
    virtual bool GetValue(int frame, Zeus::CVector3f& pPos, Zeus::CVector3f& pVel) const=0;
};

}

#endif // __PSHAG_IELEMENT_HPP__
