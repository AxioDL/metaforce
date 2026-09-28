#ifndef _CINT32POINODE
#define _CINT32POINODE

#include "Kyoto/Animation/CPOINode.hpp"

class CInt32POINode : public CPOINode {
public:
  CInt32POINode(const rstl::string name = rstl::string_l(""), const EPOIType type = kPT_EmptyInt32,
                const CCharAnimTime& time = CCharAnimTime(), const int index = -1,
                const bool unique = false, const float weight = 1.f, const int charIdx = -1,
                const int flags = 0, const int value = 0,
                const rstl::string& locatorName = rstl::string_l("root"))
  : CPOINode(name, type, time, index, unique, weight, charIdx, flags)
  , mVal(value)
  , mLctrName(locatorName) {}

  explicit CInt32POINode(CInputStream& in);

  static CInt32POINode CopyNodeMinusStartTime(const CInt32POINode& node,
                                              const CCharAnimTime& startTime);

  int GetValue() const { return mVal; }
  const rstl::string& GetLocatorName() const { return mLctrName; }

private:
  int mVal;
  rstl::string mLctrName;
};

#endif // _CINT32POINODE
