#ifndef _CBOOLPOINODE
#define _CBOOLPOINODE

#include "Kyoto/Animation/CPOINode.hpp"

class CBoolPOINode : public CPOINode {
public:
  CBoolPOINode(rstl::string name = "", ushort type = kPT_EmptyBool,
               const CCharAnimTime& time = CCharAnimTime(), int index = -1, bool unique = false,
               float weight = 1.f, int charIdx = -1, int flags = 0, bool value = false)
  : CPOINode(name, type, time, index, unique, weight, charIdx, flags), mVal(value) {}

  CBoolPOINode(CInputStream& in);
  static CBoolPOINode CopyNodeMinusStartTime(const CBoolPOINode& node,
                                             const CCharAnimTime& startTime);
  const bool GetValue() const { return mVal; }

private:
  bool mVal;
};

#endif // _CBOOLPOINODE
