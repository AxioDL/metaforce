#ifndef _CPASANIMINFO
#define _CPASANIMINFO

#include "Kyoto/Animation/CPASAnimParm.hpp"

#include "rstl/reserved_vector.hpp"

class CPASAnimInfo {
public:
  CPASAnimInfo(int id);
  CPASAnimInfo(int id, const rstl::reserved_vector< CPASAnimParm::UParmValue, 8 >& parms);

  CPASAnimInfo(const CPASAnimInfo& other)
  : mId(other.GetAnimId())
  , mParms(other.mParms) {}

  int GetAnimId() const { return mId; }
  CPASAnimParm GetAnimParmData(uint idx, CPASAnimParm::EParmType type) const;
  const CPASAnimParm::UParmValue& GetAnimParmValue(uint idx) const;

  bool operator<(const CPASAnimInfo& rhs) const { return mId < rhs.mId; }

private:
  int mId;
  rstl::reserved_vector< CPASAnimParm::UParmValue, 8 > mParms;
};

#endif // _CPASANIMINFO
