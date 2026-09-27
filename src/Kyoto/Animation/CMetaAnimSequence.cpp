#include "Kyoto/Animation/CMetaAnimSequence.hpp"
#include "Kyoto/Animation/CAnimTreeSequence.hpp"
#include "Kyoto/Animation/CMetaAnimFactory.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/Streams/COutputStream.hpp"

CMetaAnimSequence::CMetaAnimSequence(CInputStream& in)
: mSequence(CMetaAnimSequence::CreateSequence(in)) {}

rstl::ncrc_ptr< CAnimTreeNode >
CMetaAnimSequence::VGetAnimationTree(const CAnimSysContext& animSys,
                                     const CMetaAnimTreeBuildOrders& orders) const {
  if (orders.mRecursiveAdvance) {
    return GetAnimationTree(
        animSys, CMetaAnimTreeBuildOrders::PreAdvanceForAll(*orders.mRecursiveAdvance));
  }

  AUTO(it, mSequence.begin());
  AUTO(end, mSequence.end());
  rstl::vector< rstl::string > names;
  names.reserve(end - it);
  while (it != end) {
    rstl::rc_ptr< IMetaAnim > anim = *it;
    rstl::ncrc_ptr< CAnimTreeNode > tree = anim->GetAnimationTree(animSys, orders);
    names.push_back(tree->GetPrimitiveName());
    ++it;
  }
  return rs_new CAnimTreeSequence(mSequence, animSys,
                                  CAnimTreeSequence::CreatePrimitiveName(names));
}

void CMetaAnimSequence::GetUniquePrimitives(rstl::set< CPrimitive >& primsOut) const {
  rstl::vector< rstl::rc_ptr< IMetaAnim > >::const_iterator it = mSequence.begin();
  rstl::vector< rstl::rc_ptr< IMetaAnim > >::const_iterator end = mSequence.end();
  while (it != end) {
    it->GetPtr()->GetUniquePrimitives(primsOut);
    ++it;
  }
}

void CMetaAnimSequence::WriteAnimData(COutputStream& out) const {
  rstl::vector< rstl::rc_ptr< IMetaAnim > >::const_iterator it = mSequence.begin();
  rstl::vector< rstl::rc_ptr< IMetaAnim > >::const_iterator end = mSequence.end();
  out.WriteLong(mSequence.size());
  while (it != end) {
    it->GetPtr()->PutTo(out);
    ++it;
  }
}

rstl::vector< rstl::rc_ptr< IMetaAnim > > CMetaAnimSequence::CreateSequence(CInputStream& in) {
  rstl::vector< rstl::rc_ptr< IMetaAnim > > sequence;
  int size = in.Get< int >();
  sequence.reserve(size);
  for (int i = 0; i < size; i++) {
    rstl::rc_ptr< IMetaAnim > anim = CMetaAnimFactory::CreateMetaAnim(in);
    sequence.push_back(anim);
  }
  return sequence;
}
