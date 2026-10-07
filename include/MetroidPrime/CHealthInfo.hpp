#ifndef _CHEALTHINFO
#define _CHEALTHINFO

#include "types.h"

class CInputStream;
class COutputStream;

class CHealthInfo {
public:
  CHealthInfo(float hp, float resist)
  : mHealth(hp)
  , mKnockbackResistance(resist)
#if VERSION >= VERSION_R3IJ_00
  , mInitialHealth(hp)
#endif
  {}
  explicit CHealthInfo(CInputStream&);

  void SetHP(float hp) { mHealth = hp; }
  void SetKnockbackResistance(float resist) { mKnockbackResistance = resist; }
  float GetHP() const { return mHealth; }
  float GetKnockBackResistance() const { return mKnockbackResistance; }

  void PutTo(COutputStream& stream) const;

private:
  float mHealth;
  float mKnockbackResistance;
#if VERSION >= VERSION_R3IJ_00
  float mInitialHealth;
#endif
};
#if VERSION >= VERSION_R3IJ_00
CHECK_SIZEOF(CHealthInfo, 0xc)
#else
CHECK_SIZEOF(CHealthInfo, 0x8)
#endif

#endif // _CHEALTHINFO
