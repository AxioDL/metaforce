#ifndef _CFLAMETHROWER
#define _CFLAMETHROWER
#include "CFlameInfo.hpp"
#include "MetroidPrime/CFlameWarp.hpp"
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
  bool GetParticlesActive() const { return x400_25_particlesActive; }

  void Fire(const CTransform4f&, CStateManager& mgr, bool);

  void SetTransform(const CTransform4f& xf, CStateManager& mgr, float);

private:
  void CreateFlameParticles(CStateManager& mgr);
  void SetFlameLightActive(bool active, CStateManager& mgr);
  float UpdateFlameState(float dt, CStateManager& mgr);
  CRayCastResult DoCollisionCheck(TUniqueId& idOut, const CAABox& bounds, CStateManager& mgr);
  void ApplyDamageToActor(CStateManager& mgr, TUniqueId id, float dt);

  CTransform4f x2e8_flameXf;
  CAABox x318_flameBounds;
  float x330_particleWaitDelayTimer;
  float x334_fireStopTimer;
  float x338_flame;
  TLockedToken< CGenDescription > x33c_flamethrowerDesc;
  rstl::single_ptr< CElementGen > x348_flameGen;
  CFlameWarp x34c_flameWarp;
  EFlameState x3f0_flameState;
  CAssetId x3f4_playerSteamTextureId;
  ushort x3f8_playerHitSfx;
  CAssetId x3fc_playerIceTextureId;
  bool x400_24_active : 1;
  bool x400_25_particlesActive : 1;
  bool x400_26_zTest : 1;
  bool x400_27_coneCollision : 1;

  static const CVector3f kLightOffset;
};
CHECK_SIZEOF(CFlameThrower, 0x408)

#endif // _CFLAMETHROWER
