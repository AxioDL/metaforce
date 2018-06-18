#include "fixNES/apu.c"

uint32_t apuGetMaxBufSize()
{
    return apu.BufSizeBytes;
}

void apuResetPos()
{
    apu.curBufPos = 0;
}
