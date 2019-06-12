#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include "athena/DNA.hpp"

namespace hecl {

/**
 * @brief FourCC representation used within HECL's database
 *
 * FourCCs are efficient, mnemonic four-char-sequences used to represent types
 * while fitting comfortably in a 32-bit word. HECL uses a four-char array
 * to remain endian-independent.
 */
class FourCC {
protected:
  union {
    char fcc[4];
    uint32_t num;
  };

public:
  constexpr FourCC() /* Sentinel FourCC */
  : num(0) {}
  constexpr FourCC(const FourCC& other) : num(other.num) {}
  constexpr FourCC(const char* name) : num(*(uint32_t*)name) {}
  constexpr FourCC(uint32_t n) : num(n) {}
  bool operator==(const FourCC& other) const { return num == other.num; }
  bool operator!=(const FourCC& other) const { return num != other.num; }
  bool operator==(const char* other) const { return num == *(uint32_t*)other; }
  bool operator!=(const char* other) const { return num != *(uint32_t*)other; }
  bool operator==(int32_t other) const { return num == uint32_t(other); }
  bool operator!=(int32_t other) const { return num != uint32_t(other); }
  bool operator==(uint32_t other) const { return num == other; }
  bool operator!=(uint32_t other) const { return num != other; }
  std::string toString() const { return std::string(fcc, 4); }
  uint32_t toUint32() const { return num; }
  operator uint32_t() const { return num; }
  const char* getChars() const { return fcc; }
  char* getChars() { return fcc; }
};
#define FOURCC(chars) FourCC(SBIG(chars))

using BigDNA = athena::io::DNA<athena::Big>;

/** FourCC with DNA read/write */
class DNAFourCC final : public BigDNA, public FourCC {
public:
  DNAFourCC() : FourCC() {}
  DNAFourCC(const FourCC& other) : FourCC() { num = other.toUint32(); }
  DNAFourCC(const char* name) : FourCC(name) {}
  DNAFourCC(uint32_t n) : FourCC(n) {}
  AT_DECL_EXPLICIT_DNA_YAML
};
template <>
inline void DNAFourCC::Enumerate<BigDNA::Read>(typename Read::StreamT& r) {
  r.readUBytesToBuf(fcc, 4);
}
template <>
inline void DNAFourCC::Enumerate<BigDNA::Write>(typename Write::StreamT& w) {
  w.writeUBytes((atUint8*)fcc, 4);
}
template <>
inline void DNAFourCC::Enumerate<BigDNA::ReadYaml>(typename ReadYaml::StreamT& r) {
  std::string rs = r.readString(nullptr);
  strncpy(fcc, rs.c_str(), 4);
}
template <>
inline void DNAFourCC::Enumerate<BigDNA::WriteYaml>(typename WriteYaml::StreamT& w) {
  w.writeString(nullptr, std::string(fcc, 4));
}
template <>
inline void DNAFourCC::Enumerate<BigDNA::BinarySize>(typename BinarySize::StreamT& s) {
  s += 4;
}

} // namespace hecl

namespace std {
template <>
struct hash<hecl::FourCC> {
  size_t operator()(const hecl::FourCC& val) const noexcept { return val.toUint32(); }
};
} // namespace std
