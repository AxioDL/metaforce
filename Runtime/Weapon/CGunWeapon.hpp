#pragma once

#include <array>
#include <memory>
#include <vector>

#include "Runtime/RetroTypes.hpp"

#include "Runtime/CPlayerState.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/CToken.hpp"
#include "Runtime/rstl.hpp"
#include "Runtime/Character/CAnimCharacterSet.hpp"
#include "Runtime/Collision/CMaterialList.hpp"
#include "Runtime/Particle/CElementGen.hpp"
#include "Runtime/Particle/CWeaponDescription.hpp"
#include "Runtime/Weapon/CGunController.hpp"
#include "Runtime/Weapon/CGunMotion.hpp"
#include "Runtime/Weapon/CWeaponMgr.hpp"
#include "Runtime/Weapon/WeaponCommon.hpp"
#include "Runtime/World/CDamageInfo.hpp"

#include <zeus/CVector3f.hpp>

namespace urde {

class CActorLights;
struct CModelFlags;

enum class EChargeState { Normal, Charged };

using SWeaponInfo = DataSpec::SWeaponInfo;
using SShotParam = DataSpec::SShotParam;
using SChargedShotParam = DataSpec::SChargedShotParam;

class CVelocityInfo {
  friend class CGunWeapon;
  rstl::reserved_vector<zeus::CVector3f, 2> x0_vel;
  rstl::reserved_vector<bool, 2> x1c_targetHoming;
  rstl::reserved_vector<float, 2> x24_trat;

public:
  const zeus::CVector3f& GetVelocity(int i) const { return x0_vel[i]; }
  bool GetTargetHoming(int i) const { return x1c_targetHoming[i]; }
  void Clear() {
    x0_vel.clear();
    x1c_targetHoming.clear();
    x24_trat.clear();
  }
};

class CGunWeapon {
public:
  enum class ESecondaryFxType { None, Charge, ToCombo, CancelCharge };
  enum class EFrozenFxType { None, Frozen, Thawed };

protected:
  static const std::array<s32, 2> skShootAnim;
  zeus::CVector3f x4_scale;
  std::optional<CModelData> x10_solidModelData;
  std::optional<CModelData> x60_holoModelData;
  std::optional<CModelData> xb0_suitArmModelData;
  std::unique_ptr<CGunController> x100_gunController;
  TToken<CAnimCharacterSet> x104_gunCharacter;
  std::vector<CToken> x10c_anims;
  std::vector<CToken> x12c_deps;
  TToken<CAnimCharacterSet> x13c_armCharacter;
  rstl::reserved_vector<TCachedToken<CWeaponDescription>, 2> x144_weapons;
  TCachedToken<CGenDescription> x160_xferEffect;
  rstl::reserved_vector<TCachedToken<CGenDescription>, 2> x16c_muzzleEffects;
  rstl::reserved_vector<TCachedToken<CGenDescription>, 2> x188_frozenEffects;
  rstl::reserved_vector<std::unique_ptr<CElementGen>, 2> x1a4_muzzleGenerators;
  std::unique_ptr<CElementGen> x1b8_frozenGenerator;
  CRainSplashGenerator* x1bc_rainSplashGenerator = nullptr;
  EWeaponType x1c0_weaponType;
  TUniqueId x1c4_playerId;
  EMaterialTypes x1c8_playerMaterial;
  ESecondaryFxType x1cc_enabledSecondaryEffect = ESecondaryFxType::None;
  CVelocityInfo x1d0_velInfo;
  CPlayerState::EBeamId x200_beamId;
  EFrozenFxType x204_frozenEffect = EFrozenFxType::None;
  u32 x208_muzzleEffectIdx = 0;
  u32 x20c_shaderIdx;
  // 0x1: load request, 0x2: muzzle fx, 0x4: projectile data, 0x8: anims, 0x10: everything else
  u32 x210_loadFlags = 0;
  CAssetId x214_ancsId;
  bool x218_24 : 1 = false;
  bool x218_25_enableCharge : 1 = false;
  bool x218_26_loaded : 1 = false;
  // Initialize in selected beam's pose, rather than power beam's pose
  bool x218_27_subtypeBasePose : 1 = false;
  bool x218_28_suitArmLocked : 1 = false;
  bool x218_29_drawHologram : 1 = false;

  void AllocResPools(CPlayerState::EBeamId beam);
  void FreeResPools();
  void FillTokenVector(const std::vector<SObjectTag>& tags, std::vector<CToken>& objects);
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

  static void PointGenerator(void* ctx, const std::vector<std::pair<zeus::CVector3f, zeus::CVector3f>>& vn);

public:
  CGunWeapon(CAssetId ancsId, EWeaponType type, TUniqueId playerId, EMaterialTypes playerMaterial,
             const zeus::CVector3f& scale);
  virtual ~CGunWeapon();

  void AsyncLoadSuitArm(CStateManager& mgr);
  virtual void Reset(CStateManager& mgr);
  virtual void PlayAnim(NWeaponTypes::EGunAnimType type, bool loop);
  virtual void PreRenderGunFx(const CStateManager& mgr, const zeus::CTransform& xf);
  virtual void PostRenderGunFx(const CStateManager& mgr, const zeus::CTransform& xf);
  virtual void UpdateGunFx(bool shotSmoke, float dt, const CStateManager& mgr, const zeus::CTransform& xf);
  virtual void Fire(bool underwater, float dt, EChargeState chargeState, const zeus::CTransform& xf, CStateManager& mgr,
                    TUniqueId homingTarget, float chargeFactor1, float chargeFactor2);
  virtual void EnableFx(bool enable);
  virtual void EnableSecondaryFx(ESecondaryFxType type);
  void EnableFrozenEffect(EFrozenFxType type);
  void ActivateCharge(bool enable, bool resetEffect);
  void Touch(const CStateManager& mgr);
  void TouchHolo(const CStateManager& mgr);
  virtual void Draw(bool drawSuitArm, const CStateManager& mgr, const zeus::CTransform& xf, const CModelFlags& flags,
                    const CActorLights* lights);
  virtual void DrawMuzzleFx(const CStateManager& mgr) const;
  virtual void Update(float dt, CStateManager& mgr);
  virtual void Load(CStateManager& mgr, bool subtypeBasePose);
  virtual void Unload(CStateManager& mgr);
  virtual bool IsLoaded() const;
  void DrawHologram(const CStateManager& mgr, const zeus::CTransform& xf, const CModelFlags& flags);
  void UpdateMuzzleFx(float dt, const zeus::CVector3f& scale, const zeus::CVector3f& pos, bool emitting);
  const CVelocityInfo& GetVelocityInfo() const { return x1d0_velInfo; }
  void SetRainSplashGenerator(CRainSplashGenerator* g) { x1bc_rainSplashGenerator = g; }
  CElementGen* GetChargeMuzzleFx() const { return x1a4_muzzleGenerators[1].get(); }
  const TToken<CGenDescription>& GetComboXferDescr() const { return x160_xferEffect; }
  void ReturnToDefault(CStateManager& mgr);
  bool PlayPasAnim(SamusGun::EAnimationState state, CStateManager& mgr, float angle);
  void UnLoadFidget();
  bool IsFidgetLoaded() const;
  void AsyncLoadFidget(CStateManager& mgr, SamusGun::EFidgetType type, s32 animSet);
  void EnterFidget(CStateManager& mgr, SamusGun::EFidgetType type, s32 parm2);
  bool HasSolidModelData() const { return x10_solidModelData.operator bool(); }
  CModelData& GetSolidModelData() { return *x10_solidModelData; }
  const SWeaponInfo& GetWeaponInfo() const;
  CDamageInfo GetDamageInfo(CStateManager& mgr, EChargeState chargeState, float chargeFactor) const;
  EWeaponType GetWeaponType() const { return x1c0_weaponType; }
  zeus::CAABox GetBounds() const;
  zeus::CAABox GetBounds(const zeus::CTransform& xf) const;
  bool ComboFireOver() const { return x100_gunController->IsComboOver(); }
  bool IsChargeAnimOver() const;
  void SetDrawHologram(bool d) { x218_29_drawHologram = d; }
  void EnableCharge(bool c) { x218_25_enableCharge = c; }

  static CDamageInfo GetShotDamageInfo(const SShotParam& shotParam, CStateManager& mgr);
};
} // namespace urde
