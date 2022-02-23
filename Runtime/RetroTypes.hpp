#pragma once

#include <functional>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "GCNTypes.hpp"
#include "rstl.hpp"

#include <logvisor/logvisor.hpp>
#include <zeus/CMatrix3f.hpp>
#include <zeus/CMatrix4f.hpp>
#include <zeus/CTransform.hpp>
#include <zeus/CVector2f.hpp>
#include <zeus/CVector3f.hpp>

#undef min
#undef max

using namespace std::literals;

namespace metaforce {
class CInputStream;
class COutputStream;
using kUniqueIdType = u16;
static constexpr int kMaxEntities = 1024;
constexpr kUniqueIdType kUniqueIdSize = sizeof(u16);
constexpr kUniqueIdType kUniqueIdBits = kUniqueIdSize * 8;
constexpr kUniqueIdType kUniqueIdMax = UINT16_MAX;
constexpr kUniqueIdType kUniqueIdVersionMax = 64;
constexpr kUniqueIdType kUniqueIdVersionMask = kUniqueIdVersionMax - 1;
constexpr kUniqueIdType kUniqueIdValueMask = kMaxEntities - 1;
constexpr kUniqueIdType kUniqueIdValueBits = 10;
constexpr kUniqueIdType kUniqueIdVersionBits = 6;

#undef bswap16
#undef bswap32
#undef bswap64

/* Type-sensitive byte swappers */
template <typename T>
constexpr T bswap16(T val) noexcept {
#if __GNUC__
  return __builtin_bswap16(val);
#elif _WIN32
  return _byteswap_ushort(val);
#else
  return (val = (val << 8) | ((val >> 8) & 0xFF));
#endif
}

template <typename T>
constexpr T bswap32(T val) noexcept {
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

template <typename T>
constexpr T bswap64(T val) noexcept {
#if __GNUC__
  return __builtin_bswap64(val);
#elif _WIN32
  return _byteswap_uint64(val);
#else
  return ((val & 0xFF00000000000000ULL) >> 56) | ((val & 0x00FF000000000000ULL) >> 40) |
         ((val & 0x0000FF0000000000ULL) >> 24) | ((val & 0x000000FF00000000ULL) >> 8) |
         ((val & 0x00000000FF000000ULL) << 8) | ((val & 0x0000000000FF0000ULL) << 24) |
         ((val & 0x000000000000FF00ULL) << 40) | ((val & 0x00000000000000FFULL) << 56);
#endif
}

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
constexpr int16_t SBig(int16_t val) noexcept { return bswap16(val); }
constexpr uint16_t SBig(uint16_t val) noexcept { return bswap16(val); }
constexpr int32_t SBig(int32_t val) noexcept { return bswap32(val); }
constexpr uint32_t SBig(uint32_t val) noexcept { return bswap32(val); }
constexpr int64_t SBig(int64_t val) noexcept { return bswap64(val); }
constexpr uint64_t SBig(uint64_t val) noexcept { return bswap64(val); }
constexpr float SBig(float val) noexcept {
  union {
    float f;
    u32 i;
  } uval1 = {val};
  union {
    u32 i;
    float f;
  } uval2 = {bswap32(uval1.i)};
  return uval2.f;
}
constexpr double SBig(double val) noexcept {
  union {
    double f;
    u32 i;
  } uval1 = {val};
  union {
    u32 i;
    double f;
  } uval2 = {bswap64(uval1.i)};
  return uval2.f;
}
#ifndef SBIG
#define SBIG(q) (((q)&0x000000FF) << 24 | ((q)&0x0000FF00) << 8 | ((q)&0x00FF0000) >> 8 | ((q)&0xFF000000) >> 24)
#endif

constexpr int16_t SLittle(int16_t val) noexcept { return val; }
constexpr uint16_t SLittle(uint16_t val) noexcept { return val; }
constexpr int32_t SLittle(int32_t val) noexcept { return val; }
constexpr uint32_t SLittle(uint32_t val) noexcept { return val; }
constexpr int64_t SLittle(int64_t val) noexcept { return val; }
constexpr uint64_t SLittle(uint64_t val) noexcept { return val; }
constexpr float SLittle(float val) noexcept { return val; }
constexpr double SLittle(double val) noexcept { return val; }
#ifndef SLITTLE
#define SLITTLE(q) (q)
#endif
#else
constexpr int16_t SLittle(int16_t val) noexcept { return bswap16(val); }
constexpr uint16_t SLittle(uint16_t val) noexcept { return bswap16(val); }
constexpr int32_t SLittle(int32_t val) noexcept { return bswap32(val); }
constexpr uint32_t SLittle(uint32_t val) noexcept { return bswap32(val); }
constexpr int64_t SLittle(int64_t val) noexcept { return bswap64(val); }
constexpr uint64_t SLittle(uint64_t val) noexcept { return bswap64(val); }
constexpr float SLittle(float val) noexcept {
  int32_t ival = bswap32(*((int32_t*)(&val)));
  return *((float*)(&ival));
}
constexpr double SLittle(double val) noexcept {
  int64_t ival = bswap64(*((int64_t*)(&val)));
  return *((double*)(&ival));
}
#ifndef SLITTLE
#define SLITTLE(q) (((q)&0x000000FF) << 24 | ((q)&0x0000FF00) << 8 | ((q)&0x00FF0000) >> 8 | ((q)&0xFF000000) >> 24)
#endif

constexpr int16_t SBig(int16_t val) noexcept { return val; }
constexpr uint16_t SBig(uint16_t val) noexcept { return val; }
constexpr int32_t SBig(int32_t val) noexcept { return val; }
constexpr uint32_t SBig(uint32_t val) noexcept { return val; }
constexpr int64_t SBig(int64_t val) noexcept { return val; }
constexpr uint64_t SBig(uint64_t val) noexcept { return val; }
constexpr float SBig(float val) noexcept { return val; }
constexpr double SBig(double val) noexcept { return val; }
#ifndef SBIG
#define SBIG(q) (q)
#endif
#endif

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

  constexpr bool operator==(const FourCC& other) const noexcept { return num == other.num; }
  constexpr bool operator!=(const FourCC& other) const noexcept { return !operator==(other); }
  constexpr bool operator==(const char* other) const noexcept {
    return other[0] == fcc[0] && other[1] == fcc[1] && other[2] == fcc[2] && other[3] == fcc[3];
  }
  constexpr bool operator!=(const char* other) const noexcept { return !operator==(other); }
  constexpr bool operator==(int32_t other) const noexcept { return num == uint32_t(other); }
  constexpr bool operator!=(int32_t other) const noexcept { return !operator==(other); }
  constexpr bool operator==(uint32_t other) const noexcept { return num == other; }
  constexpr bool operator!=(uint32_t other) const noexcept { return !operator==(other); }

  std::string toString() const { return std::string(std::begin(fcc), std::end(fcc)); }
  constexpr std::string_view toStringView() const { return std::string_view(fcc, std::size(fcc)); }
  constexpr uint32_t toUint32() const noexcept { return num; }
  constexpr const char* getChars() const noexcept { return fcc; }
  constexpr char* getChars() noexcept { return fcc; }
  constexpr bool IsValid() const noexcept { return num != 0; }
};
#define FOURCC(chars) FourCC(SBIG(chars))

class CAssetId {
  u64 id = UINT64_MAX;

public:
  constexpr CAssetId() noexcept = default;
  constexpr CAssetId(u32 v) noexcept { Assign(u32(v)); }
  constexpr CAssetId(u64 v) noexcept { Assign(v); }
  explicit CAssetId(CInputStream& in);
  [[nodiscard]] constexpr bool IsValid() const noexcept { return id != UINT64_MAX; }
  [[nodiscard]] constexpr u64 Value() const noexcept { return id; }
  constexpr void Assign(u64 v) noexcept { id = (v == UINT32_MAX ? UINT64_MAX : (v == 0 ? UINT64_MAX : v)); }
  constexpr void Reset() noexcept { id = UINT64_MAX; }
  void PutTo(COutputStream& out) const;
  [[nodiscard]] constexpr bool operator==(CAssetId other) const noexcept { return id == other.id; }
  [[nodiscard]] constexpr bool operator!=(CAssetId other) const noexcept { return !operator==(other); }
  [[nodiscard]] constexpr bool operator<(CAssetId other) const noexcept { return id < other.id; }
};

//#define kInvalidAssetId CAssetId()

struct SObjectTag {
  FourCC type;
  CAssetId id;

  constexpr explicit operator bool() const noexcept { return id.IsValid(); }
  [[nodiscard]] constexpr bool operator==(const SObjectTag& other) const noexcept { return id == other.id; }
  [[nodiscard]] constexpr bool operator!=(const SObjectTag& other) const noexcept { return !operator==(other); }
  [[nodiscard]] constexpr bool operator<(const SObjectTag& other) const noexcept { return id < other.id; }
  constexpr SObjectTag() noexcept = default;
  constexpr SObjectTag(FourCC tp, CAssetId rid) noexcept : type(tp), id(rid) {}
  explicit SObjectTag(CInputStream& in);
  void ReadMLVL(CInputStream& in);
};

struct TEditorId {
  u32 id = UINT32_MAX;

  constexpr TEditorId() noexcept = default;
  constexpr TEditorId(u32 idin) noexcept : id(idin) {}
  [[nodiscard]] constexpr u8 LayerNum() const noexcept { return u8((id >> 26) & 0x3f); }
  [[nodiscard]] constexpr u16 AreaNum() const noexcept { return u16((id >> 16) & 0x3ff); }
  [[nodiscard]] constexpr u16 Id() const noexcept { return u16(id & 0xffff); }
  [[nodiscard]] constexpr bool operator<(TEditorId other) const noexcept {
    return (id & 0x3ffffff) < (other.id & 0x3ffffff);
  }
  [[nodiscard]] constexpr bool operator==(TEditorId other) const noexcept {
    return (id & 0x3ffffff) == (other.id & 0x3ffffff);
  }
  [[nodiscard]] constexpr bool operator!=(TEditorId other) const noexcept { return !operator==(other); }
};

#define kInvalidEditorId TEditorId()

struct TUniqueId {
  kUniqueIdType id = kUniqueIdMax;

  constexpr TUniqueId() noexcept = default;
  constexpr TUniqueId(kUniqueIdType value, kUniqueIdType version) noexcept
  : id(value | (version << kUniqueIdValueBits)) {}
  [[nodiscard]] constexpr kUniqueIdType Version() const noexcept {
    return kUniqueIdType((id >> kUniqueIdValueBits) & kUniqueIdVersionMask);
  }
  [[nodiscard]] constexpr kUniqueIdType Value() const noexcept { return kUniqueIdType(id & kUniqueIdValueMask); }
  [[nodiscard]] constexpr bool operator<(TUniqueId other) const noexcept { return id < other.id; }
  [[nodiscard]] constexpr bool operator==(TUniqueId other) const noexcept { return id == other.id; }
  [[nodiscard]] constexpr bool operator!=(TUniqueId other) const noexcept { return !operator==(other); }
};

#define kInvalidUniqueId TUniqueId()
using EntityList = rstl::reserved_vector<TUniqueId, kMaxEntities>;

using TAreaId = s32;

#define kInvalidAreaId TAreaId(-1)

#if 0
template <class T, size_t N>
class TRoundRobin
{
    rstl::reserved_vector<T, N> vals;

public:
    TRoundRobin(const T& val) : vals(N, val) {}

    void PushBack(const T& val) { vals.push_back(val); }

    size_t Size() const { return vals.size(); }

    const T& GetLastValue() const { return vals.back(); }

    void Clear() { vals.clear(); }

    const T& GetValue(s32) const {}
};
#endif

template <class T>
[[nodiscard]] T GetAverage(const T* v, s32 count) noexcept {
  T r = v[0];
  for (s32 i = 1; i < count; ++i)
    r += v[i];

  return r / count;
}

template <class T, size_t N>
class TReservedAverage : rstl::reserved_vector<T, N> {
public:
  TReservedAverage() = default;

  TReservedAverage(const T& t) { rstl::reserved_vector<T, N>::resize(N, t); }

  void AddValue(const T& t) {
    if (this->size() < N) {
      this->insert(this->begin(), t);
    } else {
      this->pop_back();
      this->insert(this->begin(), t);
    }
  }

  [[nodiscard]] std::optional<T> GetAverage() const {
    if (this->empty()) {
      return std::nullopt;
    }

    return {metaforce::GetAverage<T>(this->data(), this->size())};
  }

  [[nodiscard]] std::optional<T> GetEntry(int i) const {
    if (i >= this->size()) {
      return std::nullopt;
    }
    return this->operator[](i);
  }

  void Clear() { this->clear(); }

  [[nodiscard]] size_t Size() const { return this->size(); }
};

} // namespace metaforce

namespace std {
template <>
struct hash<metaforce::FourCC> {
  size_t operator()(const metaforce::FourCC& val) const noexcept { return val.toUint32(); }
};

template <>
struct hash<metaforce::SObjectTag> {
  size_t operator()(const metaforce::SObjectTag& tag) const noexcept { return tag.id.Value(); }
};

template <>
struct hash<metaforce::CAssetId> {
  size_t operator()(const metaforce::CAssetId& id) const noexcept { return id.Value(); }
};
} // namespace std

FMT_CUSTOM_FORMATTER(metaforce::CAssetId, "{:08X}", obj.Value())
FMT_CUSTOM_FORMATTER(metaforce::TEditorId, "{:08X}", obj.id)
static_assert(sizeof(metaforce::kUniqueIdType) == sizeof(u16),
              "TUniqueId size does not match expected size! Update TUniqueId format string!");
FMT_CUSTOM_FORMATTER(metaforce::TUniqueId, "{:04X}", obj.id)
FMT_CUSTOM_FORMATTER(metaforce::FourCC, "{:c}{:c}{:c}{:c}", obj.getChars()[0], obj.getChars()[1], obj.getChars()[2],
                     obj.getChars()[3])
FMT_CUSTOM_FORMATTER(metaforce::SObjectTag, "{} {}", obj.type, obj.id)

FMT_CUSTOM_FORMATTER(zeus::CVector3f, "({} {} {})", float(obj.x()), float(obj.y()), float(obj.z()))
FMT_CUSTOM_FORMATTER(zeus::CVector2f, "({} {})", float(obj.x()), float(obj.y()))
FMT_CUSTOM_FORMATTER(zeus::CMatrix3f,
                     "\n({} {} {})"
                     "\n({} {} {})"
                     "\n({} {} {})",
                     float(obj[0][0]), float(obj[1][0]), float(obj[2][0]), float(obj[0][1]), float(obj[1][1]),
                     float(obj[2][1]), float(obj[0][2]), float(obj[1][2]), float(obj[2][2]))
FMT_CUSTOM_FORMATTER(zeus::CMatrix4f,
                     "\n({} {} {} {})"
                     "\n({} {} {} {})"
                     "\n({} {} {} {})"
                     "\n({} {} {} {})",
                     float(obj[0][0]), float(obj[1][0]), float(obj[2][0]), float(obj[3][0]), float(obj[0][1]),
                     float(obj[1][1]), float(obj[2][1]), float(obj[3][1]), float(obj[0][2]), float(obj[1][2]),
                     float(obj[2][2]), float(obj[3][2]), float(obj[0][3]), float(obj[1][3]), float(obj[2][3]),
                     float(obj[3][3]))
FMT_CUSTOM_FORMATTER(zeus::CTransform,
                     "\n({} {} {} {})"
                     "\n({} {} {} {})"
                     "\n({} {} {} {})",
                     float(obj.basis[0][0]), float(obj.basis[1][0]), float(obj.basis[2][0]), float(obj.origin[0]),
                     float(obj.basis[0][1]), float(obj.basis[1][1]), float(obj.basis[2][1]), float(obj.origin[1]),
                     float(obj.basis[0][2]), float(obj.basis[1][2]), float(obj.basis[2][2]), float(obj.origin[2]))

#if defined(__has_feature)
#if __has_feature(memory_sanitizer)
#define URDE_MSAN 1
#endif
#endif
