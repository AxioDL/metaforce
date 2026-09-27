#include "Kyoto/Animation/CPrimitive.hpp"

#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/Streams/COutputStream.hpp"

CPrimitive::CPrimitive(CInputStream& in)
: mAnimId(in.Get< CAssetId >()), mAnimIdx(in.Get< int >()), mAnimName(in) {}

void CPrimitive::PutTo(COutputStream& out) const {
  out.Put(mAnimId);
  out.Put(mAnimIdx);
  out.Put(mAnimName);
}
