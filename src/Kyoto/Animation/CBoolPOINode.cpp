#include "Kyoto/Animation/CBoolPOINode.hpp"

#include "Kyoto/Streams/CInputStream.hpp"

CBoolPOINode::CBoolPOINode(CInputStream& in) : CPOINode(in), mVal(in.ReadBool()) {}

CBoolPOINode CBoolPOINode::CopyNodeMinusStartTime(const CBoolPOINode& node,
                                                  const CCharAnimTime& startTime) {
  return CBoolPOINode(node.GetString(), node.GetPoiType(), node.GetTime() - startTime,
                      node.GetIndex(), node.GetSaveState(), node.GetWeight(),
                      node.GetCharacterIndex(), node.GetFlags(), node.GetValue());
}
