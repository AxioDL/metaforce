#pragma once

#include <array>
#include <memory>
#include <utility>

#include "Runtime/Character/CCharLayoutInfo.hpp"
#include "Runtime/Character/CSegId.hpp"
#include "Runtime/RetroTypes.hpp"

#include <zeus/CMatrix3f.hpp>
#include <zeus/CTransform.hpp>
#include <zeus/CVector3f.hpp>

namespace metaforce {
class CPoseAsTransforms {
  friend class CAnimData;

private:
  CSegId x0_nextId = 0;
  CSegId x1_count;
  std::array<std::pair<CSegId, CSegId>, 100> x8_links;
  std::unique_ptr<zeus::CTransform[]> xd0_transformArr;
  CSegId xd4_lastInserted = 0;

public:
  explicit CPoseAsTransforms(u8 boneCount);

  void Clear();
  void AccumulateScaledTransform(const CSegId& id, zeus::CMatrix3f& rotation, float scale) const;
  void Insert(const CSegId& id, const zeus::CMatrix3f& rotation, const zeus::CVector3f& offset);

  [[nodiscard]] bool ContainsDataFor(const CSegId& id) const;
  [[nodiscard]] const zeus::CTransform& GetTransform(const CSegId& id) const;
  [[nodiscard]] const zeus::CVector3f& GetOffset(const CSegId& id) const;
  [[nodiscard]] const zeus::CMatrix3f& GetRotation(const CSegId& id) const;
  [[nodiscard]] CSegId GetLastInserted() const { return xd4_lastInserted; }
  [[nodiscard]] CSegId GetParent(const CSegId& id) const;
};

} // namespace metaforce
