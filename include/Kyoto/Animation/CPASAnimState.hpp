#ifndef _CPASANIMSTATE
#define _CPASANIMSTATE

#include "types.h"

#include "Kyoto/Animation/CPASAnimInfo.hpp"
#include "Kyoto/Animation/CPASAnimParm.hpp"
#include "Kyoto/Animation/CPASParmInfo.hpp"
#include "Kyoto/Animation/CharacterCommon.hpp"

#include "rstl/pair.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/vector.hpp"

class CRandom16;

class CPASAnimState {
public:
  CPASAnimState(int id);
  CPASAnimState(CInputStream& in);

  CPASAnimParm GetAnimParmData(int, unsigned int) const;
  void AddAnimParmData(int animId, const rstl::reserved_vector< CPASAnimParm, 8 >& parms);
  rstl::pair< float, int > FindBestAnimation(const rstl::reserved_vector< CPASAnimParm, 8 >& parms,
                                             CRandom16& random, int ignoreAnim) const;

  pas::EAnimationState GetStateId() const { return mId; }
  bool HasAnims() const { return static_cast< uint >(mAnims.size()) != 0; }

  bool operator<(const CPASAnimState& rhs) const { return mId < rhs.mId; }

private:
  float ComputeExactMatchWeight(uint idx, const CPASAnimParm& parm,
                                CPASAnimParm::UParmValue value) const;
  float ComputePercentErrorWeight(uint idx, const CPASAnimParm& parm,
                                  CPASAnimParm::UParmValue value) const;
  float ComputeAngularPercentErrorWeight(uint idx, const CPASAnimParm& parm,
                                         CPASAnimParm::UParmValue value) const;
  int PickRandomAnimation(CRandom16& random) const;

  pas::EAnimationState mId;
  rstl::vector< CPASParmInfo > mParms;
  rstl::vector< CPASAnimInfo > mAnims;
  mutable rstl::vector< int > mSelectionCache;
};
CHECK_SIZEOF(CPASAnimState, 0x34)

#endif // _CPASANIMSTATE
