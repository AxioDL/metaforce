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

#if VERSION >= VERSION_GM8P_00
#define FLAAHGRA_STAGE_DAMAGE_MULTIPLIER 1.f
#else
#define FLAAHGRA_STAGE_DAMAGE_MULTIPLIER 1.33f
#endif

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
  bool mBigStrike;
};
CHECK_SIZEOF(CFlaahgraProjectile, (VERSION >= VERSION_GM8P_00 ? 0x408
                                   : VERSION >= VERSION_GM8E_02 ? 0x3f0
                                                                : 0x3e0))

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
, mBigStrike(bigStrike) {
  if (mBigStrike) {
    SetDamageDuration(2.f);
  }
}

void CFlaahgraProjectile::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid,
                                          CStateManager& mgr) {
  CEnergyProjectile::AcceptScriptMsg(msg, uid, mgr);
  if (mBigStrike) {
    switch (msg) {
    case kSM_Deleted:
      if (mgr.GetPlayer()->GetUniqueId() == mLastResolvedObj) {
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
, mFaintToSmallHP(in.ReadFloat())
, mSnakeVulnerability(in)
, mProjectileRes(in.ReadLong())
, mProjectileDamage(in)
, mChargedProjectileRes(in.ReadLong())
, mChargedProjectileDamage(in)
, mGrowingPlantsRes(in.ReadLong())
, mBombSlotDamage(in)
, mActorParameters(LoadActorParameters(in))
, x140_(in.ReadFloat())
, x144_(in.ReadFloat())
, x148_(in.ReadFloat())
, mAnimationParameters(LoadAnimationParameters(in))
, x158_(in.ReadLong()) {}

CFlaahgra::CFlaahgra(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                     const CTransform4f& xf, const CAnimRes& animRes, const CPatternedInfo& pInfo,
                     const CActorParameters& actParms, const CFlaahgraData& data)
: CPatterned(kC_Flaahgra, uid, name, kFT_Zero, info, xf, CModelData::CModelDataNull(), pInfo,
             kMT_Flyer, kCT_One, kBT_Restricted, actParms, kCS_Large)
, mState(-1)
, mData(data)
, mRendererId(kInvalidUniqueId)
, mPlantsParticleGenDesc(gpSimplePool->GetObj(SObjectTag('PART', data.GetGrowingPlantsRes())))
, mNormalProjectileInfo(data.GetProjectileRes(), data.GetProjectileDamage())
, mBigStrikeProjectileInfo(data.GetChargedProjectileRes(), data.GetChargedProjectileDamage())
, mProjectilesCreated(-1)
, mTargetMirrorWaypointId(kInvalidUniqueId)
, x780_(1)
, x784_(1)
, mStage(0)
, x78c_(CVector3f::Zero())
, mMeleeInitialAnimState(pas::kAS_Invalid)
, mLeftArmCollision(nullptr)
, mRightArmCollision(nullptr)
, mSphereCollision(nullptr)
, x7a8_(-1)
, x7ac_(1)
, x7b0_(1)
, x7b4_(-1)
, x7b8_(0.f)
, x7bc_(0.f)
, x7c0_(0.f)
, mActionDuration(0.f)
, x7c8_(-4.f)
, mGenerateEndCooldown(0.f)
, mHitSomethingTime(0.f)
, mFaintTime(0.f)
, x7d8_(0.f)
, mHalfContactDamage(GetContactDamage())
, x7f8_(0)
#if VERSION >= VERSION_GM8P_00
, mProjectileAttackCount(0)
#endif
, mHeadActor(kInvalidUniqueId)
, x810_(0.f)
#if VERSION < VERSION_GM8P_00
, x814_(0.f)
#endif
, mCurHp(0.f)
, mAimPosition(xf.GetTranslation())
, mFallDirection(CVector3f::Zero())
, x8a0_(xf.GetForward())
, x8ac_(animRes)
, mLoaded(false)
, mLoading(false)
, x8e4_26_(false)
, x8e4_27_(false)
, x8e4_28_(false)
, mGetup(false)
, mBigStrike(false)
, x8e4_31_(false)
#if VERSION < VERSION_GM8P_00
, x8e5_24_(false)
#endif
, x8e5_26_(false)
, x8e5_27_(false)
, x8e5_28_(false)
, x8e5_29_(true)
#if VERSION < VERSION_GM8P_00
, x8e5_30_(false)
#endif
{
  SetDoTargetDistanceTest(false);
  mNormalProjectileInfo.Token().Lock();
  mBigStrikeProjectileInfo.Token().Lock();
  mHalfContactDamage.SetDamage(0.5f * mHalfContactDamage.GetDamage());
  SetActorLights(actParms.GetLighting().MakeActorLights());
  ActorLights()->SetCastShadows(false);
  ActorLights()->SetMaxAreaLights(2);
  ActorLights()->SetMaxDynamicLights(1);
  mKnockBackController.SetAutoResetImpulse(false);
  mKnockBackController.SetEnableLaggedBurnDeath(false);
  mDamageColor = skFlaahgraDamageColor;
  LoadDependencies(mData.GetDependencyGroup());
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
  if (!mLoaded) {
    LoadTokens(mgr);
  }
  CPatterned::PreThink(dt, mgr);
}

void CFlaahgra::Think(float dt, CStateManager& mgr) {
  if (GetActive()) {
    CPatterned::Think(dt, mgr);
    mBoneTracking->Update(dt);
    UpdateCollisionManagers(dt, mgr);
    mBoneTracking->PreRender(mgr, *AnimationData(), GetTransform(), GetModelScale(),
                                 *mBodyController);
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
    if (!mLoading && !mLoaded) {
      const TAreaId& area = GetCurrentAreaId();
      mgr.World()->Area(area)->GetPostConstructed()->mPlayerActorsLoading++;
      mLoading = true;
    }
    GetMirrorWaypoints(mgr);
    break;
  case kSM_Activate:
    GatherAssets(mgr);
    if (!x8e5_27_) {
      SetupCollisionManager(mgr);
      mRendererId = mgr.AllocateUniqueId();
      mgr.AddObject(rs_new CFlaahgraRenderer(
          mRendererId, GetUniqueId(), rstl::string_l("Flaahgra Renderer"),
          CEntityInfo(GetCurrentAreaId(), NullConnectionList), GetTransform()));
      mBodyController->SetLocomotionType(pas::kLT_Crouch);
      mBodyController->Activate(mgr);
      x8e5_27_ = true;
    }
    break;
  case kSM_Deleted:
    if (x8e5_27_) {
      mLeftArmCollision->Destroy(mgr);
      mRightArmCollision->Destroy(mgr);
      mSphereCollision->Destroy(mgr);
      mgr.DeleteObjectRequest(mRendererId);
      mRendererId = kInvalidUniqueId;
      x8e5_27_ = false;
    }
    break;
  case kSM_Touched:
    if (HealthInfo(mgr)->GetHP() > 0.f) {
      if (const CCollisionActor* colActor =
              TCastToConstPtr< CCollisionActor >(mgr.GetObjectById(uid))) {
        TUniqueId touched = colActor->GetLastTouchedObject();
        if (touched == mgr.GetPlayer()->GetUniqueId() && mCurDamageRemTime <= 0.f) {
          CDamageInfo damage = GetContactDamage();
          if (x7a8_ == 4) {
            damage = mHalfContactDamage;
          } else if (!IsSwipeAttack()) {
            damage.SetDamage(0.5f * damage.GetDamage());
          }
          if (mStage >= 2) {
            damage.SetDamage(FLAAHGRA_STAGE_DAMAGE_MULTIPLIER * damage.GetDamage());
          }
          mgr.ApplyDamage(
              GetUniqueId(), mgr.GetPlayer()->GetUniqueId(), GetUniqueId(), damage,
              CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
              CVector3f::Zero());
          mCurDamageRemTime = mDamageWaitTime;
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
            if (!IsDizzy(mgr, 0.f) && mBodyController->HasBodyState(pas::kAS_LoopReaction)) {
              TakeDamage(CVector3f::Zero(), 0.f);
              if (x810_ >=
                  mData.GetRetreatHP() - projectile->GetCurrentDamageInfo().GetDamage()) {
                mBodyController->CommandMgr().DeliverCmd(CBCLoopHitReactionCmd(pas::kRT_One));
              } else if (uid == mHeadActor &&
                         (projectile->GetCurrentDamageInfo().GetWeaponMode().IsCharged() ||
                          projectile->GetCurrentDamageInfo().GetWeaponMode().IsComboed() ||
                          projectile->GetCurrentDamageInfo().GetWeaponMode().GetType() ==
                              kWT_Missile)) {
                mBodyController->CommandMgr().DeliverCmd(
                    CBCKnockBackCmd(-GetTransform().GetForward(), pas::kS_One));
              }
            }
#if VERSION < VERSION_GM8P_00
            else {
              if (x8e5_30_) {
                TakeDamage(CVector3f::Zero(), 0.f);
              }
              if (uid == mHeadActor &&
                  (projectile->GetCurrentDamageInfo().GetWeaponMode().IsCharged() ||
                   projectile->GetCurrentDamageInfo().GetWeaponMode().IsComboed() ||
                   projectile->GetCurrentDamageInfo().GetWeaponMode().GetType() == kWT_Missile)) {
                mBodyController->CommandMgr().DeliverCmd(CBCAdditiveFlinchCmd(1.f));
              }
            }
#endif
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
    mHitSomethingTime = 3.f;
#if VERSION < VERSION_GM8P_00
    x8e5_24_ = true;
#endif
    break;
  case kSM_Action:
    if (TCastToConstPtr< CGameProjectile >(mgr.GetObjectById(uid))) {
      x7f8_ = mStage;
    }
    break;
  case kSM_SetToMax:
    mFaintTime = 0.f;
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
  return mAimPosition;
}

CProjectileInfo* CFlaahgra::ProjectileInfo() {
  if (mBigStrike) {
    return &mBigStrikeProjectileInfo;
  }
  return &mNormalProjectileInfo;
}

void CFlaahgra::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                                float dt) {
  bool handled = false;
  switch (type) {
  case kUE_Projectile: {
    CTransform4f locatorXf = GetLctrTransform(node.GetLocatorName());
    CVector3f attackTarget = GetAttackTargetPos(mgr);
    if (x7b4_ == 0 || x7b4_ == 1) {
      if (mProjectilesCreated == 0) {
        mProjectileDirs.clear();
        CVector3f intercept = ProjectileInfo()->PredictInterceptPos(
            locatorXf.GetTranslation(), attackTarget, *mgr.GetPlayer(), false, dt);
        mProjectileDirs.push_back(intercept);
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
          mProjectileDirs.push_back(intercept);
        }
      }
      if (mProjectilesCreated >= 0 && mProjectilesCreated < mProjectileDirs.size()) {
        CTransform4f xf = CTransform4f::LookAt(locatorXf.GetTranslation(),
                                               mProjectileDirs[mProjectilesCreated]);
        CreateProjectile(xf, mgr);
        ++mProjectilesCreated;
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
    mActionDuration = GetEndActionTime();
    break;
  case kUE_ScreenShake: {
    CTransform4f xf = GetLctrTransform(node.GetLocatorName());
    RattlePlayer(mgr, xf.GetTranslation());
    handled = true;
    break;
  }
  case kUE_AlignTargetRot:
    if (mTargetMirrorWaypointId != kInvalidUniqueId) {
      if (CScriptWaypoint* wp =
              TCastToPtr< CScriptWaypoint >(mgr.ObjectById(mTargetMirrorWaypointId))) {
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
          TToken< CGenDescription >(mPlantsParticleGenDesc), mData.GetActorParameters(),
          mgr.AllocateUniqueId(), GetCurrentAreaId(), GetUniqueId(), xf,
          mData.GetBombSlotDamage(), CVector3f(5.f, 10.f, 5.f));
      mgr.AddObject(plants);
      mgr.SetActorAreaId(*plants, GetCurrentAreaId());
      mGenerateEndCooldown = 8.f;
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
  if (mAlive) {
    mStateMachineState.SetState(mgr, *this, GetStateMachine(), rstl::string_l("Dead"));
    if (mBodyController->GetPercentageFrozen() > 0.f) {
      mBodyController->UnFreeze();
    }
    mAlive = false;
  }
}

bool CFlaahgra::OffLine(CStateManager& mgr, float arg) {
  bool ret = false;
  if (x8e5_29_ && x8e5_28_) {
    ret = true;
  }
  return ret;
}

bool CFlaahgra::HitSomething(CStateManager& mgr, float arg) { return mHitSomethingTime > 0.f; }

bool CFlaahgra::AIStage(CStateManager& mgr, float arg) { return x780_ == static_cast< int >(arg); }

bool CFlaahgra::ShouldTurn(CStateManager& mgr, float arg) {
  CVector2f dir = (mgr.GetPlayer()->GetTranslation() - GetTranslation()).ToVec2f();
  CVector2f front = GetTransform().GetForward().ToVec2f();
  return CVector2f::GetAngleDiff(front, dir) > CMath::Deg2Rad(15.f);
}

bool CFlaahgra::ShouldAttack(CStateManager& mgr, float arg) {
  if (mStage > 0 && mStage <= 3 && x7c0_ <= 0.f) {
    const CPlayer& player = *mgr.GetPlayer();
    if (!player.IsInsideFluid()) {
      CVector3f delta = player.GetTranslation() - GetTranslation();
      if (!x8e4_31_) {
        CVector2f diff = delta.ToVec2f();
        float dist = diff.MagSquared();
        float minSq = mMinAttackRange * mMinAttackRange;
        float maxSq = mMaxAttackRange * mMaxAttackRange;
        if ((player.GetMorphballTransitionState() != CPlayer::kMS_Morphed && dist >= minSq &&
             dist <= maxSq) ||
            (mGenerateEndCooldown <= 0.f && player.GetVelocityWR().MagSquared() >= 25.f)) {
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
  if (mStage >= 0 && mStage <= 3 && ShouldFire(mgr, arg) && mStage >= 2) {
#if VERSION >= VERSION_GM8P_00
    return mProjectileAttackCount >= 2;
#else
    return x8e5_24_;
#endif
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
  return mFaintTime >= mData.GetFaintToSmallHP() && !mGetup;
}

bool CFlaahgra::IsDizzy(CStateManager& mgr, float arg) {
  return mBodyController->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_LoopReaction;
}

bool CFlaahgra::CoverCheck(CStateManager& mgr, float arg) {
  if (x7f8_ > 0 || x7bc_ <= 0.f) {
    for (AUTO(it, mMirrorWaypoints.begin()); it != mMirrorWaypoints.end(); ++it) {
      if (const CEntity* ent = mgr.GetObjectById(*it)) {
        if (ent->GetActive()) {
          return true;
        }
      }
    }
  }
  return false;
}

bool CFlaahgra::AnimOver(CStateManager& mgr, float arg) { return mState == 4; }

void CFlaahgra::FadeOut(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == kStateMsg_Activate) {
    mSphereCollision->SetActive(mgr, true);
    mLeftArmCollision->SetActive(mgr, true);
    mRightArmCollision->SetActive(mgr, true);
    x784_ = x780_;
    x81c_ = GetModelData()->ScaleCopy().GetZ();
    UpdateScale(1.f, x81c_, mData.GetLargeScale());
    x8e4_26_ = false;
    x7c0_ = 2.f;
    x780_ = 3;
    mGetup = false;
    mDamageColor = skFlaahgraDamageColor;
    mBodyController->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_NextState));
    mBodyController->SetLocomotionType(pas::kLT_Relaxed);
    x8e5_29_ = false;
  }
}

void CFlaahgra::FadeIn(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == kStateMsg_Activate) {
    if (HealthInfo(mgr)->GetHP() > 0.f) {
      SendScriptMsgs(kSS_Exited, mgr, kSM_None);
    }
    if (!mGetup) {
      SendScriptMsgs(kSS_CloseIn, mgr, kSM_None);
    }
  }
}

void CFlaahgra::Enraged(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    mState = 0;
    break;
  case kStateMsg_Update:
    switch (mState) {
    case 0:
      if (mBodyController->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_Taunt) {
        mState = 2;
      } else {
        mBodyController->CommandMgr().DeliverCmd(CBCTauntCmd(pas::kTT_Zero));
      }
      break;
    case 2:
      if (mBodyController->GetBodyStateInfo().GetCurrentStateId() != pas::kAS_Taunt) {
        mState = 4;
      }
      break;
    }
    break;
  case kStateMsg_Deactivate:
    mHitSomethingTime = 0.f;
    break;
  }
}

void CFlaahgra::Generate(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    mState = 0;
    break;
  case kStateMsg_Update:
    switch (mState) {
    case 0:
      if (mBodyController->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_Generate) {
        mState = 2;
        mBodyController->SetLocomotionType(pas::kLT_Relaxed);
      } else {
        mBodyController->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_Zero, -1));
      }
      break;
    case 2:
      if (mBodyController->GetBodyStateInfo().GetCurrentStateId() != pas::kAS_Generate) {
        mState = 4;
      }
      break;
    }
    break;
  case kStateMsg_Deactivate:
    mSphereCollision->SetActive(mgr, true);
    x7c0_ = 11.f;
    break;
  }
}

void CFlaahgra::Growth(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    mState = 0;
    x8e4_26_ = false;
    mBodyController->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_Two, -1));
    x784_ = x780_;
    x81c_ = GetModelData()->ScaleCopy().GetZ();
    break;
  case kStateMsg_Update:
    switch (mState) {
    case 0:
      if (mBodyController->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_Generate) {
        mState = 2;
      } else {
        mBodyController->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_Two, -1));
      }
      break;
    case 2:
      if (mBodyController->GetBodyStateInfo().GetCurrentStateId() != pas::kAS_Generate) {
        mState = 4;
      } else if (x8e4_26_) {
        float time = GetEndActionTime();
        float t = mActionDuration > 0.f ? 1.f - time / mActionDuration : 1.f;
        UpdateScale(t, x81c_, mData.GetLargeScale());
      }
      mBodyController->CommandMgr().DeliverTargetVector(mgr.GetPlayer()->GetTranslation() -
                                                            GetTranslation());
      break;
    }
    break;
  case kStateMsg_Deactivate:
    UpdateScale(1.f, x81c_, mData.GetLargeScale());
    x8e4_26_ = false;
    x780_ = 3;
    mLeftArmCollision->SetActive(mgr, true);
    mRightArmCollision->SetActive(mgr, true);
    mGetup = false;
    mDamageColor = skFlaahgraDamageColor;
    break;
  }
}

void CFlaahgra::Faint(CStateManager& mgr, EStateMsg msg, float arg) {
  static const pas::ESeverity kSeverities[] = {pas::kS_Zero, pas::kS_One};
  switch (msg) {
  case kStateMsg_Activate:
    mState = 0;
    mFaintTime = 0.f;
#if VERSION < VERSION_GM8P_00
    x8e5_24_ = false;
#endif
    x7f8_ = 0;
#if VERSION >= VERSION_GM8P_00
    mProjectileAttackCount = 0;
#endif
    SendScriptMsgs(kSS_Entered, mgr, kSM_None);
    SendScriptMsgs(kSS_Retreat, mgr, kSM_None);
    mBodyController->CommandMgr().DeliverCmd(
        CBCKnockDownCmd(-GetTransform().GetForward(), kSeverities[x7ac_]));
    break;
  case kStateMsg_Update:
    switch (mState) {
    case 0:
      if (mBodyController->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_Fall) {
        mState = 2;
        CalculateFallDirection();
        UpdateHeadDamageVulnerability(mgr, true);
      } else {
        mBodyController->CommandMgr().DeliverCmd(
            CBCKnockDownCmd(-GetTransform().GetForward(), kSeverities[x7ac_]));
      }
      break;
    case 2:
      if (mBodyController->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_LieOnGround) {
        mFaintTime += arg;
        if (mFaintTime >= mData.GetFaintToSmallHP()) {
          mState = 4;
        }
      } else {
        mBodyController->FaceDirection(mFallDirection, arg);
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
    mState = 0;
    x784_ = x780_;
    x8e4_28_ = true;
    break;
  case kStateMsg_Update:
    switch (mState) {
    case 0:
      if (mBodyController->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_Getup) {
        mState = 2;
        x7a8_ = !mGetup ? 4 : -1;
        SetCollisionActorBounds(mgr, mLeftArmCollision, skProjectileOffset);
        SetCollisionActorBounds(mgr, mRightArmCollision, skProjectileOffset);
      } else {
        mBodyController->CommandMgr().DeliverCmd(
            CBCGetupCmd(static_cast< pas::EGetupType >(mGetup ? 0 : 1)));
      }
      break;
    case 2:
      if (mBodyController->GetBodyStateInfo().GetCurrentStateId() != pas::kAS_Getup) {
        mState = 4;
      }
      break;
    }
    break;
  case kStateMsg_Deactivate:
    x7c0_ = mGetup ? 5.f : 0.f;
    x7a8_ = -1;
    x8e4_28_ = false;
    mGetup = false;
    SetCollisionActorBounds(mgr, mLeftArmCollision, CVector3f::Zero());
    SetCollisionActorBounds(mgr, mRightArmCollision, CVector3f::Zero());
    mDamageColor = skFlaahgraDamageColor;
    break;
  }
}

void CFlaahgra::Suck(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    mState = 0;
    x8e4_26_ = false;
    mBodyController->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_Two, -1));
    x784_ = x780_;
    break;
  case kStateMsg_Update:
    switch (mState) {
    case 0:
      if (mBodyController->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_Getup) {
        mState = 2;
      } else {
        mBodyController->CommandMgr().DeliverCmd(CBCGetupCmd(pas::kGetup_Zero));
      }
      break;
    case 2:
      if (mBodyController->GetBodyStateInfo().GetCurrentStateId() != pas::kAS_Getup) {
        mState = 4;
      } else if (x8e4_26_) {
        float time = GetEndActionTime();
        float t = mActionDuration > 0.f ? 1.f - time / mActionDuration : 1.f;
        UpdateScale(t, mData.GetLargeScale(), mData.GetSmallScale());
      }
      break;
    }
    break;
  case kStateMsg_Deactivate:
    x8e4_26_ = false;
    x780_ = 2;
    mLeftArmCollision->SetActive(mgr, false);
    mRightArmCollision->SetActive(mgr, false);
    break;
  }
}

void CFlaahgra::TurnAround(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    mBoneTracking->SetTarget(mgr.GetPlayer()->GetUniqueId());
    mBoneTracking->SetActive(true);
    x8e5_29_ = false;
    break;
  case kStateMsg_Update:
    if (ShouldTurn(mgr, 0.f)) {
      float speed = GetModelData()->GetAnimationData()->GetPlaybackRate();
      CVector3f target = mgr.GetPlayer()->GetAimPosition(mgr, speed > 0.f ? 1.5f / speed : 0.f);
      CVector3f direction = target - GetTranslation();
      if (direction.CanBeNormalized()) {
        mBodyController->CommandMgr().DeliverCmd(
            CBCLocomotionCmd(CVector3f::Zero(), direction.AsNormalized(), 1.f));
      }
    }
    break;
  case kStateMsg_Deactivate:
    mBoneTracking->SetActive(false);
    break;
  }
}

void CFlaahgra::Attack(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    mState = 0;
    x7a8_ = FindBestMeleeAttackType(mgr);
    SendScriptMsgs(kSS_Attack, mgr, kSM_None);
    break;
  case kStateMsg_Update:
    switch (mState) {
    case 0:
      if (mBodyController->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_MeleeAttack) {
        int state = 2;
        if (skpComboChain[x7a8_] != -1) {
          state = 1;
        }
        mState = state;
        if (IsSwipeAttack()) {
          SetCollisionActorBounds(mgr, mLeftArmCollision, skExtendedBounds);
          SetCollisionActorBounds(mgr, mRightArmCollision, skExtendedBounds);
        }
        x78c_ = GetAttackTargetVector(mgr);
        mMeleeInitialAnimState = mBodyController->GetCurrentAnimId();
      } else {
        mBodyController->CommandMgr().DeliverCmd(CBCMeleeAttackCmd(skpAttackTypeLookup[x7a8_]));
      }
      break;
    case 1:
      if (mBodyController->GetBodyStateInfo().GetCurrentStateId() != pas::kAS_MeleeAttack) {
        mState = 4;
      } else if (mMeleeInitialAnimState != mBodyController->GetCurrentAnimId()) {
        mState = 2;
      } else {
        mBodyController->CommandMgr().DeliverTargetVector(x78c_);
        if (ShouldAttack(mgr, 0.f)) {
          pas::ESeverity severity = skpAttackTypeLookup[skpComboChain[x7a8_]];
          mBodyController->CommandMgr().DeliverCmd(CBCMeleeAttackCmd(severity));
        }
      }
      break;
    case 2:
      if (mBodyController->GetBodyStateInfo().GetCurrentStateId() != pas::kAS_MeleeAttack) {
        mState = 4;
      } else {
        mBodyController->CommandMgr().DeliverTargetVector(x78c_);
      }
      break;
    }
    break;
  case kStateMsg_Deactivate:
    SetCollisionActorBounds(mgr, mLeftArmCollision, CVector3f::Zero());
    SetCollisionActorBounds(mgr, mRightArmCollision, CVector3f::Zero());
    if (IsSwipeAttack()) {
      float stageScale = 1.f + mStage;
      x7c0_ =
          (mAttackTimeVariation * mgr.Random()->Float() + mAverageAttackTime) / stageScale;
    }
    x7a8_ = -1;
    if (mBodyController->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_MeleeAttack) {
      mBodyController->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_NextState));
    }
    break;
  }
}

void CFlaahgra::SpecialAttack(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    mState = 0;
#if VERSION < VERSION_GM8P_00
    x8e5_24_ = false;
#endif
    x7b4_ = 3;
#if VERSION >= VERSION_GM8P_00
    mProjectileAttackCount = 0;
#endif
    SendScriptMsgs(kSS_Attack, mgr, kSM_None);
    break;
  case kStateMsg_Update:
    switch (mState) {
    case 0:
      if (mBodyController->GetBodyStateInfo().GetCurrentStateId() ==
          pas::kAS_ProjectileAttack) {
        mState = 2;
        mBigStrike = true;
      } else {
        mBodyController->CommandMgr().DeliverCmd(
            CBCProjectileAttackCmd(pas::kS_Eight, mgr.GetPlayer()->GetTranslation(), false));
      }
      break;
    case 2:
      if (mBodyController->GetBodyStateInfo().GetCurrentStateId() !=
          pas::kAS_ProjectileAttack) {
        mState = 4;
      } else {
        mBodyController->CommandMgr().DeliverTargetVector(mgr.GetPlayer()->GetTranslation() -
                                                              GetTranslation());
      }
      break;
    }
    break;
  case kStateMsg_Deactivate: {
    float stageScale = 1.f + mStage;
    x7c0_ =
        (mAttackTimeVariation * mgr.Random()->Float() + mAverageAttackTime) / stageScale;
    mBigStrike = false;
    x7b4_ = -1;
    if (mBodyController->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_ProjectileAttack) {
      mBodyController->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_NextState));
    }
    break;
  }
  }
}

void CFlaahgra::ProjectileAttack(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    mState = 0;
    SendScriptMsgs(kSS_Attack, mgr, kSM_None);
#if VERSION >= VERSION_GM8P_00
    ++mProjectileAttackCount;
#endif
    break;
  case kStateMsg_Update:
    switch (mState) {
    case 0:
      if (mBodyController->GetBodyStateInfo().GetCurrentStateId() ==
          pas::kAS_ProjectileAttack) {
        mState = 2;
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
            mProjectilesCreated = 0;
          } else {
            severity = pas::kS_Three;
            x7b4_ = 0;
            mProjectilesCreated = 0;
          }
        } else {
          severity = pas::kS_Seven;
        }
        mBodyController->CommandMgr().DeliverCmd(
            CBCProjectileAttackCmd(severity, mgr.GetPlayer()->GetTranslation(), false));
      }
      break;
    case 2:
      if (mBodyController->GetBodyStateInfo().GetCurrentStateId() !=
          pas::kAS_ProjectileAttack) {
        mState = 4;
      } else {
        mBodyController->CommandMgr().DeliverTargetVector(mgr.GetPlayer()->GetTranslation() -
                                                              GetTranslation());
      }
      break;
    }
    break;
  case kStateMsg_Deactivate: {
    float stageScale = 1.f + mStage;
    x7c0_ =
        (mAttackTimeVariation * mgr.Random()->Float() + mAverageAttackTime) / stageScale;
    x7b4_ = -1;
    mProjectilesCreated = -1;
    if (mBodyController->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_ProjectileAttack) {
      mBodyController->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_NextState));
    }
    break;
  }
  }
}

void CFlaahgra::Dizzy(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    x7b8_ = 0.f;
#if VERSION < VERSION_GM8P_00
    x814_ = 0.f;
    x8e5_30_ = false;
#endif
    break;
  case kStateMsg_Update:
    x7b8_ += arg;
    if (x7b8_ >=
        (mStage < 2 ? mData.GetDizzyDuration() : -1.5f + mData.GetDizzyDuration())) {
      mBodyController->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_ExitState));
#if VERSION < VERSION_GM8P_00
      x8e5_30_ = true;
    } else {
      x814_ = 0.f;
#endif
    }
    break;
  case kStateMsg_Deactivate:
#if VERSION < VERSION_GM8P_00
    x8e5_30_ = false;
    x810_ = x814_;
#endif
    x7bc_ = mData.GetCoverCooldown();
    mBodyController->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_ExitState));
    break;
  }
}

void CFlaahgra::Cover(CStateManager& mgr, EStateMsg msg, float arg) {
  static const pas::ESeverity kCoverSeverities[] = {pas::kS_Eight, pas::kS_Seven};
  switch (msg) {
  case kStateMsg_Activate:
    mTargetMirrorWaypointId = GetMirrorNearestPlayer(mgr);
    mState = mTargetMirrorWaypointId != kInvalidUniqueId ? 1 : 4;
    mBoneTracking->SetTarget(mgr.GetPlayer()->GetUniqueId());
    mBoneTracking->SetActive(true);
    break;
  case kStateMsg_Update:
    switch (mState) {
    case 1:
      if (const CActor* wp =
              static_cast< const CActor* >(mgr.GetObjectById(mTargetMirrorWaypointId))) {
        CVector3f direction = wp->GetTranslation() - GetTranslation();
        CVector2f direction2d = direction.ToVec2f();
        CVector2f front2d = GetTransform().GetForward().ToVec2f();
        if (CVector2f::GetAngleDiff(front2d, direction2d) > CMath::Deg2Rad(15.f) &&
            direction.CanBeNormalized()) {
          mBodyController->CommandMgr().DeliverCmd(
              CBCLocomotionCmd(CVector3f::Zero(), direction.AsNormalized(), 1.f));
        } else {
          mState = 0;
          mBoneTracking->SetActive(false);
        }
      } else {
        mState = 4;
      }
      break;
    case 0:
      if (mBodyController->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_MeleeAttack) {
        mState = 2;
      } else {
        mBodyController->CommandMgr().DeliverCmd(CBCMeleeAttackCmd(kCoverSeverities[x7b0_]));
      }
      break;
    case 2:
      if (mBodyController->GetBodyStateInfo().GetCurrentStateId() != pas::kAS_MeleeAttack) {
        mState = 4;
      } else if (const CActor* wp =
                     static_cast< const CActor* >(mgr.GetObjectById(mTargetMirrorWaypointId))) {
        mBodyController->CommandMgr().DeliverTargetVector(wp->GetTranslation() -
                                                              GetTranslation());
      }
      break;
    }
    break;
  case kStateMsg_Deactivate:
    if (mBodyController->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_MeleeAttack) {
      mBodyController->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_NextState));
    }
    mTargetMirrorWaypointId = kInvalidUniqueId;
    x7bc_ = mData.GetCoverCooldown();
    x7b0_ = x7b0_ == 0 ? 1 : 0;
    break;
  }
}

void CFlaahgra::Dead(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    mState = (mBodyController->GetFallState() != pas::kFS_Zero ||
                  mBodyController->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_Fall)
                     ? 1
                     : 0;
    SendScriptMsgs(kSS_CloseIn, mgr, kSM_None);
    break;
  case kStateMsg_Update:
    switch (mState) {
    case 1:
      if (mBodyController->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_Getup) {
        mState = 0;
      } else {
        mBodyController->CommandMgr().DeliverCmd(CBCGetupCmd(pas::kGetup_Two));
      }
      break;
    case 0:
      if (mBodyController->GetBodyStateInfo().GetCurrentStateId() != pas::kAS_Getup) {
        if (mBodyController->GetBodyStateInfo().GetCurrentStateId() != pas::kAS_Fall) {
          mBodyController->CommandMgr().DeliverCmd(
              CBCKnockDownCmd(-GetTransform().GetForward(), pas::kS_Two));
        } else {
          mState = 2;
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
      if (mBodyController->GetBodyStateInfo().GetCurrentStateId() != pas::kAS_Fall) {
        mgr.DeleteObjectRequest(GetUniqueId());
      }
      break;
    }
    break;
  }
}

void CFlaahgra::SetupCollisionManager(CStateManager& mgr) {
  CVector3f oldScale = GetModelData()->GetScale();
  ModelData()->SetScale(CVector3f(1.f, 1.f, 1.f) * mData.GetLargeScale());
  rstl::vector< CJointCollisionDescription > leftArmJoints;
  leftArmJoints.reserve(3);
  AddCollisionList(skLeftArmJointList, 3, leftArmJoints);
  mLeftArmCollision =
      rs_new CCollisionActorManager(mgr, GetUniqueId(), GetCurrentAreaId(), leftArmJoints, true);
  SetMaterialProperties(mLeftArmCollision, mgr);
  rstl::vector< CJointCollisionDescription > rightArmJoints;
  rightArmJoints.reserve(3);
  AddCollisionList(skRightArmJointList, 3, rightArmJoints);
  mRightArmCollision =
      rs_new CCollisionActorManager(mgr, GetUniqueId(), GetCurrentAreaId(), rightArmJoints, true);
  SetMaterialProperties(mRightArmCollision, mgr);
  rstl::vector< CJointCollisionDescription > sphereJoints;
  sphereJoints.reserve(5);
  AddSphereCollisionList(skSphereJointList, 5, sphereJoints);
  mSphereCollision =
      rs_new CCollisionActorManager(mgr, GetUniqueId(), GetCurrentAreaId(), sphereJoints, true);
  SetMaterialProperties(mSphereCollision, mgr);
  SetupHealthInfo(mgr);
  SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
      CMaterialList(kMT_Solid), CMaterialList(kMT_CollisionActor, kMT_AIPassthrough, kMT_Player)));
  AddMaterial(kMT_ProjectilePassthrough, kMT_Target, kMT_Orbit, mgr);
  RemoveMaterial(kMT_Solid, mgr);
  ModelData()->SetScale(oldScale);
  mSphereCollision->AddMaterial(mgr, CMaterialList(kMT_AIJoint, kMT_CameraPassthrough));
  mLeftArmCollision->AddMaterial(mgr, CMaterialList(kMT_AIJoint, kMT_CameraPassthrough));
  mRightArmCollision->AddMaterial(mgr, CMaterialList(kMT_AIJoint, kMT_CameraPassthrough));
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
  mSphereCollision->Update(dt, mgr, CCollisionActorManager::kUO_ObjectSpace);
  mLeftArmCollision->Update(dt, mgr, CCollisionActorManager::kUO_ObjectSpace);
  mRightArmCollision->Update(dt, mgr, CCollisionActorManager::kUO_ObjectSpace);
}

void CFlaahgra::UpdateHeadDamageVulnerability(CStateManager& mgr, bool vulnerable) {
  if (CCollisionActor* head = TCastToPtr< CCollisionActor >(mgr.ObjectById(mHeadActor))) {
    if (vulnerable) {
      head->SetDamageVulnerability(mData.GetSnakeVulnerability());
    } else {
      head->SetDamageVulnerability(*static_cast< const CActor* >(this)->GetDamageVulnerability());
    }
  }
}

void CFlaahgra::SetupHealthInfo(CStateManager& mgr) {
  mSphereColliders.clear();
  for (uint i = 0; i < mSphereCollision->GetNumCollisionActors(); ++i) {
    const CJointCollisionDescription& desc = mSphereCollision->GetCollisionDescFromIndex(i);
    TUniqueId id = desc.GetCollisionActorId();
    if (CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(id))) {
      *actor->HealthInfo(mgr) = *HealthInfo(mgr);
      if (desc.GetName() == rstl::string_l("Head_1")) {
        mHeadActor = id;
      }
      mSphereColliders.push_back(id);
    }
  }
  mCurHp = HealthInfo(mgr)->GetHP();
}

void CFlaahgra::UpdateHealthInfo(CStateManager& mgr) {
  float damage = 0.f;
  for (AUTO(it, mSphereColliders.begin()); it != mSphereColliders.end(); ++it) {
    if (CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(*it))) {
      damage = CMath::Max(damage, mCurHp - actor->HealthInfo(mgr)->GetHP());
    }
  }
#if VERSION >= VERSION_GM8P_00
  if (x780_ == 3 && !IsDizzy(mgr, 0.f)) {
    x810_ += damage;
  } else {
    x810_ = 0.f;
  }
#else
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
#endif
  if (HealthInfo(mgr)->GetHP() <= 0.f) {
    Death(mgr, CVector3f::Zero(), kSS_DeathRattle);
    RemoveMaterial(kMT_Orbit, mgr);
  } else {
    for (AUTO(it, mSphereColliders.begin()); it != mSphereColliders.end(); ++it) {
      if (CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(*it))) {
        actor->HealthInfo(mgr)->SetHP(mCurHp);
      }
    }
  }
}

bool CFlaahgra::IsSphereCollider(TUniqueId uid) const {
  for (AUTO(it, mSphereColliders.begin()); it != mSphereColliders.end(); ++it) {
    if (*it == uid) {
      return true;
    }
  }
  return false;
}

void CFlaahgra::UpdateSmallScaleReGrowth(float dt) {
  if (x7c0_ > 0.f) {
    x7c0_ -= mStage < 2 ? dt : 1.25f * dt;
  }
  if (x7bc_ > 0.f) {
    x7bc_ -= dt;
  }
  if (mHitSomethingTime > 0.f) {
    mHitSomethingTime -= dt;
  }
  if (mGenerateEndCooldown > 0.f) {
    mGenerateEndCooldown -= dt;
  }
  if (mGetup && x7d8_ <= 6.f) {
    mDamageColor = CColor::Lerp(CColor(0.f, 0.f, 0.f, 1.f), skFlaahgraFaintColor,
                                    CMath::AbsF(M_PIF * CMath::FastCosR(x7d8_)));
    TakeDamage(CVector3f::Zero(), 0.f);
    x7d8_ += dt;
  }
}

void CFlaahgra::ApplyBombSlotDamage(CStateManager& mgr) {
  const CWeaponMode bombMode = CWeaponMode::Bomb();
  HealthInfo(mgr)->SetHP(HealthInfo(mgr)->GetHP() - mData.GetRetreatTime());
  mFaintTime = mData.GetFaintToSmallHP();
  mGetup = true;
  x7d8_ = 0.f;
  mDamageColor = skFlaahgraFaintColor;
  ++mStage;
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
      (mGenerateEndCooldown <= 0.f && player.GetVelocityWR().MagSquared() >= 25.f)) {
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
      player.GetAimPosition(mgr, scale * mBodyController->GetAnimTimeRemaining());
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
  return mBodyController->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_ProjectileAttack;
}

void CFlaahgra::UpdateAimPosition(CStateManager& mgr, float dt) {
  if (const CCollisionActor* head =
          TCastToConstPtr< CCollisionActor >(mgr.GetObjectById(mHeadActor))) {
    pas::EAnimationState state = mBodyController->GetBodyStateInfo().GetCurrentStateId();
    if (state != pas::kAS_GroundHit && state != pas::kAS_LieOnGround) {
      CVector3f target = CVector3f::Zero();
      if (x780_ == 0 || x8e4_28_ || IsFiringProjectile() || IsPlantStrikeAttack()) {
        target = head->GetTranslation();
      } else {
        target =
            GetTranslation() + CVector3f(0.f, 0.f, 3.7675f) +
            CVector3f::ByElementMultiply(CVector3f(0.f, 0.f, 4.155f), GetModelData()->GetScale());
      }
      CVector3f diff = target - mAimPosition;
      if (diff.CanBeNormalized()) {
        float mag = diff.Magnitude();
        float step = 125.f * dt;
        if (mag > step) {
          mAimPosition += step * ((1.f / mag) * diff);
        } else {
          mAimPosition = target;
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
  mMirrorWaypoints.clear();
  const rstl::vector< SConnection >& conns = GetConnectionList();
  for (AUTO(it, conns.begin()); it != conns.end(); ++it) {
    if (it->mState == kSS_Modify && it->mMsg == kSM_Follow) {
      TUniqueId id = mgr.GetIdForScript(it->mObjId);
      if (TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(id))) {
        mMirrorWaypoints.push_back(id);
      }
    }
  }
}

TUniqueId CFlaahgra::GetMirrorNearestPlayer(const CStateManager& mgr) const {
  TUniqueId nearest = kInvalidUniqueId;
  float prevMag = -1.f;
  CVector3f playerPos = mgr.GetPlayer()->GetTranslation();
  for (AUTO(it, mMirrorWaypoints.begin()); it != mMirrorWaypoints.end(); ++it) {
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
  mFallDirection = right;
  const rstl::reserved_vector< CVector3f, 4 >& dirs = x7ac_ == 0 ? x860_ : x82c_;
  for (AUTO(it, dirs.begin()); it != dirs.end(); ++it) {
    if (CVector3f::Dot(right, *it) >= 0.f) {
      float dot = CVector3f::Dot(front, *it);
      if (dot > bestDot) {
        mFallDirection = *it;
        bestDot = dot;
      }
    }
  }
}

CFlaahgraProjectile* CFlaahgra::CreateProjectile(const CTransform4f& xf, CStateManager& mgr) {
  CFlaahgraProjectile* projectile = nullptr;
  if (ProjectileInfo()->Token().TryCache() && mgr.CanCreateProjectile(GetUniqueId(), kWT_AI, 6)) {
    CDamageInfo damage = ProjectileInfo()->GetDamage();
    if (mStage >= 2) {
      damage.SetDamage(FLAAHGRA_STAGE_DAMAGE_MULTIPLIER * damage.GetDamage());
    }
    projectile =
        rs_new CFlaahgraProjectile(mBigStrike, ProjectileInfo()->Token(), xf, damage,
                                   mgr.AllocateUniqueId(), GetCurrentAreaId(), GetUniqueId());
    if (projectile != nullptr) {
      mgr.AddObject(projectile);
    }
  }
  return projectile;
}

void CFlaahgra::LoadTokens(CStateManager& mgr) {
  if (!mTokens.empty()) {
    for (AUTO(it, mTokens.begin()); it != mTokens.end(); ++it) {
      if (!it->IsLoaded()) {
        return;
      }
    }
    FinalizeLoad(mgr);
  }
  if (mDepGroup->IsLoaded()) {
    TLockedToken< CDependencyGroup > depGroup(*mDepGroup);
    const rstl::vector< SObjectTag >& tags = depGroup->GetObjectTagVector();
    if (tags.empty()) {
      FinalizeLoad(mgr);
      return;
    }
    if (mTokens.empty()) {
      mTokens.reserve(tags.size());
      for (int i = 0; i < tags.size(); ++i) {
        CToken token = gpSimplePool->GetObj(tags[i]);
        token.Lock();
        mTokens.push_back(token);
      }
    }
  }
}

void CFlaahgra::FinalizeLoad(CStateManager& mgr) {
  mLoaded = true;
  if (mLoading) {
    TAreaId area = GetCurrentAreaId();
    mgr.World()->Area(area)->GetPostConstructed()->mPlayerActorsLoading--;
    mLoading = false;
  }
  ResetModelDataAndBodyController();
}

void CFlaahgra::GatherAssets(CStateManager& mgr) {
  if (!mLoaded) {
    mDepGroup->GetObj();
    LoadTokens(mgr);
    if (!mLoaded) {
      for (AUTO(it, mTokens.begin()); it != mTokens.end(); ++it) {
        it->GetObj();
      }
      FinalizeLoad(mgr);
    }
  }
}

void CFlaahgra::LoadDependencies(CAssetId id) {
  if (id == kInvalidAssetId) {
    ResetModelDataAndBodyController();
    mLoaded = true;
  } else {
    mDepGroup = gpSimplePool->GetObj(SObjectTag('DGRP', id));
    mDepGroup->Lock();
  }
}

void CFlaahgra::ResetModelDataAndBodyController() {
  SetModelData(CModelData(x8ac_));
  CreateShadowIfNeeded();
  SetDrawShadow(true);
  Shadow()->SetAlwaysCalculateRadius(false);
  BuildBodyController(kBT_Restricted);
  mBoneTracking =
      rs_new CBoneTracking(*GetModelData()->GetAnimationData(), rstl::string_l("Head_1"),
                           CMath::Deg2Rad(80.f), CMath::Deg2Rad(180.f), kBTF_None);
}

CFlaahgraRenderer::CFlaahgraRenderer(TUniqueId uid, TUniqueId owner, const rstl::string& name,
                                     const CEntityInfo& info, const CTransform4f& xf)
: CActor(uid, true, name, info, xf, CModelData::CModelDataNull(), CMaterialList(kMT_Character),
         CActorParameters::None(), kInvalidUniqueId)
, mOwner(owner) {}

ENTITY_ACCEPT_IMPL(CFlaahgraRenderer)

void CFlaahgraRenderer::AddToRenderer(const CFrustumPlanes& frustum,
                                      const CStateManager& mgr) const {
  if (const CActor* owner = static_cast< const CActor* >(mgr.GetObjectById(mOwner))) {
    if (!owner->NullModel()) {
      owner->GetModelData()->RenderParticles(frustum);
    }
  }
}

rstl::optional_object< CAABox > CFlaahgraRenderer::GetTouchBounds() const {
  return rstl::optional_object< CAABox >();
}

CFlaahgraRenderer::~CFlaahgraRenderer() {}
