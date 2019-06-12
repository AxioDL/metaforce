#pragma once

#include <vector>
#include <utility>
#include <string>
#include <functional>
#include "GCNTypes.hpp"
#include "rstl.hpp"
#include "IOStreams.hpp"
#include "hecl/hecl.hpp"

#undef min
#undef max

using namespace std::literals;

namespace urde {

using FourCC = hecl::FourCC;

class CAssetId {
  u64 id = UINT64_MAX;

public:
  CAssetId() = default;
  CAssetId(u64 v) { Assign(v); }
  explicit CAssetId(CInputStream& in);
  bool IsValid() const { return id != UINT64_MAX; }
  u64 Value() const { return id; }
  void Assign(u64 v) { id = (v == UINT32_MAX ? UINT64_MAX : (v == 0 ? UINT64_MAX : v)); }
  void Reset() { id = UINT64_MAX; }
  void PutTo(COutputStream& out);
  bool operator==(const CAssetId& other) const { return id == other.id; }
  bool operator!=(const CAssetId& other) const { return id != other.id; }
  bool operator<(const CAssetId& other) const { return id < other.id; }
};

//#define kInvalidAssetId CAssetId()

struct SObjectTag {
  FourCC type;
  CAssetId id;

  operator bool() const { return id.IsValid(); }
  bool operator!=(const SObjectTag& other) const { return id != other.id; }
  bool operator==(const SObjectTag& other) const { return id == other.id; }
  bool operator<(const SObjectTag& other) const { return id < other.id; }
  SObjectTag() = default;
  SObjectTag(FourCC tp, CAssetId rid) : type(tp), id(rid) {}
  SObjectTag(CInputStream& in) {
    in.readBytesToBuf(&type, 4);
    id = CAssetId(in);
  }
  void readMLVL(CInputStream& in) {
    id = CAssetId(in);
    in.readBytesToBuf(&type, 4);
  }
};

struct TEditorId {
  TEditorId() = default;
  TEditorId(u32 idin) : id(idin) {}
  u32 id = u32(-1);
  u8 LayerNum() const { return u8((id >> 26) & 0x3f); }
  u16 AreaNum() const { return u16((id >> 16) & 0x3ff); }
  u16 Id() const { return u16(id & 0xffff); }
  bool operator<(const TEditorId& other) const { return (id & 0x3ffffff) < (other.id & 0x3ffffff); }
  bool operator!=(const TEditorId& other) const { return (id & 0x3ffffff) != (other.id & 0x3ffffff); }
  bool operator==(const TEditorId& other) const { return (id & 0x3ffffff) == (other.id & 0x3ffffff); }
};

#define kInvalidEditorId TEditorId()

struct TUniqueId {
  TUniqueId() = default;
  TUniqueId(u16 value, u16 version) : id(value | (version << 10)) {}
  u16 id = u16(-1);
  u16 Version() const { return u16((id >> 10) & 0x3f); }
  u16 Value() const { return u16(id & 0x3ff); }
  bool operator<(const TUniqueId& other) const { return (id < other.id); }
  bool operator!=(const TUniqueId& other) const { return (id != other.id); }
  bool operator==(const TUniqueId& other) const { return (id == other.id); }
};

#define kInvalidUniqueId TUniqueId()

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
T GetAverage(const T* v, s32 count) {
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

  std::optional<T> GetAverage() const {
    if (this->empty())
      return {};

    return {urde::GetAverage<T>(this->data(), this->size())};
  }

  std::optional<T> GetEntry(int i) const {
    if (i >= this->size())
      return {};
    return this->operator[](i);
  }

  void Clear() { this->clear(); }

  size_t Size() const { return this->size(); }
};

} // namespace urde

namespace std {
template <>
struct hash<urde::SObjectTag> {
  size_t operator()(const urde::SObjectTag& tag) const noexcept { return tag.id.Value(); }
};

template <>
struct hash<urde::CAssetId> {
  size_t operator()(const urde::CAssetId& id) const noexcept { return id.Value(); }
};
} // namespace std

#if defined(__has_feature)
#if __has_feature(memory_sanitizer)
#define URDE_MSAN 1
#endif
#endif
