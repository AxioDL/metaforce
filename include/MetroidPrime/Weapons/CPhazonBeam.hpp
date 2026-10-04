#ifndef _CPHAZONBEAM
#define _CPHAZONBEAM

#include "MetroidPrime/CEntityInfo.hpp"

#include "MetroidPrime/Weapons/CGunWeapon.hpp"

#include <types.h>

class CPhazonBeam : public CGunWeapon {
  friend class CPlayerGun;

public:
  CPhazonBeam(CAssetId characterId, EWeaponType type, TUniqueId playerId,
              EMaterialTypes playerMaterial, const CVector3f& scale);
  ~CPhazonBeam();

  void ReInitVariables();

  bool IsFiring(const CStateManager& mgr) const;
  void PreRenderGunFx(const CStateManager& mgr, const CTransform4f& xf) override;
  void PostRenderGunFx(const CStateManager& mgr, const CTransform4f& xf) override;
  void UpdateGunFx(const bool shotSmoke, const float dt, const CStateManager& mgr,
                   const CTransform4f& xf) override;
  void DrawMuzzleFx(const CStateManager& mgr) const override;
  void Draw(bool drawSuitArm, const CStateManager& mgr, const CTransform4f& xf,
            const CModelFlags& flags, const CActorLights* lights) const override;
  void Fire(const bool underwater, const float dt, CPlayerState::EChargeStage chargeState,
            const CTransform4f& xf, CStateManager& mgr, const TUniqueId homingTarget,
            const float chargeFactor1, const float chargeFactor2) override;
  void Update(float dt, CStateManager& mgr) override;
  void UpdateBeam(float dt, const CTransform4f& xf, const CVector3f& localBeamPos,
                  CStateManager& mgr);
  void StopBeam(CStateManager& mgr, bool reset);
  void Load(CStateManager& mgr, const bool subtypeBasePose) override;
  void Unload(CStateManager& mgr) override;
  bool IsLoaded() const override;
  void CreateBeam(CStateManager& mgr);
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);

private:
  TToken< CModel > mVeins;
  rstl::single_ptr< CModelData > mVeinsData;
  TCachedToken< CGenDescription > mPhazon2nd1;
  rstl::single_ptr< CElementGen > mChargeFxGen;
  CAABox mAABoxScale;
  CAABox mAABoxTranslate;
  float mClipWipeScale;
  float mClipWipeTranslate;
  float mIndirectAlpha;
  bool mLoaded : 1;
  bool mClipWipeActive : 1;
  bool mVeinsAlphaActive : 1;
  bool mPhazonVeinsIdx : 1;
  float mFireTime;

  static const char* const skPhazonVeins;
  static const char* const skPhazonVeins2;
  static const char* const skScaleLocator;
  static const ushort skSoundIds[];
};
#if VERSION >= VERSION_R3IJ_00
CHECK_SIZEOF(CPhazonBeam, 0x270)
#else
CHECK_SIZEOF(CPhazonBeam, 0x27c)
#endif

#endif // _CPHAZONBEAM
