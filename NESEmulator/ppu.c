#include "fixNES/ppu.c"

/* Non-invasive way to access VRAM buffer directly */

uint8_t* ppuGetVRAM()
{
    return PPU_VRAM;
}
