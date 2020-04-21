#include "Runtime/Character/CAnimTreeTweenBase.hpp"

#include "Runtime/Character/CSegIdList.hpp"
#include "Runtime/Character/CSegStatementSet.hpp"

namespace urde {

s32 CAnimTreeTweenBase::sAdvancementDepth = 0;

CAnimTreeTweenBase::CAnimTreeTweenBase(bool b1, const std::weak_ptr<CAnimTreeNode>& a,
                                       const std::weak_ptr<CAnimTreeNode>& b, int flags, std::string_view name)
: CAnimTreeDoubleChild(a, b, name), x1c_flags(flags), x20_24_b1{b1} {}

void CAnimTreeTweenBase::VGetWeightedReaders(
    rstl::reserved_vector<std::pair<float, std::weak_ptr<IAnimReader>>, 16>& out, float w) const {
  float weight = GetBlendingWeight();
  x14_a->VGetWeightedReaders(out, (1.f - weight) * w);
  x18_b->VGetWeightedReaders(out, weight * w);
}

void CAnimTreeTweenBase::VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut) const {
  float w = GetBlendingWeight();
  static int sStack = 0;
  ++sStack;
  if (w >= 1.f) {
    x18_b->VGetSegStatementSet(list, setOut);
  } else if (sStack > 3) {
    auto& n = w > 0.5f ? x18_b : x14_a;
    auto ptr = n->GetBestUnblendedChild();
    if (!ptr)
      ptr = n;
    ptr->VGetSegStatementSet(list, setOut);
  } else {
    CSegStatementSet setA, setB;
    x14_a->VGetSegStatementSet(list, setA);
    x18_b->VGetSegStatementSet(list, setB);
    for (CSegId id : list.GetList()) {
      if (w < 0.0001f) {
        setOut[id].x0_rotation = setA[id].x0_rotation;
        if (setA[id].x1c_hasOffset) {
          setOut[id].x10_offset = setA[id].x10_offset;
          setOut[id].x1c_hasOffset = true;
        }
      } else {
        setOut[id].x0_rotation = zeus::CQuaternion::slerpShort(setA[id].x0_rotation, setB[id].x0_rotation, w);
        if (setA[id].x1c_hasOffset && setB[id].x1c_hasOffset) {
          setOut[id].x10_offset = zeus::CVector3f::lerp(setA[id].x10_offset, setB[id].x10_offset, w);
          setOut[id].x1c_hasOffset = true;
        }
      }
    }
  }
  --sStack;
}

void CAnimTreeTweenBase::VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut,
                                             const CCharAnimTime& time) const {
  float w = GetBlendingWeight();
  static int sStack = 0;
  ++sStack;
  if (w >= 1.f) {
    x18_b->VGetSegStatementSet(list, setOut, time);
  } else if (sStack > 3) {
    auto& n = w > 0.5f ? x18_b : x14_a;
    n->GetBestUnblendedChild()->VGetSegStatementSet(list, setOut, time);
  } else {
    CSegStatementSet setA, setB;
    x14_a->VGetSegStatementSet(list, setA, time);
    x18_b->VGetSegStatementSet(list, setB, time);
    for (CSegId id : list.GetList()) {
      setOut[id].x0_rotation = zeus::CQuaternion::slerpShort(setA[id].x0_rotation, setB[id].x0_rotation, w);
      if (setA[id].x1c_hasOffset && setB[id].x1c_hasOffset) {
        setOut[id].x10_offset = zeus::CVector3f::lerp(setA[id].x10_offset, setB[id].x10_offset, w);
        setOut[id].x1c_hasOffset = true;
      }
    }
  }
  --sStack;
}

bool CAnimTreeTweenBase::VHasOffset(const CSegId& seg) const {
  return (x14_a->VHasOffset(seg) && x18_b->VHasOffset(seg));
}

zeus::CVector3f CAnimTreeTweenBase::VGetOffset(const CSegId& seg) const {
  const float weight = GetBlendingWeight();
  if (weight >= 1.0f)
    return x18_b->VGetOffset(seg);

  const zeus::CVector3f oA = x14_a->VGetOffset(seg);
  const zeus::CVector3f oB = x18_b->VGetOffset(seg);
  return zeus::CVector3f::lerp(oA, oB, weight);
}

zeus::CQuaternion CAnimTreeTweenBase::VGetRotation(const CSegId& seg) const {
  const float weight = GetBlendingWeight();
  if (weight >= 1.0f)
    return x18_b->VGetRotation(seg);

  const zeus::CQuaternion qA = x14_a->VGetRotation(seg);
  const zeus::CQuaternion qB = x18_b->VGetRotation(seg);
  return zeus::CQuaternion::slerp(qA, qB, weight);
}

std::optional<std::unique_ptr<IAnimReader>> CAnimTreeTweenBase::VSimplified() {
  if (x20_25_cullSelector == 0) {
    auto simpA = x14_a->Simplified();
    auto simpB = x18_b->Simplified();
    if (!simpA && !simpB)
      return {};
    auto clone = Clone();
    if (simpA)
      static_cast<CAnimTreeTweenBase&>(*clone).x14_a = CAnimTreeNode::Cast(std::move(*simpA));
    if (simpB)
      static_cast<CAnimTreeTweenBase&>(*clone).x18_b = CAnimTreeNode::Cast(std::move(*simpB));
    return {std::move(clone)};
  } else {
    auto tmp = (x20_25_cullSelector == 1) ? x18_b : x14_a;
    auto tmpUnblended = tmp->GetBestUnblendedChild();
    if (!tmpUnblended)
      return {tmp->Clone()};
    else
      return {tmpUnblended->Clone()};
  }
}
} // namespace urde
