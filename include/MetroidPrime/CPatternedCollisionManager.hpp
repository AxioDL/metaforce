#ifndef _CPATTERNEDCOLLISIONMANAGER
#define _CPATTERNEDCOLLISIONMANAGER

#include "MetroidPrime/TGameTypes.hpp"

#include "rstl/listpool.hpp"
#include "rstl/vector.hpp"

class CPatternedCollisionManager {
public:
  struct SCollisionSet;

  struct SDamageMessageInfo {
    int mType;
    TUniqueId mSender;
  };

  CPatternedCollisionManager();
  ~CPatternedCollisionManager();

private:
  uchar mNextId;
  rstl::vector< SCollisionSet > mCollisionSets;
  rstl::listpool< SDamageMessageInfo, 4 > mDamageMessages;
};

NESTED_CHECK_SIZEOF(CPatternedCollisionManager, SDamageMessageInfo, 0x8)
#if VERSION >= VERSION_R3IJ_00
CHECK_SIZEOF(CPatternedCollisionManager, 0x28)
#endif

#endif // _CPATTERNEDCOLLISIONMANAGER
