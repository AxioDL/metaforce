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
    EPaletteFormat x0_fmt;
    int x4_entryCount;
    std::unique_ptr<u16[]> x8_entries;
    /* xc_ GXTlutObj here */
public:
    CGraphicsPalette(EPaletteFormat fmt, int count)
    : x0_fmt(fmt), x4_entryCount(count), x8_entries(new u16[count]) {}
};

}

#endif // __URDE_CGRAPHICSPALETTE_HPP__
