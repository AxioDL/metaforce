#include "MetroidPrime/Enemies/CFlaahgra.hpp"

#include "Kyoto/Animation/CInt32POINode.hpp"
#include "Kyoto/Math/CMatrix3f.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/CActorLights.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CBoneTracking.hpp"
#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/CCollisionActorManager.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CSimpleShadow.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Cameras/CFirstPersonCamera.hpp"
#include "MetroidPrime/Enemies/CFlaahgraPlants.hpp"
#include "MetroidPrime/Player/CMorphBall.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWaypoint.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CEnergyProjectile.hpp"

#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

#include <float.h>

static const CVector3f skExtendedBounds(12.f, 12.f, 12.f);
static const CVector3f skProjectileOffset(0.5f, 7.f, 0.f);
static const CColor skFlaahgraDamageColor(0.5f, 0.5f, 0.f, 1.f);
static const CColor skFlaahgraFaintColor(0.5f, 0.f, 0.f, 1.f);

const int CFlaahgraData::skNumProperties = 23;

const CFlaahgra::SJointInfo CFlaahgra::skLeftArmJointList[] = {
    {"L_elbow", "L_blade", 0.6f, 1.f},
    {"L_blade", "L_CLAW_LCTR", 0.6f, 1.f},
    {"L_CLAW_LCTR", "L_CLAW_END_LCTR", 0.6f, 1.f},
};
const CFlaahgra::SJointInfo CFlaahgra::skRightArmJointList[] = {
    {"R_elbow", "R_blade", 0.6f, 1.f},
    {"R_blade", "R_CLAW_LCTR", 0.6f, 1.f},
    {"R_CLAW_LCTR", "R_CLAW_END_LCTR", 0.6f, 1.f},
};
const CFlaahgra::SSphereJointInfo CFlaahgra::skSphereJointList[] = {
    {"Head_1", 1.5f}, {"Spine_2", 1.5f}, {"Spine_4", 1.5f}, {"Spine_6", 1.5f}, {"Collar", 1.5f},
};
const pas::ESeverity CFlaahgra::skpAttackTypeLookup[] = {pas::kS_Three, pas::kS_Four, pas::kS_One,
                                                         pas::kS_Zero, pas::kS_Invalid};
const int CFlaahgra::skpComboChain[] = {-1, -1, -1, 2, -1};

class CFlaahgraProjectile : public CEnergyProjectile {
public:
  CFlaahgraProjectile(bool bigStrike, const TToken< CWeaponDescription >& desc,
                      const CTransform4f& xf, const CDamageInfo& damage, TUniqueId uid, TAreaId aid,
                      TUniqueId owner);
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;

private:
  bool x3d8_bigStrike;
};
CHECK_SIZEOF(CFlaahgraProjectile, 0x3e0)

rstl::optional_object< CAABox > CFlaahgra::GetTouchBounds() const {
  return rstl::optional_object< CAABox >();
}

CFlaahgraProjectile::CFlaahgraProjectile(bool bigStrike, const TToken< CWeaponDescription >& desc,
                                         const CTransform4f& xf, const CDamageInfo& damage,
                                         TUniqueId uid, TAreaId aid, TUniqueId owner)
: CEnergyProjectile(true, desc, kWT_AI, xf, kMT_Character, damage, uid, aid, owner,
                    kInvalidUniqueId, kPA_BigProjectile, false, CVector3f(1.f, 1.f, 1.f),
                    rstl::optional_object< TLockedToken< CGenDescription > >(),
                    CSfxManager::kInternalInvalidSfxId, false)
, x3d8_bigStrike(bigStrike) {
  if (x3d8_bigStrike) {
    SetDamageDuration(2.f);
  }
}

void CFlaahgraProjectile::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid,
                                          CStateManager& mgr) {
  CEnergyProjectile::AcceptScriptMsg(msg, uid, mgr);
  if (x3d8_bigStrike) {
    switch (msg) {
    case kSM_Deleted:
      if (mgr.GetPlayer()->GetUniqueId() == x2c2_lastResolvedObj) {
        if (CEntity* owner = mgr.ObjectById(GetOwnerId())) {
          mgr.DeliverScriptMsg(owner, GetUniqueId(), kSM_Action);
        }
      }
      break;
    }
  }
}

CFlaahgraData::CFlaahgraData(CInputStream& in, int propCount)
: x0_(in.ReadFloat())
, x4_(in.ReadFloat())
, x8_(in.ReadFloat())
, xc_faintToSmallHP(in.ReadFloat())
, x10_snakeVulnerability(in)
, x78_projectileRes(in.ReadLong())
, x7c_projectileDamage(in)
, x98_chargedProjectileRes(in.ReadLong())
, x9c_chargedProjectileDamage(in)
, xb8_growingPlantsRes(in.ReadLong())
, xbc_bombSlotDamage(in)
, xd8_actorParameters(LoadActorParameters(in))
, x140_(in.ReadFloat())
, x144_(in.ReadFloat())
, x148_(in.ReadFloat())
, x14c_animationParameters(LoadAnimationParameters(in))
, x158_(in.ReadLong()) {}

CFlaahgra::CFlaahgra(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                     const CTransform4f& xf, const CAnimRes& animRes, const CPatternedInfo& pInfo,
                     const CActorParameters& actParms, const CFlaahgraData& data)
: CPatterned(kC_Flaahgra, uid, name, kFT_Zero, info, xf, CModelData::CModelDataNull(), pInfo,
             kMT_Flyer, kCT_One, kBT_Restricted, actParms, kCS_Large)
, x568_state(-1)
, x56c_data(data)
, x6d0_rendererId(kInvalidUniqueId)
, x6d4_plantsParticleGenDesc(gpSimplePool->GetObj(SObjectTag('PART', data.GetGrowingPlantsRes())))
, x6dc_normalProjectileInfo(data.GetProjectileRes(), data.GetProjectileDamage())
, x704_bigStrikeProjectileInfo(data.GetChargedProjectileRes(), data.GetChargedProjectileDamage())
, x72c_projectilesCreated(-1)
, x77c_targetMirrorWaypointId(kInvalidUniqueId)
, x780_(1)
, x784_(1)
, x788_stage(0)
, x78c_(CVector3f::Zero())
, x798_meleeInitialAnimState(pas::kAS_Invalid)
, x79c_leftArmCollision(nullptr)
, x7a0_rightArmCollision(nullptr)
, x7a4_sphereCollision(nullptr)
, x7a8_(-1)
, x7ac_(1)
, x7b0_(1)
, x7b4_(-1)
, x7b8_(0.f)
, x7bc_(0.f)
, x7c0_(0.f)
, x7c4_actionDuration(0.f)
, x7c8_(-4.f)
, x7cc_generateEndCooldown(0.f)
, x7d0_hitSomethingTime(0.f)
, x7d4_faintTime(0.f)
, x7d8_(0.f)
, x7dc_halfContactDamage(GetContactDamage())
, x7f8_(0)
, x80c_headActor(kInvalidUniqueId)
, x810_(0.f)
, x814_(0.f)
, x818_curHp(0.f)
, x820_aimPosition(xf.GetTranslation())
, x894_fallDirection(CVector3f::Zero())
, x8a0_(xf.GetForward())
, x8ac_(animRes)
, x8e4_24_loaded(false)
, x8e4_25_loading(false)
, x8e4_26_(false)
, x8e4_27_(false)
, x8e4_28_(false)
, x8e4_29_getup(false)
, x8e4_30_bigStrike(false)
, x8e4_31_(false)
, x8e5_24_(false)
, x8e5_26_(false)
, x8e5_27_(false)
, x8e5_28_(false)
, x8e5_29_(true)
, x8e5_30_(false) {
  SetDoTargetDistanceTest(false);
  x6dc_normalProjectileInfo.Token().Lock();
  x704_bigStrikeProjectileInfo.Token().Lock();
  x7dc_halfContactDamage.SetDamage(0.5f * x7dc_halfContactDamage.GetDamage());
  SetActorLights(actParms.GetLighting().MakeActorLights());
  ActorLights()->SetCastShadows(false);
  ActorLights()->SetMaxAreaLights(2);
  ActorLights()->SetMaxDynamicLights(1);
  x460_knockBackController.SetAutoResetImpulse(false);
  x460_knockBackController.SetEnableLaggedBurnDeath(false);
  x430_damageColor = skFlaahgraDamageColor;
  LoadDependencies(x56c_data.GetDependencyGroup());
  for (float angle = 17.5f * (M_PIF / 180.f); angle < CMath::Deg2Rad(360.f);
       angle += CMath::Deg2Rad(90.f)) {
    CVector3f dir =
        GetTransform().Rotate(CVector3f(CMath::FastCosR(angle), CMath::FastSinR(angle), 0.f));
    x82c_.push_back(dir);
    const float offsetAngle = (M_PIF / 4.f) + angle;
    CVector3f offsetDir = GetTransform().Rotate(
        CVector3f(CMath::FastCosR(offsetAngle), CMath::FastSinR(offsetAngle), 0.f));
    x860_.push_back(offsetDir);
  }
}

CFlaahgra::~CFlaahgra() {}

void CFlaahgra::PreThink(float dt, CStateManager& mgr) {
  if (!x8e4_24_loaded) {
    LoadTokens(mgr);
  }
  CPatterned::PreThink(dt, mgr);
}

void CFlaahgra::Think(float dt, CStateManager& mgr) {
  if (GetActive()) {
    CPatterned::Think(dt, mgr);
    x6c8_boneTracking->Update(dt);
    UpdateCollisionManagers(dt, mgr);
    x6c8_boneTracking->PreRender(mgr, *AnimationData(), GetTransform(), GetModelScale(),
                                 *x450_bodyController);
    UpdateSmallScaleReGrowth(dt);
    UpdateHealthInfo(mgr);
    UpdateAimPosition(mgr, dt);
    SetForceWR(CVector3f::Zero());
    SetImpulseWR(CVector3f::Zero());
  }
}

void CFlaahgra::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  switch (msg) {
  case kSM_InitializedInArea:
    if (!x8e4_25_loading && !x8e4_24_loaded) {
      const TAreaId& area = GetCurrentAreaId();
      mgr.World()->Area(area)->GetPostConstructed()->x113c_playerActorsLoading++;
      x8e4_25_loading = true;
    }
    GetMirrorWaypoints(mgr);
    break;
  case kSM_Activate:
    GatherAssets(mgr);
    if (!x8e5_27_) {
      SetupCollisionManager(mgr);
      x6d0_rendererId = mgr.AllocateUniqueId();
      mgr.AddObject(rs_new CFlaahgraRenderer(
          x6d0_rendererId, GetUniqueId(), rstl::string_l("Flaahgra Renderer"),
          CEntityInfo(GetCurrentAreaId(), NullConnectionList), GetTransform()));
      x450_bodyController->SetLocomotionType(pas::kLT_Crouch);
      x450_bodyController->Activate(mgr);
      x8e5_27_ = true;
    }
    break;
  case kSM_Deleted:
    if (x8e5_27_) {
      x79c_leftArmCollision->Destroy(mgr);
      x7a0_rightArmCollision->Destroy(mgr);
      x7a4_sphereCollision->Destroy(mgr);
      mgr.DeleteObjectRequest(x6d0_rendererId);
      x6d0_rendererId = kInvalidUniqueId;
      x8e5_27_ = false;
    }
    break;
  case kSM_Touched:
    if (HealthInfo(mgr)->GetHP() > 0.f) {
      if (const CCollisionActor* colActor =
              TCastToConstPtr< CCollisionActor >(mgr.GetObjectById(uid))) {
        TUniqueId touched = colActor->GetLastTouchedObject();
        if (touched == mgr.GetPlayer()->GetUniqueId() && x420_curDamageRemTime <= 0.f) {
          CDamageInfo damage = GetContactDamage();
          if (x7a8_ == 4) {
            damage = x7dc_halfContactDamage;
          } else if (!IsSwipeAttack()) {
            damage.SetDamage(0.5f * damage.GetDamage());
          }
          if (x788_stage >= 2) {
            damage.SetDamage(1.33f * damage.GetDamage());
          }
          mgr.ApplyDamage(
              GetUniqueId(), mgr.GetPlayer()->GetUniqueId(), GetUniqueId(), damage,
              CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
              CVector3f::Zero());
          x420_curDamageRemTime = x424_damageWaitTime;
        }
      }
    }
    break;
  case kSM_Damage:
    if (HealthInfo(mgr)->GetHP() > 0.f && IsSphereCollider(uid)) {
      if (const CCollisionActor* colActor =
              TCastToConstPtr< CCollisionActor >(mgr.GetObjectById(uid))) {
        TUniqueId projectileId = colActor->GetLastTouchedObject();
        if (const CGameProjectile* projectile =
                TCastToConstPtr< CGameProjectile >(mgr.GetObjectById(projectileId))) {
          switch (x780_) {
          case 3:
            if (!IsDizzy(mgr, 0.f) && x450_bodyController->HasBodyState(pas::kAS_LoopReaction)) {
              TakeDamage(CVector3f::Zero(), 0.f);
              if (x810_ >=
                  x56c_data.GetRetreatHP() - projectile->GetCurrentDamageInfo().GetDamage()) {
                x450_bodyController->CommandMgr().DeliverCmd(CBCLoopHitReactionCmd(pas::kRT_One));
              } else if (uid == x80c_headActor &&
                         (projectile->GetCurrentDamageInfo().GetWeaponMode().IsCharged() ||
                          projectile->GetCurrentDamageInfo().GetWeaponMode().IsComboed() ||
                          projectile->GetCurrentDamageInfo().GetWeaponMode().GetType() ==
                              kWT_Missile)) {
                x450_bodyController->CommandMgr().DeliverCmd(
                    CBCKnockBackCmd(-GetTransform().GetForward(), pas::kS_One));
              }
            } else {
              if (x8e5_30_) {
                TakeDamage(CVector3f::Zero(), 0.f);
              }
              if (uid == x80c_headActor &&
                  (projectile->GetCurrentDamageInfo().GetWeaponMode().IsCharged() ||
                   projectile->GetCurrentDamageInfo().GetWeaponMode().IsComboed() ||
                   projectile->GetCurrentDamageInfo().GetWeaponMode().GetType() == kWT_Missile)) {
                x450_bodyController->CommandMgr().DeliverCmd(CBCAdditiveFlinchCmd(1.f));
              }
            }
            break;
          }
        }
      }
    }
    break;
  case kSM_Decrement:
    x780_ = 0;
    break;
  case kSM_Close:
    ApplyBombSlotDamage(mgr);
    break;
  case kSM_Start:
    x8e4_31_ = false;
    break;
  case kSM_Stop:
    x8e4_31_ = true;
    break;
  case kSM_Play:
    x7d0_hitSomethingTime = 3.f;
    x8e5_24_ = true;
    break;
  case kSM_Action:
    if (TCastToConstPtr< CGameProjectile >(mgr.GetObjectById(uid))) {
      x7f8_ = x788_stage;
    }
    break;
  case kSM_SetToMax:
    x7d4_faintTime = 0.f;
    break;
  case kSM_Reset:
    x8e5_28_ = true;
    break;
  }
  CPatterned::AcceptScriptMsg(msg, uid, mgr);
}

ENTITY_ACCEPT_IMPL(CFlaahgra)

bool CFlaahgra::CanRenderUnsorted(const CStateManager& mgr) const { return true; }

void CFlaahgra::AddToRenderer(const CFrustumPlanes& frustum, const CStateManager& mgr) const {
  if (!NullModel() && !GetPreRenderClipped()) {
    if (CanRenderUnsorted(mgr)) {
      Render(mgr);
    } else {
      EnsureRendered(mgr);
    }
  }
}

CVector3f CFlaahgra::GetAimPosition(const CStateManager& mgr, float dt) const {
  return x820_aimPosition;
}

CProjectileInfo* CFlaahgra::ProjectileInfo() {
  if (x8e4_30_bigStrike) {
    return &x704_bigStrikeProjectileInfo;
  }
  return &x6dc_normalProjectileInfo;
}

void CFlaahgra::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                                float dt) {
  bool handled = false;
  switch (type) {
  case kUE_Projectile: {
    CTransform4f locatorXf = GetLctrTransform(node.GetLocatorName());
    CVector3f attackTarget = GetAttackTargetPos(mgr);
    if (x7b4_ == 0 || x7b4_ == 1) {
      if (x72c_projectilesCreated == 0) {
        x730_projectileDirs.clear();
        CVector3f intercept = ProjectileInfo()->PredictInterceptPos(
            locatorXf.GetTranslation(), attackTarget, *mgr.GetPlayer(), false, dt);
        x730_projectileDirs.push_back(intercept);
        CMatrix3f basis = GetTransform().BuildMatrix3f();
        CMatrix3f rot = CMatrix3f::RotateZ(
            CRelAngle(x7b4_ == 1 ? -4.f * (M_PIF / 180.f) : 4.f * (M_PIF / 180.f)));
        for (int i = 1; i < 5; ++i) {
          CVector3f local = GetTransform().TransposeRotate(intercept - GetTranslation());
          basis = basis * rot;
          CVector3f vec = basis * local;
          CVector3f origin = GetTranslation();
          intercept.SetX(origin.GetX() + vec.GetX());
          intercept.SetY(origin.GetY() + vec.GetY());
          x730_projectileDirs.push_back(intercept);
        }
      }
      if (x72c_projectilesCreated >= 0 && x72c_projectilesCreated < x730_projectileDirs.size()) {
        CTransform4f xf = CTransform4f::LookAt(locatorXf.GetTranslation(),
                                               x730_projectileDirs[x72c_projectilesCreated]);
        CreateProjectile(xf, mgr);
        ++x72c_projectilesCreated;
      }
    } else {
      CVector3f intercept = ProjectileInfo()->PredictInterceptPos(
          locatorXf.GetTranslation(), attackTarget, *mgr.GetPlayer(), false, dt);
      CVector3f target = intercept;
      CVector3f front = GetTransform().GetForward();
      CVector3f direction((target - locatorXf.GetTranslation()).ToVec2f(), 0.f);
      if (CVector3f::GetAngleDiff(front, direction) > CMath::Deg2Rad(45.f)) {
        if (direction.CanBeNormalized()) {
          CVector3f turnDirection =
              CVector3f::Slerp(front, direction.AsNormalized(), CRelAngle::FromDegrees(45.f));
          target = locatorXf.GetTranslation() + direction.Magnitude() * turnDirection;
        } else {
          float magnitude = direction.Magnitude();
          target = locatorXf.GetTranslation() + magnitude * locatorXf.GetForward();
        }
      }
      CTransform4f xf = CTransform4f::LookAt(locatorXf.GetTranslation(), target);
      CreateProjectile(xf, mgr);
    }
    handled = true;
    break;
  }
  case kUE_BeginAction:
    x8e4_26_ = true;
    x7c4_actionDuration = GetEndActionTime();
    break;
  case kUE_ScreenShake: {
    CTransform4f xf = GetLctrTransform(node.GetLocatorName());
    RattlePlayer(mgr, xf.GetTranslation());
    handled = true;
    break;
  }
  case kUE_AlignTargetRot:
    if (x77c_targetMirrorWaypointId != kInvalidUniqueId) {
      if (CScriptWaypoint* wp =
              TCastToPtr< CScriptWaypoint >(mgr.ObjectById(x77c_targetMirrorWaypointId))) {
        mgr.DeliverScriptMsg(wp, GetUniqueId(), kSM_Arrived);
        if (x7f8_ > 0) {
          --x7f8_;
        }
      }
    }
    break;
  case kUE_GenerateEnd: {
    CTransform4f locatorXf = GetLctrTransform(node.GetLocatorName());
    CVector3f aimPos = mgr.GetPlayer()->GetAimPosition(mgr, 0.f);
    CVector3f boneOrigin = locatorXf.GetTranslation();
    boneOrigin[kDZ] = rstl::max_val(aimPos[kDZ], boneOrigin[kDZ]);
    CTransform4f xf = GetTransform();
    xf.SetTranslation(boneOrigin);
    CMaterialFilter filter = CMaterialFilter::MakeInclude(CMaterialList(kMT_Floor));
    CRayCastResult result =
        mgr.RayStaticIntersection(xf.GetTranslation(), CVector3f::Down(), 100.f, filter);
    if (result.IsValid()) {
      xf.SetTranslation(result.GetPoint());
      CFlaahgraPlants* plants = rs_new CFlaahgraPlants(
          TToken< CGenDescription >(x6d4_plantsParticleGenDesc), x56c_data.GetActorParameters(),
          mgr.AllocateUniqueId(), GetCurrentAreaId(), GetUniqueId(), xf,
          x56c_data.GetBombSlotDamage(), CVector3f(5.f, 10.f, 5.f));
      mgr.AddObject(plants);
      mgr.SetActorAreaId(*plants, GetCurrentAreaId());
      x7cc_generateEndCooldown = 8.f;
    }
    x8e4_27_ = true;
    break;
  }
  case kUE_ObjectDrop:
    SendScriptMsgs(kSS_Modify, mgr, kSM_None);
    break;
  }
  if (!handled) {
    CPatterned::DoUserAnimEvent(mgr, node, type, dt);
  }
}

void CFlaahgra::Death(CStateManager& mgr, const CVector3f& direction, EScriptObjectState state) {
  if (x400_25_alive) {
    x330_stateMachineState.SetState(mgr, *this, GetStateMachine(), rstl::string_l("Dead"));
    if (x450_bodyController->GetPercentageFrozen() > 0.f) {
      x450_bodyController->UnFreeze();
    }
    x400_25_alive = false;
  }
}

bool CFlaahgra::OffLine(CStateManager& mgr, float arg) {
  bool ret = false;
  if (x8e5_29_ && x8e5_28_) {
    ret = true;
  }
  return ret;
}

bool CFlaahgra::HitSomething(CStateManager& mgr, float arg) { return x7d0_hitSomethingTime > 0.f; }

bool CFlaahgra::AIStage(CStateManager& mgr, float arg) { return x780_ == static_cast< int >(arg); }

bool CFlaahgra::ShouldTurn(CStateManager& mgr, float arg) {
  CVector2f dir = (mgr.GetPlayer()->GetTranslation() - GetTranslation()).ToVec2f();
  CVector2f front = GetTransform().GetForward().ToVec2f();
  return CVector2f::GetAngleDiff(front, dir) > CMath::Deg2Rad(15.f);
}

bool CFlaahgra::ShouldAttack(CStateManager& mgr, float arg) {
  if (x788_stage > 0 && x788_stage <= 3 && x7c0_ <= 0.f) {
    const CPlayer& player = *mgr.GetPlayer();
    if (!player.IsInsideFluid()) {
      CVector3f delta = player.GetTranslation() - GetTranslation();
      if (!x8e4_31_) {
        CVector2f diff = delta.ToVec2f();
        float dist = diff.MagSquared();
        float minSq = x2fc_minAttackRange * x2fc_minAttackRange;
        float maxSq = x300_maxAttackRange * x300_maxAttackRange;
        if ((player.GetMorphballTransitionState() != CPlayer::kMS_Morphed && dist >= minSq &&
             dist <= maxSq) ||
            (x7cc_generateEndCooldown <= 0.f && player.GetVelocityWR().MagSquared() >= 25.f)) {
          CVector2f target = delta.ToVec2f();
          CVector2f front = GetTransform().GetForward().ToVec2f();
          return CVector2f::GetAngleDiff(front, target) < CMath::Deg2Rad(45.f);
        }
      }
    }
  }
  return false;
}

bool CFlaahgra::ShouldSpecialAttack(CStateManager& mgr, float arg) {
  if (x788_stage >= 0 && x788_stage <= 3 && ShouldFire(mgr, arg) && x788_stage >= 2) {
    return x8e5_24_;
  }
  return false;
}

bool CFlaahgra::ShouldFire(CStateManager& mgr, float arg) {
  if (x7c0_ <= 0.f && !mgr.GetPlayer()->IsInsideFluid()) {
    CVector3f delta = mgr.GetPlayer()->GetTranslation() - GetTranslation();
    CVector2f dir = delta.ToVec2f();
    CVector2f front = GetTransform().GetForward().ToVec2f();
    return CVector2f::GetAngleDiff(front, dir) < CMath::Deg2Rad(45.f);
  }
  return false;
}

bool CFlaahgra::BreakAttack(CStateManager& mgr, float arg) {
  return x7d4_faintTime >= x56c_data.GetFaintToSmallHP() && !x8e4_29_getup;
}

bool CFlaahgra::IsDizzy(CStateManager& mgr, float arg) {
  return x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_LoopReaction;
}

bool CFlaahgra::CoverCheck(CStateManager& mgr, float arg) {
  if (x7f8_ > 0 || x7bc_ <= 0.f) {
    for (AUTO(it, x770_mirrorWaypoints.begin()); it != x770_mirrorWaypoints.end(); ++it) {
      if (const CEntity* ent = mgr.GetObjectById(*it)) {
        if (ent->GetActive()) {
          return true;
        }
      }
    }
  }
  return false;
}

bool CFlaahgra::AnimOver(CStateManager& mgr, float arg) { return x568_state == 4; }

void CFlaahgra::FadeOut(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == kStateMsg_Activate) {
    x7a4_sphereCollision->SetActive(mgr, true);
    x79c_leftArmCollision->SetActive(mgr, true);
    x7a0_rightArmCollision->SetActive(mgr, true);
    x784_ = x780_;
    x81c_ = GetModelData()->ScaleCopy().GetZ();
    UpdateScale(1.f, x81c_, x56c_data.GetLargeScale());
    x8e4_26_ = false;
    x7c0_ = 2.f;
    x780_ = 3;
    x8e4_29_getup = false;
    x430_damageColor = skFlaahgraDamageColor;
    x450_bodyController->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_NextState));
    x450_bodyController->SetLocomotionType(pas::kLT_Relaxed);
    x8e5_29_ = false;
  }
}

void CFlaahgra::FadeIn(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == kStateMsg_Activate) {
    if (HealthInfo(mgr)->GetHP() > 0.f) {
      SendScriptMsgs(kSS_Exited, mgr, kSM_None);
    }
    if (!x8e4_29_getup) {
      SendScriptMsgs(kSS_CloseIn, mgr, kSM_None);
    }
  }
}

void CFlaahgra::Enraged(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    x568_state = 0;
    break;
  case kStateMsg_Update:
    switch (x568_state) {
    case 0:
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_Taunt) {
        x568_state = 2;
      } else {
        x450_bodyController->CommandMgr().DeliverCmd(CBCTauntCmd(pas::kTT_Zero));
      }
      break;
    case 2:
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() != pas::kAS_Taunt) {
        x568_state = 4;
      }
      break;
    }
    break;
  case kStateMsg_Deactivate:
    x7d0_hitSomethingTime = 0.f;
    break;
  }
}

void CFlaahgra::Generate(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    x568_state = 0;
    break;
  case kStateMsg_Update:
    switch (x568_state) {
    case 0:
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_Generate) {
        x568_state = 2;
        x450_bodyController->SetLocomotionType(pas::kLT_Relaxed);
      } else {
        x450_bodyController->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_Zero, -1));
      }
      break;
    case 2:
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() != pas::kAS_Generate) {
        x568_state = 4;
      }
      break;
    }
    break;
  case kStateMsg_Deactivate:
    x7a4_sphereCollision->SetActive(mgr, true);
    x7c0_ = 11.f;
    break;
  }
}

void CFlaahgra::Growth(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    x568_state = 0;
    x8e4_26_ = false;
    x450_bodyController->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_Two, -1));
    x784_ = x780_;
    x81c_ = GetModelData()->ScaleCopy().GetZ();
    break;
  case kStateMsg_Update:
    switch (x568_state) {
    case 0:
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_Generate) {
        x568_state = 2;
      } else {
        x450_bodyController->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_Two, -1));
      }
      break;
    case 2:
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() != pas::kAS_Generate) {
        x568_state = 4;
      } else if (x8e4_26_) {
        float time = GetEndActionTime();
        float t = x7c4_actionDuration > 0.f ? 1.f - time / x7c4_actionDuration : 1.f;
        UpdateScale(t, x81c_, x56c_data.GetLargeScale());
      }
      x450_bodyController->CommandMgr().DeliverTargetVector(mgr.GetPlayer()->GetTranslation() -
                                                            GetTranslation());
      break;
    }
    break;
  case kStateMsg_Deactivate:
    UpdateScale(1.f, x81c_, x56c_data.GetLargeScale());
    x8e4_26_ = false;
    x780_ = 3;
    x79c_leftArmCollision->SetActive(mgr, true);
    x7a0_rightArmCollision->SetActive(mgr, true);
    x8e4_29_getup = false;
    x430_damageColor = skFlaahgraDamageColor;
    break;
  }
}

void CFlaahgra::Faint(CStateManager& mgr, EStateMsg msg, float arg) {
  static const pas::ESeverity kSeverities[] = {pas::kS_Zero, pas::kS_One};
  switch (msg) {
  case kStateMsg_Activate:
    x568_state = 0;
    x7d4_faintTime = 0.f;
    x8e5_24_ = false;
    x7f8_ = 0;
    SendScriptMsgs(kSS_Entered, mgr, kSM_None);
    SendScriptMsgs(kSS_Retreat, mgr, kSM_None);
    x450_bodyController->CommandMgr().DeliverCmd(
        CBCKnockDownCmd(-GetTransform().GetForward(), kSeverities[x7ac_]));
    break;
  case kStateMsg_Update:
    switch (x568_state) {
    case 0:
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_Fall) {
        x568_state = 2;
        CalculateFallDirection();
        UpdateHeadDamageVulnerability(mgr, true);
      } else {
        x450_bodyController->CommandMgr().DeliverCmd(
            CBCKnockDownCmd(-GetTransform().GetForward(), kSeverities[x7ac_]));
      }
      break;
    case 2:
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_LieOnGround) {
        x7d4_faintTime += arg;
        if (x7d4_faintTime >= x56c_data.GetFaintToSmallHP()) {
          x568_state = 4;
        }
      } else {
        x450_bodyController->FaceDirection(x894_fallDirection, arg);
      }
      break;
    }
    break;
  case kStateMsg_Deactivate:
    x8e4_27_ = false;
    x7ac_ = x7ac_ == 0 ? 1 : 0;
    x780_ = 3;
    UpdateHeadDamageVulnerability(mgr, false);
    break;
  }
}

void CFlaahgra::GetUp(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    x568_state = 0;
    x784_ = x780_;
    x8e4_28_ = true;
    break;
  case kStateMsg_Update:
    switch (x568_state) {
    case 0:
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_Getup) {
        x568_state = 2;
        x7a8_ = !x8e4_29_getup ? 4 : -1;
        SetCollisionActorBounds(mgr, x79c_leftArmCollision, skProjectileOffset);
        SetCollisionActorBounds(mgr, x7a0_rightArmCollision, skProjectileOffset);
      } else {
        x450_bodyController->CommandMgr().DeliverCmd(
            CBCGetupCmd(static_cast< pas::EGetupType >(x8e4_29_getup ? 0 : 1)));
      }
      break;
    case 2:
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() != pas::kAS_Getup) {
        x568_state = 4;
      }
      break;
    }
    break;
  case kStateMsg_Deactivate:
    x7c0_ = x8e4_29_getup ? 5.f : 0.f;
    x7a8_ = -1;
    x8e4_28_ = false;
    x8e4_29_getup = false;
    SetCollisionActorBounds(mgr, x79c_leftArmCollision, CVector3f::Zero());
    SetCollisionActorBounds(mgr, x7a0_rightArmCollision, CVector3f::Zero());
    x430_damageColor = skFlaahgraDamageColor;
    break;
  }
}

void CFlaahgra::Suck(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    x568_state = 0;
    x8e4_26_ = false;
    x450_bodyController->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_Two, -1));
    x784_ = x780_;
    break;
  case kStateMsg_Update:
    switch (x568_state) {
    case 0:
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_Getup) {
        x568_state = 2;
      } else {
        x450_bodyController->CommandMgr().DeliverCmd(CBCGetupCmd(pas::kGetup_Zero));
      }
      break;
    case 2:
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() != pas::kAS_Getup) {
        x568_state = 4;
      } else if (x8e4_26_) {
        float time = GetEndActionTime();
        float t = x7c4_actionDuration > 0.f ? 1.f - time / x7c4_actionDuration : 1.f;
        UpdateScale(t, x56c_data.GetLargeScale(), x56c_data.GetSmallScale());
      }
      break;
    }
    break;
  case kStateMsg_Deactivate:
    x8e4_26_ = false;
    x780_ = 2;
    x79c_leftArmCollision->SetActive(mgr, false);
    x7a0_rightArmCollision->SetActive(mgr, false);
    break;
  }
}

void CFlaahgra::TurnAround(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    x6c8_boneTracking->SetTarget(mgr.GetPlayer()->GetUniqueId());
    x6c8_boneTracking->SetActive(true);
    x8e5_29_ = false;
    break;
  case kStateMsg_Update:
    if (ShouldTurn(mgr, 0.f)) {
      float speed = GetModelData()->GetAnimationData()->GetPlaybackRate();
      CVector3f target = mgr.GetPlayer()->GetAimPosition(mgr, speed > 0.f ? 1.5f / speed : 0.f);
      CVector3f direction = target - GetTranslation();
      if (direction.CanBeNormalized()) {
        x450_bodyController->CommandMgr().DeliverCmd(
            CBCLocomotionCmd(CVector3f::Zero(), direction.AsNormalized(), 1.f));
      }
    }
    break;
  case kStateMsg_Deactivate:
    x6c8_boneTracking->SetActive(false);
    break;
  }
}

void CFlaahgra::Attack(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    x568_state = 0;
    x7a8_ = FindBestMeleeAttackType(mgr);
    SendScriptMsgs(kSS_Attack, mgr, kSM_None);
    break;
  case kStateMsg_Update:
    switch (x568_state) {
    case 0:
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_MeleeAttack) {
        int state = 2;
        if (skpComboChain[x7a8_] != -1) {
          state = 1;
        }
        x568_state = state;
        if (IsSwipeAttack()) {
          SetCollisionActorBounds(mgr, x79c_leftArmCollision, skExtendedBounds);
          SetCollisionActorBounds(mgr, x7a0_rightArmCollision, skExtendedBounds);
        }
        x78c_ = GetAttackTargetVector(mgr);
        x798_meleeInitialAnimState = x450_bodyController->GetCurrentAnimId();
      } else {
        x450_bodyController->CommandMgr().DeliverCmd(CBCMeleeAttackCmd(skpAttackTypeLookup[x7a8_]));
      }
      break;
    case 1:
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() != pas::kAS_MeleeAttack) {
        x568_state = 4;
      } else if (x798_meleeInitialAnimState != x450_bodyController->GetCurrentAnimId()) {
        x568_state = 2;
      } else {
        x450_bodyController->CommandMgr().DeliverTargetVector(x78c_);
        if (ShouldAttack(mgr, 0.f)) {
          pas::ESeverity severity = skpAttackTypeLookup[skpComboChain[x7a8_]];
          x450_bodyController->CommandMgr().DeliverCmd(CBCMeleeAttackCmd(severity));
        }
      }
      break;
    case 2:
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() != pas::kAS_MeleeAttack) {
        x568_state = 4;
      } else {
        x450_bodyController->CommandMgr().DeliverTargetVector(x78c_);
      }
      break;
    }
    break;
  case kStateMsg_Deactivate:
    SetCollisionActorBounds(mgr, x79c_leftArmCollision, CVector3f::Zero());
    SetCollisionActorBounds(mgr, x7a0_rightArmCollision, CVector3f::Zero());
    if (IsSwipeAttack()) {
      float stageScale = 1.f + x788_stage;
      x7c0_ =
          (x308_attackTimeVariation * mgr.Random()->Float() + x304_averageAttackTime) / stageScale;
    }
    x7a8_ = -1;
    if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_MeleeAttack) {
      x450_bodyController->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_NextState));
    }
    break;
  }
}

void CFlaahgra::SpecialAttack(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    x568_state = 0;
    x8e5_24_ = false;
    x7b4_ = 3;
    SendScriptMsgs(kSS_Attack, mgr, kSM_None);
    break;
  case kStateMsg_Update:
    switch (x568_state) {
    case 0:
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() ==
          pas::kAS_ProjectileAttack) {
        x568_state = 2;
        x8e4_30_bigStrike = true;
      } else {
        x450_bodyController->CommandMgr().DeliverCmd(
            CBCProjectileAttackCmd(pas::kS_Eight, mgr.GetPlayer()->GetTranslation(), false));
      }
      break;
    case 2:
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() !=
          pas::kAS_ProjectileAttack) {
        x568_state = 4;
      } else {
        x450_bodyController->CommandMgr().DeliverTargetVector(mgr.GetPlayer()->GetTranslation() -
                                                              GetTranslation());
      }
      break;
    }
    break;
  case kStateMsg_Deactivate: {
    float stageScale = 1.f + x788_stage;
    x7c0_ =
        (x308_attackTimeVariation * mgr.Random()->Float() + x304_averageAttackTime) / stageScale;
    x8e4_30_bigStrike = false;
    x7b4_ = -1;
    if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_ProjectileAttack) {
      x450_bodyController->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_NextState));
    }
    break;
  }
  }
}

void CFlaahgra::ProjectileAttack(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    x568_state = 0;
    SendScriptMsgs(kSS_Attack, mgr, kSM_None);
    break;
  case kStateMsg_Update:
    switch (x568_state) {
    case 0:
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() ==
          pas::kAS_ProjectileAttack) {
        x568_state = 2;
      } else {
        x7b4_ = 3;
        pas::ESeverity severity;
        if (x8e4_31_) {
          x7b4_ = 2;
          severity = pas::kS_Six;
        } else if (mgr.GetPlayer()->GetVelocityWR().MagSquared() > 100.f) {
          CVector3f right = GetTransform().GetRight();
          if (CVector3f::Dot(right, mgr.GetPlayer()->GetVelocityWR()) < 0.f) {
            x7b4_ = 1;
            severity = pas::kS_Four;
            x72c_projectilesCreated = 0;
          } else {
            severity = pas::kS_Three;
            x7b4_ = 0;
            x72c_projectilesCreated = 0;
          }
        } else {
          severity = pas::kS_Seven;
        }
        x450_bodyController->CommandMgr().DeliverCmd(
            CBCProjectileAttackCmd(severity, mgr.GetPlayer()->GetTranslation(), false));
      }
      break;
    case 2:
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() !=
          pas::kAS_ProjectileAttack) {
        x568_state = 4;
      } else {
        x450_bodyController->CommandMgr().DeliverTargetVector(mgr.GetPlayer()->GetTranslation() -
                                                              GetTranslation());
      }
      break;
    }
    break;
  case kStateMsg_Deactivate: {
    float stageScale = 1.f + x788_stage;
    x7c0_ =
        (x308_attackTimeVariation * mgr.Random()->Float() + x304_averageAttackTime) / stageScale;
    x7b4_ = -1;
    x72c_projectilesCreated = -1;
    if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_ProjectileAttack) {
      x450_bodyController->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_NextState));
    }
    break;
  }
  }
}

void CFlaahgra::Dizzy(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    x7b8_ = 0.f;
    x814_ = 0.f;
    x8e5_30_ = false;
    break;
  case kStateMsg_Update:
    x7b8_ += arg;
    if (x7b8_ >=
        (x788_stage < 2 ? x56c_data.GetDizzyDuration() : -1.5f + x56c_data.GetDizzyDuration())) {
      x450_bodyController->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_ExitState));
      x8e5_30_ = true;
    } else {
      x814_ = 0.f;
    }
    break;
  case kStateMsg_Deactivate:
    x8e5_30_ = false;
    x810_ = x814_;
    x7bc_ = x56c_data.GetCoverCooldown();
    x450_bodyController->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_ExitState));
    break;
  }
}

void CFlaahgra::Cover(CStateManager& mgr, EStateMsg msg, float arg) {
  static const pas::ESeverity kCoverSeverities[] = {pas::kS_Eight, pas::kS_Seven};
  switch (msg) {
  case kStateMsg_Activate:
    x77c_targetMirrorWaypointId = GetMirrorNearestPlayer(mgr);
    x568_state = x77c_targetMirrorWaypointId != kInvalidUniqueId ? 1 : 4;
    x6c8_boneTracking->SetTarget(mgr.GetPlayer()->GetUniqueId());
    x6c8_boneTracking->SetActive(true);
    break;
  case kStateMsg_Update:
    switch (x568_state) {
    case 1:
      if (const CActor* wp =
              static_cast< const CActor* >(mgr.GetObjectById(x77c_targetMirrorWaypointId))) {
        CVector3f direction = wp->GetTranslation() - GetTranslation();
        CVector2f direction2d = direction.ToVec2f();
        CVector2f front2d = GetTransform().GetForward().ToVec2f();
        if (CVector2f::GetAngleDiff(front2d, direction2d) > CMath::Deg2Rad(15.f) &&
            direction.CanBeNormalized()) {
          x450_bodyController->CommandMgr().DeliverCmd(
              CBCLocomotionCmd(CVector3f::Zero(), direction.AsNormalized(), 1.f));
        } else {
          x568_state = 0;
          x6c8_boneTracking->SetActive(false);
        }
      } else {
        x568_state = 4;
      }
      break;
    case 0:
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_MeleeAttack) {
        x568_state = 2;
      } else {
        x450_bodyController->CommandMgr().DeliverCmd(CBCMeleeAttackCmd(kCoverSeverities[x7b0_]));
      }
      break;
    case 2:
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() != pas::kAS_MeleeAttack) {
        x568_state = 4;
      } else if (const CActor* wp =
                     static_cast< const CActor* >(mgr.GetObjectById(x77c_targetMirrorWaypointId))) {
        x450_bodyController->CommandMgr().DeliverTargetVector(wp->GetTranslation() -
                                                              GetTranslation());
      }
      break;
    }
    break;
  case kStateMsg_Deactivate:
    if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_MeleeAttack) {
      x450_bodyController->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_NextState));
    }
    x77c_targetMirrorWaypointId = kInvalidUniqueId;
    x7bc_ = x56c_data.GetCoverCooldown();
    x7b0_ = x7b0_ == 0 ? 1 : 0;
    break;
  }
}

void CFlaahgra::Dead(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    x568_state = (x450_bodyController->GetFallState() != pas::kFS_Zero ||
                  x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_Fall)
                     ? 1
                     : 0;
    SendScriptMsgs(kSS_CloseIn, mgr, kSM_None);
    break;
  case kStateMsg_Update:
    switch (x568_state) {
    case 1:
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_Getup) {
        x568_state = 0;
      } else {
        x450_bodyController->CommandMgr().DeliverCmd(CBCGetupCmd(pas::kGetup_Two));
      }
      break;
    case 0:
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() != pas::kAS_Getup) {
        if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() != pas::kAS_Fall) {
          x450_bodyController->CommandMgr().DeliverCmd(
              CBCKnockDownCmd(-GetTransform().GetForward(), pas::kS_Two));
        } else {
          x568_state = 2;
          CVector3f target = GetTranslation() + x8a0_;
          CTransform4f xf = CTransform4f::LookAt(GetTranslation(), target);
          xf.SetTranslation(GetTranslation());
          SetTransform(xf);
          SendScriptMsgs(kSS_Dead, mgr, kSM_None);
          x8e5_26_ = true;
        }
      }
      break;
    case 2:
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() != pas::kAS_Fall) {
        mgr.DeleteObjectRequest(GetUniqueId());
      }
      break;
    }
    break;
  }
}

void CFlaahgra::SetupCollisionManager(CStateManager& mgr) {
  CVector3f oldScale = GetModelData()->GetScale();
  ModelData()->SetScale(CVector3f(1.f, 1.f, 1.f) * x56c_data.GetLargeScale());
  rstl::vector< CJointCollisionDescription > leftArmJoints;
  leftArmJoints.reserve(3);
  AddCollisionList(skLeftArmJointList, 3, leftArmJoints);
  x79c_leftArmCollision =
      rs_new CCollisionActorManager(mgr, GetUniqueId(), GetCurrentAreaId(), leftArmJoints, true);
  SetMaterialProperties(x79c_leftArmCollision, mgr);
  rstl::vector< CJointCollisionDescription > rightArmJoints;
  rightArmJoints.reserve(3);
  AddCollisionList(skRightArmJointList, 3, rightArmJoints);
  x7a0_rightArmCollision =
      rs_new CCollisionActorManager(mgr, GetUniqueId(), GetCurrentAreaId(), rightArmJoints, true);
  SetMaterialProperties(x7a0_rightArmCollision, mgr);
  rstl::vector< CJointCollisionDescription > sphereJoints;
  sphereJoints.reserve(5);
  AddSphereCollisionList(skSphereJointList, 5, sphereJoints);
  x7a4_sphereCollision =
      rs_new CCollisionActorManager(mgr, GetUniqueId(), GetCurrentAreaId(), sphereJoints, true);
  SetMaterialProperties(x7a4_sphereCollision, mgr);
  SetupHealthInfo(mgr);
  SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
      CMaterialList(kMT_Solid), CMaterialList(kMT_CollisionActor, kMT_AIPassthrough, kMT_Player)));
  AddMaterial(kMT_ProjectilePassthrough, kMT_Target, kMT_Orbit, mgr);
  RemoveMaterial(kMT_Solid, mgr);
  ModelData()->SetScale(oldScale);
  x7a4_sphereCollision->AddMaterial(mgr, CMaterialList(kMT_AIJoint, kMT_CameraPassthrough));
  x79c_leftArmCollision->AddMaterial(mgr, CMaterialList(kMT_AIJoint, kMT_CameraPassthrough));
  x7a0_rightArmCollision->AddMaterial(mgr, CMaterialList(kMT_AIJoint, kMT_CameraPassthrough));
}

void CFlaahgra::AddCollisionList(const SJointInfo* list, int count,
                                 rstl::vector< CJointCollisionDescription >& out) {
  const CAnimData& animData = *GetModelData()->GetAnimationData();
  for (int i = 0; i < count; ++i) {
    CSegId from = animData.GetLocatorSegId(rstl::string_l(list[i].from));
    CSegId to = animData.GetLocatorSegId(rstl::string_l(list[i].to));
    if (from != CSegId::Invalid() && to != CSegId::Invalid()) {
      CJointCollisionDescription desc = CJointCollisionDescription::SphereSubdivideCollision(
          from, to, list[i].radius, list[i].separation, CJointCollisionDescription::kOT_One,
          rstl::string_l(list[i].from), 10.f);
      out.push_back(desc);
    }
  }
}

void CFlaahgra::AddSphereCollisionList(const SSphereJointInfo* list, int count,
                                       rstl::vector< CJointCollisionDescription >& out) {
  const CAnimData& animData = *GetModelData()->GetAnimationData();
  for (int i = 0; i < count; ++i) {
    CSegId id = animData.GetLocatorSegId(rstl::string_l(list[i].name));
    if (id != CSegId::Invalid()) {
      CJointCollisionDescription desc = CJointCollisionDescription::SphereCollision(
          id, list[i].radius, rstl::string_l(list[i].name), 10.f);
      out.push_back(desc);
    }
  }
}

void CFlaahgra::SetMaterialProperties(rstl::single_ptr< CCollisionActorManager >& colMgr,
                                      CStateManager& mgr) {
  for (uint i = 0; i < colMgr->GetNumCollisionActors(); ++i) {
    TUniqueId id = colMgr->GetCollisionDescFromIndex(i).GetCollisionActorId();
    if (CCollisionActor* actor = static_cast< CCollisionActor* >(mgr.ObjectById(id))) {
      actor->SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
          CMaterialList(kMT_Player),
          CMaterialList(kMT_Trigger, kMT_CollisionActor, kMT_NoStaticCollision, kMT_Immovable)));
      actor->AddMaterial(kMT_Trigger, kMT_ScanPassthrough, mgr);
      actor->SetDamageVulnerability(*static_cast< const CActor* >(this)->GetDamageVulnerability());
    }
  }
}

void CFlaahgra::UpdateCollisionManagers(float dt, CStateManager& mgr) {
  x7a4_sphereCollision->Update(dt, mgr, CCollisionActorManager::kUO_ObjectSpace);
  x79c_leftArmCollision->Update(dt, mgr, CCollisionActorManager::kUO_ObjectSpace);
  x7a0_rightArmCollision->Update(dt, mgr, CCollisionActorManager::kUO_ObjectSpace);
}

void CFlaahgra::UpdateHeadDamageVulnerability(CStateManager& mgr, bool vulnerable) {
  if (CCollisionActor* head = TCastToPtr< CCollisionActor >(mgr.ObjectById(x80c_headActor))) {
    if (vulnerable) {
      head->SetDamageVulnerability(x56c_data.GetSnakeVulnerability());
    } else {
      head->SetDamageVulnerability(*static_cast< const CActor* >(this)->GetDamageVulnerability());
    }
  }
}

void CFlaahgra::SetupHealthInfo(CStateManager& mgr) {
  x7fc_sphereColliders.clear();
  for (uint i = 0; i < x7a4_sphereCollision->GetNumCollisionActors(); ++i) {
    const CJointCollisionDescription& desc = x7a4_sphereCollision->GetCollisionDescFromIndex(i);
    TUniqueId id = desc.GetCollisionActorId();
    if (CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(id))) {
      *actor->HealthInfo(mgr) = *HealthInfo(mgr);
      if (desc.GetName() == rstl::string_l("Head_1")) {
        x80c_headActor = id;
      }
      x7fc_sphereColliders.push_back(id);
    }
  }
  x818_curHp = HealthInfo(mgr)->GetHP();
}

void CFlaahgra::UpdateHealthInfo(CStateManager& mgr) {
  float damage = 0.f;
  for (AUTO(it, x7fc_sphereColliders.begin()); it != x7fc_sphereColliders.end(); ++it) {
    if (CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(*it))) {
      damage = CMath::Max(damage, x818_curHp - actor->HealthInfo(mgr)->GetHP());
    }
  }
  if (x780_ == 3) {
    if (IsDizzy(mgr, 0.f)) {
      x814_ += damage;
    } else {
      x810_ += damage;
    }
  } else {
    x814_ = 0.f;
    x810_ = 0.f;
  }
  if (HealthInfo(mgr)->GetHP() <= 0.f) {
    Death(mgr, CVector3f::Zero(), kSS_DeathRattle);
    RemoveMaterial(kMT_Orbit, mgr);
  } else {
    for (AUTO(it, x7fc_sphereColliders.begin()); it != x7fc_sphereColliders.end(); ++it) {
      if (CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(*it))) {
        actor->HealthInfo(mgr)->SetHP(x818_curHp);
      }
    }
  }
}

bool CFlaahgra::IsSphereCollider(TUniqueId uid) const {
  for (AUTO(it, x7fc_sphereColliders.begin()); it != x7fc_sphereColliders.end(); ++it) {
    if (*it == uid) {
      return true;
    }
  }
  return false;
}

void CFlaahgra::UpdateSmallScaleReGrowth(float dt) {
  if (x7c0_ > 0.f) {
    x7c0_ -= x788_stage < 2 ? dt : 1.25f * dt;
  }
  if (x7bc_ > 0.f) {
    x7bc_ -= dt;
  }
  if (x7d0_hitSomethingTime > 0.f) {
    x7d0_hitSomethingTime -= dt;
  }
  if (x7cc_generateEndCooldown > 0.f) {
    x7cc_generateEndCooldown -= dt;
  }
  if (x8e4_29_getup && x7d8_ <= 6.f) {
    x430_damageColor = CColor::Lerp(CColor(0.f, 0.f, 0.f, 1.f), skFlaahgraFaintColor,
                                    CMath::AbsF(M_PIF * CMath::FastCosR(x7d8_)));
    TakeDamage(CVector3f::Zero(), 0.f);
    x7d8_ += dt;
  }
}

void CFlaahgra::ApplyBombSlotDamage(CStateManager& mgr) {
  const CWeaponMode bombMode = CWeaponMode::Bomb();
  HealthInfo(mgr)->SetHP(HealthInfo(mgr)->GetHP() - x56c_data.GetRetreatTime());
  x7d4_faintTime = x56c_data.GetFaintToSmallHP();
  x8e4_29_getup = true;
  x7d8_ = 0.f;
  x430_damageColor = skFlaahgraFaintColor;
  ++x788_stage;
}

void CFlaahgra::UpdateScale(float t, float minScale, float maxScale) {
  CVector3f scale(1.f, 1.f, 1.f);
  scale *= t * (maxScale - minScale) + minScale;
  ModelData()->SetScale(scale);
}

float CFlaahgra::GetEndActionTime() const {
  CCharAnimTime time = GetModelData()->GetAnimationData()->GetTimeOfUserEvent(
      kUE_EndAction, CCharAnimTime::Infinity());
  if (time == CCharAnimTime::Infinity()) {
    return 0.f;
  }
  return time.GetSeconds();
}

uint CFlaahgra::FindBestMeleeAttackType(CStateManager& mgr) const {
  const CPlayer& player = *mgr.GetPlayer();
  if (player.GetMorphballTransitionState() == CPlayer::kMS_Morphed ||
      (x7cc_generateEndCooldown <= 0.f && player.GetVelocityWR().MagSquared() >= 25.f)) {
    CVector3f right = GetTransform().GetRight();
    if (CVector3f::Dot(right, player.GetVelocityWR()) > 0.f) {
      return 1;
    }
    return 0;
  }
  return 3;
}

CVector3f CFlaahgra::GetAttackTargetVector(CStateManager& mgr) const {
  CPlayer& player = *mgr.Player();
  bool plantStrike =
      IsPlantStrikeAttack() && player.GetMorphballTransitionState() != CPlayer::kMS_Morphed;
  float scale = plantStrike ? 0.75f : 0.5f;
  CVector3f target =
      player.GetAimPosition(mgr, scale * x450_bodyController->GetAnimTimeRemaining());
  return target - GetTranslation();
}

CVector3f CFlaahgra::GetAttackTargetPos(CStateManager& mgr) const {
  const CPlayer& player = *mgr.GetPlayer();
  CVector3f pos = player.GetTranslation();
  if (player.GetMorphballTransitionState() != CPlayer::kMS_Morphed) {
    pos += CVector3f(0.f, 0.f, -0.5f + player.GetEyeHeight());
  } else {
    pos = player.GetMorphBall()->GetBallToWorld().GetTranslation();
  }
  return pos;
}

bool CFlaahgra::IsSwipeAttack() const { return x7a8_ == 2 || x7a8_ == 3 || x7a8_ == 4; }

bool CFlaahgra::IsPlantStrikeAttack() const { return x7a8_ == 0 || x7a8_ == 1; }

bool CFlaahgra::IsFiringProjectile() const {
  return x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_ProjectileAttack;
}

void CFlaahgra::UpdateAimPosition(CStateManager& mgr, float dt) {
  if (const CCollisionActor* head =
          TCastToConstPtr< CCollisionActor >(mgr.GetObjectById(x80c_headActor))) {
    pas::EAnimationState state = x450_bodyController->GetBodyStateInfo().GetCurrentStateId();
    if (state != pas::kAS_GroundHit && state != pas::kAS_LieOnGround) {
      CVector3f target = CVector3f::Zero();
      if (x780_ == 0 || x8e4_28_ || IsFiringProjectile() || IsPlantStrikeAttack()) {
        target = head->GetTranslation();
      } else {
        target =
            GetTranslation() + CVector3f(0.f, 0.f, 3.7675f) +
            CVector3f::ByElementMultiply(CVector3f(0.f, 0.f, 4.155f), GetModelData()->GetScale());
      }
      CVector3f diff = target - x820_aimPosition;
      if (diff.CanBeNormalized()) {
        float mag = diff.Magnitude();
        float step = 125.f * dt;
        if (mag > step) {
          x820_aimPosition += step * ((1.f / mag) * diff);
        } else {
          x820_aimPosition = target;
        }
      }
    }
  }
}

void CFlaahgra::SetCollisionActorBounds(CStateManager& mgr,
                                        const rstl::single_ptr< CCollisionActorManager >& colMgr,
                                        const CVector3f& extendedBounds) {
  for (uint i = 0; i < colMgr->GetNumCollisionActors(); ++i) {
    TUniqueId id = colMgr->GetCollisionDescFromIndex(i).GetCollisionActorId();
    if (CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(id))) {
      actor->SetExtendedTouchBounds(extendedBounds);
    }
  }
}

void CFlaahgra::RattlePlayer(CStateManager& mgr, const CVector3f& vec) {
  CPlayer& player = *mgr.Player();
  CVector3f diff = vec - player.GetTranslation();
  float distance = diff.Magnitude();
  if (player.GetSurfaceRestraint() != CPlayer::kSR_Air && !player.IsInsideFluid()) {
    if (player.GetMorphballTransitionState() != CPlayer::kMS_Morphed) {
      TUniqueId firstPersonId = mgr.GetCameraManager()->GetFirstPersonCamera()->GetUniqueId();
      if (mgr.GetCameraManager()->GetCurrentCameraId() == firstPersonId) {
        mgr.CameraManager()->AddCameraShaker(CCameraShakeData::HardVertShake(2.f, 0.75f), true);
      }
    } else {
      CVector3f velocity = 25.f * CVector3f::Up();
      CVector3f impulse = 0.75f * velocity;
      player.ApplyImpulseWR(player.GetMass() * impulse, CAxisAngle::Identity());
      player.SetMoveState(NPlayer::kMS_ApplyJump, mgr);
    }
  }
}

void CFlaahgra::GetMirrorWaypoints(CStateManager& mgr) {
  x770_mirrorWaypoints.clear();
  const rstl::vector< SConnection >& conns = GetConnectionList();
  for (AUTO(it, conns.begin()); it != conns.end(); ++it) {
    if (it->x0_state == kSS_Modify && it->x4_msg == kSM_Follow) {
      TUniqueId id = mgr.GetIdForScript(it->x8_objId);
      if (TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(id))) {
        x770_mirrorWaypoints.push_back(id);
      }
    }
  }
}

TUniqueId CFlaahgra::GetMirrorNearestPlayer(const CStateManager& mgr) const {
  TUniqueId nearest = kInvalidUniqueId;
  float prevMag = -1.f;
  CVector3f playerPos = mgr.GetPlayer()->GetTranslation();
  for (AUTO(it, x770_mirrorWaypoints.begin()); it != x770_mirrorWaypoints.end(); ++it) {
    if (const CActor* actor = static_cast< const CActor* >(mgr.GetObjectById(*it))) {
      if (actor->GetActive()) {
        CVector3f diff = actor->GetTranslation() - playerPos;
        float mag = diff.MagSquared();
        if (mag > prevMag) {
          nearest = *it;
          prevMag = mag;
        }
      }
    }
  }
  return nearest;
}

void CFlaahgra::CalculateFallDirection() {
  CVector3f front = GetTransform().GetForward();
  CVector3f right = x7ac_ == 0 ? -GetTransform().GetRight() : GetTransform().GetRight();
  float bestDot = FLT_MIN;
  x894_fallDirection = right;
  const rstl::reserved_vector< CVector3f, 4 >& dirs = x7ac_ == 0 ? x860_ : x82c_;
  for (AUTO(it, dirs.begin()); it != dirs.end(); ++it) {
    if (CVector3f::Dot(right, *it) >= 0.f) {
      float dot = CVector3f::Dot(front, *it);
      if (dot > bestDot) {
        x894_fallDirection = *it;
        bestDot = dot;
      }
    }
  }
}

CFlaahgraProjectile* CFlaahgra::CreateProjectile(const CTransform4f& xf, CStateManager& mgr) {
  CFlaahgraProjectile* projectile = nullptr;
  if (ProjectileInfo()->Token().TryCache() && mgr.CanCreateProjectile(GetUniqueId(), kWT_AI, 6)) {
    CDamageInfo damage = ProjectileInfo()->GetDamage();
    if (x788_stage >= 2) {
      damage.SetDamage(1.33f * damage.GetDamage());
    }
    projectile =
        rs_new CFlaahgraProjectile(x8e4_30_bigStrike, ProjectileInfo()->Token(), xf, damage,
                                   mgr.AllocateUniqueId(), GetCurrentAreaId(), GetUniqueId());
    if (projectile != nullptr) {
      mgr.AddObject(projectile);
    }
  }
  return projectile;
}

void CFlaahgra::LoadTokens(CStateManager& mgr) {
  if (!x8d4_tokens.empty()) {
    for (AUTO(it, x8d4_tokens.begin()); it != x8d4_tokens.end(); ++it) {
      if (!it->IsLoaded()) {
        return;
      }
    }
    FinalizeLoad(mgr);
  }
  if (x8c8_depGroup->IsLoaded()) {
    TLockedToken< CDependencyGroup > depGroup(*x8c8_depGroup);
    const rstl::vector< SObjectTag >& tags = depGroup->GetObjectTagVector();
    if (tags.empty()) {
      FinalizeLoad(mgr);
      return;
    }
    if (x8d4_tokens.empty()) {
      x8d4_tokens.reserve(tags.size());
      for (int i = 0; i < tags.size(); ++i) {
        CToken token = gpSimplePool->GetObj(tags[i]);
        token.Lock();
        x8d4_tokens.push_back(token);
      }
    }
  }
}

void CFlaahgra::FinalizeLoad(CStateManager& mgr) {
  x8e4_24_loaded = true;
  if (x8e4_25_loading) {
    TAreaId area = GetCurrentAreaId();
    mgr.World()->Area(area)->GetPostConstructed()->x113c_playerActorsLoading--;
    x8e4_25_loading = false;
  }
  ResetModelDataAndBodyController();
}

void CFlaahgra::GatherAssets(CStateManager& mgr) {
  if (!x8e4_24_loaded) {
    x8c8_depGroup->GetObj();
    LoadTokens(mgr);
    if (!x8e4_24_loaded) {
      for (AUTO(it, x8d4_tokens.begin()); it != x8d4_tokens.end(); ++it) {
        it->GetObj();
      }
      FinalizeLoad(mgr);
    }
  }
}

void CFlaahgra::LoadDependencies(CAssetId id) {
  if (id == kInvalidAssetId) {
    ResetModelDataAndBodyController();
    x8e4_24_loaded = true;
  } else {
    x8c8_depGroup = gpSimplePool->GetObj(SObjectTag('DGRP', id));
    x8c8_depGroup->Lock();
  }
}

void CFlaahgra::ResetModelDataAndBodyController() {
  SetModelData(CModelData(x8ac_));
  CreateShadowIfNeeded();
  SetDrawShadow(true);
  Shadow()->SetAlwaysCalculateRadius(false);
  BuildBodyController(kBT_Restricted);
  x6c8_boneTracking =
      rs_new CBoneTracking(*GetModelData()->GetAnimationData(), rstl::string_l("Head_1"),
                           CMath::Deg2Rad(80.f), CMath::Deg2Rad(180.f), kBTF_None);
}

CFlaahgraRenderer::CFlaahgraRenderer(TUniqueId uid, TUniqueId owner, const rstl::string& name,
                                     const CEntityInfo& info, const CTransform4f& xf)
: CActor(uid, true, name, info, xf, CModelData::CModelDataNull(), CMaterialList(kMT_Character),
         CActorParameters::None(), kInvalidUniqueId)
, xe8_owner(owner) {}

ENTITY_ACCEPT_IMPL(CFlaahgraRenderer)

void CFlaahgraRenderer::AddToRenderer(const CFrustumPlanes& frustum,
                                      const CStateManager& mgr) const {
  if (const CActor* owner = static_cast< const CActor* >(mgr.GetObjectById(xe8_owner))) {
    if (!owner->NullModel()) {
      owner->GetModelData()->RenderParticles(frustum);
    }
  }
}

rstl::optional_object< CAABox > CFlaahgraRenderer::GetTouchBounds() const {
  return rstl::optional_object< CAABox >();
}

CFlaahgraRenderer::~CFlaahgraRenderer() {}
