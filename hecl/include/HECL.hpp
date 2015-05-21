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

}

#endif // HECL_HPP
