#ifndef HECL_FOURCC_HPP
#define HECL_FOURCC_HPP

#include <cstdint>
#include <cstddef>
#include <string>

namespace hecl
{

/**
 * @brief FourCC representation used within HECL's database
 *
 * FourCCs are efficient, mnemonic four-char-sequences used to represent types
 * while fitting comfortably in a 32-bit word. HECL uses a four-char array
 * to remain endian-independent.
 */
class FourCC
{
protected:
    union
    {
        char fcc[4];
        uint32_t num;
    };
public:
    FourCC() /* Sentinel FourCC */
        : num(0) {}
    FourCC(const FourCC& other)
    {num = other.num;}
    FourCC(const char* name)
    : num(*(uint32_t*)name) {}
    FourCC(uint32_t n)
    : num(n) {}
    bool operator==(const FourCC& other) const {return num == other.num;}
    bool operator!=(const FourCC& other) const {return num != other.num;}
    bool operator==(const char* other) const {return num == *(uint32_t*)other;}
    bool operator!=(const char* other) const {return num != *(uint32_t*)other;}
    bool operator==(int32_t other) const { return num == other;}
    bool operator!=(int32_t other) const { return num != other;}
    bool operator==(uint32_t other) const {return num == other;}
    bool operator!=(uint32_t other) const {return num != other;}
    std::string toString() const {return std::string(fcc, 4);}
    uint32_t toUint32() const {return num;}
    operator uint32_t() const {return num;}
    const char* getChars() const {return fcc;}
    char* getChars() {return fcc;}
};
#define FOURCC(chars) FourCC(SBIG(chars))

}

namespace std
{
template <> struct hash<hecl::FourCC>
{
    size_t operator()(const hecl::FourCC& val) const noexcept
    {return val.toUint32();}
};
}

#endif // HECL_FOURCC_HPP
