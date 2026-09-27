#ifndef _CAUXWEAPON
#define _CAUXWEAPON

#include "types.h"

#include "MetroidPrime/CEntityInfo.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "MetroidPrime/Weapons/WeaponCommon.hpp"

#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/TToken.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"

class CTransform4f;
class CWeaponDescription;
class CGenDescription;
class CElementGen;

class CAuxWeapon {
public:
  CAuxWeapon(TUniqueId playerId);

  bool IsComboFxActive(const CStateManager& mgr) const;
  const bool UpdateComboFx(float dt, const CVector3f& scale, const CVector3f& firePos,
                           const CTransform4f& targetXf, CStateManager& mgr);
  TUniqueId HasTarget(const CStateManager& mgr) const;
  void SetNewTarget(TUniqueId target, CStateManager& mgr);
  void StopComboFx(CStateManager& mgr, bool deactivate);
  void Load(int curBeam, CStateManager& mgr);
  void LoadIdle();
  void Fire(float dt, const bool underwater, int currentBeam,
            CPlayerState::EChargeStage chargeState, const CTransform4f& xf, CStateManager& mgr,
            EWeaponType type, TUniqueId homingId);
  void RenderMuzzleFx() const;
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);

  bool IsLoaded() const { return mIsLoaded; }

private:
  void FreeComboVoiceId();
  void DeleteWaveBusterBeam(CStateManager& mgr);
  void DeleteFlameThrower(CStateManager& mgr);
  void CreateWaveBusterBeam(uint attribs, TUniqueId target, const CTransform4f& xf,
                            CStateManager& mgr);
  void CreateFlameThrower(const CTransform4f& xf, CStateManager& mgr, float dt);
  void LaunchMissile(float dt, bool underwater, bool charged, int currentBeam, uint attribs,
                     const CTransform4f& xf, TUniqueId target, CStateManager& mgr);
  void InitComboData();
  static const ushort skSoundId[5];

  TLockedToken< CWeaponDescription > mMissile;
  TLockedToken< CGenDescription > mFlameMuzzle;
  TLockedToken< CGenDescription > mBusterMuzzle;
  rstl::single_ptr< CElementGen > mMuzzleFxGen;
  rstl::reserved_vector< TCachedToken< CWeaponDescription >, 5 > mCombos;
  float mAmmoConsumeTimer;
  TUniqueId mPlayerId;
  TUniqueId mFlameThrowerId;
  TUniqueId mWaveBusterId;
  CPlayerState::EBeamId mFiringBeamId;
  int mLoadBeamId;
  CSfxHandle mComboSfx;
  bool mIsLoaded : 1;
};
CHECK_SIZEOF(CAuxWeapon, 0x84)

#endif // _CAUXWEAPON
