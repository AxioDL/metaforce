#include "MetroidPrime/Enemies/CAtomicBeta.hpp"

#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Enemies/CKnockBackMgr.hpp"
#include "MetroidPrime/Enemies/CPatternedInfo.hpp"
#include "MetroidPrime/TCastTo.hpp"

#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerGun.hpp"
#include "MetroidPrime/Weapons/CElectricBeamProjectile.hpp"
#include "rstl/math.hpp"

const char* CAtomicBeta::skBombLocators[] = {
    "bomb2_LCTR",
    "bomb3_LCTR",
    "bomb4_LCTR",
};
CAtomicBeta::CAtomicBeta(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                         const CTransform4f& xf, CModelData mData, const CActorParameters& actParms,
                         const CPatternedInfo& pInfo, CAssetId electricId, CAssetId weaponId,
                         const CDamageInfo& dInfo, CAssetId particleId, float beamFadeSpeed,
                         float beamRadius, float beamDamageInterval,
                         const CDamageVulnerability& frozenDVuln, float moveSpeed, float minSpeed,
                         float maxSpeed, ushort sId1, ushort sId2, ushort sId3, float speedStep)
: CPatterned(kC_AtomicBeta, uid, name, kFT_Zero, info, xf, mData, pInfo, kMT_Flyer, kCT_One,
             kBT_RestrictedFlyer, actParms, kCS_Small)
, mProjectileIds()
, mBeamFired(false)
, mMinSpeed(minSpeed)
, mMaxSpeed(maxSpeed)
, mSpeedStep(speedStep)
, mCurrentSpeed(mMinSpeed)
, mFrozenDamage(frozenDVuln)
, mMoveSpeed(moveSpeed)
, mDirection(xf.GetColumn(kDY))
, mElectricWeapon(gpSimplePool->GetObj(SObjectTag('ELSC', electricId)))
, mWeaponDesc(gpSimplePool->GetObj(SObjectTag('WPSC', weaponId)))
, mProjectileDamage(dInfo)
, mBeamParticle(particleId)
, mBeamFadeSpeed(beamFadeSpeed)
, mBeamRadius(beamRadius)
, mBeamDamageInterval(beamDamageInterval)
, x63c_(1.f)
, x640_(10.f)
, mSfxId1(CSfxManager::TranslateSFXID(sId1))
, mSfxId2(CSfxManager::TranslateSFXID(sId2))
, mSfxId3(CSfxManager::TranslateSFXID(sId3)) {
  mKnockBackController.SetAutoResetImpulse(false);
  mKnockBackController.SetEnableFreeze(false);
  mKnockBackController.SetX82_24(false);
}

CAtomicBeta::~CAtomicBeta() {
  StopLoopedSound(mSfxHandle2);
  StopLoopedSound(mSfxHandle3);
  StopLoopedSound(mSfxHandle1);
}

ENTITY_ACCEPT_IMPL(CAtomicBeta)

void CAtomicBeta::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  switch (msg) {
  case kSM_Registered:
    BodyCtrl()->Activate(mgr);
    CreateBeams(mgr);
    break;
  case kSM_Deactivate:
    UpdateBeams(mgr, false);
    StopLoopedSound(mSfxHandle2);
    StopLoopedSound(mSfxHandle3);
    StopLoopedSound(mSfxHandle1);
    break;
  case kSM_Deleted:
    DestroyBeams(mgr);
    break;
  }

  CPatterned::AcceptScriptMsg(msg, uid, mgr);
}

void CAtomicBeta::CreateBeams(CStateManager& mgr) {
  const SElectricBeamInfo beamInfo(mElectricWeapon, 50.f, mBeamRadius, 10.f,
                                   mBeamParticle, mBeamFadeSpeed, mBeamDamageInterval);

  for (int i = 0; i < ARRAY_SIZE(skBombLocators); ++i) {
    TUniqueId beamId = mgr.AllocateUniqueId();
    mProjectileIds.push_back(beamId);

    mgr.AddObject(rs_new CElectricBeamProjectile(
        mWeaponDesc, kWT_AI, beamInfo, CTransform4f::Identity(), kMT_Character,
        mProjectileDamage, beamId, GetCurrentAreaId(), GetUniqueId(), CWeapon::kPA_None));
  }
}

void CAtomicBeta::UpdateBeams(CStateManager& mgr, bool fire) {
  if (mBeamFired == fire) {
    return;
  }

  for (int i = 0; i < ARRAY_SIZE(skBombLocators); ++i) {
    CTransform4f locatorXf = GetTransform() * GetScaledLocatorTransform(skBombLocators[i]);
    CTransform4f lookXf = CTransform4f::LookAt(locatorXf.GetTranslation(),
                                               locatorXf.GetTranslation() + locatorXf.GetForward());

    if (CElectricBeamProjectile* ent =
            static_cast< CElectricBeamProjectile* >(mgr.ObjectById(mProjectileIds[i]))) {
      if (fire) {
        ent->Fire(GetTransform() * GetScaledLocatorTransform(skBombLocators[i]), mgr, false);
      } else {
        ent->ResetBeam(mgr, false);
      }
    }
  }

  mBeamFired = fire;
}

void CAtomicBeta::DestroyBeams(CStateManager& mgr) {
  for (int i = 0; i < mProjectileIds.size(); ++i) {
    mgr.DeleteObjectRequest(mProjectileIds[i]);
  }

  mProjectileIds.clear();
}

void CAtomicBeta::Think(const float dt, CStateManager& mgr) {
  CPatterned::Think(dt, mgr);
  const CVector3f moveVec = BodyCtrl()->CommandMgr().GetMoveVector();
  BodyCtrl()->CommandMgr().ClearLocomotionCmds();
  if (moveVec.IsNonZero()) {
    BodyCtrl()->CommandMgr().DeliverCmd(CBCLocomotionCmd(moveVec, mDirection, 1.f));
  }

  const CVector3f diffVec = (mgr.GetPlayer()->GetTranslation() - GetTranslation());
  const float diffMag = diffVec.MagSquared();
  float staticMag = x63c_ * rstl::max_val(1.f - diffMag / (x640_ * x640_), 0.f);
  if (!close_enough(staticMag, 0.f)) {
    mgr.PlayerState()->StaticInterference().AddSource(GetUniqueId(), staticMag, 0.5f);
  }

  if (InMaxRange(mgr, dt)) {
    UpdateBeams(mgr, true);
    PlayLoopedSound(mSfxHandle2, mSfxId2, GetTranslation(), 96);
    PlayLoopedSound(mSfxHandle3, mSfxId3, GetTranslation(), 96);
    StopLoopedSound(mSfxHandle1);
  } else {
    UpdateBeams(mgr, false);
    StopLoopedSound(mSfxHandle2);
    StopLoopedSound(mSfxHandle3);
    PlayLoopedSound(mSfxHandle1, mSfxId1, GetTranslation(), 96);
  }

  for (int i = 0; i < ARRAY_SIZE(skBombLocators); ++i) {
    CElectricBeamProjectile* beam =
        static_cast< CElectricBeamProjectile* >(mgr.ObjectById(mProjectileIds[i]));
    if (beam && beam->GetActive()) {
      const CTransform4f locatorXf = GetTransform() * GetScaledLocatorTransform(skBombLocators[i]);
      CTransform4f lookXf = CTransform4f::LookAt(
          locatorXf.GetTranslation(), locatorXf.GetTranslation() + locatorXf.GetForward());
      beam->UpdateFx(lookXf, dt, mgr);
    }
  }

  mCurrentSpeed = CMath::Clamp(
      mMinSpeed, mSpeedStep * (dt * (IsCharging(mgr) ? 1.f : -1.f)) + mCurrentSpeed,
      mMaxSpeed);
  mSpeed = mCurrentSpeed;
  BodyCtrl()->SetRestrictedFlyerMoveSpeed(mMoveSpeed * mCurrentSpeed);
}

void CAtomicBeta::PlayLoopedSound(CSfxHandle& handle, const ushort sfxId, const CVector3f pos,
                                  const uchar vol) const {
  if (!handle) {
    handle = CSfxManager::AddEmitter(sfxId, pos, CVector3f::Zero(), vol, true, true,
                                     CSfxManager::kMedPriority, GetCurrentAreaId().Value());
  } else {
    CSfxManager::UpdateEmitter(handle, pos, CVector3f::Zero(), vol);
  }
}

void CAtomicBeta::StopLoopedSound(CSfxHandle& handle) const {
  if (handle) {
    CSfxManager::RemoveEmitter(handle);
    handle = CSfxHandle();
  }
}

bool CAtomicBeta::IsCharging(const CStateManager& mgr) {
  if (mgr.GetPlayer()->GetPlayerGun()->GetChargePercentage() > 0.1f) {
    return true;
  }

  return false;
}

const CDamageVulnerability* CAtomicBeta::GetDamageVulnerability() const {
  if (close_enough(GetBodyCtrl()->GetPercentageFrozen(), 0.f)) {
    return CPatterned::GetDamageVulnerability();
  }
  return &mFrozenDamage;
}

void CAtomicBeta::Touch(CActor& other, CStateManager& mgr) {
  if (!IsAlive()) {
    return;
  }

  const CGameProjectile* proj = TCastToConstPtr< CGameProjectile >(other);
  if (proj && proj->GetOwnerId() == mgr.GetPlayer()->GetUniqueId() &&
      (proj->GetAttribField() & CWeapon::kPA_Ice) == CWeapon::kPA_Ice &&
      GetBodyCtrl()->GetPercentageFrozen() == 0.f) {
    KnockBack(CVector3f::Forward(), mgr, proj->GetCurrentDamageInfo(), 0.f, true, false);
  }
  CPatterned::Touch(other, mgr);
}

EWeaponCollisionResponseTypes CAtomicBeta::GetCollisionResponseType(const CVector3f&,
                                                                    const CVector3f&,
                                                                    const CWeaponMode& wMode,
                                                                    int) const {

  return GetDamageVulnerability()->WeaponHits(wMode, CDamageVulnerability::kRD_No)
             ? kWCR_AtomicBeta
             : kWCR_AtomicBetaReflect;
}

void CAtomicBeta::Death(CStateManager& mgr, const CVector3f& direction, EScriptObjectState state) {
  UpdateBeams(mgr, false);
  StopLoopedSound(mSfxHandle2);
  StopLoopedSound(mSfxHandle3);
  StopLoopedSound(mSfxHandle1);
  CPatterned::Death(mgr, direction, state);
}
