#ifndef __RETRO_CUVELEMENT_HPP__
#define __RETRO_CUVELEMENT_HPP__

#include "IElement.hpp"

namespace Retro
{
class CToken;

struct SUVElementSet
{
    float xMin, yMin, xMax, yMax;
};

class CUVElement : public IElement
{
public:
    virtual CToken GetValueTexture(int frame) const=0;
    virtual void GetValueUV(int frame, SUVElementSet& valOut) const=0;
};

struct CUVEConstant : public CUVElement
{
public:
    CToken GetValueTexture(int frame) const;
    void GetValueUV(int frame, SUVElementSet& valOut) const;
};

struct CUVEAnimTexture : public CUVElement
{
public:
    CToken GetValueTexture(int frame) const;
    void GetValueUV(int frame, SUVElementSet& valOut) const;
};

}

#endif // __RETRO_CUVELEMENT_HPP__
