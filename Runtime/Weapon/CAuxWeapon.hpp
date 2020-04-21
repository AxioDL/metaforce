#pragma once

#include <memory>

#include "Runtime/RetroTypes.hpp"
#include "Runtime/rstl.hpp"
#include "Runtime/CPlayerState.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/Weapon/CGunWeapon.hpp"
#include "Runtime/Weapon/CWeapon.hpp"

namespace urde {

class CAuxWeapon {
  TCachedToken<CWeaponDescription> x0_missile;
  TCachedToken<CGenDescription> xc_flameMuzzle;
  TCachedToken<CGenDescription> x18_busterMuzzle;
  std::unique_ptr<CElementGen> x24_muzzleFxGen;
  rstl::reserved_vector<TCachedToken<CWeaponDescription>, 5> x28_combos;
  float x68_ammoConsumeTimer = 0.f;
  TUniqueId x6c_playerId;
  TUniqueId x6e_flameThrowerId = kInvalidUniqueId;
  TUniqueId x70_waveBusterId = kInvalidUniqueId;
  CPlayerState::EBeamId x74_firingBeamId = CPlayerState::EBeamId::Invalid;
  CPlayerState::EBeamId x78_loadBeamId = CPlayerState::EBeamId::Power;
  CSfxHandle x7c_comboSfx;
  bool x80_24_isLoaded : 1 = false;
  void InitComboData();
  void FreeComboVoiceId();
  void DeleteFlameThrower(CStateManager& mgr);
  void CreateFlameThrower(const zeus::CTransform& xf, CStateManager& mgr, float dt);
  void DeleteWaveBusterBeam(CStateManager& mgr);
  void CreateWaveBusterBeam(EProjectileAttrib attribs, TUniqueId homingTarget, const zeus::CTransform& xf,
                            CStateManager& mgr);
  void LaunchMissile(float dt, bool underwater, bool charged, CPlayerState::EBeamId currentBeam,
                     EProjectileAttrib attrib, const zeus::CTransform& xf, TUniqueId homingId, CStateManager& mgr);

public:
  explicit CAuxWeapon(TUniqueId playerId);
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr);
  bool IsComboFxActive(const CStateManager& mgr) const;
  void Load(CPlayerState::EBeamId curBeam, CStateManager& mgr);
  void StopComboFx(CStateManager& mgr, bool deactivate);
  bool UpdateComboFx(float dt, const zeus::CVector3f& scale, const zeus::CVector3f& pos, const zeus::CTransform& xf,
                     CStateManager& mgr);
  void Fire(float dt, bool underwater, CPlayerState::EBeamId currentBeam, EChargeState chargeState,
            const zeus::CTransform& xf, CStateManager& mgr, EWeaponType type, TUniqueId homingId);
  void LoadIdle();
  bool IsLoaded() const { return x80_24_isLoaded; }
  void RenderMuzzleFx() const;
  TUniqueId HasTarget(const CStateManager& mgr) const;
  void SetNewTarget(TUniqueId targetId, CStateManager& mgr);
};

} // namespace urde
