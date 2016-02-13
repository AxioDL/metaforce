#ifndef __PSHAG_CUVELEMENT_HPP__
#define __PSHAG_CUVELEMENT_HPP__

#include "IElement.hpp"
#include "CToken.hpp"
#include "CTexture.hpp"

namespace pshag
{
class CToken;

struct SUVElementSet
{
    float xMin, yMin, xMax, yMax;
};

class CUVElement : public IElement
{
public:
    virtual TLockedToken<CTexture> GetValueTexture(int frame) const=0;
    virtual void GetValueUV(int frame, SUVElementSet& valOut) const=0;
    virtual bool HasConstantTexture() const=0;
    virtual bool HasConstantUV() const=0;
};

struct CUVEConstant : public CUVElement
{
    TLockedToken<CTexture> x4_tex;
public:
    CUVEConstant(TToken<CTexture>&& tex)
    : x4_tex(std::move(tex)) {}
    TLockedToken<CTexture> GetValueTexture(int frame) const
    {
        return TLockedToken<CTexture>(x4_tex);
    }
    void GetValueUV(int frame, SUVElementSet& valOut) const
    {
        valOut = {0.f, 0.f, 1.f, 1.f};
    }
    bool HasConstantTexture() const {return true;}
    bool HasConstantUV() const {return true;}
};

struct CUVEAnimTexture : public CUVElement
{
    TLockedToken<CTexture> x4_tex;
    int x10_tileW, x14_tileH, x18_strideW, x1c_strideH;
    int x20_tiles;
    bool x24_loop;
    std::unique_ptr<CIntElement> x28_cycleFrameRate;
    std::vector<SUVElementSet> x2c_uvElems;
public:
    CUVEAnimTexture(TToken<CTexture>&& tex, CIntElement* a, CIntElement* b,
                    CIntElement* c, CIntElement* d, CIntElement* e, bool f);
    TLockedToken<CTexture> GetValueTexture(int frame) const
    {
        return TLockedToken<CTexture>(x4_tex);
    }
    void GetValueUV(int frame, SUVElementSet& valOut) const;
    bool HasConstantTexture() const {return true;}
    bool HasConstantUV() const {return false;}
};

}

#endif // __PSHAG_CUVELEMENT_HPP__
