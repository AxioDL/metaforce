#include "Kyoto/Animation/CMetaAnimPhaseBlend.hpp"
#include "Kyoto/Animation/CAnimTreeBlend.hpp"
#include "Kyoto/Animation/CAnimTreeTimeScale.hpp"
#include "Kyoto/Animation/CMetaAnimFactory.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/Streams/COutputStream.hpp"

CMetaAnimPhaseBlend::CMetaAnimPhaseBlend(CInputStream& in)
: mAnimA(CMetaAnimFactory::CreateMetaAnim(in))
, mAnimB(CMetaAnimFactory::CreateMetaAnim(in))
, mBlend(in.ReadFloat())
, mCharacterSpaceBlend(in.ReadBool()) {}

rstl::ncrc_ptr< CAnimTreeNode >
CMetaAnimPhaseBlend::VGetAnimationTree(const CAnimSysContext& animSys,
                                       const CMetaAnimTreeBuildOrders& orders) const {
  if (orders.mRecursiveAdvance) {
    return GetAnimationTree(
        animSys, CMetaAnimTreeBuildOrders::PreAdvanceForAll(*orders.mRecursiveAdvance));
  }

  rstl::ncrc_ptr< CAnimTreeNode > a =
      mAnimA->GetAnimationTree(animSys, CMetaAnimTreeBuildOrders::NoSpecialOrders());
  rstl::ncrc_ptr< CAnimTreeNode > b =
      mAnimB->GetAnimationTree(animSys, CMetaAnimTreeBuildOrders::NoSpecialOrders());
  const CSteadyStateAnimInfo& infoA =
      a->GetContributionOfHighestInfluence().GetSteadyStateAnimInfo();
  CCharAnimTime durationA = infoA.GetDuration();
  const CSteadyStateAnimInfo& infoB =
      b->GetContributionOfHighestInfluence().GetSteadyStateAnimInfo();
  CCharAnimTime durationB = infoB.GetDuration();
  CCharAnimTime duration = durationA + (durationB - durationA) * mBlend;
  float scaleA = durationA / duration;
  float scaleB = durationB / duration;
  rstl::ncrc_ptr< CAnimTreeNode > scaledA = rs_new CAnimTreeTimeScale(
      a, scaleA,
      CAnimTreeTimeScale::CreatePrimitiveName(a, scaleA, CCharAnimTime::Infinity(), -1.f));
  rstl::ncrc_ptr< CAnimTreeNode > scaledB = rs_new CAnimTreeTimeScale(
      b, scaleB,
      CAnimTreeTimeScale::CreatePrimitiveName(b, scaleB, CCharAnimTime::Infinity(), -1.f));
  return rs_new CAnimTreeBlend(mCharacterSpaceBlend, scaledA, scaledB, mBlend,
                               CAnimTreeBlend::CreatePrimitiveName(scaledA, scaledB, mBlend));
}

void CMetaAnimPhaseBlend::GetUniquePrimitives(rstl::set< CPrimitive >& primsOut) const {
  mAnimA->GetUniquePrimitives(primsOut);
  mAnimB->GetUniquePrimitives(primsOut);
}

void CMetaAnimPhaseBlend::WriteAnimData(COutputStream& out) const {
  mAnimA->PutTo(out);
  mAnimB->PutTo(out);
  out.WriteReal32(mBlend);
  out.WriteChar(bool(mCharacterSpaceBlend));
}
