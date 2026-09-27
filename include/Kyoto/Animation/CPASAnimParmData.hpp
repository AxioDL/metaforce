#ifndef _CPASANIMPARMDATA
#define _CPASANIMPARMDATA

#include "Kyoto/Animation/CPASAnimState.hpp"

class CPASAnimParmData {
  pas::EAnimationState mStateId;
  rstl::reserved_vector< CPASAnimParm, 8 > mParms;

public:
  CPASAnimParmData(pas::EAnimationState stateId,
                   const CPASAnimParm& parm1 = CPASAnimParm::NoParameter(),
                   const CPASAnimParm& parm2 = CPASAnimParm::NoParameter(),
                   const CPASAnimParm& parm3 = CPASAnimParm::NoParameter(),
                   const CPASAnimParm& parm4 = CPASAnimParm::NoParameter(),
                   const CPASAnimParm& parm5 = CPASAnimParm::NoParameter(),
                   const CPASAnimParm& parm6 = CPASAnimParm::NoParameter(),
                   const CPASAnimParm& parm7 = CPASAnimParm::NoParameter(),
                   const CPASAnimParm& parm8 = CPASAnimParm::NoParameter())
  : mStateId(stateId) {
    mParms.push_back(parm1);
    mParms.push_back(parm2);
    mParms.push_back(parm3);
    mParms.push_back(parm4);
    mParms.push_back(parm5);
    mParms.push_back(parm6);
    mParms.push_back(parm7);
    mParms.push_back(parm8);
  }
  ~CPASAnimParmData() {}

  pas::EAnimationState GetStateId() const { return mStateId; }
  const rstl::reserved_vector< CPASAnimParm, 8 >& GetAnimParmData() const { return mParms; }

  static CPASAnimParmData NoParameters(pas::EAnimationState stateId) {
    return CPASAnimParmData(stateId);
  }
};

#endif // _CPASANIMPARMDATA
