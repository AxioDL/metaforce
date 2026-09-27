#ifndef _CGUNWEAPON
#define _CGUNWEAPON

#include "types.h"

#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/Weapons/GunController/CGunMotion.hpp"
#include "MetroidPrime/Weapons/WeaponCommon.hpp"
#include "MetroidPrime/Weapons/WeaponTypes.hpp"

#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/TToken.hpp"

#include "Collision/CMaterialList.hpp"

#include "rstl/optional_object.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"

class CGenDescription;
class CGunController;
class CAnimCharacterSet;
class CWeaponDescription;
class CElementGen;
class CRainSplashGenerator;
class CTransform4f;
class CModelFlags;
class CActorLights;
struct SWeaponInfo;

enum EFrozenFxType {
  kFFT_None,
  kFFT_Frozen,
  kFFT_Thawed,
};

class CVelocityInfo {
public:
  ~CVelocityInfo();

  CVector3f& Velocity(int i) { return mVel[i]; }
  const CVector3f& GetVelocity(int i) const { return mVel[i]; }
  bool GetTargetHoming(int i) const { return mTargetHoming[i]; }

  void Clear();

  void AddVelocity(const CVector3f& vel) { mVel.push_back(vel); }
#if VERSION >= VERSION_GM8P_00
  void AddTargetHoming(const bool homing) { mTargetHoming.push_back(homing); }
#else
  void AddTargetHoming(const bool& homing) { mTargetHoming.push_back(homing); }
#endif
  void AddTrat(const float& trat) { mTrat.push_back(trat); }

private:
  rstl::reserved_vector< CVector3f, 2 > mVel;
  rstl::reserved_vector< bool, 2 > mTargetHoming;
  rstl::reserved_vector< float, 2 > mTrat;
};

class CGunWeapon {
  friend class CPlayerGun;

public:
  CGunWeapon(CAssetId ancsId, EWeaponType type, TUniqueId playerId, EMaterialTypes playerMaterial,
             const CVector3f& scale);
  virtual ~CGunWeapon();

  enum ESecondaryFxType {
    kSFT_None,
    kSFT_Charge,
    kSFT_ToCombo,
    kSFT_CancelCharge,
  };

  // Virtual Methods
  virtual void Reset(CStateManager& mgr);
  virtual void PlayAnim(const NWeaponTypes::EGunAnimType type, bool loop);
  virtual void PreRenderGunFx(const CStateManager& mgr, const CTransform4f& xf) {}
  virtual void PostRenderGunFx(const CStateManager& mgr, const CTransform4f& xf);
  virtual void UpdateGunFx(const bool shotSmoke, const float dt, const CStateManager& mgr,
                           const CTransform4f& xf);
  virtual void Fire(const bool underwater, const float dt,
                    const CPlayerState::EChargeStage chargeState, const CTransform4f& xf,
                    CStateManager& mgr, const TUniqueId homingTarget, const float chargeFactor1,
                    const float chargeFactor2);
  virtual void EnableFx(const bool enable) {}
  virtual void EnableSecondaryFx(const ESecondaryFxType type) {
    mEnabledSecondaryEffect = type;
  }
  virtual void Draw(const bool drawSuitArm, const CStateManager& mgr, const CTransform4f& xf,
                    const CModelFlags& flags, const CActorLights* lights) const;
  virtual void DrawMuzzleFx(const CStateManager& mgr) const;
  virtual void Update(const float dt, CStateManager& mgr);
  virtual void Load(CStateManager& mgr, const bool subtypeBasePose);
  virtual void Unload(CStateManager& mgr);
  virtual bool IsLoaded() const;

  const CVelocityInfo& GetVelocityInfo() const { return mVelInfo; }
  rstl::optional_object< CModelData >& SolidModelData() { return mSolidModelData; }
  const CModelData& GetSolidModelData() const { return mSolidModelData.data(); }

  EWeaponType GetType() const { return mWeaponType; }
  TUniqueId GetPlayerId() const { return mPlayerId; }
  EMaterialTypes GetPlayerMaterial() const { return mPlayerMaterial; }

  CAABox GetBounds() const;
  CAABox GetBounds(const CTransform4f& xf) const;
  const SWeaponInfo& GetWeaponInfo() const;
  void ActivateCharge(bool enable, bool resetEffect);
  bool IsCharged() const { return mEnableCharge; }
  void EnableCharge(bool enable) { mEnableCharge = enable; }
  bool PlayPasAnim(SamusGun::EAnimationState state, CStateManager& mgr, float angle);
  bool IsChargeAnimOver() const;
  void UpdateMuzzleFx(const float dt, const CVector3f& scale, const CVector3f& pos,
                      const bool emitting);
  CElementGen* GetChargeMuzzleFx() const;
  void DrawHologram(const CStateManager& mgr, const CTransform4f& xf,
                    const CModelFlags& flags) const;
  void ReturnToDefault(CStateManager& mgr);
  bool ComboFireOver() const;
  void EnterFidget(CStateManager& mgr, SamusGun::EFidgetType type, int parm2);
  void Touch(const CStateManager& mgr);
  void TouchHolo(const CStateManager& mgr);
  void AsyncLoadSuitArm(CStateManager& mgr);
  void AsyncLoadFidget(CStateManager& mgr, SamusGun::EFidgetType type, int animSet);
  void UnLoadFidget();
  bool IsFidgetLoaded();
  void EnableFrozenEffect(EFrozenFxType type);

  CDamageInfo GetDamageInfo(CStateManager& mgr, CPlayerState::EChargeStage chargeState,
                            float chargeFactor);
  static const char* GetMuzzleLocator() { return skMuzzleLocator; }

protected:
  // x0 is vtable
  CVector3f mScale;
  rstl::optional_object< CModelData > mSolidModelData;
  rstl::optional_object< CModelData > mHoloModelData;
  rstl::optional_object< CModelData > mSuitArmModelData;
  rstl::single_ptr< CGunController > mGunController;
  TToken< CAnimCharacterSet > mGunCharacter;
  rstl::vector< CToken > mAnims;
  rstl::vector< CToken > mUnk;
  rstl::vector< CToken > mDeps;
  TToken< CAnimCharacterSet > mArmCharacter;
  rstl::reserved_vector< TCachedToken< CWeaponDescription >, 2 > mWeapons;
  TCachedToken< CGenDescription > mXferEffect;
  rstl::reserved_vector< TCachedToken< CGenDescription >, 2 > mMuzzleEffects;
  rstl::reserved_vector< TCachedToken< CGenDescription >, 2 > mFrozenEffects;
  rstl::reserved_vector< rstl::auto_ptr< CElementGen >, 2 > mMuzzleGenerators;
  rstl::single_ptr< CElementGen > mFrozenGenerator;
  CRainSplashGenerator* mRainSplashGenerator;
  EWeaponType mWeaponType;
  TUniqueId mPlayerId;
  EMaterialTypes mPlayerMaterial;
  ESecondaryFxType mEnabledSecondaryEffect;
  CVelocityInfo mVelInfo;
  CPlayerState::EBeamId mBeamId;
  EFrozenFxType mFrozenEffect;
  uint mMuzzleEffectIdx;
  uint mShaderIdx;
  // 0x1: load request, 0x2: muzzle fx, 0x4: projectile data, 0x8: anims, 0x10: everything else
  int mLoadFlags;
  CAssetId mAncsId;
  bool x218_24 : 1;
  bool mEnableCharge : 1;
  bool mLoaded : 1;
  // Initialize in selected beam's pose, rather than power beam's pose
  bool mSubtypeBasePose : 1;
  bool mSuitArmLocked : 1;
  bool mDrawHologram : 1;

  static const char* const skMuzzleLocator;
  static const char* const skElbowLocator;
  static const int skShootAnim[2];
  static const char* const skMuzzleNames[10];
  static const char* const skFrozenNames[10];
  static const char* const skBeamXferNames[5];
  static const char* const skAnimDependencyNames[5];
  static const char* const skDependencyNames[5];
  static const char* const skSuitArmNames[8];
  static const int skAnimTypeList[11];

  void AllocResPools(CPlayerState::EBeamId beam);
  void FreeResPools();
  static void FillTokenVector(const rstl::vector< SObjectTag >& tags,
                              rstl::vector< CToken >& objects, bool includeTxtr);
  void BuildDependencyList(CPlayerState::EBeamId beam);
  void LoadSuitArm(CStateManager& mgr);
  void LoadGunModels(CStateManager& mgr);
  void LoadAnimations();
  bool IsAnimsLoaded() const;
  void LoadMuzzleFx(float dt);
  void LoadProjectileData(CStateManager& mgr);
  void LoadFxIdle(float dt, CStateManager& mgr);
  void LockTokens(CStateManager& mgr);
  void UnlockTokens();

  static void PointGenerator(void*, const CVector3f*, const CVector3f*, int);
};
CHECK_SIZEOF(CGunWeapon, 0x21c)

#endif // _CGUNWEAPON
