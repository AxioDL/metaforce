#include "CTextRenderBuffer.hpp"
#include "CFontImageDef.hpp"
#include "Graphics/CGraphicsPalette.hpp"

namespace urde
{

CTextRenderBuffer::CTextRenderBuffer(EMode mode)
: x0_mode(mode)
{
    for (int i=0 ; i<64 ; ++i)
    {
        x54_palettes[i].reset(new CGraphicsPalette(EPaletteFormat::RGB5A3, 4));
        ++x50_paletteCount;
    }
}

void CTextRenderBuffer::SetPrimitive(const Primitive&, int)
{
}

CTextRenderBuffer::Primitive CTextRenderBuffer::GetPrimitive(int) const
{
}

void CTextRenderBuffer::GetOutStream()
{
}

void CTextRenderBuffer::SetMode(EMode mode)
{
    x0_mode = mode;
}

void CTextRenderBuffer::VerifyBuffer()
{
    if (x34_blob.empty())
        x34_blob.resize(x44_blobSize);
}

void CTextRenderBuffer::Render(const zeus::CColor& col, float) const
{
}

void CTextRenderBuffer::AddImage(const zeus::CVector2i& vec, const CFontImageDef&)
{
}

void CTextRenderBuffer::AddCharacter(const zeus::CVector2i& vec, s16, const zeus::CColor&)
{
}

void CTextRenderBuffer::AddPaletteChange(const CGraphicsPalette& palette)
{
}

void CTextRenderBuffer::AddFontChange(const TToken<CRasterFont>& font)
{
}

int CTextRenderBuffer::GetMatchingPaletteIndex(const CGraphicsPalette& palette)
{
}

CGraphicsPalette* CTextRenderBuffer::GetNextAvailablePalette()
{
    if (x254_nextPalette >= 64)
        x254_nextPalette = 0;
    return x54_palettes[x254_nextPalette++].get();
}

}
