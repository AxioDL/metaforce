#ifndef __PSHAG_CTEXTURE_HPP__
#define __PSHAG_CTEXTURE_HPP__

#include "GCNTypes.hpp"
#include "IObj.hpp"
#include "IOStreams.hpp"
#include "Graphics/CGraphics.hpp"
#include "boo/graphicsdev/IGraphicsDataFactory.hpp"

namespace urde
{
class CVParamTransfer;

class CTexture
{
    ETexelFormat x0_fmt;
    u16 x4_w;
    u16 x6_h;
    u32 x8_mips;
    boo::GraphicsDataToken m_booToken;
    boo::ITexture* m_booTex;

    size_t ComputeMippedTexelCount();
    size_t ComputeMippedBlockCountDXT1();
    void BuildI4FromGCN(CInputStream& in);
    void BuildI8FromGCN(CInputStream& in);
    void BuildIA4FromGCN(CInputStream& in);
    void BuildIA8FromGCN(CInputStream& in);
    void BuildC4FromGCN(CInputStream& in);
    void BuildC8FromGCN(CInputStream& in);
    void BuildC14X2FromGCN(CInputStream& in);
    void BuildRGB565FromGCN(CInputStream& in);
    void BuildRGB5A3FromGCN(CInputStream& in);
    void BuildRGBA8FromGCN(CInputStream& in);
    void BuildDXT1FromGCN(CInputStream& in);

public:
    CTexture(CInputStream& in);
    enum class EClampMode
    {
        None,
        One
    };
    ETexelFormat GetTexelFormat() const {return x0_fmt;}
    u16 GetWidth() const {return x4_w;}
    u16 GetHeight() const {return x6_h;}
    void Load(int slot, EClampMode clamp);
    boo::ITexture* GetBooTexture() {return m_booTex;}
};

std::unique_ptr<IObj> FTextureFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& vparms);

}

#endif // __PSHAG_CTEXTURE_HPP__
