#pragma once

#include "Runtime/Character/CPOINode.hpp"

namespace urde {
class IAnimSourceInfo;

class CBoolPOINode : public CPOINode {
  bool x38_val = false;

public:
  CBoolPOINode();
  CBoolPOINode(CInputStream& in);
  bool GetValue() const { return x38_val; }
  static CBoolPOINode CopyNodeMinusStartTime(const CBoolPOINode& node, const CCharAnimTime& startTime);
};

} // namespace urde
