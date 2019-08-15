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
    uint32_t num = 0;
  };

public:
  // Sentinel FourCC
  constexpr FourCC() noexcept = default;
  constexpr FourCC(const FourCC& other) noexcept = default;
  constexpr FourCC(FourCC&& other) noexcept = default;
  constexpr FourCC(const char* name) noexcept : fcc{name[0], name[1], name[2], name[3]} {}
  constexpr FourCC(uint32_t n) noexcept : num(n) {}

  constexpr FourCC& operator=(const FourCC&) noexcept = default;
  constexpr FourCC& operator=(FourCC&&) noexcept = default;

  bool operator==(const FourCC& other) const { return num == other.num; }
  bool operator!=(const FourCC& other) const { return !operator==(other); }
  bool operator==(const char* other) const {
    return std::memcmp(fcc, other, sizeof(fcc)) == 0;
  }
  bool operator!=(const char* other) const { return !operator==(other); }
  bool operator==(int32_t other) const { return num == uint32_t(other); }
  bool operator!=(int32_t other) const { return !operator==(other); }
  bool operator==(uint32_t other) const { return num == other; }
  bool operator!=(uint32_t other) const { return !operator==(other); }

  std::string toString() const { return std::string(std::begin(fcc), std::end(fcc)); }
  uint32_t toUint32() const { return num; }
  const char* getChars() const { return fcc; }
  char* getChars() { return fcc; }
  bool IsValid() const { return num != 0; }
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
inline void DNAFourCC::Enumerate<BigDNA::Read>(Read::StreamT& r) {
  r.readUBytesToBuf(fcc, std::size(fcc));
}
template <>
inline void DNAFourCC::Enumerate<BigDNA::Write>(Write::StreamT& w) {
  w.writeBytes(fcc, std::size(fcc));
}
template <>
inline void DNAFourCC::Enumerate<BigDNA::ReadYaml>(ReadYaml::StreamT& r) {
  const std::string rs = r.readString(nullptr);
  rs.copy(fcc, std::size(fcc));
}
template <>
inline void DNAFourCC::Enumerate<BigDNA::WriteYaml>(WriteYaml::StreamT& w) {
  w.writeString(nullptr, std::string_view{fcc, std::size(fcc)});
}
template <>
inline void DNAFourCC::Enumerate<BigDNA::BinarySize>(BinarySize::StreamT& s) {
  s += std::size(fcc);
}

} // namespace hecl

namespace std {
template <>
struct hash<hecl::FourCC> {
  size_t operator()(const hecl::FourCC& val) const noexcept { return val.toUint32(); }
};
} // namespace std

FMT_CUSTOM_FORMATTER(hecl::FourCC, "{:c}{:c}{:c}{:c}",
                     obj.getChars()[0], obj.getChars()[1], obj.getChars()[2], obj.getChars()[3])
FMT_CUSTOM_FORMATTER(hecl::DNAFourCC, "{:c}{:c}{:c}{:c}",
                     obj.getChars()[0], obj.getChars()[1], obj.getChars()[2], obj.getChars()[3])
