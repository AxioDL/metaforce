#ifndef __RETRO_CUVELEMENT_HPP__
#define __RETRO_CUVELEMENT_HPP__

#include "IElement.hpp"
#include "CToken.hpp"
#include "CTexture.hpp"

namespace Retro
{
class CToken;

struct SUVElementTexture
{
    TToken<CTexture> m_tex;
    CTexture* m_directTex;
    SUVElementTexture(TToken<CTexture>&& tex, CTexture* directTex)
    : m_tex(std::move(tex)), m_directTex(directTex) {}
};

struct SUVElementSet
{
    float xMin, yMin, xMax, yMax;
};

class CUVElement : public IElement
{
public:
    virtual SUVElementTexture GetValueTexture(int frame) const=0;
    virtual void GetValueUV(int frame, SUVElementSet& valOut) const=0;
    virtual bool HasConstantTexture() const=0;
    virtual bool HasConstantUV() const=0;
};

struct CUVEConstant : public CUVElement
{
    TToken<CTexture> x4_tex;
    CTexture* xc_directTex;
public:
    CUVEConstant(TToken<CTexture>&& tex)
    : x4_tex(std::move(tex)), xc_directTex(x4_tex.GetObj()) {}
    SUVElementTexture GetValueTexture(int frame) const
    {
        return SUVElementTexture(TLockedToken<CTexture>(x4_tex), xc_directTex);
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
    TToken<CTexture> x4_tex;
    CTexture* xc_directTex;
    int x10_tileW, x14_tileH, x18_strideW, x1c_strideH;
    int x20_tiles;
    bool x24_loop;
    std::unique_ptr<CIntElement> x28_cycleFrameRate;
    std::vector<SUVElementSet> x2c_uvElems;
public:
    CUVEAnimTexture(TToken<CTexture>&& tex, CIntElement* a, CIntElement* b,
                    CIntElement* c, CIntElement* d, CIntElement* e, bool f);
    SUVElementTexture GetValueTexture(int frame) const
    {
        return SUVElementTexture(TLockedToken<CTexture>(x4_tex), xc_directTex);
    }
    void GetValueUV(int frame, SUVElementSet& valOut) const;
    bool HasConstantTexture() const {return true;}
    bool HasConstantUV() const {return false;}
};

}

#endif // __RETRO_CUVELEMENT_HPP__
