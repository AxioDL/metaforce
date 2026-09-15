#ifndef _CPASANIMINFO
#define _CPASANIMINFO

#include "Kyoto/Animation/CPASAnimParm.hpp"

#include "rstl/reserved_vector.hpp"

class CPASAnimInfo {
public:
  CPASAnimInfo(int id);
  CPASAnimInfo(int id, const rstl::reserved_vector< CPASAnimParm::UParmValue, 8 >& parms);

  CPASAnimInfo(const CPASAnimInfo& other)
  : x0_id(other.GetAnimId())
  , x4_parms(other.x4_parms) {}

  int GetAnimId() const { return x0_id; }
  CPASAnimParm GetAnimParmData(uint idx, CPASAnimParm::EParmType type) const;
  const CPASAnimParm::UParmValue& GetAnimParmValue(uint idx) const;

  bool operator<(const CPASAnimInfo& rhs) const { return x0_id < rhs.x0_id; }

private:
  int x0_id;
  rstl::reserved_vector< CPASAnimParm::UParmValue, 8 > x4_parms;
};

#endif // _CPASANIMINFO
