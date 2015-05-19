#include <stdlib.h>
#include <stdint.h>
#include "blowfish.h"

#define N               16

extern const unsigned long BLOWFISH_P[N + 2];
extern const unsigned long BLOWFISH_S[4][256];

#define P BLOWFISH_P
#define S BLOWFISH_S

static unsigned long F(unsigned long x)
{
    unsigned short a;
    unsigned short b;
    unsigned short c;
    unsigned short d;
    unsigned long  y;

    d = x & 0x00FF;
    x >>= 8;
    c = x & 0x00FF;
    x >>= 8;
    b = x & 0x00FF;
    x >>= 8;
    a = x & 0x00FF;
    y = S[0][a] + S[1][b];
    y = y ^ S[2][c];
    y = y + S[3][d];

    return y;
}

void Blowfish_encipher(unsigned long *xl, unsigned long *xr)
{
    unsigned long  Xl;
    unsigned long  Xr;
    unsigned long  temp;
    short          i;

    Xl = *xl;
    Xr = *xr;

    for (i = 0; i < N; ++i) {
        Xl = Xl ^ P[i];
        Xr = F(Xl) ^ Xr;

        temp = Xl;
        Xl = Xr;
        Xr = temp;
    }

    temp = Xl;
    Xl = Xr;
    Xr = temp;

    Xr = Xr ^ P[N];
    Xl = Xl ^ P[N + 1];

    *xl = Xl;
    *xr = Xr;
}

void Blowfish_decipher(unsigned long *xl, unsigned long *xr)
{
    unsigned long  Xl;
    unsigned long  Xr;
    unsigned long  temp;
    short          i;

    Xl = *xl;
    Xr = *xr;

    for (i = N + 1; i > 1; --i) {
        Xl = Xl ^ P[i];
        Xr = F(Xl) ^ Xr;

        /* Exchange Xl and Xr */
        temp = Xl;
        Xl = Xr;
        Xr = temp;
    }

    /* Exchange Xl and Xr */
    temp = Xl;
    Xl = Xr;
    Xr = temp;

    Xr = Xr ^ P[1];
    Xl = Xl ^ P[0];

    *xl = Xl;
    *xr = Xr;
}

int64_t Blowfish_hash(const void* buf, size_t len)
{
    unsigned i,j;
    union
    {
        unsigned long h32[2];
        int64_t h64;
    } hash = {};

    for (i=0 ; i<len/4 ; ++i)
    {
        unsigned long block = *(unsigned long*)buf;
        Blowfish_encipher(&hash.h32[i&1], &block);
        buf += 4;
    }

    unsigned rem = len % 4;
    if (rem)
    {
        union
        {
            unsigned long b32;
            char b[4];
        } block = {0};
        for (j=0 ; j<rem ; ++j)
            block.b[j] = *(unsigned long*)buf;
        Blowfish_encipher(&hash.h32[i&1], &block.b32);
    }

    return hash.h64;
}

