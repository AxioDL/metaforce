#ifndef _CBOOLPOINODE
#define _CBOOLPOINODE

#include "Kyoto/Animation/CPOINode.hpp"

class CBoolPOINode : public CPOINode {
public:
  CBoolPOINode(const rstl::string name = "", const EPOIType type = kPT_EmptyBool,
               const CCharAnimTime& time = CCharAnimTime(), const int index = -1,
               const bool unique = false, const float weight = 1.f, const int charIdx = -1,
               const int flags = 0, const bool value = false)
  : CPOINode(name, type, time, index, unique, weight, charIdx, flags), mVal(value) {}

  CBoolPOINode(CInputStream& in);
  static CBoolPOINode CopyNodeMinusStartTime(const CBoolPOINode& node,
                                             const CCharAnimTime& startTime);
  const bool GetValue() const { return mVal; }

private:
  bool mVal;
};

#endif // _CBOOLPOINODE
