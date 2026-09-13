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
, x568_projectileIds()
, x574_beamFired(false)
, x578_minSpeed(minSpeed)
, x57c_maxSpeed(maxSpeed)
, x580_speedStep(speedStep)
, x584_currentSpeed(x578_minSpeed)
, x588_frozenDamage(frozenDVuln)
, x5f0_moveSpeed(moveSpeed)
, x5f4_direction(xf.GetColumn(kDY))
, x600_electricWeapon(gpSimplePool->GetObj(SObjectTag('ELSC', electricId)))
, x608_weaponDesc(gpSimplePool->GetObj(SObjectTag('WPSC', weaponId)))
, x610_projectileDamage(dInfo)
, x62c_beamParticle(particleId)
, x630_beamFadeSpeed(beamFadeSpeed)
, x634_beamRadius(beamRadius)
, x638_beamDamageInterval(beamDamageInterval)
, x63c_(1.f)
, x640_(10.f)
, x644_sfxId1(CSfxManager::TranslateSFXID(sId1))
, x646_sfxId2(CSfxManager::TranslateSFXID(sId2))
, x648_sfxId3(CSfxManager::TranslateSFXID(sId3)) {
  x460_knockBackController.SetAutoResetImpulse(false);
  x460_knockBackController.SetEnableFreeze(false);
  x460_knockBackController.SetX82_24(false);
}

CAtomicBeta::~CAtomicBeta() {
  StopLoopedSound(x650_sfxHandle2);
  StopLoopedSound(x654_sfxHandle3);
  StopLoopedSound(x64c_sfxHandle1);
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
    StopLoopedSound(x650_sfxHandle2);
    StopLoopedSound(x654_sfxHandle3);
    StopLoopedSound(x64c_sfxHandle1);
    break;
  case kSM_Deleted:
    DestroyBeams(mgr);
    break;
  }

  CPatterned::AcceptScriptMsg(msg, uid, mgr);
}

void CAtomicBeta::CreateBeams(CStateManager& mgr) {
  const SElectricBeamInfo beamInfo(x600_electricWeapon, 50.f, x634_beamRadius, 10.f,
                                   x62c_beamParticle, x630_beamFadeSpeed, x638_beamDamageInterval);

  for (int i = 0; i < ARRAY_SIZE(skBombLocators); ++i) {
    TUniqueId beamId = mgr.AllocateUniqueId();
    x568_projectileIds.push_back(beamId);

    mgr.AddObject(rs_new CElectricBeamProjectile(
        x608_weaponDesc, kWT_AI, beamInfo, CTransform4f::Identity(), kMT_Character,
        x610_projectileDamage, beamId, GetCurrentAreaId(), GetUniqueId(), CWeapon::kPA_None));
  }
}

void CAtomicBeta::UpdateBeams(CStateManager& mgr, bool fire) {
  if (x574_beamFired == fire) {
    return;
  }

  for (int i = 0; i < ARRAY_SIZE(skBombLocators); ++i) {
    CTransform4f locatorXf = GetTransform() * GetScaledLocatorTransform(skBombLocators[i]);
    CTransform4f lookXf = CTransform4f::LookAt(locatorXf.GetTranslation(),
                                               locatorXf.GetTranslation() + locatorXf.GetForward());

    if (CElectricBeamProjectile* ent =
            static_cast< CElectricBeamProjectile* >(mgr.ObjectById(x568_projectileIds[i]))) {
      if (fire) {
        ent->Fire(GetTransform() * GetScaledLocatorTransform(skBombLocators[i]), mgr, false);
      } else {
        ent->ResetBeam(mgr, false);
      }
    }
  }

  x574_beamFired = fire;
}

void CAtomicBeta::DestroyBeams(CStateManager& mgr) {
  for (int i = 0; i < x568_projectileIds.size(); ++i) {
    mgr.DeleteObjectRequest(x568_projectileIds[i]);
  }

  x568_projectileIds.clear();
}

void CAtomicBeta::Think(const float dt, CStateManager& mgr) {
  CPatterned::Think(dt, mgr);
  const CVector3f moveVec = BodyCtrl()->CommandMgr().GetMoveVector();
  BodyCtrl()->CommandMgr().ClearLocomotionCmds();
  if (moveVec.IsNonZero()) {
    BodyCtrl()->CommandMgr().DeliverCmd(CBCLocomotionCmd(moveVec, x5f4_direction, 1.f));
  }

  const CVector3f diffVec = (mgr.GetPlayer()->GetTranslation() - GetTranslation());
  const float diffMag = diffVec.MagSquared();
  float staticMag = x63c_ * rstl::max_val(1.f - diffMag / (x640_ * x640_), 0.f);
  if (!close_enough(staticMag, 0.f)) {
    mgr.PlayerState()->StaticInterference().AddSource(GetUniqueId(), staticMag, 0.5f);
  }

  if (InMaxRange(mgr, dt)) {
    UpdateBeams(mgr, true);
    PlayLoopedSound(x650_sfxHandle2, x646_sfxId2, GetTranslation(), 96);
    PlayLoopedSound(x654_sfxHandle3, x648_sfxId3, GetTranslation(), 96);
    StopLoopedSound(x64c_sfxHandle1);
  } else {
    UpdateBeams(mgr, false);
    StopLoopedSound(x650_sfxHandle2);
    StopLoopedSound(x654_sfxHandle3);
    PlayLoopedSound(x64c_sfxHandle1, x644_sfxId1, GetTranslation(), 96);
  }

  for (int i = 0; i < ARRAY_SIZE(skBombLocators); ++i) {
    CElectricBeamProjectile* beam =
        static_cast< CElectricBeamProjectile* >(mgr.ObjectById(x568_projectileIds[i]));
    if (beam && beam->GetActive()) {
      const CTransform4f locatorXf = GetTransform() * GetScaledLocatorTransform(skBombLocators[i]);
      CTransform4f lookXf = CTransform4f::LookAt(
          locatorXf.GetTranslation(), locatorXf.GetTranslation() + locatorXf.GetForward());
      beam->UpdateFx(lookXf, dt, mgr);
    }
  }

  x584_currentSpeed = CMath::Clamp(
      x578_minSpeed, x580_speedStep * (dt * (IsCharging(mgr) ? 1.f : -1.f)) + x584_currentSpeed,
      x57c_maxSpeed);
  x3b4_speed = x584_currentSpeed;
  BodyCtrl()->SetRestrictedFlyerMoveSpeed(x5f0_moveSpeed * x584_currentSpeed);
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
  return &x588_frozenDamage;
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
  StopLoopedSound(x650_sfxHandle2);
  StopLoopedSound(x654_sfxHandle3);
  StopLoopedSound(x64c_sfxHandle1);
  CPatterned::Death(mgr, direction, state);
}
