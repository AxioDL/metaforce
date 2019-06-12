#include "fixNES/ppu.c"

uint8_t* ppuGetVRAM()
{
    return ppu.VRAM;
}
