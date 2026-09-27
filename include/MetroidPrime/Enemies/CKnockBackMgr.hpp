#ifndef _CKNOCKBACKMGR
#define _CKNOCKBACKMGR

#include "types.h"

#include "MetroidPrime/Weapons/WeaponCommon.hpp"

#include "Kyoto/Animation/CPASAnimState.hpp"
#include "Kyoto/Math/CVector3f.hpp"

class CStateManager;
class CDamageInfo;
class CPatterned;

enum ECreatureSize {
  kCS_Small,
  kCS_Medium,
  kCS_Large,
};
enum EKnockBackWeaponType {
  kKBWT_Invalid = -1,
  kKBWT_Power,
  kKBWT_PowerCharged,
  kKBWT_PowerComboed,
  kKBWT_PowerComboedDirect,
  kKBWT_Wave,
  kKBWT_WaveCharged,
  kKBWT_WaveComboed,
  kKBWT_WaveComboedDirect,
  kKBWT_Ice,
  kKBWT_IceCharged,
  kKBWT_IceComboed,
  kKBWT_IceComboedDirect,
  kKBWT_Plasma,
  kKBWT_PlasmaCharged,
  kKBWT_PlasmaComboed,
  kKBWT_Missile,
  kKBWT_Bomb,
  kKBWT_PowerBomb,
  kKBWT_Phazon,
};
enum EKnockBackCharacterState {
  kKBCS_Alive,
  kKBCS_Dead,
  kKBCS_FrozenAlive,
  kKBCS_FrozenDead,
};
enum EAnimReaction {
  kAR_Invalid = -1,
  kAR_None,
  kAR_Flinch,
  kAR_KnockBack,
  kAR_Hurled,
  kAR_Fall,
};
enum EKnockBackAnimationFollowUp {
  kKBAFU_Invalid = -1,
  kKBAFU_None,
  kKBAFU_Freeze,
  kKBAFU_Shock,
  kKBAFU_Burn,
  kKBAFU_PhazeOut,
  kKBAFU_Death,
  kKBAFU_ExplodeDeath,
  kKBAFU_IceDeath,
  kKBAFU_BurnDeath,
  kKBAFU_LaggedBurnDeath,
};

class CKnockBackMgr {
  friend class CPatterned;

public:
  struct KnockBackParms {
    EAnimReaction mAnimState;
    EKnockBackAnimationFollowUp mAnimFollowup;
    float mFollowupDuration;
    float mIntoFreezeDur;
  };

  explicit CKnockBackMgr(ECreatureSize variant);
  void SetCreatureSize(ECreatureSize size);
  void DeferKnockBack(EWeaponType tp);
  void sub80233d40(int i, float f1, float f2);
  void SetAutoResetImpulse(bool b);
  void SetImpulseDurationIdx(int i);
  void SetAnimationStateRange(EAnimReaction a, EAnimReaction b);
  void Update(float dt, CStateManager& mgr, CPatterned& parent);
  void KnockBack(const CVector3f& backVec, CStateManager& mgr, CPatterned& parent,
                 const CDamageInfo& info, float magnitude, bool direct);

  void ApplyImpulse(float dt, CPatterned& parent);
  bool TickDeferredTimer(float dt);
  EKnockBackCharacterState GetKnockBackCharacterState(CPatterned& parent);
  void ValidateState(CPatterned& parent);
  float CalculateExtraHurlVelocity(CStateManager& mgr, float magnitude, float kbResistance);
  void DoKnockBackAnimation(const CVector3f& backVec, CStateManager& mgr, CPatterned& parent,
                            float magnitude);
  void ResetKnockBackImpulse(CPatterned& parent, const CVector3f& backVec, float magnitude);
  void DoDeferredKnockBack(CStateManager& mgr, CPatterned& parent);
  EKnockBackWeaponType GetKnockBackWeaponType(const CDamageInfo& info, EWeaponType wType,
                                              bool direct);
  void SelectDamageState(CPatterned& parent, const CDamageInfo& info, EWeaponType wType,
                         bool direct);

  void SetSeverity(pas::ESeverity v) { mSeverity = v; }
  void SetEnableFreeze(bool b) { mEnableFreeze = b; }
  bool GetEnableFreeze() const { return mEnableFreeze; }
  void SetEnableShock(bool b) { mEnableShock = b; }
  void SetEnableBurn(bool b) { mEnableBurn = b; }
  void SetEnableBurnDeath(bool b) { mEnableBurnDeath = b; }
  void SetEnableExplodeDeath(bool b) { mEnableExplodeDeath = b; }
  void SetEnableLaggedBurnDeath(bool b) { mEnableLaggedBurnDeath = b; }
  void SetX81_31(bool b) { x81_31_ = b; }
  void SetX82_24(bool b) { x82_24_ = b; }
  void SetLocomotionDuringElectrocution(bool b) { mLocomotionDuringElectrocution = b; }
  const KnockBackParms& GetActiveParms() const { return mActiveParms; }
  ECreatureSize GetCreatureSize() const { return mSize; }
  float GetFlinchRemTime() const { return mFlinchRemTime; }
  void EnableAnimReaction(EAnimReaction s, bool b);
  bool TestAvailableState(int s) const;

private:
  ECreatureSize mSize;
  KnockBackParms mActiveParms;
  EWeaponType mDeferWeaponType;
  EAnimReaction mMinAnimState;
  EAnimReaction mMaxAnimState;
  int mImpulseDurationIdx;
  rstl::reserved_vector< rstl::pair< float, float >, 5 > x24_;
  CVector3f mImpulseDir;
  float mImpulseMag;
  float mImpulseRemTime;
  float mFlinchRemTime;
  float mDeferRemTime;
  uint x6c_;
  uint x70_;
  uint x74_;
  uint x78_;
  pas::ESeverity mSeverity;
  uchar mAvailableStates;
  bool mAutoResetImpulse : 1;
  bool mEnableFreeze : 1;
  bool mEnableShock : 1;
  bool mEnableBurn : 1;
  bool mEnableBurnDeath : 1;
  bool mEnableExplodeDeath : 1;
  bool mEnableLaggedBurnDeath : 1;
  bool x81_31_ : 1;
  bool x82_24_ : 1;
  bool mInDeferredKnockBack : 1;
  bool mLocomotionDuringElectrocution : 1;
  static const KnockBackParms skDefaultParameters;
  static const KnockBackParms skKnockBackParameters[3][19][4];
};
CHECK_SIZEOF(CKnockBackMgr, 0x84)

#endif // _CKNOCKBACKMGR
