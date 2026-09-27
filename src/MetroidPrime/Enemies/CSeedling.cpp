#include "MetroidPrime/Enemies/CSeedling.hpp"

#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"

const char* const CSeedling::skSpikeLocators[12] = {
    "A_spike1_LCTR_SDK", "A_spike2_LCTR_SDK", "A_spike3_LCTR_SDK", "A_spike4_LCTR_SDK",
    "A_spike5_LCTR_SDK", "A_spike6_LCTR_SDK", "B_spike1_LCTR_SDK", "B_spike2_LCTR_SDK",
    "B_spike3_LCTR_SDK", "B_spike4_LCTR_SDK", "B_spike5_LCTR_SDK", "B_spike6_LCTR_SDK",
};

CSeedling::CSeedling(const TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                     const CTransform4f& xf, const CModelData& mData, const CPatternedInfo& pInfo,
                     const CActorParameters& actParms, const CAssetId& needleModel,
                     const CAssetId& weaponId, const CDamageInfo& projectileDamage,
                     const CDamageInfo& deathDamage, const float f1, const float f2, const float f3,
                     const float f4)
: CWallWalker(kC_Seedling, uid, name, kFT_Zero, info, xf, mData, pInfo, kMT_Flyer, kCT_Zero,
              kBT_WallWalker, actParms, kCS_Small, f3, kWT_Seedling, false, f2, f1, f4)
, mSearchPath(nullptr, 1, pInfo.GetPathfindingIndex(), 1.f, 1.f)
, mSpikeData(rs_new CModelData(CStaticRes(needleModel, mData.ScaleCopy())))
, mProjectileInfo(weaponId, projectileDamage)
, mDeathDamage(deathDamage)
, mModelBounds(CAABox::MakeNullBox())
, mAttackCoolOff(0.f)
, mPrevObj(kInvalidUniqueId)
, mRenderOnlyClusterA(true)
, mCurNeedleCluster(false) {
  mProjectileInfo.Token().Lock();
  SetDrawShadow(false);
  MakeThermalColdAndHot();
}

CSeedling::~CSeedling() {}

void CSeedling::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  CPatterned::AcceptScriptMsg(msg, uid, mgr);

  switch (msg) {
  case kSM_Registered:
    BodyCtrl()->Activate(mgr);
    mModelBounds = GetModelData()->GetBounds();
    break;
  case kSM_InitializedInArea: {
    const TAreaId aid = GetCurrentAreaId();
    mSearchPath.SetArea(
        mgr.GetWorld()->GetAreaAlways(aid).GetPostConstructed()->mPathArea);
  } break;
  case kSM_Activate: {
    mDisableMove = false;
    const TUniqueId id = GetConnectedObject(mgr, kSS_Patrol, kSM_Follow);
    if (id != kInvalidUniqueId) {
      mDestObj = id;
    }
  } break;
  }
}

void CSeedling::Render(const CStateManager& mgr) const {
  if (IsAlive() && !mSpikeData.null()) {
    const int start = mRenderOnlyClusterA ? 0 : mCurNeedleCluster ? 6 : 0;
    const int max = mRenderOnlyClusterA ? 12 : 6;

    for (int i = 0; i < max; i++) {
      CTransform4f lctrXf(GetLctrTransform(rstl::string_l(skSpikeLocators[i + start])));
      mSpikeData->Render(mgr, lctrXf, GetActorLights(), CModelFlags::Normal());
    }
  }
  CWallWalker::Render(mgr);
}

void CSeedling::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  ++mThinkCounter;
  mPlayerObstructed = false;

  if (mgr.GetWorld()->GetArea(GetCurrentAreaId())->GetOcclusionState() != CGameArea::kOS_Visible) {
    mPlayerObstructed = true;
  }

  if (!mPlayerObstructed) {
    const CVector3f playerPos = mgr.GetPlayer()->GetTranslation();
    const float distance = (playerPos - GetTranslation()).Magnitude();
    if (distance > mPlayerObstructionMinDist) {
      CRayCastResult result =
          mgr.RayStaticIntersection(playerPos, (GetTranslation() - playerPos).AsNormalized(),
                                    distance, CMaterialFilter::skPassEverything);
      if (result.IsValid()) {
        mPlayerObstructed = true;
      }
    }
  }

  if (mPlayerObstructed) {
    mMovable = false;
  } else {
    mMovable = mAlignToFloor == false ? 1 : 0;
    CWallWalker::Think(dt, mgr);
    if (!mDisableMove && close_enough(mBodyController->GetPercentageFrozen(), 0.f) &&
        mAlignToFloor) {
      AlignToFloor(mgr, mColSphere.GetSphere().GetRadius(),
                   GetTranslation() + (2.f * (dt * GetVelocityWR())), dt);
    }

    if (mAttackCoolOff > 0.f) {
      mAttackCoolOff -= dt;
    }
  }
}

void CSeedling::Patrol(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate: {
    BodyCtrl()->SetLocomotionType(pas::kLT_Relaxed);
    mAlignToFloor = true;
    mMomentum = CVector3f::Zero();
    mHasAlignSurface = false;
    SetMovable(false);
    const TUniqueId id =
        (mPrevObj != kInvalidUniqueId ? mPrevObj
                                          : GetConnectedObject(mgr, kSS_Patrol, kSM_Follow));
    if (id != kInvalidUniqueId) {
      mDestObj = id;
    }
  } break;
  case kStateMsg_Update: {
    GotoNextWaypoint(mgr);
    const CVector3f upVec = GetTransform().GetUp();
    CVector3f direction = mDestPos - GetTranslation();
    direction.Normalize();
    BodyCtrl()->CommandMgr().DeliverCmd(
        CBCLocomotionCmd(ProjectVectorToPlane(direction, upVec), CVector3f::Zero(), 0.f));
    const CVector3f direction2 =
        1.f * ProjectVectorToPlane(mSteeringBehaviors.Seek(*this, mDestPos), upVec);
    BodyCtrl()->CommandMgr().DeliverCmd(
        CBCLocomotionCmd(ProjectVectorToPlane(direction2, upVec), CVector3f::Zero(), 1.f));
    BodyCtrl()->CommandMgr().DeliverCmd(
        CBCLocomotionCmd(1.f * GetTransform().GetForward(), CVector3f::Zero(), 0.f));
  } break;
  case kStateMsg_Deactivate:
    mPrevObj = mDestObj;
    break;
  }
}

void CSeedling::Active(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyCtrl()->SetLocomotionType(pas::kLT_Lurk);
    break;
  case kStateMsg_Update:
    break;
  case kStateMsg_Deactivate:
    break;
  }

  CPatterned::Patrol(mgr, msg, arg);
}

void CSeedling::Enraged(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyCtrl()->SetLocomotionType(pas::kLT_Internal8);
    break;
  case kStateMsg_Update:
    break;
  case kStateMsg_Deactivate:
    break;
  }
}

void CSeedling::ProjectileAttack(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimState = kAS_Ready;
    break;
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_ProjectileAttack, &CPatterned::TryProjectileAttack, 0);
    break;
  case kStateMsg_Deactivate:
    mAnimState = kAS_NotReady;
    mAttackCoolOff = mAttackTimeVariation * mgr.Random()->Float() + mAverageAttackTime;
    break;
  }
}

void CSeedling::Generate(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimState = kAS_Ready;
    break;
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_Generate, &CPatterned::TryGenerate, 0);
    break;
  case kStateMsg_Deactivate:
    mAnimState = kAS_NotReady;
    break;
  }
}

CProjectileInfo* CSeedling::ProjectileInfo() { return &mProjectileInfo; }

void CSeedling::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                                float dt) {
  bool skip = false;
  switch (type) {
  case kUE_BeginAction:
    skip = true;
    mRenderOnlyClusterA = true;
    break;
  case kUE_Projectile:
    FireSpikes(mgr);
    skip = true;
    break;
  default:
    break;
  }

  if (!skip) {
    CPatterned::DoUserAnimEvent(mgr, node, type, dt);
  }
}

void CSeedling::FireSpikes(CStateManager& mgr) {
  int start = mCurNeedleCluster ? 0 : 6;
  for (int i = 0; i < 6; i++) {
    const CTransform4f locatorXf(GetLctrTransform(rstl::string_l(skSpikeLocators[i + start])));
    LaunchProjectile(locatorXf, mgr, 6, CWeapon::kPA_None, true, rstl::optional_object_null(),
                     CSfxManager::kInternalInvalidSfxId, false, GetModelScale());
  }

  mCurNeedleCluster = !mCurNeedleCluster;
  mRenderOnlyClusterA = false;
}

bool CSeedling::ShouldAttack(CStateManager& mgr, float arg) {
  return mAttackCoolOff <= 0.f && mgr.CanCreateProjectile(GetUniqueId(), kWT_AI, 6);
}

rstl::optional_object< CAABox > CSeedling::GetTouchBounds() const {
  return mModelBounds.GetTransformedAABox(GetTransform());
}

void CSeedling::Touch(CActor& actor, CStateManager& mgr) {
  if (IsAlive() && TCastToPtr< CPlayer >(actor)) {
    MassiveDeath(mgr);
  }

  CPatterned::Touch(actor, mgr);
}

void CSeedling::MassiveDeath(CStateManager& mgr) {
  if (IsAlive()) {
    mgr.ApplyDamageToWorld(
        GetUniqueId(), *this, GetTranslation(), mDeathDamage,
        CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()));
    FireSpikes(mgr);
  }
  CPatterned::MassiveDeath(mgr);
}