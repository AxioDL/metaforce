#ifndef _CFLAMETHROWER
#define _CFLAMETHROWER
#include "MetroidPrime/CFlameWarp.hpp"
#include "MetroidPrime/Weapons/CFlameInfo.hpp"
#include "MetroidPrime/Weapons/CGameProjectile.hpp"

class CFlameThrower : public CGameProjectile {
public:
  enum EFlameState {
    kFS_Default,
    kFS_FireStart,
    kFS_FireActive,
    kFS_FireStopTimer,
    kFS_FireWaitForParticlesDone,
  };
  CFlameThrower(const TToken< CWeaponDescription >& wDesc, const rstl::string& name,
                const EWeaponType wType, const CFlameInfo& flameInfo, const CTransform4f& xf,
                const EMaterialTypes matType, const CDamageInfo& dInfo, const TUniqueId uid,
                const TAreaId aId, const TUniqueId owner, EProjectileAttrib attribs,
                const CAssetId playerSteamTxtr, const ushort playerHitSfx,
                const CAssetId playerIceTxtr);
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;
  void Think(float dt, CStateManager& mgr) override;
  void AddToRenderer(const CFrustumPlanes& frustum, const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor& actor, CStateManager& mgr) override;

  void Reset(CStateManager& mgr, bool resetWarp);
  bool GetParticlesActive() const { return mParticlesActive; }

  void Fire(const CTransform4f&, CStateManager& mgr, bool);

  void SetTransform(const CTransform4f& xf, CStateManager& mgr, float);

private:
  void CreateFlameParticles(CStateManager& mgr);
  void SetFlameLightActive(bool active, CStateManager& mgr);
  float UpdateFlameState(float dt, CStateManager& mgr);
  CRayCastResult DoCollisionCheck(TUniqueId& idOut, const CAABox& bounds, CStateManager& mgr);
#if VERSION >= VERSION_GM8P_00
  void DoRadialFreeze(const CVector3f&, const CDamageInfo&, CStateManager& mgr);
  void ApplyFlameDamageToActor(TUniqueId id, CStateManager& mgr);
#endif
  void ApplyFlameDamageToActors(CStateManager& mgr, TUniqueId id, float dt);
  void ApplyDamageToWorld(CStateManager& mgr, TUniqueId id, const CVector3f& point,
                          const CDamageInfo& dInfo, const CMaterialFilter& filter);

  CTransform4f mFlameXf;
  CAABox mFlameBounds;
  float mParticleWaitDelayTimer;
  float mFireStopTimer;
  float mFlame;
  TLockedToken< CGenDescription > mFlamethrowerDesc;
  rstl::single_ptr< CElementGen > mFlameGen;
  CFlameWarp mFlameWarp;
  EFlameState mFlameState;
  CAssetId mPlayerSteamTextureId;
  ushort mPlayerHitSfx;
  CAssetId mPlayerIceTextureId;
  bool mActive : 1;
  bool mParticlesActive : 1;
  bool mZTest : 1;
  bool mConeCollision : 1;
  bool x428_28_ : 1;

  static const CVector3f kLightOffset;
};
CHECK_SIZEOF(CFlameThrower, (VERSION >= VERSION_GM8P_00 ? 0x430
                             : VERSION >= VERSION_GM8E_02                             ? 0x418
                                                                                      : 0x408))

#endif // _CFLAMETHROWER
