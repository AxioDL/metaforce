#ifndef _CTRILOGYUTIL
#define _CTRILOGYUTIL

#include "types.h"

class CTrilogyUtil {
public:
  // Values inferred from the executable-presence checks in GetGameType.
  enum EGameType { kGT_Prime1, kGT_Prime2, kGT_Trilogy };
  static EGameType GetGameType();
};

#endif // _CTRILOGYUTIL
