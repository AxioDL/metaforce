#ifndef RETRO_CRC32_HPP
#define RETRO_CRC32_HPP
#include <cstdint>

namespace pshag
{

class CCRC32
{
    static const uint32_t crc32Table[256];
    static inline uint32_t permute(uint32_t checksum, uint8_t b)
    { return (checksum >> 8) ^ crc32Table[(checksum & 0xFF) ^ b]; }

public:
    static uint32_t Calculate(const void* data, uint32_t length);
};

}

#endif
