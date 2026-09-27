#include "MetroidPrime/Enemies/CEnergyBall.hpp"

#include "Kyoto/Audio/CSfxManager.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/ScriptObjects/CHUDBillboardEffect.hpp"
#include "MetroidPrime/ScriptObjects/CSustainedPlayerDamage.hpp"

static EMaterialTypes sSolidMaterial = kMT_Solid;
static EMaterialTypes sPlayerMaterial = kMT_Player;

CEnergyBall::CEnergyBall(const TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                         const CTransform4f& xf, const CModelData& mData,
                         const CActorParameters& actParms, const CPatternedInfo& pInfo,
                         const int w1, const float f1, const CDamageInfo& dInfo1, const float f2,
                         const CAssetId a1, const ushort sfxId1, const CAssetId a2,
                         const CAssetId a3, const ushort sfxId2, const float f3, const float f4,
                         const CAssetId a4, const CDamageInfo& dInfo2, const float f5)
: CPatterned(kC_EnergyBall, uid, name, kFT_Zero, info, xf, mData, pInfo, kMT_Flyer, kCT_One,
             kBT_NewFlyer, actParms, kCS_Medium)
, x56c(0.f)
, mBallType(w1)
, x574(f1)
, x578(dInfo1)
, mInitialTurnSpeed(pInfo.GetTurnSpeed())
, x598(f2)
, x59c(a1)
, x5a0(sfxId1)
, x5a4(a2)
, x5a8(gpSimplePool->GetObj(SObjectTag('ELSC', a3)))
, x5b4(sfxId2)
, x5b8(f3)
, x5bc(f4)
, x5c0(gpSimplePool->GetObj(SObjectTag('PART', a4)))
, x5cc(dInfo2)
, x5e8(f5) {
  SetDrawShadow(false);
  KnockBackCtrl().SetEnableExplodeDeath(false);
  KnockBackCtrl().SetAutoResetImpulse(false);
  KnockBackCtrl().SetEnableBurnDeath(false);
  KnockBackCtrl().SetX82_24(false);
  KnockBackCtrl().SetEnableBurn(false);
  KnockBackCtrl().SetEnableLaggedBurnDeath(false);
  KnockBackCtrl().SetEnableShock(false);
  KnockBackCtrl().SetEnableFreeze(false);
  KnockBackCtrl().SetX81_31(false);
}

CEnergyBall::~CEnergyBall() {}

ENTITY_ACCEPT_IMPL(CEnergyBall)

void CEnergyBall::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  if (msg == kSM_Registered) {
    SetMaterialFilter(CMaterialFilter::MakeInclude(CMaterialList(sPlayerMaterial)));
    RemoveMaterial(kMT_Solid, mgr);
  }
  CPatterned::AcceptScriptMsg(msg, uid, mgr);
}

void CEnergyBall::Generate(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimState = kAS_Ready;
    // Fall through.
  case kStateMsg_Update:
    TryGenerateDeactivate(mgr, 0);
    if (!BodyCtrl()->GetIsActive()) {
      BodyCtrl()->Activate(mgr);
    }
    break;
  case kStateMsg_Deactivate:
    mAnimState = kAS_NotReady;
    break;
  }
}

void CEnergyBall::Attack(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    break;
  case kStateMsg_Update: {
    const CVector3f eyePos = mgr.GetPlayer()->GetEyePosition();
    const CVector3f seek = mSteeringBehaviors.Seek(*this, eyePos);
    BodyCtrl()->FaceDirection3D(seek, GetTransform().GetForward(), arg);
    break;
  }
  case kStateMsg_Deactivate:
    break;
  }
}

void CEnergyBall::Think(float dt, CStateManager& mgr) {
  const float turnRatio = (x56c - 2.5f) / 8.f;
  const float turnSpeed = mInitialTurnSpeed * rstl::max_val(0.f, rstl::min_val(1.f, turnRatio));
  mTurnSpeed = turnSpeed;
  BodyCtrl()->SetTurnSpeed(turnSpeed);
  CPatterned::Think(dt, mgr);
  const float damageRatio = mDamageCooldownTimer / skDamageHitTime;
  ModelData()->AnimationData()->GetParticleDB().SetModulationColorAllActiveEffects(CColor::Lerp(
      CColor::White(), CColor::Red(), rstl::max_val(0.f, rstl::min_val(1.f, damageRatio))));

  bool shouldDetonate = false;
  if (GetActive() && IsAlive()) {
    x56c += dt;
    if (x56c > x574) {
      shouldDetonate = true;
    }
    if (InMaxRange(mgr, dt)) {
      shouldDetonate = true;
    }
  }
  if (shouldDetonate) {
    Detonate(mgr);
  }
}

void CEnergyBall::Detonate(CStateManager& mgr) {
  const CVector3f& delta = mgr.GetPlayer()->GetTranslation() - GetTranslation();
  if (delta.MagSquared() <= x578.GetRadius() * x578.GetRadius()) {
    bool breakFrozen = true;
    switch (mBallType) {
    case 0: {
      const float duration = x598;
      mIsMakingBigStrike = duration > 0.f;
      mDamageDur = duration;
      break;
    }
    case 1: {
      const ushort freezeSfx = x5a0;
      mgr.Player()->SetFrozenState(mgr, x59c, freezeSfx, x5a4);
      breakFrozen = false;
      break;
    }
    case 2:
      CreateVisorEffect(rstl::optional_object< TToken< CGenDescription > >(), x5a8,
                        rstl::string_l("PlasmaElectricFx"), mgr);
      CSfxManager::SfxStart(x5b4, 127, 64, false, CSfxManager::kMedPriority, false,
                            CSfxManager::kAllAreas);
      mgr.PlayerState()->StaticInterference().AddSource(GetUniqueId(), x5b8, x5bc);
      break;
    case 3:
      CreateVisorEffect(x5c0, rstl::optional_object< TToken< CElectricDescription > >(),
                        rstl::string_l("PlasmaVisorFx"), mgr);
      mgr.AddObject(rs_new CSustainedPlayerDamage(
          mgr.AllocateUniqueId(),
          CEntityInfo(GetCurrentAreaId(), CEntity::NullConnectionList, kInvalidEditorId), true,
          rstl::string_l("SusDamage"), x5cc, x5e8));
      break;
    }
    if (breakFrozen && mgr.GetPlayer()->GetFrozenState()) {
      mgr.Player()->BreakFrozenState(mgr);
    }
  }
  mgr.ApplyDamageToWorld(
      GetUniqueId(), *this, GetTranslation(), x578,
      CMaterialFilter::MakeIncludeExclude(CMaterialList(sSolidMaterial), CMaterialList()));
  MassiveDeath(mgr);
}

void CEnergyBall::Death(CStateManager& mgr, const CVector3f& direction, EScriptObjectState state) {
  CHealthInfo* health = HealthInfo(mgr);
  if (health && health->GetHP() > 0.f) {
    CPatterned::Death(mgr, direction, kSS_Any);
  } else {
    CPatterned::Death(mgr, direction, kSS_DeathRattle);
  }
}

void CEnergyBall::CreateVisorEffect(
    const rstl::optional_object< TToken< CGenDescription > >& particle,
    const rstl::optional_object< TToken< CElectricDescription > >& electric,
    const rstl::string& name, CStateManager& mgr) {
  mgr.AddObject(rs_new CHUDBillboardEffect(
      particle, electric, mgr.AllocateUniqueId(), true, name,
      CHUDBillboardEffect::GetNearClipDistance(mgr), CHUDBillboardEffect::GetScaleForPOV(mgr),
      CColor(1.f, 1.f, 1.f, 1.f), CVector3f(1.f, 1.f, 1.f), CVector3f(0.f, 0.f, 0.f)));
}
