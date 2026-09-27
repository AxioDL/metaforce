#include "Kyoto/Animation/IMetaAnim.hpp"

#include "Kyoto/Animation/CAnimTreeNode.hpp"
#include "Kyoto/Animation/CBoolPOINode.hpp"
#include "Kyoto/Streams/COutputStream.hpp"
#include <string.h>

bool CPreAdvanceIndicator::IsTime() const { return mIsTime; }

const CCharAnimTime& CPreAdvanceIndicator::GetTime() const { return mTime; }

bool CPreAdvanceIndicator::IsString() const { return !IsTime(); }

const char* CPreAdvanceIndicator::GetString() const { return mString; }

CMetaAnimTreeBuildOrders CMetaAnimTreeBuildOrders::NoSpecialOrders() {
  return CMetaAnimTreeBuildOrders();
}
CMetaAnimTreeBuildOrders
CMetaAnimTreeBuildOrders::PreAdvanceForAll(const CPreAdvanceIndicator& ind) {
  CMetaAnimTreeBuildOrders ret;
  ret.mSingleAdvance = ind;
  return ret;
}

rstl::ncrc_ptr< CAnimTreeNode >
IMetaAnim::GetAnimationTree(const CAnimSysContext& animSys,
                            const CMetaAnimTreeBuildOrders& orders) const {
  if (orders.mSingleAdvance) {
    rstl::ncrc_ptr< CAnimTreeNode > tree =
        VGetAnimationTree(animSys, CMetaAnimTreeBuildOrders::NoSpecialOrders());
    if (orders.mSingleAdvance->IsTime() || orders.mSingleAdvance->IsString()) {
      AdvanceAnim(*tree, GetTime(*orders.mSingleAdvance, *tree));
    }
    return tree;
  }
  if (orders.mRecursiveAdvance) {
    rstl::ncrc_ptr< CAnimTreeNode > tree =
        VGetAnimationTree(animSys, CMetaAnimTreeBuildOrders::NoSpecialOrders());
    if (orders.mRecursiveAdvance->IsTime() || orders.mRecursiveAdvance->IsString()) {
      AdvanceAnim(*tree, GetTime(*orders.mRecursiveAdvance, *tree));
    }
    return tree;
  }
  return VGetAnimationTree(animSys, CMetaAnimTreeBuildOrders::NoSpecialOrders());
}

void IMetaAnim::PutTo(COutputStream& out) const {
  out.WriteInt32(GetType());
  WriteAnimData(out);
}

CCharAnimTime IMetaAnim::GetTime(const CPreAdvanceIndicator& ind, const IAnimReader& anim) {
  if (ind.IsTime())
    return ind.GetTime();

  CBoolPOINode nodes[64];
  const char* name = ind.GetString();
  const uint count = anim.GetBoolPOIList(anim.GetTimeRemaining(), nodes, 64, 0, 0);
  for (uint i = 0; i < count; ++i) {
    const CBoolPOINode& node = nodes[i];
    if (strcmp(node.GetString().data(), name) == 0 && node.GetValue())
      return node.GetTime();
  }
  return CCharAnimTime::ZeroFlat();
}

void IMetaAnim::AdvanceAnim(IAnimReader& anim, const CCharAnimTime& dt) {
  CCharAnimTime remaining = dt;
  while (remaining > CCharAnimTime::ZeroFlat()) {
    CAdvancementResults result = anim.AdvanceView(remaining);
    remaining = result.GetRemainder();
  }
}
