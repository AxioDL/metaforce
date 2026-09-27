#include "MetroidPrime/Weapons/CAuxWeapon.hpp"

#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "MetroidPrime/CRumbleManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Cameras/CCameraShakeData.hpp"
#include "MetroidPrime/SFX/Weapons.h"
#include "MetroidPrime/Tweaks/CTweakPlayerGun.hpp"
#include "MetroidPrime/Weapons/CEnergyProjectile.hpp"
#include "MetroidPrime/Weapons/CNewFlameThrower.hpp"
#include "MetroidPrime/Weapons/CWaveBuster.hpp"
#include "MetroidPrime/Weapons/CWeaponAssetInfo.hpp"

const ushort CAuxWeapon::skSoundId[5] = {SFXsam_a_co1fire_00, SFXsam_a_icecofir_00,
                                         SFXsam_a_wavcofir_lp_00, SFXsam_a_placofir_lp_00,
                                         SFXsam_a_co1fire_00};

CAuxWeapon::CAuxWeapon(TUniqueId playerId)
: mMissile(gpSimplePool->GetObj("Missile"))
, mFlameMuzzle(gpSimplePool->GetObj("FlameMuzzle"))
, mBusterMuzzle(gpSimplePool->GetObj("BusterMuzzle"))
, mMuzzleFxGen(nullptr)
, mAmmoConsumeTimer(0.f)
, mPlayerId(playerId)
, mFlameThrowerId(kInvalidUniqueId)
, mWaveBusterId(kInvalidUniqueId)
, mFiringBeamId(CPlayerState::kBI_Invalid)
, mLoadBeamId(CPlayerState::kBI_Power)
, mIsLoaded(false) {
  InitComboData();
}

void CAuxWeapon::Fire(float dt, bool underwater, int currentBeam,
                      CPlayerState::EChargeStage chargeState, const CTransform4f& xf,
                      CStateManager& mgr, EWeaponType type, TUniqueId homingId) {
  if (!mIsLoaded) {
    return;
  }

  const bool charged = chargeState != CPlayerState::kCS_Normal;
  uint attribs = charged ? CGameProjectile::GetBeamAttribType(type) | CWeapon::kPA_ComboShot : 0;
  if (!charged) {
    LaunchMissile(dt, underwater, charged, currentBeam, attribs, xf, homingId, mgr);
  } else {
    switch (currentBeam) {
    case CPlayerState::kBI_Power:
    case CPlayerState::kBI_Ice:
      LaunchMissile(dt, underwater, charged, currentBeam, attribs, xf, homingId, mgr);
      break;
    case CPlayerState::kBI_Wave:
      CreateWaveBusterBeam(attribs, homingId, xf, mgr);
      break;
    case CPlayerState::kBI_Plasma:
      CreateFlameThrower(xf, mgr, dt);
      break;
    }
  }
}

void CAuxWeapon::SetNewTarget(TUniqueId target, CStateManager& mgr) {
  switch (mFiringBeamId) {
  case CPlayerState::kBI_Wave:
    if (CWaveBuster* beam = static_cast< CWaveBuster* >(mgr.ObjectById(mWaveBusterId))) {
      beam->SetNewTarget(target, mgr);
    }
    break;
  }
}

TUniqueId CAuxWeapon::HasTarget(const CStateManager& mgr) const {
  switch (mFiringBeamId) {
  case CPlayerState::kBI_Wave:
    if (const CWaveBuster* beam =
            static_cast< const CWaveBuster* >(mgr.GetObjectById(mWaveBusterId))) {
      return beam->GetHomingTargetId();
    }
    break;
  }
  return kInvalidUniqueId;
}

void CAuxWeapon::RenderMuzzleFx() const {
  switch (mFiringBeamId) {
  case CPlayerState::kBI_Wave:
  case CPlayerState::kBI_Plasma:
    mMuzzleFxGen->Render();
    break;
  }
}

const bool CAuxWeapon::UpdateComboFx(float dt, const CVector3f& scale, const CVector3f& firePos,
                                     const CTransform4f& targetXf, CStateManager& mgr) {
  if (!mIsLoaded || mFiringBeamId == CPlayerState::kBI_Invalid) {
    return false;
  }

  bool firing = false;
  CPlayerState& playerState = *mgr.PlayerState();
  if (!CSfxManager::IsPlaying(mComboSfx) && mComboSfx) {
    FreeComboVoiceId();
  }

  switch (mFiringBeamId) {
  case CPlayerState::kBI_Wave:
  case CPlayerState::kBI_Plasma: {
    bool firingFx = false;
    if (mFiringBeamId == CPlayerState::kBI_Wave) {
      CWaveBuster* beam = static_cast< CWaveBuster* >(mgr.ObjectById(mWaveBusterId));
      if (beam && beam->IsFiring()) {
        beam->UpdateFx(targetXf, dt, mgr);
        firing = true;
        firingFx = true;
      } else {
        DeleteWaveBusterBeam(mgr);
        playerState.SetFiringComboBeam(false);
      }
    } else {
      CNewFlameThrower* flame =
          static_cast< CNewFlameThrower* >(mgr.ObjectById(mFlameThrowerId));
      bool needsDelete = true;
      if (flame) {
        firingFx = flame->CanRenderAuxEffects();
        if (flame->GetActive()) {
          flame->UpdateFx(targetXf, dt, mgr);
          firing = flame->IsFiring();
        }
        if (mFlameThrowerId != kInvalidUniqueId) {
          needsDelete = flame->AreEffectsFinished();
        }
      }
      if (needsDelete) {
        DeleteFlameThrower(mgr);
        playerState.SetFiringComboBeam(false);
      }
    }

    if (firingFx) {
      mAmmoConsumeTimer += dt;
      if (playerState.GetItemAmount(CPlayerState::kIT_Missiles) > 0 &&
          mAmmoConsumeTimer >= playerState.GetComboFireAmmoPeriod()) {
        playerState.DecrPickUp(CPlayerState::kIT_Missiles, 1);
        mAmmoConsumeTimer = 0.f;
      }
    }
    if (playerState.GetItemAmount(CPlayerState::kIT_Missiles) == 0) {
      StopComboFx(mgr, false);
    }

    mMuzzleFxGen->SetGlobalTranslation(firePos);
    mMuzzleFxGen->SetGlobalScale(scale);
    mMuzzleFxGen->SetParticleEmission(firingFx);
    mMuzzleFxGen->Update(dt);
    break;
  }
  }
  return firing;
}

void CAuxWeapon::StopComboFx(CStateManager& mgr, bool deactivate) {
  switch (mFiringBeamId) {
  case CPlayerState::kBI_Wave:
    if (CWaveBuster* beam = static_cast< CWaveBuster* >(mgr.ObjectById(mWaveBusterId))) {
      beam->ResetBeam(deactivate);
      DeleteWaveBusterBeam(mgr);
    }
    break;
  case CPlayerState::kBI_Plasma:
    if (CNewFlameThrower* flame =
            static_cast< CNewFlameThrower* >(mgr.ObjectById(mFlameThrowerId))) {
      mgr.PlayerState()->SetFiringComboBeam(false);
      if (flame->IsFiring()) {
        flame->Reset(mgr, deactivate);
        FreeComboVoiceId();
      } else if (flame->GetActive() && deactivate) {
        flame->Reset(mgr, deactivate);
      }
    }
    break;
  }

  if (deactivate) {
    mFiringBeamId = CPlayerState::kBI_Invalid;
    mAmmoConsumeTimer = 0.f;
  }
}

bool CAuxWeapon::IsComboFxActive(const CStateManager& mgr) const {
  switch (mFiringBeamId) {
  case CPlayerState::kBI_Wave:
    if (const CWaveBuster* beam =
            static_cast< const CWaveBuster* >(mgr.GetObjectById(mWaveBusterId))) {
      return beam->IsFiring();
    }
    break;
  case CPlayerState::kBI_Plasma:
    if (const CNewFlameThrower* flame =
            static_cast< const CNewFlameThrower* >(mgr.GetObjectById(mFlameThrowerId))) {
      return flame->IsFiring();
    }
    break;
  }
  return false;
}

void CAuxWeapon::InitComboData() {
  static const char* const skComboNames[] = {"SuperMissile", "IceCombo", "WaveBuster",
                                             "FlameThrower", "SuperMissile"};
  for (int i = 0; i < 5; ++i) {
    mCombos.push_back(gpSimplePool->GetObj(skComboNames[i]));
  }
}

void CAuxWeapon::LaunchMissile(float dt, const bool underwater, bool charged, int currentBeam,
                               uint attribs, const CTransform4f& xf, TUniqueId target,
                               CStateManager& mgr) {
  const TToken< CWeaponDescription >& combo = mCombos[currentBeam];
  const CDamageInfo& damage =
      charged ? gpTweakPlayerGun->GetComboDamage(currentBeam) : gpTweakPlayerGun->GetMissleDamage();
  const ushort sfx = charged ? skSoundId[currentBeam] : ushort(SFXsam_a_mislfire_00);
  CEntity* projectile = rs_new CEnergyProjectile(
      true, charged ? combo : static_cast< const TToken< CWeaponDescription >& >(mMissile),
      charged ? kWT_Power : kWT_Missile, xf, kMT_Player, NWeaponTypes::get_shot_damage(damage, mgr),
      mgr.AllocateUniqueId(), kInvalidAreaId, mPlayerId, target,
      attribs | CWeapon::kPA_ArmCannon, underwater, CVector3f(1.f, 1.f, 1.f),
      rstl::optional_object< TLockedToken< CGenDescription > >(),
      CSfxManager::kInternalInvalidSfxId, false);
  if (projectile) {
    mgr.AddObject(projectile);
    projectile->Think(dt, mgr);
  }

  if (charged) {
    static_cast< CEnergyProjectile* >(projectile)
        ->SetCameraShake(CCameraShakeData::HardVertShakeDistance(
            0.25f, 0.75f, 50.f, static_cast< CEnergyProjectile* >(projectile)->GetTranslation()));
    mgr.CameraManager()->AddCameraShaker(CCameraShakeData::skHardRecoil, false);
  } else {
    mgr.GetRumbleManager()->Rumble(mgr, kRFX_PlayerMissileFire, 0.5f, kRP_One);
  }
  mComboSfx = NWeaponTypes::play_sfx(sfx, underwater, false, 0x4a);
}

void CAuxWeapon::CreateFlameThrower(const CTransform4f& xf, CStateManager& mgr, float dt) {
  DeleteFlameThrower(mgr);
  if (mFlameThrowerId != kInvalidUniqueId) {
    return;
  }

  const CWeaponAssetInfo resources(NWeaponTypes::get_asset_id_from_name("NFTMainFire"),
                                   NWeaponTypes::get_asset_id_from_name("NFTMainSmoke"),
                                   NWeaponTypes::get_asset_id_from_name("NFTSwooshCenter"),
                                   NWeaponTypes::get_asset_id_from_name("NFTSwooshFire"),
                                   NWeaponTypes::get_asset_id_from_name("NFTSecondarySmoke"),
                                   NWeaponTypes::get_asset_id_from_name("NFTSecondaryFire"),
                                   NWeaponTypes::get_asset_id_from_name("NFTSecondarySparks"));
  mFlameThrowerId = mgr.AllocateUniqueId();
  CNewFlameThrower* flame = rs_new CNewFlameThrower(
      mCombos[3], rstl::string_l("Player_FlameThrower"), kWT_Plasma, resources, xf, kMT_Player,
      NWeaponTypes::get_shot_damage(gpTweakPlayerGun->GetComboDamage(3), mgr), mFlameThrowerId,
      kInvalidAreaId, mPlayerId, CWeapon::kPA_None);
  mgr.AddObject(*flame);
  flame->Think(dt, mgr);
  flame->Fire(xf, mgr, false);
  mMuzzleFxGen = rs_new CElementGen(mFlameMuzzle);
  mComboSfx = NWeaponTypes::play_sfx(SFXsam_a_placofir_lp_00, false, true, 0x4a);
  mgr.CameraManager()->AddCameraShaker(CCameraShakeData::skHardRecoil, false);
  mgr.PlayerState()->SetFiringComboBeam(true);
  mFiringBeamId = CPlayerState::kBI_Plasma;
}

void CAuxWeapon::CreateWaveBusterBeam(uint attribs, TUniqueId target, const CTransform4f& xf,
                                      CStateManager& mgr) {
  DeleteWaveBusterBeam(mgr);
  if (mWaveBusterId != kInvalidUniqueId) {
    return;
  }

  mWaveBusterId = mgr.AllocateUniqueId();
  CWaveBuster* beam =
      rs_new CWaveBuster(mCombos[2], kWT_Wave, xf, kMT_Player,
                         NWeaponTypes::get_shot_damage(gpTweakPlayerGun->GetComboDamage(2), mgr),
                         mWaveBusterId, kInvalidAreaId, mPlayerId, target, attribs);
  mgr.AddObject(*beam);
  mMuzzleFxGen = rs_new CElementGen(mBusterMuzzle);
  mComboSfx = NWeaponTypes::play_sfx(SFXsam_a_wavcofir_lp_00, false, true, 0x4a);
  mgr.CameraManager()->AddCameraShaker(CCameraShakeData::skSoftRecoil, false);
  mgr.PlayerState()->SetFiringComboBeam(true);
  mFiringBeamId = CPlayerState::kBI_Wave;
}

void CAuxWeapon::DeleteFlameThrower(CStateManager& mgr) {
  FreeComboVoiceId();
  if (mFlameThrowerId != kInvalidUniqueId) {
    mgr.DeleteObjectRequest(mFlameThrowerId);
    mFlameThrowerId = kInvalidUniqueId;
    mFiringBeamId = CPlayerState::kBI_Invalid;
    CPlayerState& playerState = *mgr.PlayerState();
    if (playerState.IsFiringComboBeam()) {
      playerState.SetFiringComboBeam(false);
    }
  }
}

void CAuxWeapon::DeleteWaveBusterBeam(CStateManager& mgr) {
  FreeComboVoiceId();
  if (mWaveBusterId != kInvalidUniqueId) {
    mgr.DeleteObjectRequest(mWaveBusterId);
    mWaveBusterId = kInvalidUniqueId;
    mFiringBeamId = CPlayerState::kBI_Invalid;
    CPlayerState& playerState = *mgr.PlayerState();
    if (playerState.IsFiringComboBeam()) {
      playerState.SetFiringComboBeam(false);
    }
  }
}

void CAuxWeapon::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) {
  switch (msg) {
  case kSM_Registered:
    break;
  case kSM_Deleted:
    DeleteFlameThrower(mgr);
    DeleteWaveBusterBeam(mgr);
    break;
  default:
    break;
  }
}

void CAuxWeapon::FreeComboVoiceId() {
  CSfxManager::SfxStop(mComboSfx);
  mComboSfx.Clear();
}

void CAuxWeapon::Load(int curBeam, CStateManager& mgr) {
  mIsLoaded = false;
  switch (mLoadBeamId) {
  case CPlayerState::kBI_Wave:
    DeleteWaveBusterBeam(mgr);
    break;
  case CPlayerState::kBI_Plasma:
    DeleteFlameThrower(mgr);
    break;
  }
  mCombos[mLoadBeamId].Unlock();
  mCombos[curBeam].Lock();
  mLoadBeamId = curBeam;
  LoadIdle();
}

void CAuxWeapon::LoadIdle() { mIsLoaded = mCombos[mLoadBeamId].TryCache(); }
