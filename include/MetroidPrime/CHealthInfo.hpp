#ifndef _CHEALTHINFO
#define _CHEALTHINFO

#include "types.h"

class CInputStream;
class CHealthInfo {
public:
  CHealthInfo(float hp, float resist) : mHealth(hp), mKnockbackResistance(resist) {}
  explicit CHealthInfo(CInputStream&);

  void SetHP(float hp) { mHealth = hp; }
  void SetKnockbackResistance(float resist) { mKnockbackResistance = resist; }
  float GetHP() const { return mHealth; }
  float GetKnockBackResistance() const { return mKnockbackResistance; }

private:
  float mHealth;
  float mKnockbackResistance;
};
CHECK_SIZEOF(CHealthInfo, 0x8)

#endif // _CHEALTHINFO
