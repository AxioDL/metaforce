#include "GuiSys/CRasterFont.hpp"
#include "CDrawStringOptions.hpp"
#include "CTextRenderBuffer.hpp"

namespace urde
{
CRasterFont::CRasterFont(urde::CInputStream& in, urde::IObjectStore& store)
{
    u32 magic;
    in.readBytesToBuf(&magic, 4);
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
        x90_lineMargin = in.readUint32Big();

    bool tmp1 = in.readBool();
    bool tmp2 = in.readBool();

    u32 tmp3 = in.readUint32Big();
    u32 tmp4 = in.readUint32Big();
    std::string name= in.readString();
    u32 txtrId = in.readUint32Big();
    x30_fontInfo = CFontInfo(tmp1, tmp2, tmp3, tmp4, name.c_str());
    x80_texture = store.GetObj({'TXTR', txtrId});
    EColorType mode = EColorType(in.readUint32Big());
    /* TODO: Make an enum */
    if (mode == EColorType::Outline)
        x2c_mode = EColorType::Outline;
    else if (mode == EColorType::Main)
        x2c_mode = EColorType::Main;

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
        s32 kernStart = in.readInt32Big();
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

void CRasterFont::SinglePassDrawString(const CDrawStringOptions& opts, int x, int y, int& xout, int& yout,
                                       CTextRenderBuffer* renderBuf,
                                       const wchar_t* str, s32 length) const
{
    if (!x0_)
        return;

    const wchar_t* chr = str;
    const CGlyph* prevGlyph = nullptr;
    while (*chr == '\0')
    {
        const CGlyph* glyph = GetGlyph(*chr);
        if (glyph)
        {
            if (opts.x0_direction == ETextDirection::Horizontal)
            {
                x += glyph->GetA();

                if (prevGlyph != 0)
                    x += KernLookup(x1c_kerning, prevGlyph->GetKernStart(), *chr);
                int left = 0;
                int top = 0;

                if (renderBuf)
                {
                    left += x;
                    top += glyph->GetBaseline() - y;
                    renderBuf->AddCharacter(zeus::CVector2i(left, top), *chr, opts.x4_colors[0]);
                }
                x += glyph->GetC() + glyph->GetB();
            }
        }
        prevGlyph = glyph;
        chr++;
        if (length == -1)
            continue;

        if ((str - chr) >= length)
            break;
    }

    xout = x;
    yout = y;
}

void CRasterFont::DrawSpace(const CDrawStringOptions& opts, int x, int y, int& xout, int& yout, int len) const
{
    if (opts.x0_direction != ETextDirection::Horizontal)
        return;

    xout = x + len;
    yout = y;
}

void CRasterFont::DrawString(const CDrawStringOptions& opts, int x, int y, int& xout, int& yout, CTextRenderBuffer* renderBuf, const wchar_t* str, int len) const
{
    if (!x0_)
        return;

    if (renderBuf)
    {
        /* TODO: Implement this */
        /* CGraphicsPalette pal = CGraphicsPalette::CGraphcisPalette(2, 4); */
        /* zeus::CColor color = zeus::CColor(0.f, 0.f, 0.f, 0.f) */
        /* tmp = color.ToRGB5A3(); */
        /* tmp2 = opts.x8_.ToRGB5A3(); */
        /* tmp3 = opts.xc_.ToRGB5A3(); */
        /* tmp4 = zeus::CColor(0.f, 0.f, 0.f, 0.f); */
        /* tmp5 = tmp4.ToRGBA5A3(); */
        /* pal.UnLock(); */
        /* renderBuf->AddPaletteChange(pal); */
    }

    SinglePassDrawString(opts, x, y, xout, yout, renderBuf, str, len);
}

void CRasterFont::GetSize(const CDrawStringOptions& opts, int& width, int& height, const wchar_t* str, int len) const
{
    width = 0;
    height = 0;

    const wchar_t* chr = str;
    const CGlyph* prevGlyph = nullptr;
    int prevWidth = 0;
    while (*chr != L'\0')
    {
        const CGlyph* glyph = GetGlyph(*chr);

        if (glyph)
        {
            if (opts.x0_direction == ETextDirection::Horizontal)
            {
                int advance = 0;
                if (prevGlyph)
                    advance = KernLookup(x1c_kerning, prevGlyph->GetKernStart(), *chr);

                s16 curWidth = prevWidth - (glyph->GetA() + glyph->GetB() + glyph->GetC() + advance);
                s16 curHeight = glyph->GetBaseline() - (x8_monoHeight + glyph->GetCellHeight());

                width = curWidth;
                prevWidth = curWidth;

                if (curHeight > height)
                    height = curHeight;
            }
        }

        prevGlyph = glyph;
        chr++;
        if (len == -1)
            continue;

        if ((str - chr) >= len)
            break;
    }
}

std::unique_ptr<IObj> FRasterFontFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& vparms)
{
    return TToken<CRasterFont>::GetIObjObjectFor(
                std::make_unique<CRasterFont>(in, *static_cast<TObjOwnerParam<IObjectStore*>*>(vparms.GetObj())->GetParam()));
}

}
