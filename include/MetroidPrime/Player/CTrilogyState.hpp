#ifndef _CTRILOGYSTATE
#define _CTRILOGYSTATE

#include "MetroidPrime/Player/CTrilogyOptions.hpp"

class CTrilogyState {
public:
  CTrilogyOptions& Options() { return mOptions; }
  const CTrilogyOptions& GetOptions() const { return mOptions; }

private:
  CTrilogyOptions mOptions;
  // Persistent game records, gallery unlocks and credits; their types are not reconstructed yet.
  uchar x28_stateData[0x170];
};
CHECK_SIZEOF(CTrilogyState, 0x198)

extern CTrilogyState* gpTrilogyState;

#endif // _CTRILOGYSTATE
