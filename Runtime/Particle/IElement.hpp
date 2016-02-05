#ifndef __RETRO_IELEMENT_HPP__
#define __RETRO_IELEMENT_HPP__

#include <memory>
#include "GCNTypes.hpp"
#include "CVector3f.hpp"
#include "CColor.hpp"
#include "IOStreams.hpp"

namespace Retro
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
    virtual bool GetValue(int frame, Zeus::CVector3f& vec1Out, Zeus::CVector3f& vec2Out) const=0;
};

class CColorElement : public IElement
{
public:
    virtual bool GetValue(int frame, Zeus::CColor& colorOut) const=0;
};

}

#endif // __RETRO_IELEMENT_HPP__
