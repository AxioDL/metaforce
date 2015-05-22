#ifndef HECL_HPP
#define HECL_HPP

#include <functional>

namespace HECL
{

#include "../extern/blowfish/blowfish.h"

/**
 * @brief Severity of a log event
 */
enum LogType
{
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR
};

/**
 * @brief Logger callback type
 */
typedef std::function<void(LogType, std::string&)> TLogger;

/**
 * @brief FourCC representation used within HECL's database
 *
 * FourCCs are efficient, mnemonic four-char-sequences used to represent types
 * while fitting comfortably in a 32-bit word. HECL uses a four-char array
 * to remain endian-independent.
 */
class FourCC
{
    union
    {
        char fcc[4];
        uint32_t num;
    };
public:
    FourCC() /* Sentinel FourCC */
    : num(0) {}
    FourCC(const char* name)
    : num(*(uint32_t*)name) {}
    inline bool operator==(FourCC& other) {return num == other.num;}
    inline bool operator!=(FourCC& other) {return num != other.num;}
    inline std::string toString() {return std::string(fcc, 4);}
};

/**
 * @brief Hash representation used for all storable and comparable objects
 *
 * Hashes are used within HECL to avoid redundant storage of objects;
 * providing a rapid mechanism to compare for equality.
 */
class ObjectHash
{
    int64_t hash;
public:
    ObjectHash(const void* buf, size_t len)
    : hash(Blowfish_hash(buf, len)) {}
    ObjectHash(int64_t hashin)
    : hash(hashin) {}
    inline bool operator==(ObjectHash& other) {return hash == other.hash;}
    inline bool operator!=(ObjectHash& other) {return hash != other.hash;}
    inline bool operator<(ObjectHash& other) {return hash < other.hash;}
    inline bool operator>(ObjectHash& other) {return hash > other.hash;}
    inline bool operator<=(ObjectHash& other) {return hash <= other.hash;}
    inline bool operator>=(ObjectHash& other) {return hash >= other.hash;}
};

inline int16_t bswap(int16_t val)
{
#if __GNUC__
    return __builtin_bswap16(val);
#elif _WIN32
    return _byteswap_ushort(val);
#else
    return (val = (val << 8) | ((val >> 8) & 0xFF));
#endif
}

inline uint16_t bswap(uint16_t val)
{
#if __GNUC__
    return __builtin_bswap16(val);
#elif _WIN32
    return _byteswap_ushort(val);
#else
    return (val = (val << 8) | ((val >> 8) & 0xFF));
#endif
}

inline int32_t bswap(int32_t val)
{
#if __GNUC__
    return __builtin_bswap32(val);
#elif _WIN32
    return _byteswap_ulong(val);
#else
    val = (val & 0x0000FFFF) << 16 | (val & 0xFFFF0000) >> 16;
    val = (val & 0x00FF00FF) << 8 | (val & 0xFF00FF00) >> 8;
    return val;
#endif
}

inline uint32_t bswap(uint32_t val)
{
#if __GNUC__
    return __builtin_bswap32(val);
#elif _WIN32
    return _byteswap_ulong(val);
#else
    val = (val & 0x0000FFFF) << 16 | (val & 0xFFFF0000) >> 16;
    val = (val & 0x00FF00FF) << 8 | (val & 0xFF00FF00) >> 8;
    return val;
#endif
}

inline int64_t bswap(int64_t val)
{
#if __GNUC__
    return __builtin_bswap64(val);
#elif _WIN32
    return _byteswap_uint64(val);
#else
    return (val = ((atInt64)((((atInt64)(val) & 0xFF00000000000000ULL) >> 56) |
                             (((atInt64)(val) & 0x00FF000000000000ULL) >> 40) |
                             (((atInt64)(val) & 0x0000FF0000000000ULL) >> 24) |
                             (((atInt64)(val) & 0x000000FF00000000ULL) >>  8) |
                             (((atInt64)(val) & 0x00000000FF000000ULL) <<  8) |
                             (((atInt64)(val) & 0x0000000000FF0000ULL) << 24) |
                             (((atInt64)(val) & 0x000000000000FF00ULL) << 40) |
                             (((atInt64)(val) & 0x00000000000000FFULL) << 56))));
#endif
}

inline uint64_t bswap(uint64_t val)
{
#if __GNUC__
    return __builtin_bswap64(val);
#elif _WIN32
    return _byteswap_uint64(val);
#else
    return (val = ((atInt64)((((atInt64)(val) & 0xFF00000000000000ULL) >> 56) |
                             (((atInt64)(val) & 0x00FF000000000000ULL) >> 40) |
                             (((atInt64)(val) & 0x0000FF0000000000ULL) >> 24) |
                             (((atInt64)(val) & 0x000000FF00000000ULL) >>  8) |
                             (((atInt64)(val) & 0x00000000FF000000ULL) <<  8) |
                             (((atInt64)(val) & 0x0000000000FF0000ULL) << 24) |
                             (((atInt64)(val) & 0x000000000000FF00ULL) << 40) |
                             (((atInt64)(val) & 0x00000000000000FFULL) << 56))));
#endif
}

}

#endif // HECL_HPP
