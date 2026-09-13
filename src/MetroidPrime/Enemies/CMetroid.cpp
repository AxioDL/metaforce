#include "MetroidPrime/Enemies/CMetroid.hpp"
#include "Collision/CRayCastResult.hpp"
#include "Kyoto/Animation/CVertexMorphEffect.hpp"
#include "Kyoto/Animation/CharacterCommon.hpp"
#include "Kyoto/Basics/CCast.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "MetroidPrime/BodyState/CBodyState.hpp"
#include "MetroidPrime/CAnimRes.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Cameras/CFirstPersonCamera.hpp"
#include "MetroidPrime/Enemies/CMetroidBeta.hpp"
#include "MetroidPrime/Enemies/CSpacePirate.hpp"
#include "MetroidPrime/Enemies/CTeamAiMgr.hpp"
#include "MetroidPrime/Player/CMorphBall.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CGameProjectile.hpp"

#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Animation/CSegId.hpp"

static EMaterialTypes skSolidMaterial = kMT_Solid;

const uint CMetroidData::skNumProperties = 20;

const CDamageVulnerability
    CMetroid::skGammaRedDamageVulnerability(kVN_Deflect, kVN_Deflect, kVN_Deflect, kVN_Immune,
                                            kVN_Deflect, kVN_Normal, kVN_Deflect, kVN_Deflect,
                                            kVN_Normal, kVN_Deflect, kVN_Deflect, kVN_Deflect,
                                            kVN_Deflect, kVN_Deflect, kVN_Deflect, kDT_None);

const CDamageVulnerability
    CMetroid::skGammaWhiteDamageVulnerability(kVN_Deflect, kVN_Immune, kVN_Deflect, kVN_Deflect,
                                              kVN_Deflect, kVN_Normal, kVN_Deflect, kVN_Deflect,
                                              kVN_Normal, kVN_Deflect, kVN_Deflect, kVN_Deflect,
                                              kVN_Deflect, kVN_Deflect, kVN_Deflect, kDT_None);
const CDamageVulnerability
    CMetroid::skGammaPurpleDamageVulnerability(kVN_Deflect, kVN_Deflect, kVN_Immune, kVN_Deflect,
                                               kVN_Deflect, kVN_Normal, kVN_Deflect, kVN_Deflect,
                                               kVN_Normal, kVN_Deflect, kVN_Deflect, kVN_Deflect,
                                               kVN_Deflect, kVN_Deflect, kVN_Deflect, kDT_None);

const CDamageVulnerability
    CMetroid::skGammaYellowDamageVulnerability(kVN_Immune, kVN_Deflect, kVN_Deflect, kVN_Deflect,
                                               kVN_Deflect, kVN_Normal, kVN_Deflect, kVN_Deflect,
                                               kVN_Normal, kVN_Deflect, kVN_Deflect, kVN_Deflect,
                                               kVN_Deflect, kVN_Deflect, kVN_Deflect, kDT_None);
const CDamageVulnerability
    CMetroid::skStandingFaceHugVulnerability(kVN_Deflect, kVN_Deflect, kVN_Deflect, kVN_Deflect,
                                             kVN_Deflect, kVN_Normal, kVN_Deflect, kVN_Deflect,
                                             kVN_Deflect, kVN_Deflect, kVN_Deflect, kVN_Deflect,
                                             kVN_Deflect, kVN_Deflect, kVN_Deflect, kDT_None);

const CColor CMetroid::skGammaRedColorMod(static_cast< uchar >(254), 0, 40);
const CColor CMetroid::skGammaWhiteColorMod(static_cast< uchar >(216), 216, 216);
const CColor CMetroid::skGammaPurpleColorMod(static_cast< uchar >(198), 14, 255);
const CColor CMetroid::skGammaYellowColorMod(static_cast< uchar >(253), 239, 86);

static const char* const skJointNameList[] = {
    "Head_1",        "L_ankle",    "L_elbow",       "L_hip",   "L_knee",  "L_shoulder",
    "L_varias2_SDK", "L_wrist",    "Pelvis",        "R_ankle", "R_elbow", "R_hip",
    "R_knee",        "R_shoulder", "R_varias2_SDK", "Spine_1", "Spine_2",
};
static const char* const skMetroidCenterJoint = "lockon_target_LCTR";
static const char* const skPirateSuckJoint = "Head_1";
static const char* const skPirateRootJoint = "Skeleton_Root";

static const float skGammaGrowthEnergy[] = {90.f, 190.f, 150.f, 90.f, 100.f};

CMetroidData::CMetroidData(CInputStream& in, int propCount)
: mFrozenVulnerability(in)
, mEnergyDrainVulnerability(in)
, mEnergyDrainPerSecond(in.Get< float >())
, mMaxEnergyDrainAllowed(in.Get< float >())
, mTelegraphAttackTime(in.Get< float >())
, mStage2GrowthScale(in.Get< float >())
, mStage2GrowthEnergy(in.Get< float >())
, mExplosionGrowthEnergy(in.Get< float >())
, mRedAnimation(rstl::optional_object< CAnimationParameters >(LoadAnimationParameters(in)))
, mWhiteAnimation(rstl::optional_object< CAnimationParameters >(LoadAnimationParameters(in)))
, mPurpleAnimation(rstl::optional_object< CAnimationParameters >(LoadAnimationParameters(in)))
, mYellowAnimation(rstl::optional_object< CAnimationParameters >(LoadAnimationParameters(in)))
, mStartsInWall(in.Get< bool >()) {}

uint CMetroidData::GetNumValidGammaModels() const {
  uint numValidGammaModels = 0;
  if (mRedAnimation) {
    numValidGammaModels++;
  }
  if (mWhiteAnimation) {
    numValidGammaModels++;
  }
  if (mPurpleAnimation) {
    numValidGammaModels++;
  }
  if (mYellowAnimation) {
    numValidGammaModels++;
  }
  return numValidGammaModels;
}

const rstl::optional_object< CAnimationParameters >&
CMetroidData::GetSpawnedGammaAnimParms(const EGammaFlavor flavor) const {
  switch (flavor) {
  case kGF_Red:
    return mRedAnimation;
  case kGF_White:
    return mWhiteAnimation;
  case kGF_Purple:
    return mPurpleAnimation;
  case kGF_Yellow:
    return mYellowAnimation;
  default:
    return mRedAnimation;
  }
}

CMetroid::CMetroid(const TUniqueId uid, const rstl::string& name, const EFlavorType flavor,
                   const CEntityInfo& info, const CTransform4f& xf, const CModelData& mData,
                   const CPatternedInfo& pInfo, const CActorParameters& aParms,
                   const CMetroidData& metroidData, const TUniqueId other)
: CPatterned(kC_Metroid, uid, name, flavor, info, xf, mData, pInfo, kMT_Flyer, kCT_One, kBT_Flyer,
             aParms, kCS_Medium)
, mState(kAiState_Invalid)
, mMetroidData(metroidData)
, mTeamAiManagerId(kInvalidUniqueId)
, mCollisionPrimitive(CSphere(CVector3f::Zero(), GetModelData()->ScaleCopy()[kDY] * 0.9f),
                      GetMaterialList())
, mPathFindSearch(nullptr, 1 | 2, pInfo.GetPathfindingIndex(), 1.f, 1.f)
, x7a4(CVector3f::Zero())
, mAttackTarget(kInvalidUniqueId)
, mAttackChance(0.f)
, mTelegraphAttackTime(0.f)
, mEnergyDrained(0.f)
, mEnergyDrainTime(0.f)
, mCollisionRestoreTime(0.f)
, mAttackState(kAttackState_None)
, mGammaType(flavor != kFT_Two ? CMetroidData::kGF_Invalid : CMetroidData::kGF_Normal)
, mScale1(ModelData()->ScaleCopy())
, mScale2(ModelData()->ScaleCopy())
, mScale3(ModelData()->ScaleCopy())
, mGrowthDuration(0.f)
, mGrowthEnergy(0.f)
, mLastGrowthEnergy(0.f)
, mSeekTime(0.f)
, mMaxSeekTime(0.f)
, mLoopAttackDistance(0.f)
, mDetachPos(CVector3f::Zero())
, mDodgeDirection(pas::kSD_Invalid)
, mPatternedInfo(pInfo)
, mActorParameters(aParms)
, mParent(other)
, x9be(0)
, mAlert(false)
, mGrowing(false)
, mShotAt(false)
, x9bf_27_(false)
, x9bf_28_(false)
, mIsAttacking(false)
, mRestoreCharacterCollision(false)
, mRestoreSolidCollision(false)
, mIsEnergyDrainVulnerable(false) {
  const CPASAnimParmData pasAnimParms(pas::kAS_LoopAttack, CPASAnimParm::FromEnum(2),
                                      CPASAnimParm::FromEnum(3));
  mLoopAttackDistance = GetAnimationDistance(pasAnimParms);

  UpdateTouchBounds();
  SetCoefficientOfRestitutionModifier(0.9f);
  KnockBackCtrl().SetX82_24(false);
  KnockBackCtrl().SetEnableBurn(false);
  KnockBackCtrl().SetEnableBurnDeath(false);
  KnockBackCtrl().SetEnableShock(false);
  if (GetFlavorType() == kFT_Two) {
    KnockBackCtrl().SetEnableFreeze(false);
  }
  mPatternedInfo.SetActive(true);
}

CMetroid::~CMetroid() {}

ENTITY_ACCEPT_IMPL(CMetroid)

void CMetroid::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  if (CTeamAiMgr::GetTeamAiRole(mgr, mTeamAiManagerId, GetUniqueId()) == nullptr) {
    SwarmAdd(mgr);
  }
  UpdateAILogicTimers(dt, mgr);
  SuckEnergyFromTarget(dt, mgr);
  PreventWorldCollisions(dt, mgr);
  UpdateTouchBounds();
  RestoreSolidCollision(mgr);
  CPatterned::Think(dt, mgr);
}

void CMetroid::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  CPatterned::AcceptScriptMsg(msg, uid, mgr);
  switch (msg) {
  case kSM_Registered:
    BodyCtrl()->Activate(mgr);
    UpdateSoundVolume();
    break;
  case kSM_Deleted:
  case kSM_Deactivate:
    SwarmRemove(mgr);
    break;
  case kSM_Damage:
  case kSM_InvulnDamage:
    if (const CGameProjectile* projectile =
            TCastToConstPtr< CGameProjectile >(mgr.GetObjectById(uid))) {
      if (GetDamageVulnerability()->WeaponHits(projectile->GetCurrentDamageInfo().GetWeaponMode(),
                                               CDamageVulnerability::kRD_No)) {
        ApplyGrowth(projectile->GetCurrentDamageInfo().GetDamage(), mgr);
      }
    }
    mAlert = true;
    break;
  case kSM_Alert:
    mAlert = true;
    break;
  case kSM_InitializedInArea:
    if (mTeamAiManagerId == kInvalidUniqueId) {
      mTeamAiManagerId = CTeamAiMgr::GetTeamAiMgr(*this, mgr);
    }
    {
      const TAreaId areaId = GetCurrentAreaId();
      mPathFindSearch.SetArea(
          mgr.GetWorld()->GetAreaAlways(areaId).GetPostConstructed()->x10bc_pathArea);
    }
    break;
  default:
    break;
  }
}

void CMetroid::Render(const CStateManager& mgr) const { CPatterned::Render(mgr); }

void CMetroid::Touch(CActor& actor, CStateManager& mgr) {
  if (!IsAlive()) {
    return;
  }
  if (CGameProjectile* projectile = TCastToPtr< CGameProjectile >(actor)) {
    if (projectile->GetOwnerId() == mgr.GetPlayer()->GetUniqueId()) {
      const CDamageVulnerability* vulnerability = GetDamageVulnerability();
      if (GetFlavorType() != kFT_Two && projectile->HasAttrib(CWeapon::kPA_Ice) &&
          vulnerability->WeaponHits(CWeaponMode::Ice(), CDamageVulnerability::kRD_No)) {
        float multiplier = 1.f;
        if (projectile->HasAttrib(CWeapon::kPA_Charged)) {
          multiplier = 2.f;
        }
        const CVector3f relativePos = projectile->GetTranslation() - GetTranslation();
        const CVector3f projectilePos = projectile->GetTranslation();
        const CUnitVector3f dir(
            GetTransform().TransposeRotate(projectilePos - projectile->GetPreviousPos()));
        Freeze(mgr, relativePos, dir, multiplier * GetFreezeDuration());
      }
    }
    mShotAt = true;
  }
  CPatterned::Touch(actor, mgr);
}

rstl::optional_object< CAABox > CMetroid::GetTouchBounds() const {
  return mCollisionPrimitive.CalculateAABox(GetTransform());
}

void CMetroid::UpdateTouchBounds() {
  const CTransform4f locXf = GetLocatorTransform(rstl::string_l(skMetroidCenterJoint));
  mCollisionPrimitive.SetSphereCenter(
      CVector3f::ByElementMultiply(locXf.GetTranslation(), GetModelData()->GetScale()));
}

void CMetroid::UpdateSoundVolume() {
  const float growth = CMath::Clamp(0.f, GetGrowthStage() - 1.f, 1.f);
  const float volume = 0.25f * growth + 0.75f;
  SetVolume(CMath::Clamp< uchar >(0, CCast::ToUint8(127.f * volume), 127));
}

const CCollisionPrimitive* CMetroid::GetCollisionPrimitive() const { return &mCollisionPrimitive; }

const CDamageVulnerability* CMetroid::GetDamageVulnerability() const {
  if (IsSuckingEnergy()) {
    if (mIsEnergyDrainVulnerable) {
      return &mMetroidData.GetEnergyDrainVulnerability();
    }
    return &skStandingFaceHugVulnerability;
  }
  if (mGrowing && !GetBodyCtrl()->IsFrozen()) {
    return &mMetroidData.GetEnergyDrainVulnerability();
  }
  if (GetBodyCtrl()->GetPercentageFrozen() > 0.f) {
    return &mMetroidData.GetFrozenVulnerability();
  }
  if (GetFlavorType() == kFT_Two) {
    switch (mGammaType) {
    case CMetroidData::kGF_Red:
      return &skGammaRedDamageVulnerability;
    case CMetroidData::kGF_White:
      return &skGammaWhiteDamageVulnerability;
    case CMetroidData::kGF_Purple:
      return &skGammaPurpleDamageVulnerability;
    case CMetroidData::kGF_Yellow:
      return &skGammaYellowDamageVulnerability;
    }
  }
  return CAi::GetDamageVulnerability();
}

const CDamageVulnerability* CMetroid::GetDamageVulnerability(const CVector3f&, const CVector3f&,
                                                             const CDamageInfo&) const {
  return GetDamageVulnerability();
}

EWeaponCollisionResponseTypes CMetroid::GetCollisionResponseType(const CVector3f&, const CVector3f&,
                                                                 const CWeaponMode& mode,
                                                                 int) const {
  EWeaponCollisionResponseTypes response = static_cast< EWeaponCollisionResponseTypes >(33);
  const bool frozen = GetBodyCtrl()->GetPercentageFrozen() > 0.f;
  if (!GetDamageVulnerability()->WeaponHurts(mode, CDamageVulnerability::kRD_No) && !frozen) {
    response = static_cast< EWeaponCollisionResponseTypes >(58);
  }
  return response;
}

void CMetroid::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                               float dt) {
  bool handled = false;
  switch (type) {
  case kUE_GenerateEnd:
    AddMaterial(kMT_Solid, mgr);
    handled = true;
    break;
  }
  if (!handled) {
    CPatterned::DoUserAnimEvent(mgr, node, type, dt);
  }
}

void CMetroid::KnockBack(const CVector3f& dir, CStateManager& mgr, const CDamageInfo& info,
                         float magnitude, bool direct, const bool inDeferred) {
  if (!IsAlive()) {
    return;
  }
  const CDamageVulnerability* vulnerability = GetDamageVulnerability();
  const bool frozen = BodyCtrl()->GetPercentageFrozen() > 0.f;
  const CWeaponMode& mode = info.GetWeaponMode();
  if (mAttackState == kAttackState_Draining) {
    if (vulnerability->WeaponHits(mode, CDamageVulnerability::kRD_No)) {
      mEnergyDrained = mMetroidData.GetMaxEnergyDrainAllowed() * GetDamageMultiplier();
    }
  } else if (vulnerability->WeaponHurts(mode, CDamageVulnerability::kRD_No)) {
    mAttackChance = x308_attackTimeVariation * mgr.Random()->Float() + x304_averageAttackTime;
    if (frozen) {
      BodyCtrl()->UnFreeze();
    }
    CPatterned::KnockBack(dir, mgr, info, magnitude, direct, inDeferred);
  } else if (!frozen && vulnerability->WeaponHits(mode, CDamageVulnerability::kRD_No) &&
             (mode.IsCharged() || mode.IsComboed() || mode.GetType() == kWT_Missile) &&
             !ShouldSpawnGammaMetroid()) {
    CPatterned::KnockBack(dir, mgr, info, magnitude, direct, false);
    mSeekTime = mMaxSeekTime;
  }
}

void CMetroid::Death(CStateManager& mgr, const CVector3f& direction, EScriptObjectState state) {
  CPatterned::Death(mgr, direction, state);
  SetVerticalMovement(false);
  SetMuted(true);
  SwarmRemove(mgr);
}

bool CMetroid::AnimOver(CStateManager& mgr, float arg) { return mState == kAiState_Over; }

bool CMetroid::AttackOver(CStateManager& mgr, float arg) {
  if (mState == kAiState_Two && !BodyCtrl()->IsFrozen()) {
    const CVector3f pos = GetTranslation();
    const float posZ = pos.GetZ();
    const CVector3f targetPos = GetAttackTargetPos(mgr);
    const float heightDifference = targetPos.GetZ() - posZ;
    const float absHeightDifference = CMath::AbsF(heightDifference);
    const float scale = 0.8f * GetModelData()->ScaleCopy().GetY();
    if (absHeightDifference < scale) {
      if (const CPhysicsActor* actor =
              TCastToConstPtr< CPhysicsActor >(mgr.GetObjectById(mAttackTarget))) {
        const CAABox actorBounds = actor->GetBoundingBox();
        CAABox collisionBounds = mCollisionPrimitive.CalculateAABox(GetTransform());
        const CVector3f min = collisionBounds.GetMinPoint() - CVector3f(scale, scale, scale);
        const CVector3f max = collisionBounds.GetMaxPoint() + CVector3f(scale, scale, scale);
        const CAABox scaledBounds(min, max);
        return scaledBounds.DoBoundsOverlap(actorBounds);
      }
    }
  }
  return false;
}

bool CMetroid::LostInterest(CStateManager& mgr, float arg) {
  if (mAttackTarget != kInvalidUniqueId) {
    if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(mAttackTarget))) {
      if (const CSpacePirate* pirate = CPatterned::CastTo< CSpacePirate >(
              TPatternedCast< CSpacePirate >(const_cast< CActor* >(actor)))) {
        if (pirate->GetAttachedActor() != kInvalidUniqueId || BodyCtrl()->HasBeenFrozen()) {
          return true;
        }
      } else if (mAttackTarget == mgr.GetPlayer()->GetUniqueId() &&
                 (IsPlayerInFluid(mgr) ||
                  mgr.GetPlayer()->GetCurrentAreaId() != GetCurrentAreaId())) {
        return true;
      }
      return false;
    }
  }
  return true;
}

bool CMetroid::PatternShagged(CStateManager& mgr, float arg) {
  if (mAttackTarget != kInvalidUniqueId) {
    if (const CSpacePirate* pirate =
            CPatterned::CastTo< CSpacePirate >(TPatternedCast< CSpacePirate >(
                const_cast< CEntity* >(mgr.GetObjectById(mAttackTarget))))) {
      if (!pirate->IsAlive()) {
        return true;
      }
    }
    if (!CanStartAttack(mgr)) {
      return true;
    }
    if (mState == kAiState_Two) {
      return mSeekTime >= mMaxSeekTime;
    }
    return false;
  }
  return true;
}

bool CMetroid::Attacked(CStateManager& mgr, float arg) {
  if (mGrowthEnergy - mLastGrowthEnergy > 0.f) {
    if (mLastGrowthEnergy < mMetroidData.GetStage2GrowthEnergy()) {
      return mGrowthEnergy >= mMetroidData.GetStage2GrowthEnergy();
    }
    if (mGrowthEnergy >= mMetroidData.GetExplosionGrowthEnergy()) {
      return true;
    }
  }
  return false;
}

bool CMetroid::ShotAt(CStateManager& mgr, float arg) { return mShotAt; }

bool CMetroid::ShouldAttack(CStateManager& mgr, float arg) {
  if (CanStartAttack(mgr)) {
    if (CTeamAiMgr* team = TCastToPtr< CTeamAiMgr >(mgr.ObjectById(mTeamAiManagerId))) {
      return team->AddMeleeAttacker(GetUniqueId());
    }
    return true;
  }
  return false;
}

bool CMetroid::InAttackPosition(CStateManager& mgr, float arg) {
  if (mAttackTarget != kInvalidUniqueId) {
    const CActor* actor = static_cast< const CActor* >(mgr.GetObjectById(mAttackTarget));
    if (actor != nullptr && actor->GetCurrentAreaId() == GetCurrentAreaId()) {
      const CPlayer& player = *mgr.GetPlayer();
      const CVector3f direction = GetTranslation() - actor->GetTranslation();
      const CVector3f actorForward = actor->GetTransform().GetForward();
      float maxAngle = M_PIF;
      if (player.GetUniqueId() == mAttackTarget) {
        if (IsPlayerInFluid(mgr)) {
          return false;
        }
        if (player.GetMorphballTransitionState() != CPlayer::kMS_Morphed && !x9bf_28_) {
          maxAngle = CRelAngle::FromDegrees(45.f).AsRadians();
        }
      }
      if (CVector2f::GetAngleDiff(direction.ToVec2f(), actorForward.ToVec2f()) < maxAngle &&
          CVector3f::Dot(direction, GetTransform().GetForward()) < 0.f) {
        bool inPosition =
            (x7a4 - GetTranslation()).MagSquared() < x300_maxAttackRange * x300_maxAttackRange;
        if (inPosition) {
          const float myZ = GetTranslation()[kDZ];
          const float targetZ = actor->GetTranslation()[kDZ];
          inPosition = myZ > targetZ && myZ < 0.5f + x7a4[kDZ];
          if (inPosition) {
            const CVector3f start = GetTranslation();
            const CVector3f attackDelta = GetAttackTargetPos(mgr) - start;
            if (attackDelta.CanBeNormalized()) {
              const CMaterialFilter filter =
                  CMaterialFilter::MakeInclude(CMaterialList(kMT_Solid, kMT_AIBlock));
              const float length = attackDelta.Magnitude();
              const CVector3f rayDirection = (1.f / length) * attackDelta;
              const CRayCastResult result =
                  mgr.RayStaticIntersection(start, rayDirection, length, filter);
              inPosition = !result.IsValid();
            }
          }
        }
        return inPosition;
      }
    }
  }
  return false;
}

bool CMetroid::InPosition(CStateManager& mgr, float arg) {
  if (GetSearchPath() != nullptr) {
    const CPathFindSearch* search = GetSearchPath();
    return search->GetCurrentWaypoint() >= search->GetWaypoints().size() - 1;
  }
  const CVector3f delta = x7a4 - GetTranslation();
  return delta.MagSquared() < 4.f;
}

bool CMetroid::InRange(CStateManager& mgr, float arg) {
  if (mAttackTarget != kInvalidUniqueId) {
    if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(mAttackTarget))) {
      if (const CSpacePirate* pirate = CPatterned::CastTo< CSpacePirate >(
              TPatternedCast< CSpacePirate >(const_cast< CActor* >(actor)))) {
        if (!IsPirateValidTarget(*pirate, mgr)) {
          return false;
        }
      }
      return (actor->GetTranslation() - GetTranslation()).MagSquared() <
             x300_maxAttackRange * x300_maxAttackRange;
    }
  }
  return false;
}

bool CMetroid::InDetectionRange(CStateManager& mgr, float arg) {
  if (mAttackTarget == kInvalidUniqueId) {
    if ((mAlert || CPatterned::InDetectionRange(mgr, arg)) && !IsPlayerInFluid(mgr) &&
        mgr.GetPlayer()->GetCurrentAreaId() == GetCurrentAreaId()) {
      return true;
    }
    const float range = x3bc_detectionRange;
    const CVector3f position = GetTranslation();
    const float x = position.GetX(), y = position.GetY(), z = position.GetZ();
    const CAABox bounds(CVector3f(x - range, y - range, z - range),
                        CVector3f(x + range, y + range, z + range));
    const CMaterialFilter filter = CMaterialFilter::MakeInclude(CMaterialList(kMT_Character));
    TEntityList nearList;
    mgr.BuildNearList(nearList, bounds, filter, nullptr);
    if (!nearList.empty()) {
      for (AUTO(it, nearList.begin()); it != nearList.end(); ++it) {
        if (CSpacePirate* pirate = CPatterned::CastTo< CSpacePirate >(
                TPatternedCast< CSpacePirate >(mgr.ObjectById(*it)))) {
          if (IsPirateValidTarget(*pirate, mgr)) {
            pirate->SetAttackTarget(GetUniqueId());
            return true;
          }
        }
      }
    }
  } else {
    if (mAttackTarget == mgr.GetPlayer()->GetUniqueId() &&
        (IsPlayerInFluid(mgr) || mgr.GetPlayer()->GetCurrentAreaId() != GetCurrentAreaId())) {
      return false;
    }
    if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(mAttackTarget))) {
      const CVector3f delta = actor->GetTranslation() - GetTranslation();
      const float heightRange = x3c0_detectionHeightRange;
      if (delta.MagSquared() < x3bc_detectionRange * x3bc_detectionRange && heightRange > 0.f) {
        return delta.GetZ() * delta.GetZ() < heightRange * heightRange;
      }
    }
  }
  return false;
}

bool CMetroid::SpotPlayer(CStateManager& mgr, float arg) {
  if (!IsPlayerInFluid(mgr) && mgr.GetPlayer()->GetCurrentAreaId() == GetCurrentAreaId()) {
    if (mAttackTarget == kInvalidUniqueId) {
      const float rangeSquared = x3bc_detectionRange * x3bc_detectionRange;
      const TUniqueId playerId = mgr.GetPlayer()->GetUniqueId();
      if (CTeamAiMgr* const team = TCastToPtr< CTeamAiMgr >(mgr.ObjectById(mTeamAiManagerId))) {
        rstl::vector< CTeamAiRole >& roles = team->GetTeamAiRoles();
        for (AUTO(it, roles.begin()); it != roles.end(); ++it) {
          if (it->GetOwnerId() != GetUniqueId()) {
            if (const CMetroid* other = CPatterned::CastTo< CMetroid >(TPatternedCast< CMetroid >(
                    const_cast< CEntity* >(mgr.GetObjectById(it->GetOwnerId()))))) {
              if (other->GetAttackTargetId() == playerId) {
                const CVector3f delta = other->GetTranslation() - GetTranslation();
                if (delta.MagSquared() < rangeSquared) {
                  return true;
                }
              }
            }
          }
        }
      }
      return false;
    }
    return mgr.GetPlayer()->GetUniqueId() == mAttackTarget;
  }
  return false;
}

bool CMetroid::AggressionCheck(CStateManager& mgr, float arg) {
  if (mAttackTarget != kInvalidUniqueId) {
    const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(mAttackTarget));
    if (const CSpacePirate* pirate = CPatterned::CastTo< CSpacePirate >(
            TPatternedCast< CSpacePirate >(const_cast< CActor* >(actor)))) {
      if (!IsPirateValidTarget(*pirate, mgr)) {
        mAttackTarget = kInvalidUniqueId;
        return false;
      }
    }
    if (actor != nullptr) {
      const CVector3f delta = actor->GetTranslation() - GetTranslation();
      if (delta.MagSquared() < x3bc_detectionRange * x3bc_detectionRange) {
        if (x3c0_detectionHeightRange > 0.f) {
          return delta.GetZ() * delta.GetZ() <
                 x3c0_detectionHeightRange * x3c0_detectionHeightRange;
        }
        return true;
      }
    } else {
      mAttackTarget = kInvalidUniqueId;
    }
  }
  return false;
}

bool CMetroid::ShouldTurn(CStateManager& mgr, float arg) {
  if (mAttackTarget != kInvalidUniqueId) {
    if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(mAttackTarget))) {
      const CTransform4f& xf = GetTransform();
      const CVector2f direction = (actor->GetTranslation() - xf.GetTranslation()).ToVec2f();
      const CVector2f forward = xf.GetForward().ToVec2f();
      const float angle = CVector2f::GetAngleDiff(forward, direction);
      return angle > CRelAngle::FromDegrees(15.f).AsRadians();
    }
  }
  return false;
}

bool CMetroid::Leash(CStateManager& mgr, float arg) {
  if (mAttackTarget == mgr.GetPlayer()->GetUniqueId() && IsPlayerInFluid(mgr)) {
    return true;
  }
  const CVector3f leashDelta = x3a0_latestLeashPosition - GetTranslation();
  if (leashDelta.MagSquared() > x3c8_leashRadius * x3c8_leashRadius) {
    if (mAttackTarget != kInvalidUniqueId) {
      if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(mAttackTarget))) {
        const CVector3f targetDelta = actor->GetTranslation() - GetTranslation();
        return targetDelta.MagSquared() > x3cc_playerLeashRadius * x3cc_playerLeashRadius &&
               x3d4_curPlayerLeashTime > x3d0_playerLeashTime;
      }
    }
    return true;
  }
  return false;
}

bool CMetroid::ShouldWallHang(CStateManager& mgr, float arg) {
  return mMetroidData.GetStartsInWall();
}

bool CMetroid::Inside(CStateManager& mgr, float arg) {
  if (mParent != kInvalidUniqueId) {
    if (const CMetroid* other = CPatterned::CastTo< CMetroid >(
            TPatternedCast< CMetroid >(const_cast< CEntity* >(mgr.GetObjectById(mParent))))) {
      const float radius = arg > 0.f ? arg * mCollisionPrimitive.GetSphere().GetRadius()
                                     : mCollisionPrimitive.GetSphere().GetRadius();
      const CVector3f delta = other->GetTranslation() - GetTranslation();
      if (delta.MagSquared() < radius * radius) {
        return true;
      }
    }
  }
  return false;
}

bool CMetroid::ShouldDodge(CStateManager& mgr, float arg) {
  if (x3fc_flavor != kFT_Two && mAttackTarget == mgr.GetPlayer()->GetUniqueId() &&
      GetCurrentAreaId() == mgr.GetPlayer()->GetCurrentAreaId()) {
    const CTeamAiRole* role = CTeamAiMgr::GetTeamAiRole(mgr, mTeamAiManagerId, GetUniqueId());
    if (role != nullptr && role->GetTeamAiRole() == CTeamAiRole::kTAR_Melee) {
      const CTransform4f& xf = GetTransform();
      const CVector3f position = xf.GetTranslation();
      float range = 9.f;
      const float x = position.GetX(), y = position.GetY(), z = position.GetZ();
      const CAABox bounds(CVector3f(x - range, y - range, z - range),
                          CVector3f(x + range, y + range, z + range));
      TEntityList nearList;
      const CMaterialFilter filter = CMaterialFilter::MakeInclude(CMaterialList(kMT_Projectile));
      mgr.BuildNearList(nearList, bounds, filter, nullptr);
      if (nearList.size() > 0) {
        const CVector3f forward = xf.GetForward();
        for (AUTO(it, nearList.begin()); it != nearList.end(); ++it) {
          if (const CGameProjectile* projectile =
                  TCastToConstPtr< CGameProjectile >(mgr.GetObjectById(*it))) {
            if (projectile->HasAttrib(CWeapon::kPA_Ice)) {
              const CVector3f direction = projectile->GetTranslation() - xf.GetTranslation();
              if (CVector3f::GetAngleDiff(forward, direction) <
                  CRelAngle::FromDegrees(10.f).AsRadians()) {
                mDodgeDirection =
                    CVector3f::Dot(xf.GetRight(), direction) > 0.f ? pas::kSD_Right : pas::kSD_Left;
                return true;
              }
            }
          }
        }
      }
    }
  }
  return false;
}

void CMetroid::Patrol(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyCtrl()->SetLocomotionType(pas::kLT_Relaxed);
    mAttackTarget = kInvalidUniqueId;
    mShotAt = false;
    break;
  }
  CPatterned::Patrol(mgr, msg, dt);
}

void CMetroid::TargetPatrol(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyCtrl()->SetLocomotionType(pas::kLT_Relaxed);
    mAttackTarget = kInvalidUniqueId;
    mShotAt = false;
    if (HasPatrolPath(mgr, 0.f)) {
      CPatterned::Patrol(mgr, msg, dt);
      CPatterned::UpdateDest(mgr);
    } else {
      SetDestPos(x3a0_latestLeashPosition);
    }
    x7a4 = x2e0_destPos;
    if (GetSearchPath() != nullptr) {
      CPatterned::PathFind(mgr, msg, dt);
    }
    break;
  case kStateMsg_Update:
    if (GetSearchPath() != nullptr && !PathShagged(mgr, 0.f)) {
      CPatterned::PathFind(mgr, msg, dt);
    } else {
      CPatterned::Patrol(mgr, msg, dt);
    }
    ApplySeparationBehavior(mgr, 9.f);
    break;
  }
}

void CMetroid::Generate(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    if (ShouldSpawnGammaMetroid()) {
      SpawnGammaMetroid(mgr);
    } else if (mGrowthEnergy >= mMetroidData.GetExplosionGrowthEnergy()) {
      MassiveDeath(mgr);
    }
    mState = kAiState_One;
    mScale2 = GetModelData()->ScaleCopy();
    mGrowing = true;
    break;
  case kStateMsg_Update:
    switch (mState) {
    case kAiState_One:
      if (BodyCtrl()->GetCurrentStateId() == pas::kAS_Generate) {
        mGrowthDuration = BodyCtrl()->GetAnimTimeRemaining();
        mState = mGrowthDuration > 0.f ? kAiState_Two : kAiState_Over;
      } else if (Attacked(mgr, 0.f)) {
        BodyCtrl()->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_Two, CVector3f::Zero()));
      } else {
        BodyCtrl()->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_Seven, CVector3f::Zero()));
      }
      break;
    case kAiState_Two:
      if (BodyCtrl()->GetCurrentStateId() != pas::kAS_Generate) {
        mState = kAiState_Over;
      } else {
        if (!BodyCtrl()->IsFrozen()) {
          const bool inside = Inside(mgr, 3.f);
          const bool attacked = Attacked(mgr, 0.f);
          if (attacked || inside) {
            const float timeRemaining = BodyCtrl()->GetAnimTimeRemaining();
            const float t = CMath::Clamp(0.f, 1.f - timeRemaining / mGrowthDuration, 1.f);
            CVector3f scale = CVector3f::Zero();
            if (t < 0.25f) {
              scale = CMath::Clamp(0.f, 1.f - 0.5f * (t / 0.25f), 1.f) * mScale2;
            } else {
              const float duration = 0.75f * mGrowthDuration;
              const CVector3f halfScale = 0.5f * mScale2;
              scale = halfScale + (duration - timeRemaining) * ((mScale1 - halfScale) / duration);
            }
            if (inside) {
              ApplySplitGammas(mgr, dt);
            }
            ModelData()->SetScale(scale);
            UpdateSoundVolume();
          }
        }
      }
      break;
    }
    break;
  case kStateMsg_Deactivate:
    mGrowthDuration = 0.f;
    mGrowing = false;
    if (Attacked(mgr, 0.f)) {
      mLastGrowthEnergy = mGrowthEnergy;
      ModelData()->SetScale(mScale1);
    }
    UpdateSoundVolume();
    break;
  }
}

void CMetroid::SelectTarget(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mState = x9bf_27_ ? kAiState_Over : kAiState_One;
    if (mAttackTarget == kInvalidUniqueId) {
      const CPlayer& player = *mgr.GetPlayer();
      const CVector3f playerDelta = player.GetTranslation() - GetTranslation();
      float closestDistance = playerDelta.MagSquared();
      mAttackTarget = player.GetUniqueId();
      if (!BodyCtrl()->HasBeenFrozen()) {
        float range = CMath::Max(GetDetectionRange(), CMath::SqrtF(closestDistance));
        const CVector3f pos = GetTranslation();
        const float x = pos.GetX(), y = pos.GetY(), z = pos.GetZ();
        const CAABox bounds(CVector3f(x - range, y - range, z - range),
                            CVector3f(x + range, y + range, z + range));
        const CMaterialFilter filter = CMaterialFilter::MakeInclude(CMaterialList(kMT_Character));
        TEntityList nearList;
        mgr.BuildNearList(nearList, bounds, filter, nullptr);
        if (!nearList.empty()) {
          CSpacePirate* closestPirate = nullptr;
          for (AUTO(it, nearList.begin()); it != nearList.end(); ++it) {
            if (CSpacePirate* pirate = CPatterned::CastTo< CSpacePirate >(
                    TPatternedCast< CSpacePirate >(mgr.ObjectById(*it)))) {
              if (IsPirateValidTarget(*pirate, mgr)) {
                const CVector3f delta = pirate->GetTranslation() - GetTranslation();
                const float distance = delta.MagSquared();
                if (distance < closestDistance) {
                  closestPirate = pirate;
                  closestDistance = distance;
                }
              }
            }
          }
          if (closestPirate != nullptr) {
            mAttackTarget = closestPirate->GetUniqueId();
            closestPirate->SetAttackTarget(GetUniqueId());
          }
        }
      }
    }
    if (CSpacePirate* pirate = CPatterned::CastTo< CSpacePirate >(
            TPatternedCast< CSpacePirate >(mgr.ObjectById(mAttackTarget)))) {
      mgr.DeliverScriptMsg(pirate, GetUniqueId(), kSM_Alert);
    }
    break;
  case kStateMsg_Update:
    switch (mState) {
    case kAiState_One:
      if (BodyCtrl()->GetCurrentStateId() == pas::kAS_Generate) {
        mState = kAiState_Two;
      } else {
        BodyCtrl()->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_Three, CVector3f::Zero()));
      }
      break;
    case kAiState_Two:
      if (BodyCtrl()->GetCurrentStateId() != pas::kAS_Generate) {
        mState = kAiState_Over;
      } else {
        if (mAttackTarget != kInvalidUniqueId) {
          if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(mAttackTarget))) {
            BodyCtrl()->CommandMgr().DeliverTargetVector(actor->GetTranslation() -
                                                         GetTranslation());
          }
        }
      }
      break;
    }
    break;
  case kStateMsg_Deactivate:
    break;
  }
}

void CMetroid::PathFind(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyCtrl()->SetLocomotionType(pas::kLT_Lurk);
    BodyCtrl()->CommandMgr().SetSteeringBlendMode(kSBM_FullSpeed);
    BodyCtrl()->CommandMgr().SetSteeringSpeedRange(1.f, 1.f);
    SetUpPathFindBehavior(mgr);
    break;
  case kStateMsg_Update:
    if (GetSearchPath() != nullptr && !PathShagged(mgr, 0.f) && !PathOver(mgr, 0.f)) {
      CPatterned::PathFind(mgr, msg, dt);
    } else {
      if (const CTeamAiRole* role =
              CTeamAiMgr::GetTeamAiRole(mgr, mTeamAiManagerId, GetUniqueId())) {
        x7a4 = role->GetTeamPosition();
      } else {
        const CVector3f targetPos = GetAttackTargetPos(mgr);
        x7a4 = GetOrigin(mgr, CTeamAiRole(GetUniqueId()), targetPos);
      }
      ApplyForwardSteering(mgr, x7a4);
    }
    ApplySeparationBehavior(mgr, 9.f);
    break;
  case kStateMsg_Deactivate:
    BodyCtrl()->CommandMgr().SetSteeringBlendMode(kSBM_Normal);
    break;
  }
}

void CMetroid::TurnAround(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Update:
    if (mAttackTarget != kInvalidUniqueId) {
      if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(mAttackTarget))) {
        const CVector3f direction = actor->GetTranslation() - GetTranslation();
        if (ShouldTurn(mgr, 0.f) && direction.CanBeNormalized()) {
          BodyCtrl()->CommandMgr().DeliverCmd(
              CBCLocomotionCmd(CVector3f::Zero(), direction.AsNormalized(), 1.f));
        }
      }
    }
    break;
  }
}

void CMetroid::Attack(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    if (AttachToTarget(mgr)) {
      mState = kAiState_One;
      mEnergyDrained = 0.f;
      mAttackState = kAttackState_Attached;
      mIsAttacking = true;
      RemoveMaterial(kMT_Orbit, kMT_Target, mgr);
      mgr.Player()->TryToBreakOrbit(GetUniqueId(), CPlayer::kOB_ActivateOrbitSource, mgr);
      DisableSolidCollision(*this);
      AddMaterial(kMT_Trigger, mgr);
    } else {
      mState = kAiState_Over;
    }
    break;
  case kStateMsg_Update:
    switch (mState) {
    case kAiState_One:
      if (BodyCtrl()->GetCurrentStateId() == pas::kAS_LoopAttack) {
        mState = kAiState_Two;
      } else {
        BodyCtrl()->CommandMgr().DeliverCmd(CBCLoopAttackCmd(pas::kLAT_Three));
      }
      break;
    case kAiState_Two: {
      if (GetModelData()->GetAnimationData()->GetIsLoop()) {
        mAttackState = kAttackState_Draining;
      }
      const CPlayer& player = *mgr.GetPlayer();
      if (mAttackTarget == player.GetUniqueId() && player.GetAttachedActor() == GetUniqueId() &&
          player.GetCurrentAreaId() != GetCurrentAreaId()) {
        DetachFromTarget(mgr);
        x401_30_pendingDeath = true;
      } else if (BodyCtrl()->GetCurrentStateId() != pas::kAS_LoopAttack) {
        mState = kAiState_Over;
      } else {
        if (ShouldReleaseFromTarget(mgr)) {
          BodyCtrl()->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_ExitState));
          DetachFromTarget(mgr);
          mAttackState = kAttackState_Over;
        }
      }
      break;
    }
    }
    break;
  case kStateMsg_Deactivate: {
    CTeamAiMgr::ResetTeamAiRole(kAT_Melee, mgr, mTeamAiManagerId, GetUniqueId(), false);
    mAttackChance = x308_attackTimeVariation * mgr.Random()->Float() + x304_averageAttackTime;
    mAttackState = kAttackState_None;
    DetachFromTarget(mgr);
    mIsAttacking = false;
    const CQuaternion rotation = CQuaternion::ZRotation(CRelAngle::FromRadians(GetYaw()));
    SetTransform(rotation.BuildTransform4f(GetTranslation()));
    AddMaterial(kMT_Orbit, kMT_Target, mgr);
    RemoveMaterial(kMT_Trigger, mgr);
    break;
  }
  }
}

void CMetroid::TelegraphAttack(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mState = kAiState_Zero;
    mTelegraphAttackTime = mMetroidData.GetTelegraphAttackTime();
    mSeekTime = 0.f;
    BodyCtrl()->CommandMgr().ClearLocomotionCmds();
    BodyCtrl()->SetLocomotionType(pas::kLT_Combat);
    break;
  case kStateMsg_Update:
    switch (mState) {
    case kAiState_Zero:
      mTelegraphAttackTime -= dt;
      if (mTelegraphAttackTime < 0.f) {
        mState = kAiState_Two;
        const CVector3f delta = GetAttackTargetPos(mgr) - GetTranslation();
        const float magnitude = delta.Magnitude();
        const float speed = x3b4_speed;
        float extraTime = 0.f;
        const float distance = 1.25f * magnitude;
        if (speed > 0.f)
          extraTime = 1.15f / speed;
        mMaxSeekTime = extraTime + distance / BodyCtrl()->GetBodyStateInfo().GetMaxSpeed();
        BodyCtrl()->SetTurnSpeed(speed > 0.f ? 20.f / speed : 20.f);
      } else if (mAttackTarget != kInvalidUniqueId) {
        if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(mAttackTarget))) {
          const CVector3f direction = actor->GetTranslation() - GetTranslation();
          if (direction.CanBeNormalized()) {
            BodyCtrl()->CommandMgr().DeliverCmd(
                CBCLocomotionCmd(CVector3f::Zero(), direction.AsNormalized(), 1.f));
          }
        }
      }
      break;
    case kAiState_One:
      break;
    case kAiState_Two: {
      mSeekTime += dt;
      const CVector3f targetPos = GetAttackTargetPos(mgr);
      const CVector3f move = x45c_steeringBehaviors.Seek(*this, targetPos);
      BodyCtrl()->CommandMgr().DeliverCmd(CBCLocomotionCmd(move, CVector3f::Zero(), 1.f));
      break;
    }
    }
    break;
  case kStateMsg_Deactivate:
    BodyCtrl()->SetTurnSpeed(x3b8_turnSpeed);
    if (Attacked(mgr, 0.f) || PatternShagged(mgr, 0.f)) {
      CTeamAiMgr::ResetTeamAiRole(kAT_Melee, mgr, mTeamAiManagerId, GetUniqueId(), false);
    }
    break;
  }
}

void CMetroid::WallHang(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyCtrl()->SetLocomotionType(pas::kLT_Crouch);
    mState = kAiState_Zero;
    RemoveMaterial(kMT_Solid, mgr);
    mRestoreSolidCollision = false;
    break;
  case kStateMsg_Update:
    switch (mState) {
    case kAiState_Zero:
      if (mAlert) {
        mState = kAiState_One;
        mRestoreSolidCollision = true;
        mDetachPos = CVector3f::Zero();
      }
      break;
    case kAiState_One:
      if (BodyCtrl()->GetCurrentStateId() == pas::kAS_Generate) {
        mState = kAiState_Two;
      } else {
        BodyCtrl()->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_Zero, CVector3f::Zero()));
      }
      break;
    case kAiState_Two:
      if (BodyCtrl()->GetCurrentStateId() != pas::kAS_Generate) {
        mState = kAiState_Over;
      }
      break;
    }
    break;
  case kStateMsg_Deactivate:
    BodyCtrl()->SetLocomotionType(pas::kLT_Relaxed);
    x9bf_27_ = true;
    break;
  }
}

void CMetroid::Dodge(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    if (mDodgeDirection != pas::kSD_Invalid) {
      BodyCtrl()->CommandMgr().DeliverCmd(CBCStepCmd(mDodgeDirection, pas::kStep_Dodge));
      mState = kAiState_Two;
    }
    break;
  case kStateMsg_Update:
    if (BodyCtrl()->GetCurrentStateId() != pas::kAS_Step) {
      mState = kAiState_Over;
    } else {
      if (mAttackTarget != kInvalidUniqueId) {
        if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(mAttackTarget))) {
          BodyCtrl()->CommandMgr().DeliverTargetVector(actor->GetTranslation() - GetTranslation());
        }
      }
    }
    break;
  case kStateMsg_Deactivate:
    mDodgeDirection = pas::kSD_Invalid;
    break;
  }
}

bool CMetroid::IsSuckingEnergy() const {
  return mAttackState == kAttackState_Draining && !GetBodyCtrl()->IsFrozen();
}

bool CMetroid::IsPirateValidTarget(const CSpacePirate& pirate, const CStateManager& mgr) const {
  if (pirate.GetAttachedActor() == kInvalidUniqueId && !pirate.IsTrooper()) {
    const CHealthInfo* health = pirate.GetHealthInfo(mgr);
    return health != nullptr && health->GetHP() > 0.f;
  }
  return false;
}

CVector3f CMetroid::GetAttackTargetPos(const CStateManager& mgr) const {
  if (mAttackTarget != kInvalidUniqueId) {
    const CPlayer& player = *mgr.GetPlayer();
    if (mAttackTarget != player.GetUniqueId()) {
      if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(GetAttackTargetId()))) {
        const CTransform4f locXf = actor->GetLocatorTransform(rstl::string_l(skPirateSuckJoint));
        const CVector3f locPos = locXf.GetTranslation();
        CVector3f result =
            actor->GetTranslation() +
            CVector3f(0.f, 0.f, locPos.GetZ() * actor->GetModelData()->ScaleCopy().GetZ() + 0.4f);
        return result;
      }
    } else {
      CVector3f pos = player.GetTranslation();
      if (player.GetMorphballTransitionState() != CPlayer::kMS_Morphed) {
        pos += CVector3f(0.f, 0.f, -0.6f + player.GetEyeHeight());
      } else {
        pos = player.GetMorphBall()->GetBallToWorld().GetTranslation();
      }
      return pos;
    }
  }
  return mgr.GetPlayer()->GetAimPosition(mgr, 0.f);
}

float CMetroid::GetGrowthStage() const {
  if (mGrowthEnergy < mMetroidData.GetStage2GrowthEnergy()) {
    return 1.f + mGrowthEnergy / mMetroidData.GetStage2GrowthEnergy();
  }
  if (mGrowthEnergy < mMetroidData.GetExplosionGrowthEnergy()) {
    return 2.f +
           (mGrowthEnergy - mMetroidData.GetStage2GrowthEnergy()) /
               (mMetroidData.GetExplosionGrowthEnergy() - mMetroidData.GetStage2GrowthEnergy());
  }
  return 3.f;
}

float CMetroid::GetDamageMultiplier() const {
  float result = 0.5f * (GetGrowthStage() - 1.f) + 1.f;
  return result;
}

bool CMetroid::PreDamageSpacePirate(CStateManager& mgr) {
  if (mAttackTarget != kInvalidUniqueId) {
    if (CSpacePirate* pirate = CPatterned::CastTo< CSpacePirate >(
            TPatternedCast< CSpacePirate >(mgr.ObjectById(mAttackTarget)))) {
      if (IsPirateValidTarget(*pirate, mgr) && pirate->AttachActorToPirate(GetUniqueId())) {
        const CDamageInfo info(CWeaponMode::Power(), pirate->GetXDamageThreshold() - 1.f, 0.f, 0.f,
                               true);
        mgr.ApplyDamage(
            GetUniqueId(), mAttackTarget, GetUniqueId(), info,
            CMaterialFilter::MakeIncludeExclude(CMaterialList(skSolidMaterial), CMaterialList()),
            CVector3f::Zero());
        return true;
      }
    }
  }
  return false;
}

void CMetroid::ApplyGrowth(float damage, CStateManager& mgr) {
  mGrowthEnergy += damage;
  const float scale = mMetroidData.GetStage2GrowthScale() - mScale3.GetY();
  const float energy = CMath::Clamp(0.f, mGrowthEnergy / mMetroidData.GetStage2GrowthEnergy(), 1.f);
  const float newScale = energy * scale + mScale3.GetY();
  mScale1 = CVector3f(newScale, newScale, newScale);
  TakeDamage(CVector3f::Zero(), 0.f);
}

void CMetroid::SetUpPathFindBehavior(CStateManager& mgr) {
  x9bf_28_ = false;
  if (GetSearchPath() != nullptr) {
    if (const CTeamAiRole* role = CTeamAiMgr::GetTeamAiRole(mgr, mTeamAiManagerId, GetUniqueId())) {
      x2e0_destPos = role->GetTeamPosition();
    } else {
      x2e0_destPos = GetOrigin(mgr, CTeamAiRole(GetUniqueId()), mgr.GetPlayer()->GetTranslation());
    }
    const CVector3f targetPos = GetAttackTargetPos(mgr);
    const CVector3f delta = x2e0_destPos - targetPos;
    if (delta.CanBeNormalized()) {
      const CMaterialFilter filter =
          CMaterialFilter::MakeInclude(CMaterialList(kMT_Solid, kMT_AIBlock));
      const float length = delta.Magnitude();
      const CVector3f direction = (1.f / length) * delta;
      const CRayCastResult result = mgr.RayStaticIntersection(targetPos, direction, length, filter);
      if (result.IsValid()) {
        x2e0_destPos = targetPos + 0.5f * (result.GetTime() * direction);
        x9bf_28_ = true;
      }
    }
    x7a4 = x2e0_destPos;
    CPatterned::PathFind(mgr, kStateMsg_Activate, 0.f);
  }
}

void CMetroid::ApplySeparationBehavior(CStateManager& mgr, float distance) {
  if (CTeamAiMgr* team = TCastToPtr< CTeamAiMgr >(mgr.ObjectById(mTeamAiManagerId))) {
    const CObjectList& list = mgr.GetObjectListById(kOL_ListeningAi);
    for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
      if (const CPatterned* ai = TCastToConstPtr< CPatterned >(list[i])) {
        if (ai != this && ai->GetCurrentAreaId() == GetCurrentAreaId()) {
          const CVector3f separation =
              x45c_steeringBehaviors.Separation(*this, ai->GetTranslation(), 2.f * distance);
          if (separation.IsNonZero()) {
            BodyCtrl()->CommandMgr().DeliverCmd(
                CBCLocomotionCmd(separation, CVector3f::Zero(), 1.f));
          }
        }
      }
    }
  }
}

void CMetroid::ApplyForwardSteering(CStateManager& mgr, const CVector3f& destination) {
  const CVector3f position = GetTranslation();
  const CVector3f delta = destination - position;
  if (delta.MagSquared() > 4.f) {
    const CVector3f arrivalPos(GetTranslation().GetX(), GetTranslation().GetY(),
                               destination.GetZ());
    const CVector3f arrival = x45c_steeringBehaviors.Arrival(*this, arrivalPos, 0.5f);
    if (arrival.MagSquared() > 0.01f) {
      BodyCtrl()->CommandMgr().DeliverCmd(CBCLocomotionCmd(arrival, CVector3f::Zero(), 3.f));
    }
    if (const CPhysicsActor* actor =
            TCastToConstPtr< CPhysicsActor >(mgr.GetObjectById(mAttackTarget))) {
      const CVector3f move =
          x45c_steeringBehaviors.Pursuit(*this, destination, actor->GetVelocityWR());
      BodyCtrl()->CommandMgr().DeliverCmd(CBCLocomotionCmd(move, CVector3f::Zero(), 1.f));
    } else {
      const CVector3f move = x45c_steeringBehaviors.Seek(*this, destination);
      BodyCtrl()->CommandMgr().DeliverCmd(CBCLocomotionCmd(move, CVector3f::Zero(), 1.f));
    }
  } else if (ShouldTurn(mgr, 0.f) && mAttackTarget != kInvalidUniqueId) {
    if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(mAttackTarget))) {
      const CVector3f direction = actor->GetTranslation() - GetTranslation();
      if (direction.CanBeNormalized()) {
        BodyCtrl()->CommandMgr().DeliverCmd(
            CBCLocomotionCmd(CVector3f::Zero(), direction.AsNormalized(), 1.f));
      }
    }
  }
}

void CMetroid::ApplySplitGammas(CStateManager& mgr, float dt) {
  if (CMetroid* other =
          CPatterned::CastTo< CMetroid >(TPatternedCast< CMetroid >(mgr.ObjectById(mParent)))) {
    const CVector3f delta = GetTranslation() - other->GetTranslation();
    const CVector3f right = GetTransform().GetRight();
    const float radius = mCollisionPrimitive.GetSphere().GetRadius();
    const float distance = delta.Magnitude();
    if (distance < 3.f * radius) {
      float separation = 3.f * radius - distance;
      const float timeRemaining = BodyCtrl()->GetAnimTimeRemaining();
      const float amount = timeRemaining > 0.f ? separation * dt / timeRemaining : separation;
      const CVector3f displacement = amount * right;
      const CVector3f impulse =
          GetMoveToORImpulseWR(GetTransform().TransposeRotate(displacement), dt);
      ApplyImpulseWR(impulse, CAxisAngle::Identity());
      const CVector3f otherImpulse =
          other->GetMoveToORImpulseWR(other->GetTransform().TransposeRotate(-displacement), dt);
      other->ApplyImpulseWR(otherImpulse, CAxisAngle::Identity());
    }
  }
}

void CMetroid::SwarmAdd(CStateManager& mgr) {
  if (mTeamAiManagerId != kInvalidUniqueId) {
    if (CTeamAiMgr* aiMgr = TCastToPtr< CTeamAiMgr >(mgr.ObjectById(mTeamAiManagerId))) {
      if (!aiMgr->IsPartOfTeam(GetUniqueId())) {
        aiMgr->AssignTeamAiRole(*this, CTeamAiRole::kTAR_Melee, CTeamAiRole::kTAR_Invalid,
                                CTeamAiRole::kTAR_Invalid);
      }
    }
  }
}

void CMetroid::SwarmRemove(CStateManager& mgr) {
  if (mTeamAiManagerId != kInvalidUniqueId) {
    if (CTeamAiMgr* aiMgr = TCastToPtr< CTeamAiMgr >(mgr.ObjectById(mTeamAiManagerId))) {
      if (aiMgr->IsPartOfTeam(GetUniqueId())) {
        aiMgr->RemoveTeamAiRole(GetUniqueId());
      }
    }
  }
}

bool CMetroid::AttachToTarget(CStateManager& mgr) {
  if (mAttackTarget == mgr.GetPlayer()->GetUniqueId()) {
    if (mgr.Player()->AttachActorToPlayer(GetUniqueId(), false)) {
      CPlayerEnergyDrain& drain = mgr.Player()->GetPlayerEnergyDrain();
      drain.AddEnergyDrainSource(GetUniqueId(), 1.f);
      return true;
    }
  } else {
    return PreDamageSpacePirate(mgr);
  }
  return false;
}

void CMetroid::DetachFromTarget(CStateManager& mgr) {
  CActor* target = nullptr;
  CVector3f direction = CVector3f::Forward();
  CTransform4f xf = CTransform4f::Identity();
  CPlayer& player = *mgr.Player();
  if (mAttackTarget == player.GetUniqueId()) {
    if (player.GetAttachedActor() == GetUniqueId()) {
      player.DetachActorFromPlayer();
      target = &player;
      if (player.GetMorphballTransitionState() != CPlayer::kMS_Morphed) {
        direction = player.GetTransform().GetForward();
        xf = player.GetTransform();
      } else {
        const CQuaternion rot =
            CQuaternion::ZRotation(CRelAngle(GetYaw())) * CQuaternion::ZRotation(CRelAngle(M_PIF));
        const CMatrix3f mat = rot.BuildTransform();
        direction = mat * CVector3f::Forward();
        xf = CTransform4f(mat, player.GetTranslation());
      }
      CPlayerEnergyDrain& drain = mgr.Player()->GetPlayerEnergyDrain();
      drain.RemoveEnergyDrainSource(GetUniqueId());
      mDetachPos = player.GetAimPosition(mgr, 0.f);
    }
  } else if (mAttackTarget != kInvalidUniqueId) {
    if (CSpacePirate* pirate = CPatterned::CastTo< CSpacePirate >(
            TPatternedCast< CSpacePirate >(mgr.ObjectById(mAttackTarget)))) {
      if (pirate->GetAttachedActor() == GetUniqueId()) {
        pirate->DetachActorFromPirate();
        target = pirate;
        direction = pirate->GetTransform().GetForward();
        xf = pirate->GetTransform();
        mDetachPos = GetTranslation();
      }
    }
  }
  SetupExitFaceHugDirection(target, mgr, direction, xf);
  mRestoreSolidCollision = mRestoreCharacterCollision = true;
}

void CMetroid::SetupExitFaceHugDirection(CActor* actor, CStateManager& mgr,
                                         const CVector3f& direction, const CTransform4f& xf) {
  if (actor == nullptr) {
    return;
  }
  if (mAttackState == kAttackState_Over) {
    return;
  }
  const CMaterialFilter filter = CMaterialFilter::MakeIncludeExclude(
      CMaterialList(kMT_Solid, kMT_AIBlock), CMaterialList(kMT_Character, kMT_Player));
  const float length = mLoopAttackDistance * GetModelData()->ScaleCopy().GetY();
  CVector3f bestDirection = -GetTransform().GetForward();
  float bestDistance = 0.f;
  static const float angles[] = {90.f, 135.f, 45.f, 180.f, 0.f, 225.f, 315.f, 270.f};
  for (uint i = 0; i < 8; ++i) {
    const float angle = (M_PIF / 180.f) * angles[i];
    const CVector3f localDirection(CMath::FastCosR(angle), CMath::FastSinR(angle), 0.f);
    const CVector3f rayDirection = xf.Rotate(localDirection).AsNormalized();
    CVector3f start = actor->GetTranslation();
    start.SetZ(GetTranslation().GetZ());
    TUniqueId hitId = kInvalidUniqueId;
    TEntityList nearList;
    mgr.BuildNearList(nearList, start, rayDirection, length, filter, this);
    const CRayCastResult result =
        mgr.RayWorldIntersection(hitId, start, rayDirection, length, filter, nearList);
    if (result.IsInvalid()) {
      bestDirection = rayDirection;
      bestDistance = length;
      CTransform4f testXf = GetTransform();
      testXf.AddTranslation(length * rayDirection);
      if (!CGameCollision::DetectStaticCollisionBoolean(mgr, mCollisionPrimitive, testXf, filter)) {
        break;
      }
    } else if (result.GetTime() > bestDistance) {
      bestDirection = rayDirection;
      bestDistance = result.GetTime();
    }
  }
  const CRelAngle maxAngle = CRelAngle::FromDegrees(360.f);
  const CQuaternion rot = CQuaternion::LookAt(
      CUnitVector3f(direction, CUnitVector3f::kN_No),
      CUnitVector3f(bestDirection[kDX], bestDirection[kDY], bestDirection[kDZ]), maxAngle);
  SetRotation(GetRotation() * rot);
}

void CMetroid::PreventWorldCollisions(float dt, CStateManager& mgr) {
  const float size = 2.f * mCollisionPrimitive.GetSphere().GetRadius();
  if (IsSuckingEnergy()) {
    if (mgr.GetPlayer()->GetUniqueId() == mAttackTarget) {
      if (CPhysicsActor* actor = TCastToPtr< CPhysicsActor >(mgr.ObjectById(mAttackTarget))) {
        float mass = 300.f;
        if (mgr.GetPlayer()->GetMorphballTransitionState() == CPlayer::kMS_Unmorphed) {
          const float scale = CMath::Min(1.f, 1.33f * mEnergyDrainTime);
          mass = 300.f * (1.f - scale) + 7500.f * scale;
        }
        CGameCollision::AvoidStaticCollisionWithinRadius(mgr, *actor, 8, dt, 0.25f, size, mass,
                                                         0.5f);
      }
    }
    mCollisionRestoreTime = 0.f;
  } else if (mRestoreSolidCollision || mRestoreCharacterCollision) {
    mCollisionRestoreTime += dt;
    if (mCollisionRestoreTime > 6.f) {
      MassiveDeath(mgr);
    } else if (mRestoreSolidCollision && mCollisionRestoreTime > 0.25f) {
      RemoveMaterial(kMT_Solid, mgr);
    }
    CGameCollision::AvoidStaticCollisionWithinRadius(mgr, *this, 8, dt, 0.25f, size, 15000.f, 0.5f);
  } else {
    mCollisionRestoreTime = 0.f;
  }
}

void CMetroid::RestoreSolidCollision(CStateManager& mgr) {
  const CMaterialFilter filter =
      CMaterialFilter::MakeInclude(CMaterialList(kMT_Solid, kMT_AIBlock));
  if (mRestoreSolidCollision && !CGameCollision::DetectStaticCollisionBoolean(
                                    mgr, mCollisionPrimitive, GetTransform(), filter)) {
    bool add = true;
    if (mDetachPos.IsNonZero()) {
      const CVector3f dir = GetTranslation() - mDetachPos;
      const float mag = dir.Magnitude();
      if (mag > 0.f) {
        const CRayCastResult result =
            mgr.RayStaticIntersection(mDetachPos, (1.f / mag) * dir, mag, filter);
        add = !result.IsValid();
      }
    }
    if (add) {
      AddMaterial(kMT_Solid, mgr);
      mRestoreSolidCollision = false;
    }
  }
  if (mRestoreCharacterCollision) {
    TEntityList nearList;
    const CMaterialFilter nearFilter =
        CMaterialFilter::MakeInclude(CMaterialList(kMT_Solid, kMT_Player, kMT_Character));
    const float radius = mLoopAttackDistance * GetModelData()->ScaleCopy().GetY();
    const CVector3f extent(radius, radius, radius);
    const CAABox box(GetTranslation() - extent, GetTranslation() + extent);
    mgr.BuildNearList(nearList, box, nearFilter, this);
    if (!CGameCollision::DetectDynamicCollisionBoolean(mCollisionPrimitive, GetTransform(),
                                                       nearList, mgr)) {
      mRestoreCharacterCollision = false;
      CMaterialFilter matFilter = GetMaterialFilter();
      matFilter.ExcludeList().Remove(CMaterialList(kMT_Character, kMT_Player));
      SetMaterialFilter(matFilter);
    }
  }
}

void CMetroid::DisableSolidCollision(CMetroid& target) {
  CMaterialFilter filter = target.GetMaterialFilter();
  filter.ExcludeList().Add(CMaterialList(kMT_Character, kMT_Player));
  target.SetMaterialFilter(filter);
}

bool CMetroid::ShouldReleaseFromTarget(CStateManager& mgr) {
  if (BodyCtrl()->IsFrozen()) {
    return true;
  }
  const CPlayer& player = *mgr.GetPlayer();
  if (mAttackTarget == player.GetUniqueId()) {
    if (mEnergyDrained >= mMetroidData.GetMaxEnergyDrainAllowed() * GetDamageMultiplier() ||
        IsPlayerInFluid(mgr)) {
      return true;
    }
    if (player.GetMorphballTransitionState() != CPlayer::kMS_Unmorphed) {
      return IsBetaMetroidAttackingPlayer(mgr);
    }
  } else if (mAttackTarget != kInvalidUniqueId) {
    const CSpacePirate* pirate = CPatterned::CastTo< CSpacePirate >(
        TPatternedCast< CSpacePirate >(const_cast< CEntity* >(mgr.GetObjectById(mAttackTarget))));
    if (pirate != nullptr) {
      return pirate->AllEnergyDrained() || pirate->GetBodyCtrl()->GetBodyStateInfo().IsDead();
    }
    return true;
  }
  return false;
}

void CMetroid::SuckEnergyFromTarget(float dt, CStateManager& mgr) {
  mIsEnergyDrainVulnerable = false;
  if (mAttackTarget == kInvalidUniqueId) {
    return;
  }
  switch (mAttackState) {
  case kAttackState_None:
    break;
  case kAttackState_Attached: {
    InterpolateToPosRot(mgr, 0.4f);
    CPlayer& player = *mgr.Player();
    if (player.GetUniqueId() == mAttackTarget) {
      x402_28_isMakingBigStrike = true;
      x504_damageDur = 0.2f;
      mgr.DeliverScriptMsg(&player, GetUniqueId(), kSM_Damage);
    }
    mEnergyDrainTime = 0.f;
    break;
  }
  case kAttackState_Draining: {
    if (CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(mAttackTarget))) {
      if (CHealthInfo* health = actor->HealthInfo(mgr)) {
        const float damage = dt * mMetroidData.GetEnergyDrainPerSec() * GetDamageMultiplier();
        mEnergyDrained += damage;
        CPlayer& player = *mgr.Player();
        if (mAttackTarget == player.GetUniqueId()) {
          const CDamageInfo info(CWeaponMode(kWT_PoisonWater), damage, 0.f, 0.f, true);
          player.SetNoDamageLoopSfx(true);
          mgr.ApplyDamage(
              GetUniqueId(), mAttackTarget, GetUniqueId(), info,
              CMaterialFilter::MakeIncludeExclude(CMaterialList(skSolidMaterial), CMaterialList()),
              CVector3f::Zero());
          player.SetNoDamageLoopSfx(false);
          mIsEnergyDrainVulnerable = player.GetMorphballTransitionState() == CPlayer::kMS_Morphed;
        } else {
          mIsEnergyDrainVulnerable = true;
          const CDamageInfo info(CWeaponMode::Power(), damage, 0.f, 0.f, true);
          mgr.ApplyDamage(
              GetUniqueId(), mAttackTarget, GetUniqueId(), info,
              CMaterialFilter::MakeIncludeExclude(CMaterialList(skSolidMaterial), CMaterialList()),
              CVector3f::Zero());
        }
        if (GetGrowthStage() < 2.f) {
          ApplyGrowth(damage, mgr);
        } else {
          TakeDamage(CVector3f::Zero(), 0.f);
        }
      }
    }
    float blend = 0.95f;
    if (mgr.GetPlayer()->GetUniqueId() == mAttackTarget) {
      const CPlayer::EPlayerMorphBallState morphState =
          mgr.GetPlayer()->GetMorphballTransitionState();
      if (morphState != CPlayer::kMS_Unmorphed && morphState != CPlayer::kMS_Morphed) {
        blend = 0.4f;
      }
      if (morphState == CPlayer::kMS_Unmorphed) {
        const float magnitude =
            CMath::Clamp(0.f, CMath::AbsF(CMath::FastSinR((M_PIF / 2.f) * mEnergyDrainTime)), 1.f);
        mgr.PlayerState()->StaticInterference().AddSource(GetUniqueId(), magnitude, 0.2f);
        if (mgr.GetPlayer()->GetStaticTimer() < 0.2f) {
          mgr.Player()->SetHudDisable(0.2f);
        }
      }
      x402_28_isMakingBigStrike = true;
      x504_damageDur = 0.2f;
    }
    InterpolateToPosRot(mgr, blend);
    mEnergyDrainTime += dt;
    break;
  }
  case kAttackState_Over: {
    const CQuaternion zRot = CQuaternion::ZRotation(CRelAngle(GetYaw()));
    const CQuaternion rot = CQuaternion::SlerpLocal(GetRotation(), zRot, 0.95f);
    SetRotation(rot.BuildNormalized());
    break;
  }
  }
}

void CMetroid::InterpolateToPosRot(CStateManager& mgr, float dt) {
  CVector3f targetPos = CVector3f::Zero();
  CQuaternion targetRot = CQuaternion::NoRotation();
  ComputeSuckTargetPosRot(mgr, targetPos, targetRot);
  const CVector3f pos = CVector3f::Lerp(GetTranslation(), targetPos, dt);
  const CQuaternion rot = CQuaternion::SlerpLocal(GetRotation(), targetRot, dt);
  SetTranslation(pos);
  SetRotation(rot.BuildNormalized());
}

bool CMetroid::ShouldSpawnGammaMetroid() const {
  if (GetFlavorType() == kFT_Two && mParent == kInvalidUniqueId && x9be == 0) {
    if (mGrowthEnergy - mLastGrowthEnergy > 0.f &&
        mLastGrowthEnergy < mMetroidData.GetStage2GrowthEnergy() &&
        mGrowthEnergy >= mMetroidData.GetStage2GrowthEnergy()) {
      return mMetroidData.GetNumValidGammaModels() != 0;
    }
  }
  return false;
}

void CMetroid::SpawnGammaMetroid(CStateManager& mgr) {
  const CMetroidData::EGammaFlavor firstFlavor =
      PickRandomGammaFlavor(mgr, CMetroidData::kGF_Invalid);
  const CMetroidData::EGammaFlavor secondFlavor = PickRandomGammaFlavor(mgr, firstFlavor);
  if (firstFlavor != CMetroidData::kGF_Invalid && secondFlavor != CMetroidData::kGF_Invalid) {
    const CAnimationParameters& firstAnimation =
        *mMetroidData.GetSpawnedGammaAnimParms(firstFlavor);
    const CModelData firstModel(CAnimRes(firstAnimation.GetACSFile(), firstAnimation.GetCharacter(),
                                         mScale3, firstAnimation.GetInitialAnimation(), true));
    mGammaType = firstFlavor;
    CMetroidData firstData = mMetroidData;
    firstData.SetStartsInWall(false);
    firstData.SetStage2GrowthEnergy(skGammaGrowthEnergy[mGammaType]);
    firstData.SetExplosionGrowthEnergy(2.f * firstData.GetStage2GrowthEnergy());
    AnimationData()->GetParticleDB().DeleteAllLights(mgr);
    SetModelData(firstModel);
    mGrowthEnergy = mLastGrowthEnergy = firstData.GetStage2GrowthEnergy();
    mScale1 = mScale2 = mScale3;
    DisableSolidCollision(*this);
    mRestoreCharacterCollision = true;
    if (BodyCtrl()->GetCurrentStateId() != pas::kAS_Locomotion) {
      BodyCtrl()->BodyStateInfo().SetState(pas::kAS_Locomotion);
    }
    const CAnimationParameters& secondAnimation =
        *mMetroidData.GetSpawnedGammaAnimParms(secondFlavor);
    CMetroidData secondData = firstData;
    secondData.SetStage2GrowthEnergy(skGammaGrowthEnergy[secondFlavor]);
    secondData.SetExplosionGrowthEnergy(2.f * secondData.GetStage2GrowthEnergy());
    const CModelData secondModel(CAnimRes(secondAnimation.GetACSFile(),
                                          secondAnimation.GetCharacter(), mScale3,
                                          secondAnimation.GetInitialAnimation(), true));
    SendScriptMsgs(kSS_MaxReached, mgr, kSM_None);
    CMetroid* spawned = rs_new CMetroid(
        mgr.AllocateUniqueId(), rstl::string_l("Spawned Metroid"), GetFlavorType(),
        CEntityInfo(GetCurrentAreaId(), GetConnectionList(), GetEditorId()), GetTransform(),
        secondModel, mPatternedInfo, CActorParameters(mActorParameters), secondData, GetUniqueId());
    if (spawned != nullptr) {
      mgr.AddObject(spawned);
      DisableSolidCollision(*spawned);
      spawned->ApplyGrowth(secondData.GetStage2GrowthEnergy(), mgr);
      spawned->mGammaType = secondFlavor;
      spawned->mRestoreCharacterCollision = true;
      spawned->mScale1 = spawned->mScale2 = mScale3;
      spawned->x9bf_27_ = true;
      ++x9be;
    }
  }
}

CMetroidData::EGammaFlavor CMetroid::PickRandomGammaFlavor(CStateManager& mgr,
                                                           CMetroidData::EGammaFlavor previous) {
  rstl::reserved_vector< CMetroidData::EGammaFlavor, 4 > flavors;
  int result = CMetroidData::kGF_Invalid;
  for (int i = CMetroidData::kGF_Red; i <= CMetroidData::kGF_Yellow; ++i) {
    const CMetroidData::EGammaFlavor flavor = static_cast< CMetroidData::EGammaFlavor >(i);
    if (mMetroidData.GetSpawnedGammaAnimParms(flavor).valid()) {
      flavors.push_back(flavor);
    }
  }
  if (!flavors.empty()) {
    const int index = mgr.Random()->Range(0, flavors.size() - 1);
    const CMetroidData::EGammaFlavor chosen = flavors[index];
    if (previous != chosen) {
      result = chosen;
    } else {
      if (index + 1 < flavors.size()) {
        result = flavors[index + 1];
      } else if (index - 1 >= 0) {
        result = flavors[index - 1];
      } else {
        result = chosen;
      }
    }
  }
  return static_cast< CMetroidData::EGammaFlavor >(result);
}

void CMetroid::ComputeSuckTargetPosRot(CStateManager& mgr, CVector3f& pos, CQuaternion& rot) const {
  pos = GetTranslation();
  rot = GetRotation();
  if (mAttackTarget == mgr.GetPlayer()->GetUniqueId()) {
    ComputeSuckPlayerPosRot(mgr, pos, rot);
  } else {
    ComputeSuckPiratePosRot(mgr, pos, rot);
  }
}

void CMetroid::ComputeSuckPlayerPosRot(CStateManager& mgr, CVector3f& pos, CQuaternion& rot) const {
  const CPlayer& player = *mgr.GetPlayer();
  pos = player.GetTransform().GetTranslation();
  const float scaleY = GetModelData()->ScaleCopy().GetY();
  switch (player.GetMorphballTransitionState()) {
  case CPlayer::kMS_Unmorphed: {
    const CQuaternion camRotation =
        CQuaternion::FromMatrix(mgr.GetCameraManager()->GetFirstPersonCamera()->GetTransform());
    rot = camRotation * CQuaternion::ZRotation(CRelAngle::FromRadians(M_PIF));
    const CMatrix3f camMatrix = camRotation.BuildTransform();
    const CVector3f forward = 1.f * (camMatrix * CVector3f::Forward());
    const CVector3f up = (-0.6f * scaleY) * (camMatrix * CVector3f::Up());
    pos += CVector3f(0.f, 0.f, player.GetEyeHeight()) + up + forward;
    break;
  }
  case CPlayer::kMS_Morphing: {
    const float height = ComputeMorphingPlayerSuckUpPos(player);
    pos += CVector3f(0.f, 0.f, 0.4f + height);
    const float radius = player.GetMorphBall()->GetBallRadius();
    pos += 0.5f * player.GetTransform().GetForward() - radius * GetTransform().GetUp();
    rot = CQuaternion::FromMatrix(player.GetTransform()) *
          CQuaternion::YXZRotation(CRelAngle::FromRadians(0.f), CRelAngle::FromDegrees(-89.9f),
                                   CRelAngle::FromRadians(M_PIF));
    break;
  }
  case CPlayer::kMS_Unmorphing: {
    const float height = ComputeMorphingPlayerSuckUpPos(player);
    pos += CVector3f(0.f, 0.f, 0.4f + height);
    const float radius = player.GetMorphBall()->GetBallRadius();
    pos += 0.5f * player.GetTransform().GetForward() - radius * GetTransform().GetUp();
    rot = CQuaternion::FromMatrix(player.GetTransform()) *
          CQuaternion::YXZRotation(CRelAngle::FromRadians(0.f), CRelAngle::FromDegrees(-89.9f),
                                   CRelAngle::FromRadians(M_PIF));
    const float morphT = player.GetMorphBallTransitionFactor();
    if (morphT > 0.75f) {
      const CQuaternion camRotation =
          CQuaternion::FromMatrix(mgr.GetCameraManager()->GetFirstPersonCamera()->GetTransform());
      const CQuaternion targetRotation =
          camRotation * CQuaternion::ZRotation(CRelAngle::FromRadians(M_PIF));
      const CMatrix3f camMatrix = camRotation.BuildTransform();
      const CVector3f forward = 1.f * (camMatrix * CVector3f::Forward());
      const CVector3f up = (-0.6f * scaleY) * (camMatrix * CVector3f::Up());
      const CVector3f targetPos = player.GetTransform().GetTranslation() +
                                  CVector3f(0.f, 0.f, player.GetEyeHeight()) + up + forward;
      const float t = (morphT - 0.75f) / 0.25f;
      rot = CQuaternion::SlerpLocal(rot, targetRotation, t);
      pos = CVector3f::Lerp(pos, targetPos, t);
    }
    break;
  }
  case CPlayer::kMS_Morphed: {
    pos += (2.f * player.GetMorphBall()->GetBallRadius() + 0.25f) * CVector3f::Up();
    const float radius = player.GetMorphBall()->GetBallRadius();
    pos -= radius * (scaleY * GetTransform().GetUp());
    rot = CQuaternion::YXZRotation(CRelAngle::FromRadians(0.f), CRelAngle::FromDegrees(-89.9f),
                                   CRelAngle::FromRadians(GetYaw()));
    break;
  }
  }
}

void CMetroid::ComputeSuckPiratePosRot(CStateManager& mgr, CVector3f& pos, CQuaternion& rot) const {
  if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(GetAttackTargetId()))) {
    const CTransform4f headXf = actor->GetLocatorTransform(rstl::string_l(skPirateSuckJoint));
    const CTransform4f rootXf = actor->GetLocatorTransform(rstl::string_l(skPirateRootJoint));
    const CVector3f localPos = headXf.GetTranslation() + -0.5f * rootXf.GetUp();
    const CVector3f& scale = actor->GetModelData()->GetScale();
    const CVector3f scaledPos(scale.GetX() * localPos.GetX(), scale.GetY() * localPos.GetY(),
                              scale.GetZ() * localPos.GetZ());
    pos = actor->GetTranslation() + actor->GetTransform().Rotate(scaledPos);
    pos += 0.3f * actor->GetTransform().Rotate(rootXf.GetForward());
    const CQuaternion rotation = CQuaternion::FromMatrix(actor->GetTransform() * rootXf);
    rot = rotation * CQuaternion::ZRotation(CRelAngle::FromRadians(M_PIF));
  }
}

float CMetroid::ComputeMorphingPlayerSuckUpPos(const CPlayer& player) const {
  float height = 0.f;
  if (player.HasModelData()) {
    for (uint i = 0; i < 17; ++i) {
      const CTransform4f xf = player.GetLocatorTransform(rstl::string_l(skJointNameList[i]));
      const float jointZ = xf.Get23();
      const float jointHeight = jointZ * player.GetModelData()->ScaleCopy().GetZ();
      if (jointHeight > height) {
        height = jointHeight;
      }
    }
  }
  return height;
}

bool CMetroid::IsTargetGettingSucked(const CStateManager& mgr) const {
  if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(GetAttackTargetId()))) {
    if (mgr.GetPlayer()->GetUniqueId() == mAttackTarget) {
      const TUniqueId attachedActor = mgr.GetPlayer()->GetAttachedActor();
      if (attachedActor != kInvalidUniqueId && attachedActor != GetUniqueId()) {
        return true;
      }
    } else if (const CSpacePirate* pirate = CPatterned::CastTo< CSpacePirate >(
                   TPatternedCast< CSpacePirate >(const_cast< CActor* >(actor)))) {
      if (pirate->GetAttachedActor() != kInvalidUniqueId &&
          pirate->GetAttachedActor() != GetUniqueId()) {
        return true;
      }
    }
  }
  return false;
}

bool CMetroid::IsBetaMetroidAttackingPlayer(const CStateManager& mgr) const {
  if (const CTeamAiMgr* const aiMgr =
          TCastToConstPtr< CTeamAiMgr >(mgr.GetObjectById(mTeamAiManagerId))) {
    const rstl::vector< TUniqueId >& attackers = aiMgr->GetProjectileAttackers();
    if (attackers.size() != 0u) {
      for (AUTO(it, attackers.begin()); it != attackers.end(); ++it) {
        if (CPatterned::CastTo< CMetroidBeta >(TPatternedCast< CMetroidBeta >(
                const_cast< CEntity* >(mgr.GetObjectById(*it)))) != nullptr) {
          return true;
        }
      }
    }
  }
  return false;
}

CVector3f CMetroid::GetOrigin(const CStateManager& mgr, const CTeamAiRole& role,
                              const CVector3f& aimPos) const {
  const CPlayer& player = *mgr.GetPlayer();
  const CTransform4f& xf = GetTransform();
  CVector3f result;
  result.SetZ(xf.Get23());
  result.SetY(xf.Get13());
  result.SetX(xf.Get03());
  const float range = 0.5f * (x2fc_minAttackRange + x300_maxAttackRange);
  if (player.GetUniqueId() == GetAttackTargetId()) {
    if (player.GetMorphballTransitionState() == CPlayer::kMS_Morphed) {
      const CVector3f direction((GetTranslation() - player.GetTranslation()).DropZ(), 0.f);
      const CVector3f& face = direction.CanBeNormalized() ? direction.AsNormalized()
                                                          : player.GetTransform().GetForward();
      const float height = 0.5f + player.GetTranslation().GetZ();
      result = player.GetTranslation() + range * face;
      result.SetZ(height);
    } else {
      const CVector3f forward = player.GetTransform().GetForward();
      const float height = 0.5f + aimPos.GetZ();
      result = aimPos + range * forward;
      result.SetZ(height);
    }
  } else if (const CActor* actor =
                 TCastToConstPtr< CActor >(mgr.GetObjectById(GetAttackTargetId()))) {
    const CVector3f direction((GetTranslation() - actor->GetTranslation()).DropZ(), 0.f);
    const CVector3f& face =
        direction.CanBeNormalized() ? direction.AsNormalized() : actor->GetTransform().GetForward();
    const float height = 0.5f + actor->GetTranslation().GetZ();
    result = actor->GetTranslation() + range * face;
    result.SetZ(height);
  }
  return result;
}

bool CMetroid::IsPlayerInFluid(const CStateManager& mgr) const {
  const CPlayer& player = *mgr.GetPlayer();
  if (player.IsInFluid()) {
    if (player.InFluidId() != kInvalidUniqueId) {
      const float aimZ = player.GetAimPosition(mgr, 0.f).GetZ();
      if (const CScriptWater* water =
              TCastToConstPtr< CScriptWater >(mgr.GetObjectById(player.InFluidId()))) {
        return aimZ < water->GetTriggerBoundsWR().GetMaxPoint().GetZ();
      }
    }
    return true;
  }
  return false;
}

void CMetroid::UpdateAILogicTimers(float dt, CStateManager& mgr) {
  if (IsTargetGettingSucked(mgr)) {
    mAttackChance = x308_attackTimeVariation * mgr.Random()->Float() + x304_averageAttackTime;
  } else if (mAttackChance > 0.f) {
    const float delta =
        mgr.GetPlayer()->GetMorphballTransitionState() == CPlayer::kMS_Morphed ? 2.f * dt : dt;
    mAttackChance -= delta;
  }
}

bool CMetroid::CanStartAttack(CStateManager& mgr) const {
  if (mAttackChance <= 0.f) {
    const CPlayer& player = *mgr.GetPlayer();
    if (mAttackTarget == player.GetUniqueId()) {
      if (IsPlayerInFluid(mgr) ||
          (player.GetMorphballTransitionState() == CPlayer::kMS_Morphed &&
           player.GetMorphBall()->GetSpiderBallState() == CMorphBall::kSBS_Active)) {
        return false;
      }
      if (mgr.GetPlayer()->GetMorphballTransitionState() != CPlayer::kMS_Morphed &&
          IsBetaMetroidAttackingPlayer(mgr)) {
        return false;
      }
    }
    const CEntity* target = mgr.GetObjectById(mAttackTarget);
    if (target != nullptr && target->GetCurrentAreaId() == GetCurrentAreaId()) {
      return !IsTargetGettingSucked(mgr);
    }
  }
  return false;
}
