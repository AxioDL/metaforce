#ifndef _DSP_h
#define _DSP_h

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void DSPDecompressFrame(int16_t* out, const uint8_t* in,
                        const int16_t coefs[8][2], int16_t* prev1, int16_t* prev2,
                        unsigned lastSample);
void DSPDecompressFrameStereoStride(int16_t* out, const uint8_t* in,
                                    const int16_t coefs[8][2], int16_t* prev1, int16_t* prev2,
                                    unsigned lastSample);
void DSPDecompressFrameStereoDupe(int16_t* out, const uint8_t* in,
                                  const int16_t coefs[8][2], int16_t* prev1, int16_t* prev2,
                                  unsigned lastSample);

#ifdef __cplusplus
}
#endif

#endif // _DSP_h
