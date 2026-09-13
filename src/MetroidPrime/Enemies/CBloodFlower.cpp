#include "MetroidPrime/Enemies/CBloodFlower.hpp"

#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTrigger.hpp"
#include "MetroidPrime/Weapons/CTargetableProjectile.hpp"

#include <Kyoto/Animation/CInt32POINode.hpp>
#include <Kyoto/Audio/CSfxManager.hpp>
#include <Kyoto/Particles/CElementGen.hpp>
#include <rstl/algorithm.hpp>

static const char* const kMouthLctrName = "LCTR_FLOFLOWER";
static const char* sFireEffects[3] = {"Fire1", "Fire2", "Fire3"};

inline rstl::string GetMouthLocatorName() { return rstl::string_l(kMouthLctrName); }
CBloodFlower::CBloodFlower(const TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                           const CTransform4f& xf, const CModelData& mData,
                           const CPatternedInfo& pInfo, const CAssetId podEffectDescId,
                           const CAssetId projectileDescId, const CActorParameters& actParams,
                           const CAssetId wpscId2, const CDamageInfo& dInfo1,
                           const CDamageInfo& projectileDamage, const CDamageInfo& podDamage,
                           const CAssetId partId2, const CAssetId partId3, const CAssetId partId4,
                           const float f1, const CAssetId partId5, const uint soundId)
: CPatterned(kC_BloodFlower, uid, name, kFT_Zero, info, xf, mData, pInfo, kMT_Ground, kCT_One,
             kBT_Restricted, actParams, kCS_Medium)
, mPodEffectDescription(gpSimplePool->GetObj(SObjectTag('PART', podEffectDescId)))
, mPodEffect(rs_new CElementGen(mPodEffectDescription))
, mProjectileDesc(gpSimplePool->GetObj(SObjectTag('WPSC', projectileDescId)))
, mCurAttackTime(0.f)
, mProjectileOffset(0.f)
, mProjectileState(0)
, mProjectileInfo(wpscId2, dInfo1)
, x5b8_(0.f)
, mProjectileDelay(0.f)
, mLeashTime(0.f)
, mVisorSfx(CSfxManager::TranslateSFXID(soundId))
, mEffectState(0)
, mProjectileDamage(projectileDamage)
, mPodDamage(podDamage)
, x614_(f1)
, x618_(partId2)
, x61c_(partId3)
, x620_(partId4) {
  const CTransform4f locatorTransform = GetLocatorTransform(rstl::string_l(kMouthLctrName));
  mProjectileOffset = GetModelData()->GetScale().GetZ() * locatorTransform.Get23();
  mPodEffect->SetParticleEmission(false);
  mPodEffect->SetOrientation(xf.GetRotation());
  mPodEffect->SetGlobalTranslation(xf.GetTranslation());
  const CVector3f& modelScale = CVector3f(mData.GetScale());
  mPodEffect->SetGlobalScale(modelScale);
  mProjectileDesc.Lock();
  mProjectileInfo.Token().Lock();
  KnockBackCtrl().SetAutoResetImpulse(false);
  if (partId5 != kInvalidAssetId) {
    mVisorParticle = gpSimplePool->GetObj(SObjectTag('PART', partId5));
  }
}

ENTITY_ACCEPT_IMPL(CBloodFlower)

void CBloodFlower::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  CPatterned::AcceptScriptMsg(msg, uid, mgr);
  switch (msg) {
  case kSM_Activate:
    break;
  case kSM_Registered: {
    BodyCtrl()->Activate(mgr);
    x5b8_ = GetHealthInfo(mgr)->GetHP();
  } break;
  case kSM_Damage: {
    if (BodyCtrl()->IsFrozen()) {
      BodyCtrl()->FrozenBreakout();
    }
    ResetShotTimer(mgr);
    UpdateFire(mgr);
  } break;
  }
}

CProjectileInfo* CBloodFlower::ProjectileInfo() { return &mProjectileInfo; }

void CBloodFlower::InActive(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyCtrl()->SetLocomotionType(pas::kLT_Relaxed);
    SetWasHit(false);
    break;
  case kStateMsg_Update:
    break;
  case kStateMsg_Deactivate:
    mLeashTime = 0.f;
    break;
  }
}

void CBloodFlower::Active(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    SetAnimationState(kAS_Ready);
    ResetShotTimer(mgr);
    break;
  case kStateMsg_Update: {
    TryCommand(mgr, pas::kAS_LoopReaction, &CPatterned::TryLoopedReaction, 0);
    BodyCtrl()->CommandMgr().DeliverCmd(CBCAdditiveAimCmd());
    mCurAttackTime += arg;
    CVector3f diff = mgr.GetPlayer()->GetTranslation() - GetTranslation();
    rstl::swap(diff[1], diff[2]);
    const CVector3f target = GetTransform().TransposeRotate(diff);
    BodyCtrl()->CommandMgr().DeliverAdditiveTargetVector(target);
  } break;
  case kStateMsg_Deactivate:
    BodyCtrl()->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_ExitState));
    BodyCtrl()->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_AdditiveIdle));
    break;
  }
}

void CBloodFlower::BulbAttack(CStateManager& mgr, const EStateMsg msg, const float arg) {
  switch (msg) {
  case kStateMsg_Activate: {
    BodyCtrl()->CommandMgr().DeliverCmd(
        CBCProjectileAttackCmd(pas::kS_Zero, mgr.GetPlayer()->GetTranslation(), true));
    mProjectileState = 1;
  } break;
  case kStateMsg_Update: {
  } break;
  case kStateMsg_Deactivate: {
  } break;
  }
}

void CBloodFlower::TriggerPodSteam(CStateManager& mgr, const bool activate) const {
  for (AUTO(conn, GetConnectionList().begin()); conn != GetConnectionList().end(); ++conn) {
    AUTO(search, mgr.GetIdListForScript(conn->x8_objId));
    if (search.first != search.second) {
      if (CScriptTrigger* trigger =
              TCastToPtr< CScriptTrigger >(mgr.ObjectById(search.first->second))) {
        mgr.DeliverScriptMsg(trigger, GetUniqueId(), (activate ? kSM_Activate : kSM_Deactivate));
      }
    }
  }
}

void CBloodFlower::PodAttack(CStateManager& mgr, const EStateMsg msg, const float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyCtrl()->CommandMgr().DeliverCmd(CBCMeleeAttackCmd(pas::kS_Zero));
    mProjectileState = 2;
    mPodEffect->SetParticleEmission(true);
    TriggerPodSteam(mgr, true);
    break;
  case kStateMsg_Update:
    if (TooClose(mgr, 0.f)) {
      mgr.ApplyDamage(
          GetUniqueId(), mgr.GetPlayer()->GetUniqueId(), GetUniqueId(),
          GetPodDamage().MakeScaledForTime(arg),
          CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()));
    }
    break;
  case kStateMsg_Deactivate:
    mPodEffect->SetParticleEmission(false);
    TriggerPodSteam(mgr, false);
    BodyCtrl()->CommandMgr().DeliverCmd(CBCKnockBackCmd(CVector3f::Zero(), pas::kS_One));
    break;
  }
}

bool CBloodFlower::Leash(CStateManager&, float) { return mLeashTime > GetPlayerLeashTime(); }

bool CBloodFlower::ShouldTurn(CStateManager& mgr, const float arg) {
  if (TooClose(mgr, 0.f)) {
    return false;
  }

  CVector3f frontVec = GetTransform().GetForward();
  CVector3f diff = mgr.GetPlayer()->GetTranslation() - GetTranslation();
  frontVec.SetZ(0.f);
  frontVec.Normalize();
  diff.SetZ(0.f);
  diff.Normalize();

  return CVector3f::Dot(frontVec, diff) < 0.996f;
}

bool CBloodFlower::ShouldAttack(CStateManager& mgr, float) {
  if (TooClose(mgr, 0.f)) {
    return false;
  }

  if (mCurAttackTime > GetAverageAttackTime()) {
    float height = mgr.GetPlayer()->GetTransform().Get23();
    height += mgr.GetPlayer()->GetEyeHeight();

    const float projectileOffset = GetTranslation().GetZ() + x614_ + mProjectileOffset;
    ResetShotTimer(mgr);
    if (height < projectileOffset) {
      return true;
    }
  }
  return false;
}

void CBloodFlower::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node,
                                   const EUserEventType type, const float dt) {
  bool skipCall = false;
  switch (type) {
  case kUE_Delete: {
    if (mEffectState > 0) {
      TurnEffectsOff(mEffectState <= 3 ? mEffectState - 1 : 2, mgr);
    }
    break;
  }
  case kUE_Projectile: {
    switch (mProjectileState) {
    case 1: {
      if (mProjectileDelay <= 0.f) {
        LaunchPollenProjectile(GetLctrTransform(node.GetLocatorName()), mgr, x614_, 5);
        mProjectileState = 0;
        mProjectileDelay = 0.5f;
      }
    } break;
    case 2: {
    } break;
    default:
      break;
    }
    skipCall = true;
    break;
  }
  default:
    break;
  }

  if (!skipCall) {
    CPatterned::DoUserAnimEvent(mgr, node, type, dt);
  }
}

void CBloodFlower::Think(const float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  CPatterned::Think(dt, mgr);
  mPodEffect->Update(dt);
  if (mProjectileDelay > 0.f) {
    mProjectileDelay -= dt;
  }
  mLeashTime += dt;
}

void CBloodFlower::Render(const CStateManager& mgr) const {
  CPatterned::Render(mgr);
  mPodEffect->Render();
}

void CBloodFlower::ResetShotTimer(CStateManager& mgr) {
  const float rnd = -mgr.Random()->Float();
  mCurAttackTime = x308_attackTimeVariation * rnd;
}

EWeaponCollisionResponseTypes CBloodFlower::GetCollisionResponseType(const CVector3f& position,
                                                                     const CVector3f& direction,
                                                                     const CWeaponMode& mode,
                                                                     int attrib) const {
  return GetDamageVulnerability()->WeaponHurts(mode, CDamageVulnerability::kRD_No) ? kWCR_Unknown28
                                                                                   : kWCR_Unknown78;
}

CTargetableProjectile* CBloodFlower::CreateArcProjectile(CStateManager& mgr,
                                                         const TToken< CWeaponDescription >& desc,
                                                         const CTransform4f& xf,
                                                         const CDamageInfo& damage, TUniqueId uid) {
  if (mProjectileDesc.TryCache()) {
    TUniqueId projId = mgr.AllocateUniqueId();
    CTargetableProjectile* proj = rs_new CTargetableProjectile(
        desc, kWT_AI, xf, kMT_Character, damage, mProjectileDamage, projId, GetCurrentAreaId(),
        GetUniqueId(), mProjectileDesc, uid, CWeapon::kPA_None, mVisorParticle, mVisorSfx, false);

    if (mgr.GetPlayer()->GetOrbitTargetId() == GetUniqueId()) {
      mgr.Player()->SetAimTargetId(projId);
      mgr.Player()->SetOrbitTargetId(projId, mgr);
    }

    return proj;
  }
  return nullptr;
}

void CBloodFlower::UpdateFire(CStateManager& mgr) {
  switch (mEffectState) {
  case 0:
    TurnEffectsOn(0, mgr);
    break;
  case 1:
    TurnEffectsOff(0, mgr);
    TurnEffectsOn(1, mgr);
    break;
  case 2:
    TurnEffectsOff(1, mgr);
    TurnEffectsOn(2, mgr);
    break;
  }

  mEffectState++;
}

void CBloodFlower::TurnEffectsOn(const int effectIndex, CStateManager& mgr) {
  AnimationData()->SetParticleEffectState(rstl::string_l(sFireEffects[effectIndex]), true, mgr);
}

void CBloodFlower::TurnEffectsOff(const int effectIndex, CStateManager& mgr) {
  AnimationData()->SetParticleEffectState(rstl::string_l(sFireEffects[effectIndex]), false, mgr);
}

void CBloodFlower::Touch(CActor&, CStateManager&) {}
void CBloodFlower::LaunchPollenProjectile(const CTransform4f& xf, CStateManager& mgr, float height,
                                          int maxProjectiles) {
  static float sProjectileTickPeriod = CProjectileWeapon::GetTickPeriod();
  CProjectileInfo* projInfo = ProjectileInfo();

  if (!projInfo->Token().TryCache()) {
    return;
  }
  if (!mgr.CanCreateProjectile(GetUniqueId(), kWT_AI, maxProjectiles)) {
    return;
  }

  const CVector3f aimPos = mgr.GetPlayer()->GetAimPosition(mgr, 0.f);
  const CVector3f& xfTranslation = xf.GetTranslation();
  const float zDiff = xfTranslation.GetZ() - aimPos.GetZ();
  const float rise = zDiff > 0.f ? height : -zDiff + height;
  if (zDiff > 0.f) {
    height = zDiff + height;
  }
  const float riseTime = CMath::SqrtF(2.f * rise / 4.905f);
  const float flightTime = riseTime + CMath::SqrtF(2.f * height / 4.905f);
  const float invTime = 1.f / flightTime;
  const CVector3f& position = xf.GetTranslation();
  const CVector3f velocity(invTime * (aimPos.GetX() - position.GetX()),
                           invTime * (aimPos.GetY() - position.GetY()),
                           2.4525f * flightTime + (-zDiff / flightTime));
  const CTransform4f projXf = CTransform4f::Translate(position);

  CTargetableProjectile* proj = CreateArcProjectile(
      mgr, ProjectileInfo()->Token(), projXf, ProjectileInfo()->GetDamage(), kInvalidUniqueId);

  if (!proj) {
    return;
  }
  CProjectileWeapon& wp = proj->ProjectileWeapon();
  wp.SetVelocity(sProjectileTickPeriod * velocity);
  wp.SetGravity(sProjectileTickPeriod * CVector3f(0.f, 0.f, -4.905f));
  mgr.AddObject(*proj);
}
