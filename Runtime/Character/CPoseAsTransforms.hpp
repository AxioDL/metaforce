#pragma once

#include "RetroTypes.hpp"
#include "CSegId.hpp"
#include "zeus/CTransform.hpp"

namespace urde {
class CCharLayoutInfo;

class CPoseAsTransforms {
public:
  struct Transform {
    zeus::CTransform m_originToAccum;
    zeus::CTransform m_restPoseToAccum;
  };

private:
  CSegId x0_nextId = 0;
  CSegId x1_count;
  std::pair<CSegId, CSegId> x8_links[100];
  std::unique_ptr<Transform[]> xd0_transformArr;
  CSegId xd4_lastInserted = 0;

public:
  CPoseAsTransforms(u8 boneCount);
  bool ContainsDataFor(const CSegId& id) const;
  void Clear();
  void AccumulateScaledTransform(const CSegId& id, zeus::CMatrix3f& rotation, float scale) const;
  const zeus::CTransform& GetTransform(const CSegId& id) const;
  const zeus::CTransform& GetRestToAccumTransform(const CSegId& id) const;
  const zeus::CVector3f& GetOffset(const CSegId& id) const;
  const zeus::CMatrix3f& GetRotation(const CSegId& id) const;
  const zeus::CMatrix3f& GetTransformMinusOffset(const CSegId& id) const { return GetRotation(id); }
  void Insert(const CSegId& id, const zeus::CMatrix3f& rotation, const zeus::CVector3f& offset,
              const zeus::CVector3f& restOffset);
};

} // namespace urde
