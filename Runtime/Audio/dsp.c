#include "dsp.h"

static const int NibbleToInt[16] = {0,1,2,3,4,5,6,7,-8,-7,-6,-5,-4,-3,-2,-1};

void DSPDecompressFrame(int16_t* out, const uint8_t* in,
                        const int16_t coefs[8][2], int16_t* prev1, int16_t* prev2,
                        unsigned lastSample)
{
    uint8_t cIdx = (in[0]>>4) & 0xf;
    int16_t factor1 = coefs[cIdx][0];
    int16_t factor2 = coefs[cIdx][1];
    uint8_t exp = in[0] & 0xf;
    for (int s=0 ; s<14 && s<lastSample ; ++s)
    {
        int sampleData = (s&1)?
        NibbleToInt[(in[s/2+1])&0xf]:
        NibbleToInt[(in[s/2+1]>>4)&0xf];
        sampleData <<= exp;
        sampleData <<= 11;
        sampleData += 1024;
        sampleData +=
        factor1 * *prev1 +
        factor2 * *prev2;
        sampleData >>= 11;
        sampleData = DSPSampClamp(sampleData);
        out[s] = sampleData;
        *prev2 = *prev1;
        *prev1 = sampleData;
    }
}

void DSPDecompressFrameStereoStride(int16_t* out, const uint8_t* in,
                                    const int16_t coefs[8][2], int16_t* prev1, int16_t* prev2,
                                    unsigned lastSample)
{
    uint8_t cIdx = (in[0]>>4) & 0xf;
    int16_t factor1 = coefs[cIdx][0];
    int16_t factor2 = coefs[cIdx][1];
    uint8_t exp = in[0] & 0xf;
    for (int s=0 ; s<14 && s<lastSample ; ++s)
    {
        int sampleData = (s&1)?
        NibbleToInt[(in[s/2+1])&0xf]:
        NibbleToInt[(in[s/2+1]>>4)&0xf];
        sampleData <<= exp;
        sampleData <<= 11;
        sampleData += 1024;
        sampleData +=
        factor1 * *prev1 +
        factor2 * *prev2;
        sampleData >>= 11;
        sampleData = DSPSampClamp(sampleData);
        out[s*2] = sampleData;
        *prev2 = *prev1;
        *prev1 = sampleData;
    }
}

void DSPDecompressFrameStereoDupe(int16_t* out, const uint8_t* in,
                                  const int16_t coefs[8][2], int16_t* prev1, int16_t* prev2,
                                  unsigned lastSample)
{
    uint8_t cIdx = (in[0]>>4) & 0xf;
    int16_t factor1 = coefs[cIdx][0];
    int16_t factor2 = coefs[cIdx][1];
    uint8_t exp = in[0] & 0xf;
    for (int s=0 ; s<14 && s<lastSample ; ++s)
    {
        int sampleData = (s&1)?
        NibbleToInt[(in[s/2+1])&0xf]:
        NibbleToInt[(in[s/2+1]>>4)&0xf];
        sampleData <<= exp;
        sampleData <<= 11;
        sampleData += 1024;
        sampleData +=
        factor1 * *prev1 +
        factor2 * *prev2;
        sampleData >>= 11;
        sampleData = DSPSampClamp(sampleData);
        out[s*2] = sampleData;
        out[s*2+1] = sampleData;
        *prev2 = *prev1;
        *prev1 = sampleData;
    }
}
