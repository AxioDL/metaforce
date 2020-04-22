#include "Runtime/MP1/World/CAtomicBeta.hpp"

#include <array>

#include "Runtime/CSimplePool.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Particle/CWeaponDescription.hpp"
#include "Runtime/Weapon/CElectricBeamProjectile.hpp"
#include "Runtime/Weapon/CPlayerGun.hpp"
#include "Runtime/World/CPlayer.hpp"

namespace urde::MP1 {
constexpr std::array skBombLocators{
    "bomb2_LCTR"sv,
    "bomb3_LCTR"sv,
    "bomb4_LCTR"sv,
};

CAtomicBeta::CAtomicBeta(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                         CModelData&& mData, const CActorParameters& actParms, const CPatternedInfo& pInfo,
                         CAssetId electricId, CAssetId weaponId, const CDamageInfo& dInfo, CAssetId particleId,
                         float f1, float beamRadius, float f3, const CDamageVulnerability& dVuln, float f4, float f5,
                         float f6, s16 sId1, s16 sId2, s16 sId3, float f7)
: CPatterned(ECharacter::AtomicBeta, uid, name, EFlavorType::Zero, info, xf, std::move(mData), pInfo,
             EMovementType::Flyer, EColliderType::One, EBodyType::RestrictedFlyer, actParms, EKnockBackVariant::Small)
, x578_(f5)
, x57c_(f6)
, x580_(f7)
, x584_(x578_)
, x588_frozenDamage(dVuln)
, x5f0_(f4)
, x5f4_(xf.basis[1])
, x600_electricWeapon(g_SimplePool->GetObj({SBIG('ELSC'), electricId}))
, x608_(g_SimplePool->GetObj({SBIG('WPSC'), weaponId}))
, x610_projectileDamage(dInfo)
, x62c_beamParticle(particleId)
, x630_(f1)
, x634_beamRadius(beamRadius)
, x638_(f3)
, x644_(CSfxManager::TranslateSFXID(sId1))
, x646_(CSfxManager::TranslateSFXID(sId2))
, x648_(CSfxManager::TranslateSFXID(sId3)) {
  x460_knockBackController.SetAutoResetImpulse(false);
  x460_knockBackController.SetEnableFreeze(false);
  x460_knockBackController.SetX82_24(false);
}

void CAtomicBeta::CreateBeams(CStateManager& mgr) {
  const SElectricBeamInfo beamInfo{x600_electricWeapon, 50.f, x634_beamRadius, 10.f, x62c_beamParticle, x630_, x638_};

  for (size_t i = 0; i < kBombCount; ++i) {
    const TUniqueId id = mgr.AllocateUniqueId();
    x568_projectileIds.push_back(id);
    mgr.AddObject(new CElectricBeamProjectile(x608_, EWeaponType::AI, beamInfo, {}, EMaterialTypes::Character,
                                              x610_projectileDamage, id, GetAreaIdAlways(), GetUniqueId(),
                                              EProjectileAttrib::None));
  }
}

void CAtomicBeta::UpdateBeams(CStateManager& mgr, bool fireBeam) {
  if (x574_beamFired == fireBeam) {
    return;
  }

  for (size_t i = 0; i < x568_projectileIds.size(); ++i) {
    // zeus::CTransform xf = GetTransform() * GetScaledLocatorTransform(skBombLocators[i]);
    // zeus::CTransform newXf = zeus::lookAt(xf.origin, xf.origin + xf.basis[1], zeus::skUp);
    if (auto* const proj = static_cast<CElectricBeamProjectile*>(mgr.ObjectById(x568_projectileIds[i]))) {
      if (fireBeam) {
        proj->Fire(GetTransform() * GetScaledLocatorTransform(skBombLocators[i]), mgr, false);
      } else {
        proj->ResetBeam(mgr, false);
      }
    }
  }
  x574_beamFired = fireBeam;
}

void CAtomicBeta::FreeBeams(CStateManager& mgr) {
  for (TUniqueId uid : x568_projectileIds) {
    mgr.FreeScriptObject(uid);
  }
  x568_projectileIds.clear();
}

void CAtomicBeta::UpdateOrCreateEmitter(CSfxHandle& handle, u16 id, const zeus::CVector3f& pos, float maxVol) {
  if (handle)
    CSfxManager::UpdateEmitter(handle, pos, {}, maxVol);
  else
    handle = CSfxManager::AddEmitter(id, pos, {}, maxVol, true, true, 0x7F, GetAreaIdAlways());
}

void CAtomicBeta::DestroyEmitter(CSfxHandle& handle) {
  if (handle) {
    CSfxManager::RemoveEmitter(handle);
    handle.reset();
  }
}

void CAtomicBeta::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  if (msg == EScriptObjectMessage::Registered) {
    x450_bodyController->Activate(mgr);
    CreateBeams(mgr);
  } else if (msg == EScriptObjectMessage::Deactivate) {
    UpdateBeams(mgr, false);
    DestroyEmitter(x650_);
    DestroyEmitter(x654_);
    DestroyEmitter(x64c_);
  } else if (msg == EScriptObjectMessage::Deleted) {
    FreeBeams(mgr);
  }
  CPatterned::AcceptScriptMsg(msg, uid, mgr);
}

void CAtomicBeta::Think(float dt, CStateManager& mgr) {
  CPatterned::Think(dt, mgr);
  zeus::CVector3f movementVec = x450_bodyController->GetCommandMgr().GetMoveVector();
  x450_bodyController->GetCommandMgr().ClearLocomotionCmds();
  if (!movementVec.isZero())
    x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(movementVec, x5f4_, 1.f));

  const float mag =
      x63c_ * std::max(1.f - (mgr.GetPlayer().GetTranslation() - GetTranslation()).magSquared() / (x640_ * x640_), 0.f);
  if (!zeus::close_enough(mag, 0.f))
    mgr.GetPlayerState()->GetStaticInterference().AddSource(GetUniqueId(), mag, 0.5f);

  if (InMaxRange(mgr, dt)) {
    UpdateBeams(mgr, true);
    UpdateOrCreateEmitter(x650_, x646_, GetTranslation(), 96 / 127.f);
    UpdateOrCreateEmitter(x654_, x648_, GetTranslation(), 96 / 127.f);
    DestroyEmitter(x64c_);
  } else {
    UpdateBeams(mgr, false);
    DestroyEmitter(x650_);
    DestroyEmitter(x654_);
    UpdateOrCreateEmitter(x64c_, x644_, GetTranslation(), 96 / 127.f);
  }

  // was hardcoded to 3 (kBombCount), but that segfaults after FreeBeams
  for (size_t i = 0; i < x568_projectileIds.size(); ++i) {
    if (auto* const proj = static_cast<CElectricBeamProjectile*>(mgr.ObjectById(x568_projectileIds[i]))) {
      if (!proj->GetActive()) {
        continue;
      }
      const zeus::CTransform xf = GetTransform() * GetScaledLocatorTransform(skBombLocators[i]);
      proj->UpdateFx(zeus::lookAt(xf.origin, xf.origin + xf.frontVector(), zeus::skUp), dt, mgr);
    }
  }

  float speed = x580_ * (dt * (IsPlayerBeamChargedEnough(mgr) ? 1.f : -1.f)) + x584_;
  x584_ = zeus::clamp(x578_, speed, x57c_);
  x3b4_speed = x584_;
  x450_bodyController->SetRestrictedFlyerMoveSpeed(x5f0_ * x584_);
}

const CDamageVulnerability* CAtomicBeta::GetDamageVulnerability() const {
  if (zeus::close_enough(x450_bodyController->GetPercentageFrozen(), 0.f))
    return CPatterned::GetDamageVulnerability();
  return &x588_frozenDamage;
}

void CAtomicBeta::Death(CStateManager& mgr, const zeus::CVector3f& dir, EScriptObjectState state) {
  UpdateBeams(mgr, false);
  DestroyEmitter(x650_);
  DestroyEmitter(x654_);
  DestroyEmitter(x64c_);
  CPatterned::Death(mgr, dir, state);
}

bool CAtomicBeta::IsPlayerBeamChargedEnough(const CStateManager& mgr) {
  const CPlayerGun* gun = mgr.GetPlayer().GetPlayerGun();
  return (gun->IsCharging() ? gun->GetChargeBeamFactor() : 0.f) > 0.1f;
}
} // namespace urde::MP1
