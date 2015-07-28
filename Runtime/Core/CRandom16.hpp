#ifndef RETRO_CRANDOM16_HPP
#define RETRO_CRANDOM16_HPP

#include <stdint.h>

namespace Retro
{

extern class CRandom16* GLOBAL_RANDOM;
extern class CGlobalRandom* GLOBAL_RANDOM_TOKEN;

class CRandom16
{
    uint32_t m_seed;
public:
    CRandom16(uint32_t p) : m_seed(p) {}

    inline uint32_t Next()
    {
        m_seed = m_seed * 0x41c64e6d + 0x00003039;
        return m_seed >> 16;
    }

    inline uint32_t GetSeed() const
    {
        return m_seed;
    }

    inline void SetSeed(time_t p)
    {
        m_seed = p;
    }

    inline float Float()
    {
        return Next() * 0.000015259022;
    }

    inline float Range(float min, float max)
    {
        return min + Float() * (max - min);
    }

    inline int32_t Range(int32_t min, int32_t max)
    {
        int64_t diff = max - min;
        int64_t rand = -1;
        while (rand < 0)
            rand = int32_t((Next() << 16) | Next());
        return rand % diff + min;
    }

    static CRandom16* GetRandomNumber() {return GLOBAL_RANDOM;}
    static void SetRandomNumber(CRandom16* rnd) {GLOBAL_RANDOM = rnd;}
};

class CGlobalRandom
{
    CRandom16& m_random;
    CGlobalRandom* m_prev;
public:
    CGlobalRandom(CRandom16& rand)
    : m_random(rand), m_prev(GLOBAL_RANDOM_TOKEN)
    {
        GLOBAL_RANDOM_TOKEN = this;
        CRandom16::SetRandomNumber(&m_random);
    }
    ~CGlobalRandom()
    {
        GLOBAL_RANDOM_TOKEN = m_prev;
        if (m_prev)
            CRandom16::SetRandomNumber(&m_prev->m_random);
        else
            CRandom16::SetRandomNumber(nullptr);
    }
};

}

#endif // RETRO_CRANDOM16_HPP
