#pragma once

#include "Runtime/RetroTypes.hpp"
#include "Runtime/rstl.hpp"
#include "Runtime/Collision/CCollisionInfo.hpp"

namespace urde {
class CCollisionInfoList {
  rstl::reserved_vector<CCollisionInfo, 32> x0_list;

public:
  CCollisionInfoList() = default;

  zeus::CVector3f GetAverageLeftNormal() const {
    zeus::CVector3f ret;
    for (const auto& inf : x0_list) {
      ret += inf.GetNormalLeft();
    }

    return ret / x0_list.size();
  }
  zeus::CVector3f GetAveragePoint() const {
    zeus::CVector3f ret;
    for (const auto& inf : x0_list) {
      ret += inf.GetPoint();
    }

    return ret / x0_list.size();
  }
  CMaterialList GetUnionOfAllLeftMaterials() const {
    CMaterialList list;
    for (const auto& inf : x0_list) {
      list.Union(inf.GetMaterialLeft());
    }

    return list;
  }
  size_t GetCount() const { return x0_list.size(); }
  void Swap(s32 idx) {
    if (idx >= x0_list.size())
      return;
    x0_list[idx].Swap();
  }

  void Add(const CCollisionInfo& info, bool swap) {
    if (x0_list.size() == 32)
      return;
    if (!swap)
      x0_list.push_back(info);
    else
      x0_list.push_back(info.GetSwapped());
  }
  void Clear() { x0_list.clear(); }
  const CCollisionInfo& Front() const { return x0_list.front(); }
  const CCollisionInfo& GetItem(int i) const { return x0_list[i]; }
  rstl::reserved_vector<CCollisionInfo, 32>::iterator end() { return x0_list.end(); }
  rstl::reserved_vector<CCollisionInfo, 32>::const_iterator end() const { return x0_list.end(); }
  rstl::reserved_vector<CCollisionInfo, 32>::iterator begin() { return x0_list.begin(); }
  rstl::reserved_vector<CCollisionInfo, 32>::const_iterator begin() const { return x0_list.begin(); }
};
} // namespace urde
