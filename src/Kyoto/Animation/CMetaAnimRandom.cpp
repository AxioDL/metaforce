#include "Kyoto/Animation/CMetaAnimRandom.hpp"
#include "Kyoto/Animation/CAnimSysContext.hpp"
#include "Kyoto/Animation/CAnimTreeNode.hpp"
#include "Kyoto/Animation/CMetaAnimFactory.hpp"
#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/Streams/COutputStream.hpp"
#include "rstl/rc_ptr.hpp"

CMetaAnimRandom::CMetaAnimRandom(CInputStream& in) : mRandomData(CreateRandomData(in)) {}

rstl::ncrc_ptr< CAnimTreeNode >
CMetaAnimRandom::VGetAnimationTree(const CAnimSysContext& animSys,
                                   const CMetaAnimTreeBuildOrders& orders) const {
  const int r = animSys.GetRandomNumberGenerator().Range(1, 100);

  CMetaAnimRandom::RandomData::const_iterator rd = mRandomData.begin();
  bool found = false;
  while (!found) {
    if (r <= rd->second) {
      found = true;
    } else {
      rd++;
    }
  }

  const rstl::ncrc_ptr< CAnimTreeNode >& tree = rd->first->GetAnimationTree(animSys, orders);
  return tree;
}

void CMetaAnimRandom::GetUniquePrimitives(rstl::set< CPrimitive >& primsOut) const {
  CMetaAnimRandom::RandomData::const_iterator it = mRandomData.begin();
  CMetaAnimRandom::RandomData::const_iterator end = mRandomData.end();
  for (; it != end; ++it)
    it->first->GetUniquePrimitives(primsOut);
}

void CMetaAnimRandom::WriteAnimData(COutputStream& out) const {
  CMetaAnimRandom::RandomData::const_iterator it = mRandomData.begin();
  CMetaAnimRandom::RandomData::const_iterator end = mRandomData.end();
  out.WriteInt32(mRandomData.size());
  while (it != end) {
    rstl::rc_ptr< IMetaAnim > anim = it->first;
    int weight = it->second;
    anim->PutTo(out);
    out.WriteLong(weight);
    ++it;
  }
}

CMetaAnimRandom::RandomData CMetaAnimRandom::CreateRandomData(CInputStream& in) {
  CMetaAnimRandom::RandomData ret;
  int randCount = in.Get< int >();
  ret.reserve(randCount);

  for (int i = 0; i < randCount; ++i) {
    rstl::rc_ptr< IMetaAnim > metaAnim = CMetaAnimFactory::CreateMetaAnim(in);
    ret.push_back(rstl::pair< rstl::rc_ptr< IMetaAnim >, int >(metaAnim, in.ReadLong()));
  }

  return ret;
}
