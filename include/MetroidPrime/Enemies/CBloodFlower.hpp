#ifndef _CBLOODFLOWER
#define _CBLOODFLOWER
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/Weapons/CProjectileInfo.hpp"

class CTargetableProjectile;
class CWeaponDescription;
class CElementGen;
class CBloodFlower : public CPatterned {
public:
  CBloodFlower(const TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
               const CTransform4f& xf, const CModelData& mData, const CPatternedInfo& pInfo,
               const CAssetId podEffectDescId, const CAssetId projectileDescId,
               const CActorParameters& actParams, const CAssetId wpscId2, const CDamageInfo& dInfo1,
               const CDamageInfo& projectileDamage, const CDamageInfo& podDamage,
               const CAssetId partId2, const CAssetId partId3, const CAssetId partId4,
               const float f1, const CAssetId partId5, const uint soundId);

  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;
  CProjectileInfo* ProjectileInfo() override;

  void InActive(CStateManager&, EStateMsg, float) override;
  void Active(CStateManager&, EStateMsg, float) override;
  void BulbAttack(CStateManager&, EStateMsg, float) override;
  void PodAttack(CStateManager&, EStateMsg, float) override;
  bool Leash(CStateManager&, float) override;
  bool ShouldTurn(CStateManager&, float) override;
  bool ShouldAttack(CStateManager&, float) override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                       float dt) override;

  void Think(float dt, CStateManager& mgr) override;
  void Render(const CStateManager&) const override;
  EWeaponCollisionResponseTypes GetCollisionResponseType(const CVector3f& position,
                                                         const CVector3f& direction,
                                                         const CWeaponMode& mode,
                                                         int attrib) const override;
  void Touch(CActor&, CStateManager&) override;

private:
  const CDamageInfo& GetPodDamage() const { return mPodDamage; }
  CTransform4f GetMouthLocator(const rstl::string& name);
  void TriggerPodSteam(CStateManager& mgr, const bool activate) const;
  CTargetableProjectile* CreateArcProjectile(CStateManager& mgr,
                                             const TToken< CWeaponDescription >& desc,
                                             const CTransform4f& xf, const CDamageInfo& dInfo,
                                             TUniqueId uid);
  void ResetShotTimer(CStateManager& mgr);
  void UpdateFire(CStateManager& mgr);
  void TurnEffectsOn(int effectIndex, CStateManager& mgr);
  void TurnEffectsOff(int effectIndex, CStateManager& mgr);
  void LaunchPollenProjectile(const CTransform4f& xf, CStateManager& mgr, float var_f1,
                            int maxProjectiles);
  TLockedToken< CGenDescription > mPodEffectDescription;
  rstl::single_ptr< CElementGen > mPodEffect;
  TCachedToken< CWeaponDescription > mProjectileDesc;
  float mCurAttackTime;
  float mProjectileOffset;
  int mProjectileState;
  CProjectileInfo mProjectileInfo;
  float x5b8_;
  float mProjectileDelay;
  float mLeashTime;
  rstl::optional_object< TLockedToken< CGenDescription > > mVisorParticle;
  ushort mVisorSfx;
  int mEffectState;
  CDamageInfo mProjectileDamage;
  CDamageInfo mPodDamage;
  float x614_;
  CAssetId x618_;
  CAssetId x61c_;
  CAssetId x620_;
  
};
#endif
