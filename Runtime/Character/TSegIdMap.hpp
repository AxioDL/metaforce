#pragma once

#include <array>
#include <memory>
#include <utility>

#include "Runtime/Character/CSegId.hpp"

namespace urde {

template <class T>
class TSegIdMap {
  CSegId x0_boneCount = 0;
  CSegId x1_capacity = 0;
  u32 x4_maxCapacity = 100;
  std::array<std::pair<CSegId, CSegId>, 100> x8_indirectionMap;
  std::unique_ptr<T[]> xd0_bones;
  CSegId xd4_curPrevBone = 0;

public:
  explicit TSegIdMap(const CSegId& capacity) : x1_capacity(capacity), xd0_bones(new T[capacity]) {}

  T& operator[](const CSegId& id) { return SetElement(id); }
  const T& operator[](const CSegId& id) const { return xd0_bones[x8_indirectionMap[id].second]; }

  T& SetElement(const CSegId& id, T&& obj) {
    size_t idx;

    if (HasElement(id)) {
      idx = x8_indirectionMap[id].second;
    } else {
      x8_indirectionMap[id] = std::make_pair(xd4_curPrevBone, x0_boneCount);
      xd4_curPrevBone = id;
      idx = x0_boneCount;
      ++x0_boneCount;
    }

    xd0_bones[idx] = std::move(obj);
    return xd0_bones[idx];
  }

  T& SetElement(const CSegId& id) {
    size_t idx;

    if (HasElement(id)) {
      idx = x8_indirectionMap[id].second;
    } else {
      x8_indirectionMap[id] = std::make_pair(xd4_curPrevBone, x0_boneCount);
      xd4_curPrevBone = id;
      idx = x0_boneCount;
      ++x0_boneCount;
    }

    return xd0_bones[idx];
  }

  void DelElement(const CSegId& id) {
    if (!HasElement(id)) {
      return;
    }

    if (id == xd4_curPrevBone) {
      xd4_curPrevBone = x8_indirectionMap[id].first;
    }

    x8_indirectionMap[id] = {};
    --x0_boneCount;
  }

  bool HasElement(const CSegId& id) const { return x8_indirectionMap[id].first.IsValid(); }

  u32 GetCapacity() const { return x1_capacity; }
};

} // namespace urde
