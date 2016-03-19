#ifndef __URDE_CTEXTRENDERBUFFER_HPP__
#define __URDE_CTEXTRENDERBUFFER_HPP__

#include "zeus/CColor.hpp"
#include "zeus/CVector2i.hpp"
#include "CToken.hpp"
#include "RetroTypes.hpp"

namespace urde
{
class CFontImageDef;
class CGraphicsPalette;
class CRasterFont;

class CTextRenderBuffer
{
public:
    struct Primitive
    {
        u32 x0_;
        u32 x4_;
        u16 x8_;
        u16 xa_;
        u16 xc_;
        u16 xe_;
    };
    enum class EMode
    {
        Zero,
        One
    };
private:
    EMode x0_mode;
    std::vector<TToken<CRasterFont>> x4_fonts;
    std::vector<CFontImageDef> x14_images;
    std::vector<int> x24_primOffsets;
    std::vector<char> x34_blob;
    u32 x44_blobSize = 0;
    u32 x50_paletteCount = 0;
    std::unique_ptr<CGraphicsPalette> x54_palettes[64];
    u32 x254_nextPalette = 0;
public:
    CTextRenderBuffer(EMode mode);
    void SetPrimitive(const Primitive&, int);
    Primitive GetPrimitive(int) const;
    void GetOutStream();
    void SetMode(EMode mode);
    void VerifyBuffer();
    void Render(const zeus::CColor& col, float) const;
    void AddImage(const zeus::CVector2i& vec, const CFontImageDef&);
    void AddCharacter(const zeus::CVector2i& vec, s16, const zeus::CColor&);
    void AddPaletteChange(const CGraphicsPalette& palette);
    void AddFontChange(const TToken<CRasterFont>& font);
    int GetMatchingPaletteIndex(const CGraphicsPalette& palette);
    CGraphicsPalette* GetNextAvailablePalette();
};

}

#endif // __URDE_CTEXTRENDERBUFFER_HPP__
