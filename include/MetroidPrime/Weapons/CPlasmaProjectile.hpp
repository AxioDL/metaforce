#ifndef _CPLASMAPROJECTILE
#define _CPLASMAPROJECTILE

#include "MetroidPrime/Weapons/CBeamInfo.hpp"
#include "MetroidPrime/Weapons/CBeamProjectile.hpp"
#include "MetroidPrime/Weapons/CWeaponAssetInfo.hpp"

class CElectricDescription;

class CPlasmaProjectile : public CBeamProjectile {
public:
  enum EExpansionState { kES_Inactive, kES_Attack, kES_Sustain, kES_Release, kES_Done };

  CPlasmaProjectile(const TToken< CWeaponDescription >& wDesc, const rstl::string& name,
                    EWeaponType wType, const CBeamInfo& bInfo, const CTransform4f& xf,
                    EMaterialTypes matType, const CDamageInfo& dInfo, TUniqueId uid, TAreaId aid,
                    TUniqueId owner, const CWeaponAssetInfo& res, bool growingBeam,
                    EProjectileAttrib attribs);

  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) override;
  void ResetBeam(CStateManager& mgr, bool fullReset) override;
  void UpdateFx(const CTransform4f& xf, float dt, CStateManager& mgr) override;
  void Fire(const CTransform4f& xf, CStateManager& mgr, bool b) override;
  void Touch(CActor& other, CStateManager& mgr) override;
  bool CanRenderUnsorted(const CStateManager& mgr) const override;
  void AddToRenderer(const CFrustumPlanes& frustum, const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  CColor GetInnerColor() const { return mInnerColor; }
  CColor GetOuterColor() const { return mOuterColor; }
  bool IsFiring() const { return mFiring; }
  void SetPlayerSustainedDamage(float duration, const CDamageInfo& damage);

private:
  static const int kMaxPlasmaLights;
  static const float kInvMaxPlasmaLights;

  float UpdateBeamState(float dt, CStateManager& mgr);
  void MakeBillboardEffect(const rstl::optional_object< TToken< CGenDescription > >& particle,
                           const rstl::optional_object< TToken< CElectricDescription > >& electric,
                           const rstl::string& name, CStateManager& mgr);
  void UpdatePlayerEffects(float dt, CStateManager& mgr);
  void RenderBeam(int subdivs, float width, const CColor& color, int flags) const;
  void RenderMotionBlur() const;
  void UpdateEnergyPulse(float dt);
  void SetLightsActive(bool active, CStateManager& mgr);
  void CreatePlasmaLights(uint sourceId, const CLight& light, CStateManager& mgr);
  void DeletePlasmaLights(CStateManager& mgr);
  void UpdateLights(float expansion, float dt, CStateManager& mgr);

  rstl::vector< TUniqueId > mLights;
  int mBeamAttributes;
  float mLifeTime;
  float mPulseSpeed;
  float mShutdownTime;
  float mExpansionSpeed;
  float mMaxLength;
  CColor mInnerColor;
  CColor mOuterColor;
  CDamageInfo mPhazonDamage;
  EExpansionState mExpansionState;
  float mBeamWidth;
  float mLifeTimer;
  float mExpansionT;
  float mExpansion;
  float mBeamAngle;
  float mEnergyPulseStartY;
  float mShutdownTimer;
  float mContactPulseTimer;
  float mEnergyPulseTimer;
  float mPlayerEffectPulseTimer;
  float mPlayerDamageDuration;
  float mPlayerDamageTimer;
  TCachedToken< CTexture > mTexture;
  TCachedToken< CTexture > mGlowTexture;
  TLockedToken< CGenDescription > mContactFxDesc;
  TLockedToken< CGenDescription > mPulseFxDesc;
  rstl::single_ptr< CElementGen > mContactGen;
  rstl::single_ptr< CElementGen > mPulseGen;
  rstl::single_ptr< CElementGen > mWeaponGen;
  CAssetId mFreezeSteamTxtr;
  CAssetId mFreezeIceTxtr;
  rstl::optional_object< TToken< CElectricDescription > > mVisorElectric;
  rstl::optional_object< TToken< CGenDescription > > mVisorParticle;
  u16 mFreezeSfx;
  u16 mElectricSfx;
  bool x548_24_ : 1;
  bool mEnableEnergyPulse : 1;
  bool mFiring : 1;
  bool mTexturesLoaded : 1;
  bool mDrawOwnerFirst : 1;
  bool mActivePlayerPhazon : 1;
};
CHECK_SIZEOF(CPlasmaProjectile, (VERSION >= VERSION_GM8P_00 ? 0x578
                                 : VERSION >= VERSION_GM8E_02                             ? 0x560
                                                                                          : 0x550))

#endif // _CPLASMAPROJECTILE
