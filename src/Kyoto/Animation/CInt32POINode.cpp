#include "Kyoto/Animation/CInt32POINode.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

CInt32POINode::CInt32POINode(CInputStream& in)
: CPOINode(in), mVal(in.ReadInt32()), mLctrName(in) {}

CInt32POINode CInt32POINode::CopyNodeMinusStartTime(const CInt32POINode& node,
                                                    const CCharAnimTime& startTime) {
  return CInt32POINode(node.GetString(), node.GetPoiType(), node.GetTime() - startTime, node.GetIndex(),
                       node.GetSaveState(), node.GetWeight(), node.GetCharacterIndex(), node.GetFlags(),
                       node.GetValue(), node.GetLocatorName());
}
