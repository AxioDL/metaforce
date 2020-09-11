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

  auto end() noexcept { return x0_list.end(); }
  auto end() const noexcept { return x0_list.end(); }
  auto begin() noexcept { return x0_list.begin(); }
  auto begin() const noexcept { return x0_list.begin(); }

  

  
  void AccumulateNewContactsInto(CCollisionInfoList& other_list) {
    for (CCollisionInfo const& cur_info : x0_list) {
      bool dont_add_new_info = false;
      for (CCollisionInfo& other_info : other_list) {
        if (!zeus::close_enough(other_info.GetPoint(), cur_info.GetPoint(), 0.1f)) {
          continue;
        }
        zeus::CVector3f norm = other_info.GetNormalLeft().normalized();
        if (zeus::close_enough(norm, cur_info.GetNormalLeft(), 1.2f)) {
          dont_add_new_info = true;
          other_info.x0_point = (other_info.x0_point + cur_info.x0_point) * 0.5f;
          other_info.x38_materialLeft.Add(cur_info.x38_materialLeft);
          other_info.x40_materialRight.Add(cur_info.x40_materialRight);
          other_info.x48_normalLeft = other_info.x48_normalLeft + cur_info.x48_normalLeft;
          break;
        }
      }
      if (!dont_add_new_info) {
        other_list.Add(cur_info, false);
      }
    }
    for (CCollisionInfo& other_info : other_list.x0_list) {
      other_info.x48_normalLeft.normalize();
      other_info.x54_normalRight = -other_info.x48_normalLeft;
    }
  }
};
} // namespace urde
