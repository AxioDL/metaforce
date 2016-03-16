#include "GuiSys/CRasterFont.hpp"

namespace urde
{
CRasterFont::CRasterFont(urde::CInputStream& in, urde::IObjectStore& store)
{
    u32 magic = in.readUint32Big();
    if (magic != SBIG('FONT'))
        return;

    u32 version = in.readUint32Big();
    x4_monoWidth = in.readUint32Big();
    x8_monoHeight = in.readUint32Big();

    if (version >= 1)
        x8c_baseline = in.readUint32Big();
    else
        x8c_baseline = x8_monoHeight;

    if (version >= 2)
        x90_ = in.readUint32Big();

    bool tmp1 = in.readBool();
    bool tmp2 = in.readBool();

    u32 tmp3 = in.readUint32Big();
    u32 tmp4 = in.readUint32Big();
    std::string name= in.readString();
    u32 txtrId = in.readUint32Big();
    x80_texture = store.GetObj({'TXTR', txtrId});
    x30_fontInfo = CFontInfo(tmp1, tmp2, tmp3, tmp4, name.c_str());
    u32 mode = in.readUint32Big();
    /* TODO: Make an enum */
    if (mode == 1)
        x2c_mode = 1;
    else if (mode == 0)
        x2c_mode = 0;

    u32 glyphCount = in.readUint32Big();
    xc_glyphs.reserve(glyphCount);

    for (u32 i = 0 ; i < glyphCount ; ++i)
    {
        wchar_t chr = in.readUint16Big();
        float startU = in.readFloatBig();
        float startV = in.readFloatBig();
        float endU = in.readFloatBig();
        float endV = in.readFloatBig();
        s32 a = in.readInt32Big();
        s32 b = in.readInt32Big();
        s32 c = in.readInt32Big();
        s32 cellWidth = in.readInt32Big();
        s32 cellHeight = in.readInt32Big();
        s32 baseline = in.readInt32Big();
        s32 kernStart = in.readUint32();
        xc_glyphs[i] = std::make_pair(chr, CGlyph(a, b, c, startU, startV, endU, endV,
                                                  cellWidth, cellHeight, baseline, kernStart));
    }

    std::sort(xc_glyphs.begin(), xc_glyphs.end(), [=](auto& a, auto& b) -> bool{
        return a.first < b.first;
    });

    u32 kernCount = in.readUint32Big();
    x1c_kerning.reserve(kernCount);

    for (u32 i = 0 ; i < kernCount ; ++i)
    {
        wchar_t first = in.readUint16Big();
        wchar_t second = in.readUint16Big();
        s32 howMuch = in.readUint32Big();
        x1c_kerning[i] = CKernPair(first, second, howMuch);
    }
}

std::unique_ptr<IObj> FRasterFontFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& vparms)
{
    return TToken<CRasterFont>::GetIObjObjectFor(std::make_unique<CRasterFont>(in, *(reinterpret_cast<IObjectStore*>(vparms.GetObj()))));
}

}
