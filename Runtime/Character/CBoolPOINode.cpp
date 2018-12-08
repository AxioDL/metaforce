#include "CBoolPOINode.hpp"
#include "CAnimSourceReader.hpp"

namespace urde {

CBoolPOINode::CBoolPOINode() : CPOINode("root", EPOIType::EmptyBool, CCharAnimTime(), -1, false, 1.f, -1, 0) {}

CBoolPOINode::CBoolPOINode(CInputStream& in) : CPOINode(in), x38_val(in.readBool()) {}

CBoolPOINode CBoolPOINode::CopyNodeMinusStartTime(const CBoolPOINode& node, const CCharAnimTime& startTime) {
  CBoolPOINode ret = node;
  ret.x1c_time -= startTime;
  return ret;
}

} // namespace urde
