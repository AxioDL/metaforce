#ifndef _WEAPONTYPES
#define _WEAPONTYPES

enum EWeaponType {
  kWT_None = -1,
  kWT_Power = 0,
  kWT_Ice = 1,
  kWT_Wave = 2,
  kWT_Plasma = 3,
  kWT_Bomb = 4,
  kWT_PowerBomb = 5,
  kWT_Missile = 6,
  kWT_BoostBall = 7,
  kWT_Phazon = 8,
  kWT_AI = 9,
  kWT_PoisonWater = 10,
  kWT_Lava = 11,
  kWT_Heat = 12,
  kWT_Unused1 = 13,
  kWT_OrangePhazon = 14,
  kWT_Max = 15
};

class CWeaponMode {
public:
  CWeaponMode(const EWeaponType type = kWT_None, const bool charged = false,
              const bool comboed = false, const bool instaKill = false)
  : mWeaponType(type)
  , mCharged(charged)
  , mComboed(comboed)
  , mInstantKill(instaKill) {}

  CWeaponMode(int type, int flags) : mWeaponType(EWeaponType(type)) {}

  EWeaponType GetType() const { return mWeaponType; }

  bool IsCharged() const { return mCharged; }
  bool IsComboed() const { return mComboed; }
  bool IsInstantKill() const { return mInstantKill; }

  static CWeaponMode Invalid() { return CWeaponMode(kWT_None); }
  static CWeaponMode Phazon() { return CWeaponMode(kWT_Phazon); }
  static CWeaponMode Plasma() { return CWeaponMode(kWT_Plasma); }
  static CWeaponMode Wave() { return CWeaponMode(kWT_Wave); }
  static CWeaponMode BoostBall() { return CWeaponMode(kWT_BoostBall); }
  static CWeaponMode Ice() { return CWeaponMode(kWT_Ice); }
  static CWeaponMode Power() { return CWeaponMode(kWT_Power); }
  static CWeaponMode Bomb() { return CWeaponMode(kWT_Bomb); }
  static CWeaponMode PowerBomb() { return CWeaponMode(kWT_PowerBomb); }

private:
  EWeaponType mWeaponType;
  bool mCharged : 1;
  bool mComboed : 1;
  bool mInstantKill : 1;
};

#endif // _WEAPONTYPES
