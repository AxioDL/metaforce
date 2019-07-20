#include "CInt32POINode.hpp"
#include "CAnimSourceReader.hpp"

namespace urde {
CInt32POINode::CInt32POINode()
: CInt32POINode(""sv, EPOIType::EmptyInt32, CCharAnimTime(), -1, false, 1.f, -1, 0, 0, "root"sv) {}

CInt32POINode::CInt32POINode(std::string_view name, EPOIType type, const CCharAnimTime& time, s32 index, bool unique, float weight, s32 charIndex,
                             s32 flags, s32 val, std::string_view locator)
: CPOINode(name, type, time, index, unique, weight, charIndex, flags), x38_val(val), x3c_locatorName(locator) {}

CInt32POINode::CInt32POINode(CInputStream& in)
: CPOINode(in), x38_val(in.readUint32Big()), x3c_locatorName(in.readString()) {}

CInt32POINode CInt32POINode::CopyNodeMinusStartTime(const CInt32POINode& node, const CCharAnimTime& startTime) {
  CInt32POINode ret = node;
  ret.x1c_time -= startTime;
  return ret;
}

} // namespace urde
