#include "CNESEmulator.hpp"
#include "CGameState.hpp"
#include <cstring>

namespace urde::MP1
{

void CNESEmulator::DescrambleROM(u8* dataIn, u8* dataOut, u32 dataOutLen, u8 descrambleInit,
                                 u32 descramble2Len, u32 descramble2Magic)
{
    for (int i=0 ; i<256 ; ++i)
    {
        descrambleInit += dataIn[i];
        dataIn[i] = descrambleInit;
    }

    for (int i=0 ; i<128 ; ++i)
        std::swap(dataIn[255 - i], dataIn[i]);

    struct BitstreamState
    {
        u32 x0_remBits = 0;
        u32 x4_lastByte;
        u8* x8_ptr;
        BitstreamState(u8* ptr) : x8_ptr(ptr) {}
    } bState = {dataIn + 256};

    u8* dataOutAlpha = dataOut;
    while (dataOutLen != 0)
    {
        u32 r12 = 0;

        if (bState.x0_remBits == 0)
        {
            bState.x4_lastByte = *bState.x8_ptr;
            bState.x8_ptr += 1;
            bState.x0_remBits = 8;
        }

        u32 r11 = r12 << 1;
        if (bState.x4_lastByte & 0x80)
            r11 |= 1;

        bState.x4_lastByte <<= 1;
        bState.x0_remBits -= 1;

        if (r11 == 1)
        {
            r12 = 0;

            for (int i=0 ; i<4 ; ++i)
            {
                if (bState.x0_remBits == 0)
                {
                    bState.x4_lastByte = *bState.x8_ptr;
                    bState.x8_ptr += 1;
                    bState.x0_remBits = 8;
                }

                r12 <<= 1;
                if (bState.x4_lastByte & 0x80)
                    r12 |= 1;

                bState.x4_lastByte <<= 1;
                bState.x0_remBits -= 1;

                if (bState.x0_remBits == 0)
                {
                    bState.x4_lastByte = *bState.x8_ptr;
                    bState.x8_ptr += 1;
                    bState.x0_remBits = 8;
                }

                r12 <<= 1;
                if (bState.x4_lastByte & 0x80)
                    r12 |= 1;

                bState.x4_lastByte <<= 1;
                bState.x0_remBits -= 1;
            }

            *dataOutAlpha++ = dataIn[r12 + 73];
        }
        else
        {
            r12 = 0;

            if (bState.x0_remBits == 0)
            {
                bState.x4_lastByte = *bState.x8_ptr;
                bState.x8_ptr += 1;
                bState.x0_remBits = 8;
            }

            r11 = r12 << 1;
            if (bState.x4_lastByte & 0x80)
                r11 |= 1;

            bState.x4_lastByte <<= 1;
            bState.x0_remBits -= 1;

            if (r11 == 1)
            {
                r12 = 0;

                for (int i=0 ; i<3 ; ++i)
                {
                    if (bState.x0_remBits == 0)
                    {
                        bState.x4_lastByte = *bState.x8_ptr;
                        bState.x8_ptr += 1;
                        bState.x0_remBits = 8;
                    }

                    r12 <<= 1;
                    if (bState.x4_lastByte & 0x80)
                        r12 |= 1;

                    bState.x4_lastByte <<= 1;
                    bState.x0_remBits -= 1;

                    if (bState.x0_remBits == 0)
                    {
                        bState.x4_lastByte = *bState.x8_ptr;
                        bState.x8_ptr += 1;
                        bState.x0_remBits = 8;
                    }

                    r12 <<= 1;
                    if (bState.x4_lastByte & 0x80)
                        r12 |= 1;

                    bState.x4_lastByte <<= 1;
                    bState.x0_remBits -= 1;
                }

                *dataOutAlpha++ = dataIn[r12 + 9];
            }
            else
            {
                r12 = 0;

                if (bState.x0_remBits == 0)
                {
                    bState.x4_lastByte = *bState.x8_ptr;
                    bState.x8_ptr += 1;
                    bState.x0_remBits = 8;
                }

                r11 = r12 << 1;
                if (bState.x4_lastByte & 0x80)
                    r11 |= 1;

                bState.x4_lastByte <<= 1;
                bState.x0_remBits -= 1;

                if (r11 == 1)
                {
                    r12 = 0;

                    for (int i=0 ; i<3 ; ++i)
                    {
                        if (bState.x0_remBits == 0)
                        {
                            bState.x4_lastByte = *bState.x8_ptr;
                            bState.x8_ptr += 1;
                            bState.x0_remBits = 8;
                        }

                        r12 <<= 1;
                        if (bState.x4_lastByte & 0x80)
                            r12 |= 1;

                        bState.x4_lastByte <<= 1;
                        bState.x0_remBits -= 1;
                    }

                    *dataOutAlpha++ = dataIn[r12 + 1];
                }
                else
                {
                    *dataOutAlpha++ = dataIn[0];
                }
            }
        }

        --dataOutLen;
    }

    u32 tmpWord = 0;
    for (int i=0 ; i<descramble2Len ; ++i)
    {
        tmpWord ^= dataOut[i];
        for (int j=0 ; j<8 ; ++j)
        {
            if (tmpWord & 0x1)
                tmpWord = (tmpWord >> 1) ^ descramble2Magic;
            else
                tmpWord = (tmpWord >> 1);
        }
    }

    dataOut[descramble2Len - 1] = (tmpWord >> 8) & 0xff;
    dataOut[descramble2Len - 2] = tmpWord & 0xff;
}

void CNESEmulator::ProcessUserInput(const CFinalInput& input, int)
{

}

void CNESEmulator::Update()
{

}

void CNESEmulator::Draw(const zeus::CColor& mulColor, bool filtering)
{

}

void CNESEmulator::LoadState(const u8* state)
{
    memmove(x39_loadState, state, 18);
    x38_stateLoaded = true;
}

}
