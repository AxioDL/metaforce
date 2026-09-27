#include "Kyoto/Animation/CAnimTreeTweenBase.hpp"
#include "Kyoto/Animation/CCharAnimMemoryMetrics.hpp"
#include "Kyoto/Animation/CSegIdList.hpp"
#include "Kyoto/Animation/CSegStatementSet.hpp"
#include "Kyoto/Animation/CAnimMathUtils.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "rstl/auto_ptr.hpp"
#include "rstl/optional_object.hpp"

s32 CAnimTreeTweenBase::sAdvancementDepth = 0;

CAnimTreeTweenBase::CAnimTreeTweenBase(bool b1, const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                       const rstl::ncrc_ptr< CAnimTreeNode >& b, int flags,
                                       const rstl::string& name)
: CAnimTreeDoubleChild(a, b, name), mFlags(flags), mCharacterSpaceBlend(b1), mCullSelector(0) {
  CCharAnimMemoryMetrics::AddToTotalSize(8, CCharAnimMemoryMetrics::kASS_Two);
}

CAnimTreeTweenBase::~CAnimTreeTweenBase() {
  CCharAnimMemoryMetrics::SubtractFromTotalSize(8, CCharAnimMemoryMetrics::kASS_Two);
}

bool CAnimTreeTweenBase::VHasOffset(const CSegId& seg) const {
  return mA->VHasOffset(seg) && mB->VHasOffset(seg);
}

CVector3f CAnimTreeTweenBase::VGetOffset(const CSegId& seg) const {
  float blend_weight = GetBlendingWeight();
  if (blend_weight >= 1.0) {
    return mB->VGetOffset(seg);
  } else {
    CVector3f start_offset = mA->VGetOffset(seg);
    CVector3f end_offset = mB->VGetOffset(seg);
    return start_offset.Lerp(start_offset, end_offset, blend_weight);
  }
}

CQuaternion CAnimTreeTweenBase::VGetRotation(const CSegId& seg) const {
  float blend_weight = GetBlendingWeight();
  if (blend_weight >= 1.0) {
    return mB->VGetRotation(seg);
  } else {
    CQuaternion start_offset = mA->VGetRotation(seg);
    CQuaternion end_offset = mB->VGetRotation(seg);
    return CAnimMathUtils::SlerpLocal(start_offset, end_offset, blend_weight);
  }
}

void CAnimTreeTweenBase::VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut) const {
  float weight = GetBlendingWeight();
  static int sStack = 0;
  ++sStack;
  if (weight >= 1.0) {
    mB->GetSegStatementSet(list, setOut);
  } else if (sStack > 3) {
    const rstl::ncrc_ptr< CAnimTreeNode >& child = weight > 0.5f ? mB : mA;
    rstl::rc_ptr< CAnimTreeNode > best = child->GetBestUnblendedChild();
    if (!best)
      best = child;
    best->GetSegStatementSet(list, setOut);
  } else {
    CStackSegStatementSet setA;
    mA->GetSegStatementSet(list, setA);
    CStackSegStatementSet setB;
    mB->GetSegStatementSet(list, setB);
    int count = list.Size();
    for (int i = 0; i < count; ++i) {
      const CSegId& id = list[i];
      const CQuaternion& rotationA = setA.GetData(id).Orientation();
      if (weight < CAnimMathUtils::kInterpolationThreshold) {
        setOut.Set(id, rotationA);
        if (setA.GetData(id).OffsetValid())
          setOut.Set(id, setA.GetData(id).Offset());
      } else {
        setOut.Set(id, CAnimMathUtils::SlerpLocal(rotationA, setB.GetData(id).Orientation(), weight));
        if (setA.GetData(id).OffsetValid() && setB.GetData(id).OffsetValid())
          setOut.Set(id, CVector3f::Lerp(setA.GetData(id).Offset(), setB.GetData(id).Offset(), weight));
      }
    }
  }
  --sStack;
}

void CAnimTreeTweenBase::VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut,
                                          const CCharAnimTime& time) const {
  float weight = GetBlendingWeight();
  static int sStack = 0;
  ++sStack;
  if (weight >= 1.0) {
    mB->GetSegStatementSet(list, setOut, time);
  } else if (sStack > 3) {
    const rstl::ncrc_ptr< CAnimTreeNode >& child = weight > 0.5f ? mB : mA;
    rstl::rc_ptr< CAnimTreeNode > best = child->GetBestUnblendedChild();
    if (!best)
      best = child;
    best->GetSegStatementSet(list, setOut, time);
  } else {
    CStackSegStatementSet setA;
    mA->GetSegStatementSet(list, setA, time);
    CStackSegStatementSet setB;
    mB->GetSegStatementSet(list, setB, time);
    int count = list.Size();
    for (int i = 0; i < count; ++i) {
      const CSegId& id = list[i];
      const CQuaternion& rotationA = setA.GetData(id).Orientation();
      setOut.Set(id, CAnimMathUtils::SlerpLocal(rotationA, setB.GetData(id).Orientation(), weight));
      if (setA.GetData(id).OffsetValid() && setB.GetData(id).OffsetValid())
        setOut.Set(id, CVector3f::Lerp(setA.GetData(id).Offset(), setB.GetData(id).Offset(), weight));
    }
  }
  --sStack;
}

float CAnimTreeTweenBase::VGetRightChildWeight() const { return GetBlendingWeight(); }

float CAnimTreeTweenBase::GetBlendingWeight() const { return VGetBlendingWeight(); }

bool CAnimTreeTweenBase::ShouldCullTree() { return sAdvancementDepth >= 3; }

rstl::optional_object< rstl::ownership_transfer< IAnimReader > > CAnimTreeTweenBase::VSimplified() {
  if (mCullSelector == 0) {
    rstl::optional_object< rstl::ownership_transfer< IAnimReader > > a = mA->Simplified();
    rstl::optional_object< rstl::ownership_transfer< IAnimReader > > b = mB->Simplified();
    const bool simplifyA = a.valid();
    const bool simplifyB = b.valid();
    if (!simplifyA && !simplifyB)
      return rstl::optional_object_null();
    CAnimTreeTweenBase* clone = static_cast< CAnimTreeTweenBase* >(Clone().take_ownership());
    if (simplifyA)
      clone->ReplaceLeftChild(static_cast< CAnimTreeNode* >(a->take_ownership()));
    if (simplifyB)
      clone->ReplaceRightChild(static_cast< CAnimTreeNode* >(b->take_ownership()));
    return rstl::ownership_transfer< IAnimReader >(clone);
  } else {
    const rstl::ncrc_ptr< CAnimTreeNode >& child = mCullSelector == 1 ? mB : mA;
    rstl::rc_ptr< CAnimTreeNode > best = child->GetBestUnblendedChild();
    if (!best)
      return child->Clone();
    else
      return best->Clone();
  }
}

rstl::optional_object< rstl::ownership_transfer< IAnimReader > > CAnimTreeTweenBase::VReverseSimplified() {
  return CAnimTreeTweenBase::VSimplified();
}

void CAnimTreeTweenBase::VGetWeightedReaders(
    float w, rstl::reserved_vector< rstl::pair< float, IAnimReader* >, 16 >& out) const {
  float blend_weight = GetBlendingWeight();
  mA->VGetWeightedReaders(w * (1.0f - blend_weight), out);
  mB->VGetWeightedReaders(w * blend_weight, out);
}
