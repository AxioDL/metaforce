#include "MetroidPrime/Enemies/CIceSheegoth.hpp"
#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/CCollisionActorManager.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Cameras/CCameraShakeData.hpp"
#include "MetroidPrime/Cameras/CFirstPersonCamera.hpp"
#include "MetroidPrime/Factories/CScannableObjectInfo.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CFlameThrower.hpp"
#include "MetroidPrime/Weapons/CGameProjectile.hpp"
#include "MetroidPrime/Weapons/CWeapon.hpp"
#include "rstl/algorithm.hpp"

#include "Kyoto/Animation/CInt32POINode.hpp"
#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Math/CFrustumPlanes.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CVector2f.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CParticleElectric.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/BodyState/CBodyState.hpp"
#include "rstl/math.hpp"

const int CIceSheegothData::skNumProperties = 37;

const CVector3f CIceSheegoth::skChargingBounds(2.f, 2.f, 2.f);

const CIceSheegoth::SJointInfo CIceSheegoth::skLeftLegJointList[] = {
    {"L_hip", "L_knee", 0.4f, 0.75f},
    {"L_ankle", "L_Toe_3", 0.4f, 0.75f},
};

const CIceSheegoth::SJointInfo CIceSheegoth::skRightLegJointList[] = {
    {"R_hip", "R_knee", 0.4f, 0.75f},
    {"R_ankle", "R_Toe_3", 0.4f, 0.75f},
};

const CIceSheegoth::SSphereJointInfo CIceSheegoth::skSphereJointList[] = {
    {"Jaw_end_LCTR", 0.55f},   {"Pelvis", 1.1f},     {"LCTR_SHEMOUTH", 1.1f}, {"butt_LCTR", 0.7f},
    {"Ice_Shards_LCTR", 1.2f}, {"GillL_LCTR", 0.6f}, {"GillR_LCTR", 0.6f},
};

const char* const CIceSheegoth::skpIceShardsLCTR = "Ice_Shards_LCTR";
const char* const CIceSheegoth::skpMouthDamageJoint = "LCTR_SHEMOUTH";
const char* const CIceSheegoth::skpJawJoint = "Jaw_end_LCTR";
const char* const CIceSheegoth::skpLeftGillJoint = "GillL_LCTR";
const char* const CIceSheegoth::skpRightGillJoint = "GillR_LCTR";

CIceSheegothData::CIceSheegothData(CInputStream& in, int propCount)
: x0_(CRelAngle::FromDegrees(in.Get< float >()).AsRadians())
, x4_(CRelAngle::FromDegrees(in.Get< float >()).AsRadians())
, x8_(in.Get< CVector3f >())
, x14_(in.Get< float >())
, x18_(in)
, x80_(in)
, xe8_(in)
, x150_(in.Get< CAssetId >())
, x154_(in)
, x170_(in.Get< float >())
, x174_(in.Get< float >())
, x178_(in.Get< CAssetId >())
, x17c_fireBreathResId(in.Get< CAssetId >())
, x180_fireBreathDamage(in)
, x19c_(in.Get< CAssetId >())
, x1a0_(in.Get< CAssetId >())
, x1a4_(in.Get< CAssetId >())
, x1a8_(in.Get< CAssetId >())
, x1ac_(in.Get< CAssetId >())
, x1b0_(in.Get< float >())
, x1b4_(in.Get< float >())
, x1b8_(in)
, x1d4_(CSfxManager::TranslateSFXID(in.Get< uint >()))
, x1d8_(in.Get< float >())
, x1dc_(in.Get< float >())
, x1e0_maxInterestTime(in.Get< float >())
, x1e4_(in.Get< CAssetId >())
, x1e8_(CSfxManager::TranslateSFXID(in.Get< uint >()))
, x1ec_(in.Get< CAssetId >())
, x1f0_24_(in.Get< bool >())
, x1f0_25_(in.Get< bool >()) {}

CIceSheegoth::CIceSheegoth(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                           const CTransform4f& xf, const CModelData& mData,
                           const CPatternedInfo& pInfo, const CActorParameters& actParms,
                           const CIceSheegothData& sheegothData)
: CPatterned(kC_IceSheegoth, uid, name, kFT_Zero, info, xf, mData, pInfo, kMT_Ground, kCT_One,
             kBT_BiPedal, actParms, kCS_Large)
, x568_state(-1)
, x56c_data(sheegothData)
, x760_pathSearch(nullptr, 1, pInfo.GetPathfindingIndex(), 1.f, 1.f)
, x844_approachSearch(nullptr, 1, pInfo.GetPathfindingIndex(), 1.f, 1.f)
, x928_pathFindMode(kPFM_Normal)
, x92c_lastDest(CVector3f::Zero())
, x938_(CVector3f::Zero())
, x944_(1.f)
, x948_(1.f)
, x94c_(x3b4_speed)
, x950_(0.f)
, x954_attackTimeLeft(0.f)
, x958_(0.f)
, x95c_(0.f)
, x960_(0.f)
//, x964_(0.f)
, x968_interestTimer(0.f)
, x96c_(2.f)
, x970_maxHp(0.f)
, x974_(sheegothData.GetX174())
, x978_(0.f)
, x97c_(0.f)
, x980_(CVector3f::Zero())
, x98c_mouthVulnerability(pInfo.GetDamageVulnerability())
, x9f4_boneTracking(*GetAnimationData(), rstl::string_l("Head_1"), CMath::Deg2Rad(80.f),
                    CMath::Deg2Rad(180.f), kBTF_None)
, xa30_(GetBoundingBox(), GetMaterialList())
, xa58_projectileInfo(sheegothData.GetX150(), sheegothData.GetX154())
, xa80_flameThrowerId(kInvalidUniqueId)
, xa84_(sheegothData.GetX178() != kInvalidAssetId
            ? gpSimplePool->GetObj(SObjectTag('WPSC', sheegothData.GetX178()))
            : gpSimplePool->GetObj("FlameThrower"))
, xa8c_(gpSimplePool->GetObj(SObjectTag('PART', sheegothData.GetX1a0())))
, xa98_(rs_new CElementGen(xa8c_))
, xaa0_(gpSimplePool->GetObj(SObjectTag('PART', sheegothData.GetX1a4())))
, xaac_(rs_new CElementGen(xaa0_))
, xab4_(gpSimplePool->GetObj(SObjectTag('PART', sheegothData.GetX1a8())))
, xabc_(rs_new CElementGen(xab4_))
, xac8_(gpSimplePool->GetObj(SObjectTag('ELSC', sheegothData.GetX1ac())))
, xad4_(rs_new CParticleElectric(xac8_))
, xadc_(gpSimplePool->GetObj(SObjectTag('PART', sheegothData.GetX19c())))
, xaf4_mouthLocator(0xFF)
, xaf6_iceShardsCollider(kInvalidUniqueId)
, xaf8_mouthCollider(kInvalidUniqueId)
, xb28_24_shotAt(false)
, xb28_25_(false)
, xb28_26_(false)
, xb28_27_(false)
, xb28_28_(false)
, xb28_29_(false)
, xb28_30_(false)
, xb28_31_(false)
, xb29_24_(false)
, xb29_25_(false)
, xb29_26_(false)
, xb29_27_(false)
, xb29_28_(false)
, xb29_29_scanned(false) {
  xa58_projectileInfo.Token().Lock();
  UpdateTouchBounds();
  KnockBackCtrl().SetEnableFreeze(false);
  KnockBackCtrl().SetX82_24(false);
  KnockBackCtrl().SetEnableLaggedBurnDeath(false);
  KnockBackCtrl().SetEnableExplodeDeath(false);
  const CPASAnimParmData parms(pas::kAS_Step, CPASAnimParm::FromEnum(1), CPASAnimParm::FromEnum(0));
  x950_ = GetAnimationDistance(parms) * GetModelScale().GetY();
  xa98_->SetGlobalScale(GetModelScale());
  xaac_->SetGlobalScale(GetModelScale());
  xabc_->SetGlobalScale(GetModelScale());
  xad4_->SetGlobalScale(GetModelScale());
  BodyCtrl()->BodyStateInfo().SetLocoAnimChangeAtEndOfAnimOnly(true);
  MakeThermalColdAndHot();
  x328_31_energyAttractor = true;
}

CIceSheegoth::~CIceSheegoth() {}

ENTITY_ACCEPT_IMPL(CIceSheegoth)

void CIceSheegoth::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  CPatterned::Think(dt, mgr);
  AttractProjectiles(mgr);
  UpdateAILogicTimers(dt);
  UpdateAimTarget(mgr);

  if (!IsAlive()) {
    x974_ = rstl::max_val(0.f, x974_ - (dt * x56c_data.GetX170()));
    if (GetBodyCtrl()->GetBodyStateInfo().GetCurrentState()->IsDying()) {
      SetShootThrough(mgr);
    }
  }

  x96c_ -= dt;

  if (x96c_ < 0.f) {
    BodyCtrl()->CommandMgr().DeliverCmd(CBCAdditiveReactionCmd(pas::kART_Four, 1.f, false));
    x96c_ = 3.f * mgr.Random()->Float() + 2.f;
  }

  ModelData()->AnimationData()->PreRender();
  UpdateHeadTracking(dt, mgr);
  xa2c_collisionActorManager->Update(dt, mgr, CCollisionActorManager::kUO_ObjectSpace);
  PreventPlayerPenetration(mgr, dt);
  UpdateHealthInfo(mgr);
  UpdateSteeringBlendSpeed(dt);
  UpdateParticleEffects(dt, mgr);
  UpdateThermalFrozenState(x428_damageCooldownTimer > 0.f);
}

void CIceSheegoth::AcceptScriptMsg(EScriptObjectMessage msg, const TUniqueId uid,
                                   CStateManager& mgr) {
  switch (msg) {
  case kSM_Registered: {
    if (!HasPatrolPath(mgr, 0.f)) {
      BodyCtrl()->SetLocomotionType(pas::kLT_Crouch);
    }
    BodyCtrl()->Activate(mgr);
    SetupCollisionManager(mgr);
    CreateFlameThrower(mgr);
    const float maxSpeed = BodyCtrl()->GetBodyStateInfo().GetMaxSpeed();
    if (maxSpeed > 0.f) {
      x944_ = x948_ =
          (0.9f * BodyCtrl()->GetBodyStateInfo().GetLocomotionSpeed(pas::kLA_Walk)) / maxSpeed;
    }
    BodyCtrl()->CommandMgr().SetSteeringBlendMode(kSBM_FullSpeed);
    xaf4_mouthLocator = GetAnimationData()->GetLocatorSegId(rstl::string_l(skpMouthDamageJoint));
    break;
  }
  case kSM_Activate:
    xa2c_collisionActorManager->SetActive(mgr, true);
    break;
  case kSM_Deactivate:
    xa2c_collisionActorManager->SetActive(mgr, false);
    break;
  case kSM_Deleted:
    xa2c_collisionActorManager->Destroy(mgr);
    if (xa80_flameThrowerId != kInvalidUniqueId) {
      mgr.DeleteObjectRequest(xa80_flameThrowerId);
      xa80_flameThrowerId = kInvalidUniqueId;
    }
    if (xaf0_crackleSfx) {
      CSfxManager::RemoveEmitter(xaf0_crackleSfx);
    }
    break;
  case kSM_Start:
    xb28_24_shotAt = true;
    break;
  case kSM_InitializedInArea: {
    const TAreaId areaId = GetCurrentAreaId();
    const CGameArea* area = mgr.GetWorld()->GetArea(areaId);
    x760_pathSearch.SetArea(area->GetPostConstructed()->x10bc_pathArea);
    x844_approachSearch.SetArea(area->GetPostConstructed()->x10bc_pathArea);
    break;
  }
  case kSM_Touched:
    ApplyContactDamage(uid, mgr);
    if (const CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(uid))) {
      const TUniqueId touched = actor->GetLastTouchedObject();
      if (const CWeapon* weapon = TCastToConstPtr< CWeapon >(mgr.GetObjectById(touched))) {
        if (weapon->GetOwnerId() == mgr.GetPlayer()->GetUniqueId()) {
          xb28_24_shotAt = true;
        }
      }
    }
    break;
  case kSM_Damage:
    if (const CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(uid))) {
      const TUniqueId touched = actor->GetLastTouchedObject();
      if (const CWeapon* weapon = TCastToConstPtr< CWeapon >(mgr.GetObjectById(touched))) {
        if (uid == xaf6_iceShardsCollider && !xb28_27_) {
          AbsorbEnergy(weapon->GetCurrentDamageInfo().GetDamage(), mgr);
          if (!xae8_.get() || xae8_->IsSystemDeletable()) {
            xae8_ = rs_new CElementGen(xadc_);
          }
        } else {
          TakeDamage(CVector3f::Zero(), 0.f);
          if (IsGillCollisionActor(*actor)) {
            x97c_ = 0.2f;
            x980_ = weapon->GetTransform().GetForward();
          }
        }
      }
      xb28_24_shotAt = true;
      x968_interestTimer = 0.f;
      mgr.InformListeners(GetTranslation(), kLNT_PlayerFire);
    } else {
      ReDirectDamage(mgr, uid);
    }
    break;
  case kSM_InvulnDamage:
    if (uid == xaf6_iceShardsCollider && !xb28_27_) {
      if (const CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(uid))) {
        const TUniqueId touched = actor->GetLastTouchedObject();
        if (const CWeapon* weapon = TCastToConstPtr< CWeapon >(mgr.GetObjectById(touched))) {
          AbsorbEnergy(weapon->GetCurrentDamageInfo().GetDamage(), mgr);
          if (!xae8_.get() || xae8_->IsSystemDeletable()) {
            xae8_ = rs_new CElementGen(xadc_);
          }
        }
      }
    }
    mgr.InformListeners(GetTranslation(), kLNT_PlayerFire);
    xb28_24_shotAt = true;
    x968_interestTimer = 0.f;
    break;
  case kSM_SuspendedMove:
    if (xa2c_collisionActorManager.get()) {
      xa2c_collisionActorManager->SetMovable(mgr, false);
    }
    break;
  }
  CPatterned::AcceptScriptMsg(msg, uid, mgr);
}

void CIceSheegoth::Render(const CStateManager& mgr) const { CPatterned::Render(mgr); }

void CIceSheegoth::AddToRenderer(const CFrustumPlanes& frustum, const CStateManager& mgr) const {
  CPatterned::AddToRenderer(frustum, mgr);
  switch (mgr.GetThermalDrawFlag()) {
  case kTD_Hot:
    if (x428_damageCooldownTimer == 0.f && xb29_25_ && HasModelData()) {
      GetModelData()->RenderParticles(frustum);
    }
    break;
  case kTD_Cold:
  case kTD_Bypass: {
    const rstl::optional_object< CAABox > frost =
        xae8_.get() ? xae8_->GetBounds() : rstl::optional_object< CAABox >();
    const rstl::optional_object< CAABox > low =
        xa98_.get() ? xa98_->GetBounds() : rstl::optional_object< CAABox >();
    const rstl::optional_object< CAABox > medium =
        xaac_.get() ? xaac_->GetBounds() : rstl::optional_object< CAABox >();
    const rstl::optional_object< CAABox > high =
        xabc_.get() ? xabc_->GetBounds() : rstl::optional_object< CAABox >();
    const rstl::optional_object< CAABox > electric =
        xad4_.get() ? xad4_->GetBounds() : rstl::optional_object< CAABox >();
    const rstl::optional_object< CAABox > electricExtra =
        xad4_.get() ? xad4_->GetBounds() : rstl::optional_object< CAABox >();
    CAABox bounds = CAABox::MakeMaxInvertedBox();
    if (frost.valid()) {
      bounds.Include(*frost);
    }
    if (electricExtra.valid()) {
      bounds.Include(*electricExtra);
    }
    if (low.valid()) {
      bounds.Include(*low);
    }
    if (medium.valid()) {
      bounds.Include(*medium);
    }
    if (high.valid()) {
      bounds.Include(*high);
    }
    if (electric.valid()) {
      bounds.Include(*electric);
    }
    if (frustum.BoxInFrustumPlanes(bounds)) {
      if (xae8_.get()) {
        gpRender->AddParticleGen(*xae8_);
      }
      gpRender->AddParticleGen(*xa98_);
      gpRender->AddParticleGen(*xaac_);
      gpRender->AddParticleGen(*xabc_);
      gpRender->AddParticleGen(*xad4_);
    }
    break;
  }
  }
}

void CIceSheegoth::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node,
                                   EUserEventType type, float dt) {
  bool handled = false;
  switch (type) {
  case kUE_Projectile: {
    const CTransform4f xf = GetLctrTransform(node.GetLocatorName());
    const CVector3f aimPos = mgr.GetPlayer()->GetAimPosition(mgr, 0.f);
    CVector3f targetPos = ProjectileInfo()->PredictInterceptPos(xf.GetTranslation(), aimPos,
                                                                *mgr.GetPlayer(), true, dt);
    const CVector3f forward = GetTransform().GetForward();
    const CVector3f delta = targetPos - xf.GetTranslation();
    if (CVector3f::GetAngleDiff(forward, delta) > CMath::Deg2Rad(60.f)) {
      if (delta.CanBeNormalized()) {
        const CVector3f direction =
            CVector3f::Slerp(forward, delta.AsNormalized(), CRelAngle::FromDegrees(60.f));
        targetPos = xf.GetTranslation() + direction * delta.Magnitude();
      } else {
        targetPos = xf.GetTranslation() + xf.GetForward() * delta.Magnitude();
      }
    }
    const CTransform4f launchXf =
        CTransform4f::LookAt(xf.GetTranslation(), targetPos, CVector3f::Up());
    LaunchProjectile(launchXf, mgr, 4, CWeapon::kPA_None, false,
                     rstl::optional_object< TLockedToken< CGenDescription > >(),
                     CSfxManager::kInternalInvalidSfxId, false, CVector3f(1.f, 1.f, 1.f));
    const float consumed = (1.f / 3.f) * x56c_data.GetX170();
    x974_ = rstl::max_val(0.f, x974_ - consumed);
    if (xb28_27_ && !ShouldSpecialAttack(mgr, 0.f)) {
      BodyCtrl()->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_ExitState));
    }
    handled = true;
    break;
  }
  case kUE_ScreenShake:
    ProcessStompGround(mgr);
    handled = true;
    break;
  case kUE_BecomeShootThrough:
    SetShootThrough(mgr);
    handled = true;
    break;
  case kUE_DamageOn:
    if (xb28_26_) {
      if (CFlameThrower* flame =
              static_cast< CFlameThrower* >(mgr.ObjectById(xa80_flameThrowerId))) {
        flame->Fire(GetTransform(), mgr, false);
      }
    }
    break;
  case kUE_DamageOff:
    if (xb28_26_) {
      if (CFlameThrower* flame =
              static_cast< CFlameThrower* >(mgr.ObjectById(xa80_flameThrowerId))) {
        flame->Reset(mgr, false);
      }
    }
    break;
  }
  if (!handled) {
    CPatterned::DoUserAnimEvent(mgr, node, type, dt);
  }
}

CVector3f CIceSheegoth::GetAimPosition(const CStateManager& mgr, float dt) const {
  if (GetBodyCtrl()->GetLocomotionType() != pas::kLT_Crouch) {
    if (mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_Thermal) {
      CVector3f pos = CVector3f::Zero();
      for (AUTO(it, xafc_gillColliders.begin()); it != xafc_gillColliders.end(); ++it) {
        if (const CCollisionActor* actor =
                TCastToConstPtr< CCollisionActor >(mgr.GetObjectById(*it))) {
          pos += actor->GetTranslation();
        }
      }
      if (pos.IsNonZero()) {
        pos *= 1.f / static_cast< float >(xafc_gillColliders.size());
        return pos;
      }
    } else if (xb29_29_scanned ||
               mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_Scan) {
      if (const CCollisionActor* actor =
              TCastToConstPtr< CCollisionActor >(mgr.GetObjectById(xaf8_mouthCollider))) {
        return actor->GetTranslation();
      }
    } else if (const CCollisionActor* actor =
                   TCastToConstPtr< CCollisionActor >(mgr.GetObjectById(xaf6_iceShardsCollider))) {
      return actor->GetTranslation();
    }
  }
  return CPatterned::GetAimPosition(mgr, dt);
}

const CDamageVulnerability* CIceSheegoth::GetDamageVulnerability() const {
  return &CDamageVulnerability::PassThroughVulnerability();
}

const CDamageVulnerability* CIceSheegoth::GetDamageVulnerability(const CVector3f&, const CVector3f&,
                                                                 const CDamageInfo&) const {
  return &CDamageVulnerability::PassThroughVulnerability();
}

bool CIceSheegoth::InMaxRange(CStateManager& mgr, float arg) {
  if (x56c_data.GetX1f0_24()) {
    return true;
  }
  return CPatterned::InMaxRange(mgr, arg);
}

bool CIceSheegoth::ShotAt(CStateManager&, float) { return GetWasHit(); }

bool CIceSheegoth::TooClose(CStateManager& mgr, float) {
  if (mgr.GetPlayer()->GetMorphballTransitionState() == CPlayer::kMS_Morphed) {
    const CVector3f delta = mgr.GetPlayer()->GetTranslation() - GetTranslation();
    return delta.GetZ() < 0.f && delta.MagSquared() < x978_;
  }
  return false;
}

bool CIceSheegoth::OffLine(CStateManager& mgr, float) {
  SetPathFindMode(kPFM_Normal);
  return PathShagged(mgr, 0.f);
}

bool CIceSheegoth::LostInterest(CStateManager&, float) {
  return x968_interestTimer >= x56c_data.GetMaxInterestTime();
}

bool CIceSheegoth::Leash(CStateManager& mgr, float) {
  const CVector3f delta = x3a0_latestLeashPosition - GetTranslation();
  if (delta.MagSquared() > x3c8_leashRadius * x3c8_leashRadius) {
    const CVector3f playerDelta = mgr.GetPlayer()->GetTranslation() - GetTranslation();
    return playerDelta.MagSquared() > x3cc_playerLeashRadius * x3cc_playerLeashRadius &&
           x3d4_curPlayerLeashTime > x3d0_playerLeashTime;
  }
  return false;
}

bool CIceSheegoth::ShouldAttack(CStateManager& mgr, float) {
  const CPlayer& player = *mgr.GetPlayer();
  if (GetCurrentAreaId() == player.GetCurrentAreaId() && !x56c_data.GetX1f0_24() &&
      !player.GetFrozenState()) {
    const CVector3f aim = player.GetAimPosition(mgr, 0.f);
    const CVector3f delta = aim - GetTranslation();
    const float distanceSquared = delta.MagSquared();
    if ((x954_attackTimeLeft <= 0.f && x960_ <= 0.f) ||
        distanceSquared <= x2fc_minAttackRange * x2fc_minAttackRange) {
      const float range = BodyCtrl()->GetBodyStateInfo().GetMaxSpeed() * x56c_data.GetX1d8();
      if (range * range > distanceSquared &&
          !ShouldTurn(mgr, CRelAngle::FromDegrees(15.f).AsRadians())) {
        const CTransform4f mouthXf = GetLctrTransform(xaf4_mouthLocator);
        const CVector3f mouth = mouthXf.GetTranslation();
        return !IsPatternObstructed(mgr, mouth, aim);
      }
    }
  }
  return false;
}

bool CIceSheegoth::ShouldSpecialAttack(CStateManager& mgr, float) {
  if (GetCurrentAreaId() == mgr.GetPlayer()->GetCurrentAreaId() && x954_attackTimeLeft <= 0.f &&
      x974_ >= (1.f / 3.f) * x56c_data.GetX170() && AllowSpecialAttackByChance()) {
    const CVector3f aim = mgr.GetPlayer()->GetAimPosition(mgr, 0.f);
    const CTransform4f mouthXf = GetLctrTransform(xaf4_mouthLocator);
    const CVector3f mouth = mouthXf.GetTranslation();
    const CVector3f delta = aim - mouth;
    if (delta.MagSquared() >= x2fc_minAttackRange * x2fc_minAttackRange &&
        !ShouldTurn(mgr, CRelAngle::FromDegrees(15.f).AsRadians()) &&
        !IsPatternObstructed(mgr, mouth, aim)) {
      return true;
    }
  }
  return false;
}

bool CIceSheegoth::ShouldDoubleSnap(CStateManager& mgr, float) {
  if (GetCurrentAreaId() == mgr.GetPlayer()->GetCurrentAreaId() && x958_ <= 0.f) {
    SetPathFindMode(kPFM_Approach);
    const CVector3f delta = x92c_lastDest - GetTranslation();
    return PathShagged(mgr, 0.f) ||
           x844_approachSearch.GetCurrentWaypoint() >=
               x844_approachSearch.GetWaypoints().size() - 1 ||
           delta.MagSquared() < 81.f;
  }
  return false;
}

bool CIceSheegoth::ShouldFire(CStateManager& mgr, float) {
  if (GetCurrentAreaId() == mgr.GetPlayer()->GetCurrentAreaId() && x954_attackTimeLeft <= 0.f) {
    const CVector3f pos = mgr.GetPlayer()->GetTranslation();
    const CTransform4f mouthXf = GetLctrTransform(xaf4_mouthLocator);
    const CVector3f mouth = mouthXf.GetTranslation();
    const CVector3f delta = pos - mouth;
    const CVector3f& translation = GetTranslation();
    const float playerHeight = pos.GetZ() - translation.GetZ();
    const float mouthHeight = mouth.GetZ() - translation.GetZ();
    if (delta.MagSquared() <= x300_maxAttackRange * x300_maxAttackRange &&
        CMath::AbsF(playerHeight) < mouthHeight &&
        !ShouldTurn(mgr, CRelAngle::FromDegrees(15.f).AsRadians())) {
      return !IsPatternObstructed(mgr, mouth, pos);
    }
  }
  return false;
}

bool CIceSheegoth::ShouldTurn(CStateManager& mgr, float arg) {
  const float angle = arg == 0.f ? CRelAngle::FromDegrees(45.f).AsRadians() : arg;
  const CVector3f aim = mgr.GetPlayer()->GetAimPosition(mgr, 0.f);
  const CVector3f& delta = aim - GetTranslation();
  const CVector2f target = delta.ToVec2f();
  const CVector2f forward = GetTransform().GetForward().ToVec2f();
  return CVector2f::GetAngleDiff(forward, target) > angle;
}

bool CIceSheegoth::ShouldFlinch(CStateManager&, float) { return xb29_25_ && x97c_ > 0.f; }

bool CIceSheegoth::AggressionCheck(CStateManager& mgr, float) {
  if (IsAlive() && !xb28_30_ && IsEnraged(mgr)) {
    return true;
  }
  return false;
}

bool CIceSheegoth::InPosition(CStateManager&, float) {
  const CVector3f delta = x92c_lastDest - GetTranslation();
  return delta.MagSquared() < 81.f;
}

bool CIceSheegoth::InDetectionRange(CStateManager& mgr, float arg) {
  const bool spotted = xb28_24_shotAt;
  if (!spotted) {
    const CVector3f delta = mgr.GetPlayer()->GetTranslation() - GetTranslation();
    const float factor = arg > 0.f ? arg : 1.f;
    const float range = factor * x3bc_detectionRange;
    if (delta.MagSquared() < range * range) {
      if (x3c0_detectionHeightRange > 0.f) {
        return delta.GetZ() * delta.GetZ() < x3c0_detectionHeightRange * x3c0_detectionHeightRange;
      }
      return true;
    }
  }
  return spotted;
}

bool CIceSheegoth::SpotPlayer(CStateManager& mgr, float arg) {
  const bool spotted = xb28_24_shotAt;
  return spotted ? spotted : CPatterned::SpotPlayer(mgr, arg);
}

bool CIceSheegoth::AnimOver(CStateManager&, float) { return x568_state == 4; }

void CIceSheegoth::Patrol(CStateManager& mgr, EStateMsg msg, float arg) {
  CPatterned::Patrol(mgr, msg, arg);
}

void CIceSheegoth::ProjectileAttack(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    x568_state = 0;
    xb28_26_ = true;
    xb29_25_ = true;
    xb29_27_ = false;
    x968_interestTimer = 0.f;
    EnableGillDamage(mgr, true);
    break;
  case kStateMsg_Update: {
    const CVector3f target = mgr.GetPlayer()->GetTranslation();
    switch (x568_state) {
    case 0:
      if (BodyCtrl()->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_ProjectileAttack) {
        x568_state = 3;
        x3b4_speed = 2.f * x94c_;
      } else {
        BodyCtrl()->CommandMgr().DeliverCmd(CBCProjectileAttackCmd(pas::kS_Two, target, false));
      }
      break;
    case 3:
      if (BodyCtrl()->GetBodyStateInfo().GetCurrentStateId() != pas::kAS_ProjectileAttack) {
        x568_state = 4;
      }
      break;
    }
    break;
  }
  case kStateMsg_Deactivate:
    x3b4_speed = x94c_;
    if (!IsEnraged(mgr)) {
      x954_attackTimeLeft =
          x308_attackTimeVariation * mgr.Random()->Float() + x304_averageAttackTime;
    }
    if (CFlameThrower* flame = static_cast< CFlameThrower* >(mgr.ObjectById(xa80_flameThrowerId))) {
      flame->Reset(mgr, false);
    }
    EnableGillDamage(mgr, false);
    if (BodyCtrl()->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_ProjectileAttack) {
      BodyCtrl()->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_NextState));
    }
    xb29_25_ = false;
    xb28_26_ = false;
    break;
  }
}

void CIceSheegoth::Attack(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    x568_state = 0;
    xb28_29_ = xb28_25_ = true;
    x2e0_destPos = mgr.GetPlayer()->GetAimPosition(mgr, 0.f);
    SetPathFindMode(kPFM_Normal);
    CPatterned::PathFind(mgr, msg, dt);
    ExtendTouchBounds(mgr, skChargingBounds);
    x95c_ = 0.f;
    break;
  case kStateMsg_Update:
    switch (x568_state) {
    case 0:
      x95c_ += dt;
      if (x95c_ < x56c_data.GetX1d8()) {
        if (!xb28_29_) {
          x568_state = 3;
          xb28_29_ = true;
          const pas::ESeverity severity =
              mgr.GetPlayer()->GetMorphballTransitionState() == CPlayer::kMS_Morphed ? pas::kS_Two
                                                                                     : pas::kS_One;
          BodyCtrl()->CommandMgr().DeliverCmd(CBCMeleeAttackCmd(severity));
        } else {
          const CTransform4f mouthXf = GetLctrTransform(xaf4_mouthLocator);
          const CVector3f delta = mgr.GetPlayer()->GetTranslation() - mouthXf.GetTranslation();
          const CVector3f forward = GetTransform().GetForward();
          if (CVector3f::Dot(forward, delta) > 0.f) {
            SetPathFindMode(kPFM_Normal);
            if (GetSearchPath() != nullptr && !PathShagged(mgr, 0.f)) {
              CPatterned::PathFind(mgr, msg, dt);
            } else {
              x568_state = 4;
            }
          } else {
            x568_state = 4;
          }
        }
      } else {
        x568_state = 4;
      }
      break;
    case 3:
      if (BodyCtrl()->GetBodyStateInfo().GetCurrentStateId() != pas::kAS_MeleeAttack) {
        x568_state = 4;
      }
      break;
    }
    break;
  case kStateMsg_Deactivate:
    if (!IsEnraged(mgr)) {
      x954_attackTimeLeft =
          x308_attackTimeVariation * mgr.Random()->Float() + x304_averageAttackTime;
    }
    ExtendTouchBounds(mgr, CVector3f::Zero());
    xb28_25_ = xb28_29_ = false;
    x960_ = x56c_data.GetX1dc();
    x95c_ = 0.f;
    break;
  }
  x968_interestTimer = 0.f;
}

void CIceSheegoth::SpecialAttack(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    x568_state = 0;
    x968_interestTimer = 0.f;
    xb28_27_ = true;
    xb29_27_ = false;
    break;
  case kStateMsg_Update:
    switch (x568_state) {
    case 0:
      if (BodyCtrl()->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_LoopAttack) {
        x568_state = 3;
        x3b4_speed = 2.f * x94c_;
      } else {
        BodyCtrl()->CommandMgr().DeliverCmd(CBCLoopAttackCmd(pas::kLAT_Zero, true));
      }
      break;
    case 3:
      if (BodyCtrl()->GetBodyStateInfo().GetCurrentStateId() != pas::kAS_LoopAttack) {
        x568_state = 4;
      }
      break;
    }
    break;
  case kStateMsg_Deactivate:
    if (!IsEnraged(mgr)) {
      x954_attackTimeLeft =
          x308_attackTimeVariation * mgr.Random()->Float() + x304_averageAttackTime;
    }
    xb28_27_ = false;
    x3b4_speed = x94c_;
    break;
  }
}

void CIceSheegoth::DoubleSnap(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    x568_state = 0;
    xb28_28_ = xb28_29_ = true;
    break;
  case kStateMsg_Update:
    switch (x568_state) {
    case 0:
      if (BodyCtrl()->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_MeleeAttack) {
        x568_state = 3;
      } else if (IsOnGround()) {
        BodyCtrl()->CommandMgr().DeliverCmd(CBCMeleeAttackCmd(pas::kS_Zero));
      }
      break;
    case 3:
      if (BodyCtrl()->GetBodyStateInfo().GetCurrentStateId() != pas::kAS_MeleeAttack) {
        x568_state = 4;
      }
      break;
    }
    break;
  case kStateMsg_Deactivate:
    if (!IsEnraged(mgr) || !TooClose(mgr, 0.f)) {
      x958_ = x308_attackTimeVariation * mgr.Random()->Float() + x304_averageAttackTime;
    }
    xb28_28_ = xb28_29_ = false;
    break;
  }
}

void CIceSheegoth::PathFind(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    xb28_24_shotAt = false;
    xb29_28_ = false;
    x968_interestTimer = 0.f;
    BodyCtrl()->SetLocomotionType(pas::kLT_Relaxed);
    x9f4_boneTracking.SetTarget(mgr.GetPlayer()->GetUniqueId());
    x9f4_boneTracking.SetActive(true);
    UpdateAttackPosition(mgr, x2e0_destPos);
    SetPathFindMode(kPFM_Normal);
    if (!x56c_data.GetX1f0_24()) {
      CPatterned::PathFind(mgr, msg, dt);
    }
    break;
  case kStateMsg_Update: {
    SetPathFindMode(kPFM_Normal);
    if (!x56c_data.GetX1f0_24() && GetSearchPath() != nullptr && !PathShagged(mgr, 0.f) &&
        x760_pathSearch.GetCurrentWaypoint() < x760_pathSearch.GetWaypoints().size() - 1) {
      CPatterned::PathFind(mgr, msg, dt);
      x968_interestTimer = 0.f;
      const CVector3f forward = GetTransform().GetForward();
      const CVector3f move = BodyCtrl()->CommandMgr().GetMoveVector();
      if (CVector3f::Dot(forward, move) < 0.f && move.CanBeNormalized()) {
        BodyCtrl()->CommandMgr().ClearLocomotionCmds();
        BodyCtrl()->CommandMgr().DeliverCmd(
            CBCLocomotionCmd(CVector3f::Zero(), move.AsNormalized(), 1.f));
      }
    } else {
      const CVector3f delta = mgr.GetPlayer()->GetTranslation() - GetTranslation();
      if (PredictShouldTurn(mgr, CRelAngle::FromDegrees(15.f).AsRadians()) &&
          delta.CanBeNormalized()) {
        BodyCtrl()->CommandMgr().DeliverCmd(
            CBCLocomotionCmd(CVector3f::Zero(), delta.AsNormalized(), 1.f));
      }
    }
    const float speedScale =
        BodyCtrl()->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_Turn ? 2.f : 1.f;
    x3b4_speed = speedScale * x94c_;
    break;
  }
  case kStateMsg_Deactivate:
    x9f4_boneTracking.SetActive(false);
    x3b4_speed = x94c_;
    break;
  }
}

void CIceSheegoth::Approach(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyCtrl()->SetLocomotionType(pas::kLT_Relaxed);
    x9f4_boneTracking.SetTarget(mgr.GetPlayer()->GetUniqueId());
    x9f4_boneTracking.SetActive(true);
    UpdateAttackPosition(mgr, x2e0_destPos);
    SetPathFindMode(kPFM_Normal);
    CPatterned::PathFind(mgr, kStateMsg_Activate, dt);
    if (!xb29_27_) {
      BodyCtrl()->CommandMgr().ClearLocomotionCmds();
    }
    xb29_24_ = !x56c_data.GetX1f0_24();
    xb29_28_ = true;
    x92c_lastDest = mgr.GetPlayer()->GetTranslation();
    x92c_lastDest.SetZ(GetTranslation().GetZ());
    break;
  case kStateMsg_Update: {
    SetPathFindMode(kPFM_Approach);
    if (xb29_24_) {
      x2e0_destPos = x92c_lastDest;
      if (x844_approachSearch.FindClosestReachablePoint(GetTranslation(), x2e0_destPos) ==
          CPathFindSearch::kR_Success) {
        const CVector3f delta = x2e0_destPos - GetTranslation();
        if (delta.MagSquared() < 81.f) {
          x2e0_destPos = GetTranslation();
        }
        x92c_lastDest = x2e0_destPos;
        CPatterned::PathFind(mgr, kStateMsg_Activate, dt);
        if (!xb29_27_) {
          BodyCtrl()->CommandMgr().ClearLocomotionCmds();
        }
      }
      xb29_24_ = false;
    }
    if (xb29_27_ && GetSearchPath() != nullptr && !PathShagged(mgr, 0.f) &&
        x844_approachSearch.GetCurrentWaypoint() < x844_approachSearch.GetWaypoints().size() - 1) {
      CPatterned::PathFind(mgr, msg, dt);
    } else {
      const CVector3f delta = mgr.GetPlayer()->GetTranslation() - GetTranslation();
      if (PredictShouldTurn(mgr, CRelAngle::FromDegrees(15.f).AsRadians()) &&
          delta.CanBeNormalized()) {
        BodyCtrl()->CommandMgr().DeliverCmd(
            CBCLocomotionCmd(CVector3f::Zero(), delta.AsNormalized(), 1.f));
      }
    }
    xb29_27_ = true;
    const float speedScale =
        BodyCtrl()->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_Turn ? 2.f : 1.f;
    x3b4_speed = speedScale * x94c_;
    break;
  }
  case kStateMsg_Deactivate:
    x9f4_boneTracking.SetActive(false);
    SetPathFindMode(kPFM_Normal);
    x3b4_speed = x94c_;
    break;
  }
}

void CIceSheegoth::TargetPatrol(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate: {
    BodyCtrl()->SetLocomotionType(pas::kLT_Relaxed);
    SetWasHit(false);
    if (HasPatrolPath(mgr, 0.f)) {
      CPatterned::Patrol(mgr, msg, dt);
      UpdateDest(mgr);
    } else {
      SetDestPos(x3a0_latestLeashPosition);
    }
    x92c_lastDest = x2e0_destPos;
    const CVector3f arrival = x45c_steeringBehaviors.Arrival(*this, x92c_lastDest, 15.f);
    if (GetSearchPath() != nullptr) {
      SetPathFindMode(kPFM_Normal);
      CPatterned::PathFind(mgr, msg, dt);
      const CVector3f move = BodyCtrl()->CommandMgr().GetMoveVector();
      if (move.CanBeNormalized()) {
        BodyCtrl()->CommandMgr().ClearLocomotionCmds();
        const float speed = arrival.Magnitude();
        BodyCtrl()->CommandMgr().DeliverCmd(
            CBCLocomotionCmd(speed * move.AsNormalized(), CVector3f::Zero(), 1.f));
      }
    } else {
      BodyCtrl()->CommandMgr().DeliverCmd(CBCLocomotionCmd(arrival, CVector3f::Zero(), 1.f));
    }
    break;
  }
  case kStateMsg_Update: {
    const CVector3f arrival = x45c_steeringBehaviors.Arrival(*this, x92c_lastDest, 15.f);
    if (GetSearchPath() != nullptr && !PathShagged(mgr, 0.f)) {
      SetPathFindMode(kPFM_Normal);
      CPatterned::PathFind(mgr, msg, dt);
      const CVector3f move = BodyCtrl()->CommandMgr().GetMoveVector();
      if (move.CanBeNormalized()) {
        BodyCtrl()->CommandMgr().ClearLocomotionCmds();
        const float speed = arrival.Magnitude();
        BodyCtrl()->CommandMgr().DeliverCmd(
            CBCLocomotionCmd(speed * move.AsNormalized(), CVector3f::Zero(), 1.f));
      }
    } else {
      BodyCtrl()->CommandMgr().DeliverCmd(CBCLocomotionCmd(arrival, CVector3f::Zero(), 1.f));
    }
    break;
  }
  }
}

void CIceSheegoth::Generate(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    x568_state = 0;
    x938_ = GetTransform().GetForward();
    break;
  case kStateMsg_Update:
    switch (x568_state) {
    case 0:
      if (BodyCtrl()->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_Generate) {
        BodyCtrl()->SetLocomotionType(pas::kLT_Relaxed);
        x568_state = 3;
        SendScriptMsgs(kSS_Attack, mgr, kSM_None);
      } else {
        BodyCtrl()->CommandMgr().DeliverCmd(
            CBCGenerateCmd(x56c_data.GetX1f0_25() ? pas::kGType_Eight : pas::kGType_Zero, -1));
      }
      break;
    case 3:
      if (BodyCtrl()->GetBodyStateInfo().GetCurrentStateId() != pas::kAS_Generate) {
        x568_state = 4;
      }
      break;
    }
    break;
  }
  x968_interestTimer = 0.f;
}

void CIceSheegoth::Deactivate(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    x568_state = 1;
    break;
  case kStateMsg_Update:
    switch (x568_state) {
    case 1: {
      const CVector3f& home = x3a0_latestLeashPosition;
      const CVector3f delta = home - GetTranslation();
      if (delta.MagSquared() <= 1.f) {
        x568_state = 2;
      } else {
        const CVector3f arrival = x45c_steeringBehaviors.Arrival(*this, home, 15.f);
        BodyCtrl()->CommandMgr().DeliverCmd(CBCLocomotionCmd(arrival, CVector3f::Zero(), 1.f));
      }
      break;
    }
    case 2: {
      const CVector3f forward = GetTransform().GetForward();
      if (CVector3f::GetAngleDiff(forward, x938_) > CRelAngle::FromDegrees(5.f).AsRadians()) {
        BodyCtrl()->CommandMgr().DeliverCmd(CBCLocomotionCmd(CVector3f::Zero(), x938_, 1.f));
      } else {
        x568_state = 0;
      }
      break;
    }
    case 0:
      if (BodyCtrl()->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_Generate) {
        x568_state = 3;
        BodyCtrl()->SetLocomotionType(pas::kLT_Crouch);
      } else {
        BodyCtrl()->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_One, -1));
      }
      break;
    case 3:
      if (BodyCtrl()->GetBodyStateInfo().GetCurrentStateId() != pas::kAS_Generate) {
        x568_state = 4;
      }
      break;
    }
    break;
  }
}

void CIceSheegoth::Crouch(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    RemoveMaterial(kMT_Orbit, kMT_Target, mgr);
    mgr.Player()->TryToBreakOrbit(GetUniqueId(), CPlayer::kOB_ActivateOrbitSource, mgr);
    BodyCtrl()->SetLocomotionType(pas::kLT_Crouch);
    x968_interestTimer = x56c_data.GetMaxInterestTime();
    SetWasHit(false);
    break;
  case kStateMsg_Deactivate:
    AddMaterial(kMT_Orbit, kMT_Target, mgr);
    break;
  }
}

void CIceSheegoth::Taunt(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    x568_state = 0;
    xb29_25_ = true;
    EnableMouthDamage(mgr, true);
    break;
  case kStateMsg_Update:
    switch (x568_state) {
    case 0:
      if (BodyCtrl()->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_Taunt) {
        x568_state = 3;
      } else {
        BodyCtrl()->CommandMgr().DeliverCmd(CBCTauntCmd(pas::kTT_Zero));
      }
      break;
    case 3:
      if (BodyCtrl()->GetBodyStateInfo().GetCurrentStateId() != pas::kAS_Taunt) {
        x568_state = 4;
      }
      break;
    }
    break;
  case kStateMsg_Deactivate:
    EnableMouthDamage(mgr, false);
    xb29_25_ = false;
    break;
  }
}

void CIceSheegoth::Enraged(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    x568_state = 0;
    xb28_30_ = true;
    break;
  case kStateMsg_Update:
    switch (x568_state) {
    case 0:
      if (BodyCtrl()->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_Generate) {
        x568_state = 3;
      } else {
        BodyCtrl()->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_Three, -1));
      }
      break;
    case 3:
      if (BodyCtrl()->GetBodyStateInfo().GetCurrentStateId() != pas::kAS_Generate) {
        x568_state = 4;
      }
      break;
    }
    break;
  }
}

void CIceSheegoth::TurnAround(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    x9f4_boneTracking.SetTarget(mgr.GetPlayer()->GetUniqueId());
    x9f4_boneTracking.SetActive(true);
    UpdateAttackPosition(mgr, x2e0_destPos);
    SetPathFindMode(kPFM_Normal);
    CPatterned::PathFind(mgr, kStateMsg_Activate, dt);
    BodyCtrl()->CommandMgr().ClearLocomotionCmds();
    break;
  case kStateMsg_Update:
    if (PredictShouldTurn(mgr, CRelAngle::FromDegrees(15.f).AsRadians())) {
      const float speed = GetAnimationData()->GetPlaybackRate();
      const CVector3f aim = mgr.GetPlayer()->GetAimPosition(mgr, speed > 0.f ? 1.25f / speed : 0.f);
      CVector3f delta = aim - GetTranslation();
      delta.SetZ(0.f);
      if (delta.CanBeNormalized()) {
        BodyCtrl()->CommandMgr().DeliverCmd(
            CBCLocomotionCmd(CVector3f::Zero(), delta.AsNormalized(), 1.f));
      }
    }
    {
      const float speedScale =
          BodyCtrl()->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_Turn ? 2.f : 1.f;
      x3b4_speed = speedScale * x94c_;
    }
    break;
  case kStateMsg_Deactivate:
    x9f4_boneTracking.SetActive(false);
    x3b4_speed = x94c_;
    break;
  }
}

void CIceSheegoth::Flinch(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    x568_state = 0;
    xb29_25_ = true;
    EnableGillDamage(mgr, true);
    EnableMouthDamage(mgr, true);
    break;
  case kStateMsg_Update:
    switch (x568_state) {
    case 0:
      if (BodyCtrl()->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_KnockBack) {
        x568_state = 3;
      } else {
        BodyCtrl()->CommandMgr().DeliverCmd(CBCKnockBackCmd(x980_, pas::kS_One));
      }
      break;
    case 3:
      if (BodyCtrl()->GetBodyStateInfo().GetCurrentStateId() != pas::kAS_KnockBack) {
        x568_state = 4;
      }
      break;
    }
    break;
  case kStateMsg_Deactivate:
    EnableGillDamage(mgr, true);
    EnableMouthDamage(mgr, true);
    xb29_25_ = false;
    break;
  }
}

bool CIceSheegoth::IsEnraged(CStateManager& mgr) const {
  const CHealthInfo* info = GetHealthInfo(mgr);
  if (info != nullptr) {
    return info->GetHP() < 0.3f * x970_maxHp;
  }
  return false;
}

void CIceSheegoth::AttractProjectiles(CStateManager& mgr) {
  if (IsAlive()) {
    const float radius = x56c_data.GetX14();
    const CVector3f pos = GetTranslation();
    const CVector3f max = pos + CVector3f(radius, radius, radius);
    CVector3f min = pos;
    min -= CVector3f(radius, radius, radius);
    const CAABox projectileBounds((CVector3f(min)), CVector3f(max));
    const CMaterialFilter projectileFilter =
        CMaterialFilter::MakeInclude(CMaterialList(kMT_Projectile));
    rstl::reserved_vector< TUniqueId, 1024 > projectiles;
    mgr.BuildNearList(projectiles, projectileBounds, projectileFilter, nullptr);
    if (!projectiles.empty()) {
      const CVector3f attractionPos = GetEnergyAttractionPos(mgr);
      const CAABox characterBounds((CVector3f(min)), CVector3f(max));
      const CMaterialFilter characterFilter =
          CMaterialFilter::MakeInclude(CMaterialList(kMT_Character));
      rstl::reserved_vector< TUniqueId, 1024 > characters;
      mgr.BuildNearList(characters, characterBounds, characterFilter, nullptr);
      for (AUTO(it, projectiles.begin()); it != projectiles.end(); ++it) {
        if (CGameProjectile* const projectile =
                TCastToPtr< CGameProjectile >(mgr.ObjectById(*it))) {
          if (ShouldAttractProjectile(*projectile, mgr)) {
            const CVector3f delta = attractionPos - projectile->GetTranslation();
            if (projectile->GetVelocity().CanBeNormalized() &&
                IsClosestSheegoth(mgr, characters, projectile->GetTranslation())) {
              const float distance = delta.Magnitude();
              const CVector3f b = projectile->GetTranslation() +
                                  (0.5f * distance) * projectile->GetVelocity().AsNormalized();
              const CVector3f c = attractionPos + CVector3f(0.f, 0.f, 0.4f * (0.4f * distance));
              CProjectileWeapon& weapon = projectile->Projectile();
              CVector3f p1 =
                  CMath::GetBezierPoint(projectile->GetTranslation(), b, c, attractionPos, 0.333f);
              const CVector3f p2 =
                  CMath::GetBezierPoint(projectile->GetTranslation(), b, c, attractionPos, 0.666f);
              const float length =
                  (attractionPos - p2).Magnitude() +
                  ((p1 - projectile->GetTranslation()).Magnitude() + (p2 - p1).Magnitude());
              const float step = projectile->GetVelocity().Magnitude();
              const CVector3f p3 = CMath::GetBezierPoint(projectile->GetTranslation(), b, c,
                                                         attractionPos, step / length);
              const CVector3f look = p3 - projectile->GetTranslation();
              if (look.CanBeNormalized()) {
                CTransform4f xf(CTransform4f::LookAt(CVector3f::Zero(), look, CVector3f::Up()));
                xf.Orthonormalize();
                weapon.SetWorldSpaceOrientation(xf);
                const CVector3f normalVelocity = weapon.GetVelocity().AsNormalized();
                const CVector3f scaledVelocity = normalVelocity * 0.8f;
                const CVector3f velocity =
                    CVector3f::Lerp(weapon.GetVelocity(), scaledVelocity, 0.6f);
                weapon.SetVelocity(velocity);
              }
            }
          }
        }
      }
    }
  }
}

bool CIceSheegoth::IsClosestSheegoth(CStateManager& mgr,
                                     const rstl::reserved_vector< TUniqueId, 1024 >& nearList,
                                     const CVector3f& pos) const {
  CVector3f delta = pos - GetTranslation();
  const float distanceSquared = delta.MagSquared();
  for (AUTO(it, nearList.begin()); it != nearList.end(); ++it) {
    const CIceSheegoth* sheegoth = PATTERNED_CAST_TO(CIceSheegoth, const_cast< CEntity* >(mgr.GetObjectById(*it)));
    if (sheegoth && sheegoth->GetUniqueId() != GetUniqueId()) {
      delta = pos - sheegoth->GetTranslation();
      if (delta.MagSquared() < distanceSquared) {
        return false;
      }
    }
  }
  return true;
}

bool CIceSheegoth::ShouldAttractProjectile(const CGameProjectile& projectile,
                                           const CStateManager& mgr) const {
  const EWeaponType type = projectile.GetType();
  const bool canAttract =
      type != kWT_Missile && type != kWT_Plasma &&
      (!projectile.GetCurrentDamageInfo().GetWeaponMode().IsComboed() || type != kWT_Power);
  const CActor* owner = static_cast< const CActor* >(mgr.GetObjectById(projectile.GetOwnerId()));
  if (canAttract && owner) {
    if (!PATTERNED_CAST_TO(CIceSheegoth, const_cast< CActor* >(owner)) &&
        projectile.GetCurrentAreaId() == GetCurrentAreaId()) {
      const CVector3f offset = GetTransform().Rotate(x56c_data.GetX8());
      const CVector3f attractionPos = GetTranslation() + offset;
      const CVector3f delta = projectile.GetTranslation() - owner->GetTranslation();
      if (type == kWT_Wave && !projectile.GetCurrentDamageInfo().GetWeaponMode().IsCharged() &&
          !projectile.GetCurrentDamageInfo().GetWeaponMode().IsComboed() &&
          delta.MagSquared() < 100.f) {
        return false;
      }
      const CVector3f toProjectile = projectile.GetTranslation() - attractionPos;
      const CVector2f target2d = toProjectile.ToVec2f();
      const CVector3f target(target2d, 0.f);
      const CVector3f forward = GetTransform().GetForward();
      if (CVector3f::GetAngleDiff(forward, target) < x56c_data.GetX4() ||
          CVector3f::GetAngleDiff(-forward, target) < x56c_data.GetX0()) {
        return true;
      }
    }
  }
  return false;
}

CVector3f CIceSheegoth::GetEnergyAttractionPos(CStateManager& mgr) const {
  if (const CCollisionActor* actor =
          TCastToConstPtr< CCollisionActor >(mgr.GetObjectById(xaf6_iceShardsCollider))) {
    return actor->GetTranslation();
  }
  return GetTranslation();
}

void CIceSheegoth::UpdateTouchBounds() {
  x978_ = 1.75f * GetModelScale().GetY();
  const CAABox bounds(CVector3f(-x978_, -x978_, 0.f), CVector3f(x978_, x978_, 2.f * x978_));
  SetBoundingBox(bounds);
  xa30_.Box() = bounds;
  x760_pathSearch.SetCharacterRadius(x978_);
  x760_pathSearch.SetCharacterHeight(x978_);
  x760_pathSearch.SetPadding(20.f);
  x844_approachSearch.SetCharacterRadius(x978_);
  x844_approachSearch.SetCharacterHeight(x978_);
  x844_approachSearch.SetPadding(20.f);
}

void CIceSheegoth::UpdateAILogicTimers(float dt) {
  const float scale = xb29_28_ ? 2.f : 1.f;
  if (x954_attackTimeLeft > 0.f) {
    x954_attackTimeLeft -= scale * dt;
  }
  if (x960_ > 0.f) {
    x960_ -= dt;
  }
  if (x97c_ > 0.f) {
    x97c_ -= dt;
  }
  if (x958_ > 0.f) {
    x958_ -= dt;
  }
  if (x968_interestTimer < x56c_data.GetMaxInterestTime()) {
    x968_interestTimer += dt;
  }
}

void CIceSheegoth::UpdateAttackPosition(CStateManager& mgr, CVector3f& pos) {
  pos = GetTranslation();
  if (x954_attackTimeLeft <= 0.f) {
    pos = mgr.GetPlayer()->GetAimPosition(mgr, 0.f);
    const CVector3f delta = GetTranslation() - pos;
    if (delta.CanBeNormalized()) {
      const float range = x2fc_minAttackRange;
      pos += range * delta.AsNormalized();
    }
  }
}

void CIceSheegoth::ProcessStompGround(CStateManager& mgr) {
  CPlayer& player = *mgr.Player();
  const CVector3f delta = GetTranslation() - player.GetTranslation();
  const float distance = delta.Magnitude();
  const float magnitude = 0.5f - 0.01f * distance;
  if (magnitude > 0.f && player.GetSurfaceRestraint() != CPlayer::kSR_Air &&
      !player.IsInsideFluid()) {
    bool applyImpulse = true;
    if (player.GetMorphballTransitionState() != CPlayer::kMS_Morphed) {
      if (mgr.GetCameraManager()->GetCurrentCameraId() ==
          mgr.GetCameraManager()->GetFirstPersonCamera()->GetUniqueId()) {
        mgr.CameraManager()->AddCameraShaker(CCameraShakeData::HardHorizShakeDistance(
                                                 GetTranslation(), 0.5f, magnitude, 50.f),
                                             true);
      }
      applyImpulse = xb28_28_;
    }
    if (applyImpulse) {
      const float intensity = xb28_28_ ? 40.f : 25.f;
      const CVector3f direction = intensity * CVector3f::Up();
      const CVector3f upward = magnitude * direction;
      CVector3f lateral = CVector3f::Zero();
      if (distance > x978_) {
        const CVector3f horizontal(delta.ToVec2f(), 0.f);
        if (xb28_28_ && horizontal.CanBeNormalized()) {
          lateral = magnitude * (12.5f * horizontal.AsNormalized());
        }
      }
      const CVector3f velocity = upward + lateral;
      player.ApplyImpulseWR(player.GetMass() * velocity, CAxisAngle::Identity());
      player.SetMoveState(NPlayer::kMS_ApplyJump, mgr);
    }
  }
  if (xb28_28_) {
    AbsorbEnergy(0.25f * x56c_data.GetX170(), mgr);
  }
}

void CIceSheegoth::ApplyContactDamage(TUniqueId id, CStateManager& mgr) {
  if (const CCollisionActor* actor = TCastToConstPtr< CCollisionActor >(mgr.GetObjectById(id))) {
    if (HealthInfo(mgr)->GetHP() > 0.f) {
      bool bite = false;
      if (xb28_29_) {
        if (xb28_25_) {
          bite = IsMouthCollisionActor(*actor);
        } else if (xb28_28_) {
          bite = true;
        }
      }
      const TUniqueId touched = actor->GetLastTouchedObject();
      if (touched == mgr.GetPlayer()->GetUniqueId()) {
        if (bite) {
          mgr.ApplyDamage(
              GetUniqueId(), mgr.GetPlayer()->GetUniqueId(), GetUniqueId(), x56c_data.GetX1b8(),
              CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
              CVector3f::Zero());
          xb28_29_ = false;
          x420_curDamageRemTime = x424_damageWaitTime;
        } else if (x420_curDamageRemTime <= 0.f) {
          mgr.ApplyDamage(
              GetUniqueId(), mgr.GetPlayer()->GetUniqueId(), GetUniqueId(), GetContactDamage(),
              CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
              CVector3f::Zero());
          x420_curDamageRemTime = x424_damageWaitTime;
        }
      }
    }
  }
}

void CIceSheegoth::SetupCollisionManager(CStateManager& mgr) {
  rstl::vector< CJointCollisionDescription > joints;
  joints.reserve(11);
  AddSphereCollisionList(skSphereJointList, 7, joints);
  AddCollisionList(skLeftLegJointList, 2, joints);
  AddCollisionList(skRightLegJointList, 2, joints);
  xa2c_collisionActorManager =
      rs_new CCollisionActorManager(mgr, GetUniqueId(), GetCurrentAreaId(), joints, true);
  xa2c_collisionActorManager->SetActive(mgr, GetActive());
  xa2c_collisionActorManager->AddMaterial(mgr, CMaterialList(kMT_CameraPassthrough));
  xb04_.clear();
  xafc_gillColliders.clear();
  for (uint i = 0; i < xa2c_collisionActorManager->GetNumCollisionActors(); ++i) {
    const CJointCollisionDescription& joint =
        xa2c_collisionActorManager->GetCollisionDescFromIndex(i);
    const TUniqueId id = joint.GetCollisionActorId();
    if (CCollisionActor* const actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(id))) {
      actor->SetDamageVulnerability(CDamageVulnerability::ImmuneVulnerability());
      if (joint.GetName() == rstl::string_l(skpMouthDamageJoint)) {
        xaf8_mouthCollider = id;
        actor->SetDamageVulnerability(x98c_mouthVulnerability);
      } else if (joint.GetName() == rstl::string_l(skpJawJoint)) {
        actor->SetDamageVulnerability(CDamageVulnerability::PassThroughVulnerability());
      } else if (joint.GetName() == rstl::string_l(skpIceShardsLCTR)) {
        xaf6_iceShardsCollider = id;
        actor->SetWeaponCollisionResponseType(kWCR_None);
      } else if (joint.GetName() == rstl::string_l(skpLeftGillJoint) ||
                 joint.GetName() == rstl::string_l(skpRightGillJoint)) {
        xafc_gillColliders.push_back(id);
      } else {
        xb04_.push_back(id);
        actor->SetDamageVulnerability(x56c_data.GetX80());
      }
    }
  }
  SetupHealthInfo(mgr);
  SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
      CMaterialList(kMT_Solid), CMaterialList(kMT_CollisionActor, kMT_AIPassthrough, kMT_Player)));
  AddMaterial(kMT_ProjectilePassthrough, mgr);
  xa2c_collisionActorManager->AddMaterial(mgr, CMaterialList(kMT_AIJoint, kMT_CameraPassthrough));
}

void CIceSheegoth::AddCollisionList(const SJointInfo* joints, int count,
                                    rstl::vector< CJointCollisionDescription >& out) {
  const CAnimData& animData = *GetAnimationData();
  for (int i = 0; i < count; ++i) {
    const CSegId from = animData.GetLocatorSegId(rstl::string_l(joints[i].from));
    const CSegId to = animData.GetLocatorSegId(rstl::string_l(joints[i].to));
    if (from.val() != 0xff && to.val() != 0xff) {
      const CJointCollisionDescription desc = CJointCollisionDescription::SphereSubdivideCollision(
          from, to, joints[i].radius, joints[i].separation, CJointCollisionDescription::kOT_One,
          rstl::string_l(joints[i].from), 1000.f);
      out.push_back(desc);
    }
  }
}

void CIceSheegoth::AddSphereCollisionList(const SSphereJointInfo* joints, int count,
                                          rstl::vector< CJointCollisionDescription >& out) {
  const CAnimData& animData = *GetAnimationData();
  for (int i = 0; i < count; ++i) {
    const CSegId id = animData.GetLocatorSegId(rstl::string_l(joints[i].name));
    if (id.val() != 0xff) {
      const CJointCollisionDescription desc = CJointCollisionDescription::SphereCollision(
          id, joints[i].radius, rstl::string_l(joints[i].name), 1000.f);
      out.push_back(desc);
      xb1c_.push_back(id);
    }
  }
}

bool CIceSheegoth::IsMouthCollisionActor(const CCollisionActor& actor) const {
  return actor.GetUniqueId() == xaf8_mouthCollider;
}

bool CIceSheegoth::IsGillCollisionActor(const CCollisionActor& actor) const {
  for (AUTO(it, xafc_gillColliders.begin()); it != xafc_gillColliders.end(); ++it) {
    if (*it == actor.GetUniqueId()) {
      return true;
    }
  }
  return false;
}

void CIceSheegoth::ExtendTouchBounds(CStateManager& mgr, const CVector3f& bounds) {
  for (uint i = 0; i < xa2c_collisionActorManager->GetNumCollisionActors(); ++i) {
    const CJointCollisionDescription& joint =
        xa2c_collisionActorManager->GetCollisionDescFromIndex(i);
    const TUniqueId id = joint.GetCollisionActorId();
    if (CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(id))) {
      actor->SetExtendedTouchBounds(bounds);
    }
  }
}

void CIceSheegoth::SetupHealthInfo(CStateManager& mgr) {
  CHealthInfo* health = HealthInfo(mgr);
  x970_maxHp = health->GetHP();
  for (AUTO(it, xafc_gillColliders.begin()); it != xafc_gillColliders.end(); ++it) {
    if (CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(*it))) {
      *actor->HealthInfo(mgr) = *health;
    }
  }
  if (CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(xaf8_mouthCollider))) {
    *actor->HealthInfo(mgr) = *health;
  }
  for (AUTO(it, xb04_.begin()); it != xb04_.end(); ++it) {
    if (CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(*it))) {
      *actor->HealthInfo(mgr) = *health;
    }
  }
}

void CIceSheegoth::UpdateHealthInfo(CStateManager& mgr) {
  if (!IsAlive()) {
    return;
  }
  float damage = 0.f;
  if (CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(xaf8_mouthCollider))) {
    damage = CMath::Max(damage, x970_maxHp - actor->HealthInfo(mgr)->GetHP());
  }
  for (AUTO(it, xafc_gillColliders.begin()); it != xafc_gillColliders.end(); ++it) {
    if (CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(*it))) {
      damage = CMath::Max(damage, 4.f * (x970_maxHp - actor->HealthInfo(mgr)->GetHP()));
    }
  }
  for (AUTO(it, xb04_.begin()); it != xb04_.end(); ++it) {
    if (CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(*it))) {
      damage = CMath::Max(damage, x970_maxHp - actor->HealthInfo(mgr)->GetHP());
    }
  }
  HealthInfo(mgr)->SetHP(HealthInfo(mgr)->GetHP() - damage);
  if (HealthInfo(mgr)->GetHP() <= 0.f) {
    Death(mgr, CVector3f::Zero(), kSS_DeathRattle);
    xb28_26_ = true;
  } else {
    if (CCollisionActor* actor =
            TCastToPtr< CCollisionActor >(mgr.ObjectById(xaf8_mouthCollider))) {
      actor->HealthInfo(mgr)->SetHP(x970_maxHp);
    }
    for (AUTO(it, xafc_gillColliders.begin()); it != xafc_gillColliders.end(); ++it) {
      if (CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(*it))) {
        actor->HealthInfo(mgr)->SetHP(x970_maxHp);
      }
    }
    for (AUTO(it, xb04_.begin()); it != xb04_.end(); ++it) {
      if (CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(*it))) {
        actor->HealthInfo(mgr)->SetHP(x970_maxHp);
      }
    }
  }
}

void CIceSheegoth::UpdateSteeringBlendSpeed(float dt) {
  const float target = x95c_ > 0.f ? 1.f : x944_;
  if (target > x948_) {
    x948_ = rstl::min_val(x948_ + 2.f * dt, target);
  } else if (target < x948_) {
    x948_ = rstl::max_val(target, x948_ - 2.f * dt);
  }
  BodyCtrl()->CommandMgr().SetSteeringSpeedRange(x948_, x948_);
}

void CIceSheegoth::CreateFlameThrower(CStateManager& mgr) {
  if (xa80_flameThrowerId == kInvalidUniqueId) {
    const CFlameInfo info(6, 4, x56c_data.GetFireBreathResId(), 15, 0.0625f, 20.f, 1.f);
    xa80_flameThrowerId = mgr.AllocateUniqueId();
    CFlameThrower* const flame = rs_new CFlameThrower(
        xa84_, rstl::string_l("IceSheegoth_Flame"), kWT_Plasma, info, CTransform4f::Identity(),
        kMT_CollisionActor, x56c_data.GetFireBreathDamage(), xa80_flameThrowerId,
        GetCurrentAreaId(), GetUniqueId(), CWeapon::kPA_None, x56c_data.GetX1e4(),
        x56c_data.GetX1e8(), x56c_data.GetX1ec());
    mgr.AddObject(*flame);
  }
}

void CIceSheegoth::UpdateParticleEffects(float dt, CStateManager& mgr) {
  if (CFlameThrower* flame = static_cast< CFlameThrower* >(mgr.ObjectById(xa80_flameThrowerId))) {
    if (flame->GetActive()) {
      const CTransform4f xf = GetLctrTransform(rstl::string_l("LCTR_SHEMOUTH"));
      flame->SetTransform(xf, mgr, dt);
    }
  }
  if (const CCollisionActor* actor =
          TCastToConstPtr< CCollisionActor >(mgr.GetObjectById(xaf6_iceShardsCollider))) {
    const float threshold = (1.f / 3.f) * x56c_data.GetX170();
    const float maxThreshold = 3.f * threshold;
    const float energy = x974_;
    if (energy >= maxThreshold) {
      xa98_->SetParticleEmission(false);
      xaac_->SetParticleEmission(false);
      xabc_->SetParticleEmission(true);
      xad4_->SetParticleEmission(true);
    } else if (energy >= 2.f * threshold) {
      xa98_->SetParticleEmission(false);
      xaac_->SetParticleEmission(true);
      xabc_->SetParticleEmission(false);
      xad4_->SetParticleEmission(false);
    } else if (energy > 0.f) {
      xa98_->SetParticleEmission(true);
      xaac_->SetParticleEmission(false);
      xabc_->SetParticleEmission(false);
      xad4_->SetParticleEmission(false);
    } else {
      xa98_->SetParticleEmission(false);
      xaac_->SetParticleEmission(false);
      xabc_->SetParticleEmission(false);
      xad4_->SetParticleEmission(false);
    }
    xa98_->SetOrientation(GetTransform().GetRotation());
    xa98_->SetGlobalTranslation(actor->GetTranslation());
    xaac_->SetOrientation(GetTransform().GetRotation());
    xaac_->SetGlobalTranslation(actor->GetTranslation());
    xabc_->SetOrientation(GetTransform().GetRotation());
    xabc_->SetGlobalTranslation(actor->GetTranslation());
    xad4_->SetOrientation(GetTransform().GetRotation());
    xad4_->SetGlobalTranslation(GetTranslation());
    if (xae8_.get()) {
      xae8_->SetParticleEmission(true);
      xae8_->SetOrientation(GetTransform().GetRotation());
      xae8_->SetGlobalTranslation(GetTranslation());
      xae8_->SetGlobalScale(GetModelScale());
      xae8_->Update(dt);
    }
  } else {
    xa98_->SetParticleEmission(false);
    xaac_->SetParticleEmission(false);
    xabc_->SetParticleEmission(false);
    xad4_->SetParticleEmission(false);
  }
  xa98_->Update(dt);
  xaac_->Update(dt);
  xabc_->Update(dt);
  xad4_->Update(dt);
  if (x974_ >= 2.f * ((1.f / 3.f) * x56c_data.GetX170())) {
    if (!xaf0_crackleSfx) {
      xaf0_crackleSfx = CSfxManager::AddEmitter(x56c_data.GetX1d4(), GetTranslation(),
                                                CVector3f::Zero(), false, true);
    }
  } else if (xaf0_crackleSfx) {
    CSfxManager::RemoveEmitter(xaf0_crackleSfx);
    xaf0_crackleSfx.Clear();
  }
}

void CIceSheegoth::SetPathFindMode(EPathFindMode mode) { x928_pathFindMode = mode; }

EWeaponCollisionResponseTypes CIceSheegoth::GetCollisionResponseType(const CVector3f& pos,
                                                                     const CVector3f& dir,
                                                                     const CWeaponMode& weapon,
                                                                     int attrib) const {
  if (weapon.GetType() == kWT_Ice) {
    return kWCR_None;
  }
  return CPatterned::GetCollisionResponseType(pos, dir, weapon, attrib);
}

bool CIceSheegoth::PredictShouldTurn(const CStateManager& mgr, float minAngle) const {
  const float speed = GetAnimationData()->GetPlaybackRate();
  const CVector3f aim = mgr.GetPlayer()->GetAimPosition(mgr, speed > 0.f ? 1.25f / speed : 0.f);
  const CVector3f& delta = aim - GetTranslation();
  const CVector2f target = delta.ToVec2f();
  const CVector2f forward = GetTransform().GetForward().ToVec2f();
  return CVector2f::GetAngleDiff(forward, target) > minAngle;
}

bool CIceSheegoth::AllowSpecialAttackByChance() const {
  if (xb28_27_) {
    return true;
  }
  return xb29_26_;
}

void CIceSheegoth::AbsorbEnergy(float damage, CStateManager& mgr) {
  x974_ = CMath::Clamp(0.f, x974_ + damage, x56c_data.GetX170());
  float threshold = (1.f / 3.f) * x56c_data.GetX170();
  if (threshold > 0.f) {
    const float chance = x974_ / (3.f * threshold);
    xb29_26_ = mgr.Random()->Float() <= chance;
  } else {
    xb29_26_ = true;
  }
}

void CIceSheegoth::SetShootThrough(CStateManager& mgr) {
  for (uint i = 0; i < xa2c_collisionActorManager->GetNumCollisionActors(); ++i) {
    const CJointCollisionDescription& joint =
        xa2c_collisionActorManager->GetCollisionDescFromIndex(i);
    const TUniqueId id = joint.GetCollisionActorId();
    if (CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(id))) {
      actor->AddMaterial(kMT_ProjectilePassthrough, mgr);
    }
  }
}

void CIceSheegoth::EnableGillDamage(CStateManager& mgr, bool enabled) {
  const CDamageVulnerability vulnerability =
      enabled ? x56c_data.GetX18() : CDamageVulnerability::ImmuneVulnerability();
  for (AUTO(it, xafc_gillColliders.begin()); it != xafc_gillColliders.end(); ++it) {
    if (CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(*it))) {
      actor->SetDamageVulnerability(vulnerability);
    }
  }
}

void CIceSheegoth::EnableMouthDamage(CStateManager& mgr, bool enabled) {
  const CDamageVulnerability vulnerability = enabled ? x56c_data.GetXe8() : x98c_mouthVulnerability;
  if (CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(xaf8_mouthCollider))) {
    actor->SetDamageVulnerability(vulnerability);
  }
}

void CIceSheegoth::ReDirectDamage(CStateManager& mgr, TUniqueId id) {
  if (const CWeapon* weapon = TCastToConstPtr< CWeapon >(mgr.GetObjectById(id))) {
    CDamageInfo damage = weapon->GetCurrentDamageInfo();
    damage.SetRadius(0.f);
    mgr.ApplyDamage(id, xaf8_mouthCollider, weapon->GetOwnerId(), damage,
                    CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
                    CVector3f::Zero());
  }
}

void CIceSheegoth::UpdateAimTarget(CStateManager& mgr) {
  if (!xb29_29_scanned) {
    if (const CScannableObjectInfo* scan = GetScannableObjectInfo()) {
      if (close_enough(1.f, mgr.GetPlayerState()->GetScanTime(scan->GetScannableObjectId()))) {
        xb29_29_scanned = true;
      }
    }
  }
}

void CIceSheegoth::PreventPlayerPenetration(CStateManager& mgr, float dt) {
  if (GetBodyCtrl()->GetLocomotionType() != pas::kLT_Crouch && IsOnGround()) {
    const CPlayer& player = *mgr.GetPlayer();
    if (player.GetMorphballTransitionState() != CPlayer::kMS_Morphed) {
      const CAABox bounds = GetModelData()->GetBounds(GetTransform());
      const CAABox playerBounds = player.GetBoundingBox();
      if (bounds.DoBoundsOverlap(playerBounds)) {
        const CVector3f& scale = GetModelScale();
        const float threshold = 0.6f * scale.GetY();
        float maxDistance = 0.f;
        CVector3f maxDeviation = CVector3f::Zero();
        const CVector3f predicted = PredictMotion(dt).GetTranslation();
        for (AUTO(it, xb1c_.begin()); it != xb1c_.end(); ++it) {
          const rstl::optional_object< CVector3f > deviation =
              xa2c_collisionActorManager->GetDeviation(mgr, *it);
          if (deviation.valid()) {
            const CVector3f delta = *deviation + predicted;
            const float distance = delta.Magnitude();
            if (distance > maxDistance) {
              maxDeviation = delta;
              maxDistance = distance;
            }
          }
        }
        if (maxDistance > threshold) {
          const float penetration = maxDistance - threshold;
          const CVector3f delta = GetTranslation() - player.GetTranslation();
          const float projection =
              CVector3f::Dot(delta, penetration * maxDeviation.AsNormalized()) / delta.MagSquared();
          const CVector3f projected = delta * projection;
          const CVector3f direction = GetTransform().TransposeRotate(projected);
          const CVector3f impulse = GetMoveToORImpulseWR(direction, dt);
          ApplyImpulseWR(impulse, CAxisAngle::Identity());
        }
      }
    }
  }
}

CAABox CIceSheegoth::GetSortingBounds(const CStateManager& mgr) const {
  const CAABox bounds = CActor::GetSortingBounds(mgr);
  const CVector3f center = bounds.GetCenterPoint();
  const CVector3f offset = 0.125f * (bounds.GetMaxPoint() - bounds.GetMinPoint());
  return CAABox(center - offset, center + offset);
}

float CIceSheegoth::GetGravityConstant() const { return kGravityAccel * 10.f; }

const CCollisionPrimitive* CIceSheegoth::GetCollisionPrimitive() const { return &xa30_; }

CPathFindSearch* CIceSheegoth::GetSearchPath() {
  if (x928_pathFindMode == kPFM_Normal) {
    return &x760_pathSearch;
  }

  return &x844_approachSearch;
}
