#include "Kyoto/Animation/CMetaAnimBlend.hpp"
#include "Kyoto/Animation/CAnimTreeBlend.hpp"
#include "Kyoto/Animation/CMetaAnimFactory.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/Streams/COutputStream.hpp"

CMetaAnimBlend::CMetaAnimBlend(CInputStream& in)
: mAnimA(CMetaAnimFactory::CreateMetaAnim(in))
, mAnimB(CMetaAnimFactory::CreateMetaAnim(in))
, mBlend(in.ReadFloat())
, mCharacterSpaceBlend(in.ReadBool()) {}

rstl::ncrc_ptr< CAnimTreeNode >
CMetaAnimBlend::VGetAnimationTree(const CAnimSysContext& animSys,
                                  const CMetaAnimTreeBuildOrders& orders) const {
  CMetaAnimTreeBuildOrders oa = CMetaAnimTreeBuildOrders::NoSpecialOrders();
  CMetaAnimTreeBuildOrders ob =
      orders.mRecursiveAdvance
          ? CMetaAnimTreeBuildOrders::PreAdvanceForAll(*orders.mRecursiveAdvance)
          : CMetaAnimTreeBuildOrders::NoSpecialOrders();
  rstl::ncrc_ptr< CAnimTreeNode > a = mAnimA->GetAnimationTree(animSys, oa);
  rstl::ncrc_ptr< CAnimTreeNode > b = mAnimB->GetAnimationTree(animSys, ob);
  return rs_new CAnimTreeBlend(mCharacterSpaceBlend, a, b, mBlend,
                               CAnimTreeBlend::CreatePrimitiveName(a, b, mBlend));
}

void CMetaAnimBlend::GetUniquePrimitives(rstl::set< CPrimitive >& primsOut) const {
  mAnimA->GetUniquePrimitives(primsOut);
  mAnimB->GetUniquePrimitives(primsOut);
}

void CMetaAnimBlend::WriteAnimData(COutputStream& out) const {
  mAnimA->PutTo(out);
  mAnimB->PutTo(out);
  out.WriteReal32(mBlend);
  out.WriteChar(mCharacterSpaceBlend ? 1 : 0);
}
