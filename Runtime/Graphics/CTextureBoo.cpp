#include "CTexture.hpp"
#include "CSimplePool.hpp"
#include "CToken.hpp"
#include "Graphics/CGraphics.hpp"

namespace urde
{

/* GX uses this upsampling technique to prevent banding on downsampled texture formats */
static inline uint8_t Convert3To8(uint8_t v)
{
    /* Swizzle bits: 00000123 -> 12312312 */
    return (v << 5) | (v << 2) | (v >> 1);
}

static inline uint8_t Convert4To8(uint8_t v)
{
    /* Swizzle bits: 00001234 -> 12341234 */
    return (v << 4) | v;
}

static inline uint8_t Convert5To8(uint8_t v)
{
    /* Swizzle bits: 00012345 -> 12345123 */
    return (v << 3) | (v >> 2);
}

static inline uint8_t Convert6To8(uint8_t v)
{
    /* Swizzle bits: 00123456 -> 12345612 */
    return (v << 2) | (v >> 4);
}

size_t CTexture::ComputeMippedTexelCount()
{
    size_t w = x4_w;
    size_t h = x6_h;
    size_t ret = w * h;
    for (int i=x8_mips ; i>1 ; --i)
    {
        if (w > 1)
            w /= 2;
        if (h > 1)
            h /= 2;
        ret += w * h;
    }
    return ret;
}

size_t CTexture::ComputeMippedBlockCountDXT1()
{
    size_t w = x4_w / 4;
    size_t h = x6_h / 4;
    size_t ret = w * h;
    for (int i=x8_mips ; i>1 ; --i)
    {
        if (w > 1)
            w /= 2;
        if (h > 1)
            h /= 2;
        ret += w * h;
    }
    return ret;
}

struct RGBA8
{
    u8 r;
    u8 g;
    u8 b;
    u8 a;
};

void CTexture::BuildI4FromGCN(CInputStream& in)
{
    size_t texelCount = ComputeMippedTexelCount();
    std::unique_ptr<RGBA8[]> buf(new RGBA8[texelCount]);

    int w = x4_w;
    int h = x6_h;
    RGBA8* targetMip = buf.get();
    for (int mip=0 ; mip<x8_mips ; ++mip)
    {
        int bwidth = (w + 7) / 8;
        int bheight = (h + 7) / 8;
        for (int by=0 ; by<bheight ; ++by)
        {
            int baseY = by * 8;
            for (int bx=0 ; bx<bwidth ; ++bx)
            {
                int baseX = bx * 8;
                for (int y=0 ; y<8 ; ++y)
                {
                    RGBA8* target = targetMip + (baseY + y) * w + baseX;
                    u8 source[4];
                    in.readBytesToBuf(source, 4);
                    for (int x=0 ; x<8 ; ++x)
                    {
                        target[x].r = Convert4To8(source[x/2] >> ((x&1)?0:4) & 0xf);
                        target[x].g = target[x].r;
                        target[x].b = target[x].r;
                        target[x].a = target[x].r;
                    }
                }
            }
        }
        targetMip += w * h;
        if (w > 1)
            w /= 2;
        if (h > 1)
            h /= 2;
    }

    m_booToken = CGraphics::CommitResources([&](boo::IGraphicsDataFactory::Context& ctx) -> bool
    {
        m_booTex = ctx.newStaticTexture(x4_w, x6_h, x8_mips, boo::TextureFormat::RGBA8,
                                        buf.get(), texelCount * 4);
        return true;
    });
}

void CTexture::BuildI8FromGCN(CInputStream& in)
{
    size_t texelCount = ComputeMippedTexelCount();
    std::unique_ptr<RGBA8[]> buf(new RGBA8[texelCount]);

    int w = x4_w;
    int h = x6_h;
    RGBA8* targetMip = buf.get();
    for (int mip=0 ; mip<x8_mips ; ++mip)
    {
        int bwidth = (w + 7) / 8;
        int bheight = (h + 3) / 4;
        for (int by=0 ; by<bheight ; ++by)
        {
            int baseY = by * 4;
            for (int bx=0 ; bx<bwidth ; ++bx)
            {
                int baseX = bx * 8;
                for (int y=0 ; y<4 ; ++y)
                {
                    RGBA8* target = targetMip + (baseY + y) * w + baseX;
                    u8 source[8];
                    in.readBytesToBuf(source, 8);
                    for (int x=0 ; x<8 ; ++x)
                    {
                        target[x].r = source[x];
                        target[x].g = source[x];
                        target[x].b = source[x];
                        target[x].a = source[x];
                    }
                }
            }
        }
        targetMip += w * h;
        if (w > 1)
            w /= 2;
        if (h > 1)
            h /= 2;
    }

    m_booToken = CGraphics::CommitResources([&](boo::IGraphicsDataFactory::Context& ctx) -> bool
    {
        m_booTex = ctx.newStaticTexture(x4_w, x6_h, x8_mips, boo::TextureFormat::RGBA8,
                                        buf.get(), texelCount * 4);
        return true;
    });
}

void CTexture::BuildIA4FromGCN(CInputStream& in)
{
    size_t texelCount = ComputeMippedTexelCount();
    std::unique_ptr<RGBA8[]> buf(new RGBA8[texelCount]);

    int w = x4_w;
    int h = x6_h;
    RGBA8* targetMip = buf.get();
    for (int mip=0 ; mip<x8_mips ; ++mip)
    {
        int bwidth = (w + 7) / 8;
        int bheight = (h + 3) / 4;
        for (int by=0 ; by<bheight ; ++by)
        {
            int baseY = by * 4;
            for (int bx=0 ; bx<bwidth ; ++bx)
            {
                int baseX = bx * 8;
                for (int y=0 ; y<4 ; ++y)
                {
                    RGBA8* target = targetMip + (baseY + y) * w + baseX;
                    u8 source[8];
                    in.readBytesToBuf(source, 8);
                    for (int x=0 ; x<8 ; ++x)
                    {
                        u8 intensity = Convert4To8(source[x] >> 4 & 0xf);
                        target[x].r = intensity;
                        target[x].g = intensity;
                        target[x].b = intensity;
                        target[x].a = Convert4To8(source[x] & 0xf);
                    }
                }
            }
        }
        targetMip += w * h;
        if (w > 1)
            w /= 2;
        if (h > 1)
            h /= 2;
    }

    m_booToken = CGraphics::CommitResources([&](boo::IGraphicsDataFactory::Context& ctx) -> bool
    {
        m_booTex = ctx.newStaticTexture(x4_w, x6_h, x8_mips, boo::TextureFormat::RGBA8,
                                        buf.get(), texelCount * 4);
        return true;
    });
}

void CTexture::BuildIA8FromGCN(CInputStream& in)
{
    size_t texelCount = ComputeMippedTexelCount();
    std::unique_ptr<RGBA8[]> buf(new RGBA8[texelCount]);

    int w = x4_w;
    int h = x6_h;
    RGBA8* targetMip = buf.get();
    for (int mip=0 ; mip<x8_mips ; ++mip)
    {
        int bwidth = (w + 3) / 4;
        int bheight = (h + 3) / 4;
        for (int by=0 ; by<bheight ; ++by)
        {
            int baseY = by * 4;
            for (int bx=0 ; bx<bwidth ; ++bx)
            {
                int baseX = bx * 4;
                for (int y=0 ; y<4 ; ++y)
                {
                    RGBA8* target = targetMip + (baseY + y) * w + baseX;
                    u16 source[4];
                    in.readBytesToBuf(source, 8);
                    for (int x=0 ; x<4 ; ++x)
                    {
                        u8 intensity = source[x] >> 8;
                        target[x].r = intensity;
                        target[x].g = intensity;
                        target[x].b = intensity;
                        target[x].a = source[x] & 0xff;
                    }
                }
            }
        }
        targetMip += w * h;
        if (w > 1)
            w /= 2;
        if (h > 1)
            h /= 2;
    }

    m_booToken = CGraphics::CommitResources([&](boo::IGraphicsDataFactory::Context& ctx) -> bool
    {
        m_booTex = ctx.newStaticTexture(x4_w, x6_h, x8_mips, boo::TextureFormat::RGBA8,
                                        buf.get(), texelCount * 4);
        return true;
    });
}

static std::vector<RGBA8> DecodePalette(int numEntries, CInputStream& in)
{
    std::vector<RGBA8> ret;
    ret.reserve(numEntries);

    enum class EPaletteType
    {
        IA8,
        RGB565,
        RGB5A3
    };

    EPaletteType format = EPaletteType(in.readUint32Big());
    in.readUint32Big();
    switch (format)
    {
    case EPaletteType::IA8:
    {
        for (int e=0 ; e<numEntries ; ++e)
        {
            u8 intensity = in.readUByte();
            u8 alpha = in.readUByte();
            ret.push_back({intensity, intensity, intensity, alpha});
        }
        break;
    }
    case EPaletteType::RGB565:
    {
        for (int e=0 ; e<numEntries ; ++e)
        {
            u16 texel = in.readUint16Big();
            ret.push_back({Convert5To8(texel >> 11 & 0x1f),
                           Convert6To8(texel >> 5 & 0x3f),
                           Convert5To8(texel & 0x1f),
                           0xff});
        }
        break;
    }
    case EPaletteType::RGB5A3:
    {
        for (int e=0 ; e<numEntries ; ++e)
        {
            u16 texel = in.readUint16Big();
            if (texel & 0x8000)
            {
                ret.push_back({Convert5To8(texel >> 10 & 0x1f),
                               Convert5To8(texel >> 5 & 0x1f),
                               Convert5To8(texel & 0x1f),
                               0xff});
            }
            else
            {
                ret.push_back({Convert4To8(texel >> 8 & 0xf),
                               Convert4To8(texel >> 4 & 0xf),
                               Convert4To8(texel & 0xf),
                               Convert3To8(texel >> 12 & 0x7)});
            }
        }
        break;
    }
    }
    return ret;
}

void CTexture::BuildC4FromGCN(CInputStream& in)
{
    size_t texelCount = ComputeMippedTexelCount();
    std::unique_ptr<RGBA8[]> buf(new RGBA8[texelCount]);
    std::vector<RGBA8> palette = DecodePalette(16, in);

    int w = x4_w;
    int h = x6_h;
    RGBA8* targetMip = buf.get();
    for (int mip=0 ; mip<x8_mips ; ++mip)
    {
        int bwidth = (w + 7) / 8;
        int bheight = (h + 7) / 8;
        for (int by=0 ; by<bheight ; ++by)
        {
            int baseY = by * 8;
            for (int bx=0 ; bx<bwidth ; ++bx)
            {
                int baseX = bx * 8;
                for (int y=0 ; y<8 ; ++y)
                {
                    RGBA8* target = targetMip + (baseY + y) * w + baseX;
                    u8 source[4];
                    in.readBytesToBuf(source, 4);
                    for (int x=0 ; x<8 ; ++x)
                        target[x] = palette[source[x/2] >> ((x&1)?0:4) & 0xf];
                }
            }
        }
        targetMip += w * h;
        if (w > 1)
            w /= 2;
        if (h > 1)
            h /= 2;
    }

    m_booToken = CGraphics::CommitResources([&](boo::IGraphicsDataFactory::Context& ctx) -> bool
    {
        m_booTex = ctx.newStaticTexture(x4_w, x6_h, x8_mips, boo::TextureFormat::RGBA8,
                                        buf.get(), texelCount * 4);
        return true;
    });
}

void CTexture::BuildC8FromGCN(CInputStream& in)
{
    size_t texelCount = ComputeMippedTexelCount();
    std::unique_ptr<RGBA8[]> buf(new RGBA8[texelCount]);
    std::vector<RGBA8> palette = DecodePalette(256, in);

    int w = x4_w;
    int h = x6_h;
    RGBA8* targetMip = buf.get();
    for (int mip=0 ; mip<x8_mips ; ++mip)
    {
        int bwidth = (w + 7) / 8;
        int bheight = (h + 3) / 4;
        for (int by=0 ; by<bheight ; ++by)
        {
            int baseY = by * 4;
            for (int bx=0 ; bx<bwidth ; ++bx)
            {
                int baseX = bx * 8;
                for (int y=0 ; y<4 ; ++y)
                {
                    RGBA8* target = targetMip + (baseY + y) * w + baseX;
                    u8 source[8];
                    in.readBytesToBuf(source, 8);
                    for (int x=0 ; x<8 ; ++x)
                        target[x] = palette[source[x]];
                }
            }
        }
        targetMip += w * h;
        if (w > 1)
            w /= 2;
        if (h > 1)
            h /= 2;
    }

    m_booToken = CGraphics::CommitResources([&](boo::IGraphicsDataFactory::Context& ctx) -> bool
    {
        m_booTex = ctx.newStaticTexture(x4_w, x6_h, x8_mips, boo::TextureFormat::RGBA8,
                                        buf.get(), texelCount * 4);
        return true;
    });
}

void CTexture::BuildC14X2FromGCN(CInputStream& in)
{

}

void CTexture::BuildRGB565FromGCN(CInputStream& in)
{
    size_t texelCount = ComputeMippedTexelCount();
    std::unique_ptr<RGBA8[]> buf(new RGBA8[texelCount]);

    int w = x4_w;
    int h = x6_h;
    RGBA8* targetMip = buf.get();
    for (int mip=0 ; mip<x8_mips ; ++mip)
    {
        int bwidth = (w + 3) / 4;
        int bheight = (h + 3) / 4;
        for (int by=0 ; by<bheight ; ++by)
        {
            int baseY = by * 4;
            for (int bx=0 ; bx<bwidth ; ++bx)
            {
                int baseX = bx * 4;
                for (int y=0 ; y<4 ; ++y)
                {
                    RGBA8* target = targetMip + (baseY + y) * w + baseX;
                    for (int x=0 ; x<4 ; ++x)
                    {
                        u16 texel = in.readUint16Big();
                        target[x].r = Convert5To8(texel >> 11 & 0x1f);
                        target[x].g = Convert6To8(texel >> 5 & 0x3f);
                        target[x].b = Convert5To8(texel & 0x1f);
                        target[x].a = 0xff;
                    }
                }
            }
        }
        targetMip += w * h;
        if (w > 1)
            w /= 2;
        if (h > 1)
            h /= 2;
    }

    m_booToken = CGraphics::CommitResources([&](boo::IGraphicsDataFactory::Context& ctx) -> bool
    {
        m_booTex = ctx.newStaticTexture(x4_w, x6_h, x8_mips, boo::TextureFormat::RGBA8,
                                        buf.get(), texelCount * 4);
        return true;
    });
}

void CTexture::BuildRGB5A3FromGCN(CInputStream& in)
{
    size_t texelCount = ComputeMippedTexelCount();
    std::unique_ptr<RGBA8[]> buf(new RGBA8[texelCount]);

    int w = x4_w;
    int h = x6_h;
    RGBA8* targetMip = buf.get();
    for (int mip=0 ; mip<x8_mips ; ++mip)
    {
        int bwidth = (w + 3) / 4;
        int bheight = (h + 3) / 4;
        for (int by=0 ; by<bheight ; ++by)
        {
            int baseY = by * 4;
            for (int bx=0 ; bx<bwidth ; ++bx)
            {
                int baseX = bx * 4;
                for (int y=0 ; y<4 ; ++y)
                {
                    RGBA8* target = targetMip + (baseY + y) * w + baseX;
                    for (int x=0 ; x<4 ; ++x)
                    {
                        u16 texel = in.readUint16Big();
                        if (texel & 0x8000)
                        {
                            target[x].r = Convert5To8(texel >> 10 & 0x1f);
                            target[x].g = Convert5To8(texel >> 5 & 0x1f);
                            target[x].b = Convert5To8(texel & 0x1f);
                            target[x].a = 0xff;
                        }
                        else
                        {
                            target[x].r = Convert4To8(texel >> 8 & 0xf);
                            target[x].g = Convert4To8(texel >> 4 & 0xf);
                            target[x].b = Convert4To8(texel & 0xf);
                            target[x].a = Convert3To8(texel >> 12 & 0x7);
                        }
                    }
                }
            }
        }
        targetMip += w * h;
        if (w > 1)
            w /= 2;
        if (h > 1)
            h /= 2;
    }

    m_booToken = CGraphics::CommitResources([&](boo::IGraphicsDataFactory::Context& ctx) -> bool
    {
        m_booTex = ctx.newStaticTexture(x4_w, x6_h, x8_mips, boo::TextureFormat::RGBA8,
                                        buf.get(), texelCount * 4);
        return true;
    });
}

void CTexture::BuildRGBA8FromGCN(CInputStream& in)
{
    size_t texelCount = ComputeMippedTexelCount();
    std::unique_ptr<RGBA8[]> buf(new RGBA8[texelCount]);

    int w = x4_w;
    int h = x6_h;
    RGBA8* targetMip = buf.get();
    for (int mip=0 ; mip<x8_mips ; ++mip)
    {
        int bwidth = (w + 3) / 4;
        int bheight = (h + 3) / 4;
        for (int by=0 ; by<bheight ; ++by)
        {
            int baseY = by * 4;
            for (int bx=0 ; bx<bwidth ; ++bx)
            {
                int baseX = bx * 4;
                for (int c=0 ; c<2 ; ++c)
                {
                    for (int y=0 ; y<4 ; ++y)
                    {
                        RGBA8* target = targetMip + (baseY + y) * w + baseX;
                        u8 source[8];
                        in.readBytesToBuf(source, 8);
                        for (int x=0 ; x<4 ; ++x)
                        {
                            if (c)
                            {
                                target[x].g = source[x*2];
                                target[x].b = source[x*2+1];
                            }
                            else
                            {
                                target[x].a = source[x*2];
                                target[x].r = source[x*2+1];
                            }
                        }
                    }
                }
            }
        }
        targetMip += w * h;
        if (w > 1)
            w /= 2;
        if (h > 1)
            h /= 2;
    }

    m_booToken = CGraphics::CommitResources([&](boo::IGraphicsDataFactory::Context& ctx) -> bool
    {
        m_booTex = ctx.newStaticTexture(x4_w, x6_h, x8_mips, boo::TextureFormat::RGBA8,
                                        buf.get(), texelCount * 4);
        return true;
    });
}

struct DXT1Block
{
    uint16_t color1;
    uint16_t color2;
    uint8_t lines[4];
};

void CTexture::BuildDXT1FromGCN(CInputStream& in)
{
    size_t blockCount = ComputeMippedBlockCountDXT1();
    std::unique_ptr<DXT1Block[]> buf(new DXT1Block[blockCount]);

    int w = x4_w / 4;
    int h = x6_h / 4;
    DXT1Block* targetMip = buf.get();
    for (int mip=0 ; mip<x8_mips ; ++mip)
    {
        int bwidth = (w + 1) / 2;
        int bheight = (h + 1) / 2;
        for (int by=0 ; by<bheight ; ++by)
        {
            int baseY = by * 2;
            for (int bx=0 ; bx<bwidth ; ++bx)
            {
                int baseX = bx * 2;
                for (int y=0 ; y<2 ; ++y)
                {
                    DXT1Block* target = targetMip + (baseY + y) * w + baseX;
                    DXT1Block source[2];
                    in.readBytesToBuf(source, 16);
                    for (int x=0 ; x<2 ; ++x)
                    {
                        target[x].color1 = hecl::SBig(source[x].color1);
                        target[x].color2 = hecl::SBig(source[x].color2);
                        for (int i=0 ; i<4 ; ++i)
                        {
                            u8 ind[4];
                            u8 packed = source[x].lines[i];
                            ind[3] = packed & 0x3;
                            ind[2] = (packed >> 2) & 0x3;
                            ind[1] = (packed >> 4) & 0x3;
                            ind[0] = (packed >> 6) & 0x3;
                            target[x].lines[i] = ind[0] | (ind[1] << 2) | (ind[2] << 4) | (ind[3] << 6);
                        }
                    }
                }
            }
        }
        targetMip += w * h;

        if (w > 1)
            w /= 2;
        if (h > 1)
            h /= 2;
    }

    m_booToken = CGraphics::CommitResources([&](boo::IGraphicsDataFactory::Context& ctx) -> bool
    {
        m_booTex = ctx.newStaticTexture(x4_w, x6_h, x8_mips, boo::TextureFormat::DXT1,
                                        buf.get(), blockCount * 8);
        return true;
    });
}

CTexture::CTexture(CInputStream& in)
{
    x0_fmt = ETexelFormat(in.readUint32Big());
    x4_w = in.readUint16Big();
    x6_h = in.readUint16Big();
    x8_mips = in.readUint32Big();

    switch (x0_fmt)
    {
    case ETexelFormat::I4:
        BuildI4FromGCN(in);
        break;
    case ETexelFormat::I8:
        BuildI8FromGCN(in);
        break;
    case ETexelFormat::IA4:
        BuildIA4FromGCN(in);
        break;
    case ETexelFormat::IA8:
        BuildIA8FromGCN(in);
        break;
    case ETexelFormat::C4:
        BuildC4FromGCN(in);
        break;
    case ETexelFormat::C8:
        BuildC8FromGCN(in);
        break;
    case ETexelFormat::C14X2:
        BuildC14X2FromGCN(in);
        break;
    case ETexelFormat::RGB565:
        BuildRGB565FromGCN(in);
        break;
    case ETexelFormat::RGB5A3:
        BuildRGB5A3FromGCN(in);
        break;
    case ETexelFormat::RGBA8:
        BuildRGBA8FromGCN(in);
        break;
    case ETexelFormat::CMPR:
        BuildDXT1FromGCN(in);
        break;
    }
}

void CTexture::Load(int slot, EClampMode clamp) const
{

}

CFactoryFnReturn FTextureFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& vparms)
{
    return TToken<CTexture>::GetIObjObjectFor(std::make_unique<CTexture>(in));
}

}
