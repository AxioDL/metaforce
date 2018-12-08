#include "CInt32POINode.hpp"
#include "CAnimSourceReader.hpp"

namespace urde {

CInt32POINode::CInt32POINode()
: CPOINode("root", EPOIType::EmptyInt32, CCharAnimTime(), -1, false, 1.f, -1, 0), x38_val(0), x3c_locatorName("root") {}

CInt32POINode::CInt32POINode(CInputStream& in)
: CPOINode(in), x38_val(in.readUint32Big()), x3c_locatorName(in.readString()) {}

CInt32POINode CInt32POINode::CopyNodeMinusStartTime(const CInt32POINode& node, const CCharAnimTime& startTime) {
  CInt32POINode ret = node;
  ret.x1c_time -= startTime;
  return ret;
}

} // namespace urde
