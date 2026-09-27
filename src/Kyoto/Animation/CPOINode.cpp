#include "Kyoto/Animation/CPOINode.hpp"

#include "Kyoto/Streams/CInputStream.hpp"

CPOINode::CPOINode(const rstl::string& name, ushort type, const CCharAnimTime& time, int index,
                   bool unique, float weight, int charIdx, int flags)
: x4_(1)
, mName(name)
, mType(type)
, mTime(time)
, mIndex(index)
, mUnique(unique)
, mWeight(weight)
, mCharIdx(charIdx)
, mFlags(flags) {}

CPOINode::CPOINode(CInputStream& in)
: x4_(in.Get<ushort>())
, mName(in)
, mType(in.Get<ushort>())
, mTime(in)
, mIndex(in.Get<uint>())
, mUnique(in.Get<bool>())
, mWeight(in.Get<float>())
, mCharIdx(in.Get<int>())
, mFlags(in.Get<int>()) {
  
}


bool CPOINode::operator>(const CPOINode& other) const {
  return mTime > other.mTime;
}

bool CPOINode::operator<(const CPOINode& other) const {
  return mTime < other.mTime;
}

int CPOINode::compare(const void* a, const void* b) {
  const CPOINode& aNode = *static_cast<const CPOINode*>(a);
  const CPOINode& bNode = *static_cast<const CPOINode*>(b);
  return aNode > bNode ? 1 : aNode < bNode ? -1 : 0;
}
