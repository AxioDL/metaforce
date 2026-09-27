#include "Kyoto/Animation/CMetaTransMetaAnim.hpp"
#include "Kyoto/Animation/CAnimTreeLoopIn.hpp"
#include "Kyoto/Animation/CMetaAnimFactory.hpp"
#include "Kyoto/Animation/IMetaAnim.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/Streams/COutputStream.hpp"

CMetaTransMetaAnim::CMetaTransMetaAnim(CInputStream& in)
: mMetaAnim(CMetaAnimFactory::CreateMetaAnim(in)) {}

rstl::ncrc_ptr< CAnimTreeNode >
CMetaTransMetaAnim::VGetTransitionTree(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                       const rstl::ncrc_ptr< CAnimTreeNode >& b,
                                       const CAnimSysContext& animSys) const {
  rstl::ncrc_ptr< CAnimTreeNode > animNode =
      mMetaAnim->GetAnimationTree(animSys, CMetaAnimTreeBuildOrders::NoSpecialOrders());
  return rs_new CAnimTreeLoopIn(a, b, animNode, animSys,
                                CAnimTreeLoopIn::CreatePrimitiveName(a, b, animNode));
}

void CMetaTransMetaAnim::WriteTransData(COutputStream& out) const { mMetaAnim->PutTo(out); }
