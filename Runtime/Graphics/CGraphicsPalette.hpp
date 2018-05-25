#ifndef __URDE_CGRAPHICSPALETTE_HPP__
#define __URDE_CGRAPHICSPALETTE_HPP__

#include <memory>
#include "RetroTypes.hpp"

namespace urde
{

enum class EPaletteFormat
{
    IA8    = 0x0,
    RGB565 = 0x1,
    RGB5A3 = 0x2,
};

class CGraphicsPalette
{
    friend class CTextRenderBuffer;
    EPaletteFormat x0_fmt;
    u32 x4_;
    int x8_entryCount;
    std::unique_ptr<u16[]> xc_entries;
    /* x10_ GXTlutObj here */
    bool x1c_ = false;
public:
    CGraphicsPalette(EPaletteFormat fmt, int count)
    : x0_fmt(fmt), x8_entryCount(count), xc_entries(new u16[count]) {}
    CGraphicsPalette(CInputStream& in)
        : x0_fmt(EPaletteFormat(in.readUint32Big()))
    {
        u16 w = in.readUint16Big();
        u16 h = in.readUint16Big();
        x8_entryCount = w * h;

        /* GX Tlut init here */
    }
};

}

#endif // __URDE_CGRAPHICSPALETTE_HPP__
