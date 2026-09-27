#include "MetroidPrime/Enemies/CAi.hpp"
#include "Collision/CMaterialList.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "MetroidPrime/CActorLights.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CEntityInfo.hpp"
#include "MetroidPrime/CFluidPlaneCPU.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "MetroidPrime/CSimpleShadow.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Enemies/CAiFuncMap.hpp"
#include "MetroidPrime/Enemies/CStateMachine.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"

#include <Kyoto/CSimplePool.hpp>
#include <Kyoto/SObjectTag.hpp>
#include <rstl/algorithm.hpp>
#include <rstl/vector.hpp>
#include <string.h>

namespace {
const float kMaxSplashEnergy = 30000.f;

struct cstr_less {
  bool operator()(const char* a, const char* b) const { return strcmp(a, b) < 0; }
};

} // namespace

CAiFuncMap* CAi::mFuncMap = nullptr;
const char* gkStateNames[] = {
    "Patrol",        "FollowPattern", "Dead",
    "PathFind",      "Start",         "SelectTarget",
    "TargetPatrol",  "TargetPlayer",  "TargetCover",
    "Halt",          "Walk",          "Run",
    "Generate",      "Deactivate",    "Attack",
    "LoopedAttack",  "JumpBack",      "DoubleSnap",
    "Shuffle",       "TurnAround",    "Skid",
    "Active",        "InActive",      "CoverAttack",
    "Crouch",        "FadeIn",        "FadeOut",
    "GetUp",         "Taunt",         "Suck",
    "Flee",          "Lurk",          "ProjectileAttack",
    "Flinch",        "Hurled",        "TelegraphAttack",
    "Jump",          "Explode",       "Dodge",
    "Retreat",       "Cover",         "Approach",
    "WallHang",      "WallDetach",    "Enraged",
    "SpecialAttack", "Growth",        "Faint",
    "Land",          "Bounce",        "PathFindEx",
    "Dizzy",         "CallForBackup", "BulbAttack",
    "PodAttack",
};

const CAiStateFunc gkStateFuncs[] = {
    &CAi::Patrol,        &CAi::FollowPattern, &CAi::Dead,
    &CAi::PathFind,      &CAi::Start,         &CAi::SelectTarget,
    &CAi::TargetPatrol,  &CAi::TargetPlayer,  &CAi::TargetCover,
    &CAi::Halt,          &CAi::Walk,          &CAi::Run,
    &CAi::Generate,      &CAi::Deactivate,    &CAi::Attack,
    &CAi::LoopedAttack,  &CAi::JumpBack,      &CAi::DoubleSnap,
    &CAi::Shuffle,       &CAi::TurnAround,    &CAi::Skid,
    &CAi::Active,        &CAi::InActive,      &CAi::CoverAttack,
    &CAi::Crouch,        &CAi::FadeIn,        &CAi::FadeOut,
    &CAi::GetUp,         &CAi::Taunt,         &CAi::Suck,
    &CAi::Flee,          &CAi::Lurk,          &CAi::ProjectileAttack,
    &CAi::Flinch,        &CAi::Hurled,        &CAi::TelegraphAttack,
    &CAi::Jump,          &CAi::Explode,       &CAi::Dodge,
    &CAi::Retreat,       &CAi::Cover,         &CAi::Approach,
    &CAi::WallHang,      &CAi::WallDetach,    &CAi::Enraged,
    &CAi::SpecialAttack, &CAi::Growth,        &CAi::Faint,
    &CAi::Land,          &CAi::Bounce,        &CAi::PathFindEx,
    &CAi::Dizzy,         &CAi::CallForBackup, &CAi::BulbAttack,
    &CAi::PodAttack,
};
const int kStateCount = ARRAY_SIZE(gkStateFuncs);

const char* gkTriggerNames[] = {
    "InAttackPosition",
    "Leash",
    "OffLine",
    "Attacked",
    "PathShagged",
    "PathOver",
    "PathFound",
    "TooClose",
    "InRange",
    "InMaxRange",
    "InDetectionRange",
    "SpotPlayer",
    "PlayerSpot",
    "PatternOver",
    "PatternShagged",
    "HasAttackPattern",
    "HasPatrolPath",
    "HasRetreatPattern",
    "Delay",
    "RandomDelay",
    "FixedDelay",
    "Default",
    "AnimOver",
    "ShouldAttack",
    "ShouldDoubleSnap",
    "InPosition",
    "ShouldTurn",
    "HitSomething",
    "ShouldJumpBack",
    "Stuck",
    "NoPathNodes",
    "Landed",
    "HearShot",
    "HearPlayer",
    "CoverCheck",
    "CoverFind",
    "CoverBlown",
    "CoverNearlyBlown",
    "CoveringFire",
    "GotUp",
    "LineOfSight",
    "AggressionCheck",
    "AttackOver",
    "ShouldTaunt",
    "Inside",
    "ShouldFire",
    "ShouldFlinch",
    "PatrolPathOver",
    "ShouldDodge",
    "ShouldRetreat",
    "ShouldCrouch",
    "ShouldMove",
    "ShotAt",
    "HasTargetingPoint",
    "ShouldWallHang",
    "SetAIStage",
    "AIStage",
    "StartAttack",
    "BreakAttack",
    "ShouldStrafe",
    "ShouldSpecialAttack",
    "LostInterest",
    "CodeTrigger",
    "BounceFind",
    "Random",
    "FixedRandom",
    "IsDizzy",
    "ShouldCallForBackup",
};

const CAiTriggerFunc gkTriggerFuncs[] = {
    &CAi::InAttackPosition,
    &CAi::Leash,
    &CAi::OffLine,
    &CAi::Attacked,
    &CAi::PathShagged,
    &CAi::PathOver,
    &CAi::PathFound,
    &CAi::TooClose,
    &CAi::InRange,
    &CAi::InMaxRange,
    &CAi::InDetectionRange,
    &CAi::SpotPlayer,
    &CAi::PlayerSpot,
    &CAi::PatternOver,
    &CAi::PatternShagged,
    &CAi::HasAttackPattern,
    &CAi::HasPatrolPath,
    &CAi::HasRetreatPattern,
    &CAi::Delay,
    &CAi::RandomDelay,
    &CAi::FixedDelay,
    &CAi::Default,
    &CAi::AnimOver,
    &CAi::ShouldAttack,
    &CAi::ShouldDoubleSnap,
    &CAi::InPosition,
    &CAi::ShouldTurn,
    &CAi::HitSomething,
    &CAi::ShouldJumpBack,
    &CAi::Stuck,
    &CAi::NoPathNodes,
    &CAi::Landed,
    &CAi::HearShot,
    &CAi::HearPlayer,
    &CAi::CoverCheck,
    &CAi::CoverFind,
    &CAi::CoverBlown,
    &CAi::CoverNearlyBlown,
    &CAi::CoveringFire,
    &CAi::GotUp,
    &CAi::LineOfSight,
    &CAi::AggressionCheck,
    &CAi::AttackOver,
    &CAi::ShouldTaunt,
    &CAi::Inside,
    &CAi::ShouldFire,
    &CAi::ShouldFlinch,
    &CAi::PatrolPathOver,
    &CAi::ShouldDodge,
    &CAi::ShouldRetreat,
    &CAi::ShouldCrouch,
    &CAi::ShouldMove,
    &CAi::ShotAt,
    &CAi::HasTargetingPoint,
    &CAi::ShouldWallHang,
    &CAi::SetAIStage,
    &CAi::AIStage,
    &CAi::StartAttack,
    &CAi::BreakAttack,
    &CAi::ShouldStrafe,
    &CAi::ShouldSpecialAttack,
    &CAi::LostInterest,
    &CAi::CodeTrigger,
    &CAi::BounceFind,
    &CAi::Random,
    &CAi::FixedRandom,
    &CAi::IsDizzy,
    &CAi::ShouldCallForBackup,
};

const int kTriggerCount = sizeof(gkTriggerNames) / sizeof(gkTriggerNames[0]);
CAiFuncMap::CAiFuncMap() {
  mStates.reserve(kStateCount);
  int i;
  for (i = 0; i < mStates.capacity(); ++i) {
    mStates.push_back(rstl::pair< const char*, CAiStateFunc >(gkStateNames[i], gkStateFuncs[i]));
  }
  mTriggers.reserve(kTriggerCount);
  for (i = 0; i < mTriggers.capacity(); ++i) {
    mTriggers.push_back(
        rstl::pair< const char*, CAiTriggerFunc >(gkTriggerNames[i], gkTriggerFuncs[i]));
  }
  rstl::sort_by_key(mStates, cstr_less());
  rstl::sort_by_key(mTriggers, cstr_less());
  CAi::CreateFuncLookup(this);
}

const CAiStateFunc CAiFuncMap::GetStateFunc(const char* const state) const {
  CAiStateFunc func = nullptr;
  rstl::vector< rstl::pair< const char*, CAiStateFunc > >::const_iterator it =
      rstl::find_by_key(mStates, state, cstr_less());
  if (it != mStates.end()) {
    func = it->second;
  }
  return func;
}

const CAiTriggerFunc CAiFuncMap::GetTriggerFunc(const char* trigger) const {
  rstl::vector< rstl::pair< const char*, CAiTriggerFunc > >::const_iterator it =
      rstl::find_by_key(mTriggers, trigger, cstr_less());
  return it->second;
}
CAi::CAi(TUniqueId uid, bool active, const rstl::string& name, const CEntityInfo& entityInfo,
         const CTransform4f& transform, const CModelData& modelData, const CAABox& bounds,
         const float mass, const CHealthInfo& healthInfo, const CDamageVulnerability& vuln,
         const CMaterialList& materialList, CAssetId stateMachine,
         const CActorParameters& actorParams, const float stepUp, const float stepDown)
: CPhysicsActor(uid, active, name, entityInfo, transform, modelData,
                materialList.Union(CMaterialList(kMT_AIBlock, kMT_CameraPassthrough)), bounds,
                SMoverData(mass), actorParams, stepUp, stepDown)
, mHealthInfo(healthInfo)
, mDamageVulnerability(vuln)
, mStateMachine(gpSimplePool->GetObj(SObjectTag(FourCC('AFSM'), stateMachine))) {
  mStateMachine.Lock();
  CreateShadowIfNeeded();
  if (GetShadow()) {
    SetDrawShadow(true);
    Shadow()->SetAlwaysCalculateRadius(false);
  }

  if (GetActorLights()) {
    ActorLights()->SetCastShadows(true);
  }
}

CAi::~CAi() {}

CHealthInfo* CAi::HealthInfo(CStateManager& mgr) { return &mHealthInfo; }
const CDamageVulnerability* CAi::GetDamageVulnerability() const {
  return &mDamageVulnerability;
}
CDamageVulnerability* CAi::GetDamageVulnerability() { return &mDamageVulnerability; }

void CAi::TakeDamage(const CVector3f& direction, float magnitude) {}

void CAi::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId other, CStateManager& mgr) {
  switch (msg) {
  case kSM_InitializedInArea:
    SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
        GetMaterialFilter().GetIncludeList().Union(CMaterialList(kMT_AIBlock)),
        GetMaterialFilter().GetExcludeList()));
  default:
    CPhysicsActor::AcceptScriptMsg(msg, other, mgr);
  }
}

void CAi::FluidFXThink(EFluidState state, CScriptWater& water, CStateManager& mgr) {
  switch (state) {
  case kFS_EnteredFluid:
  case kFS_LeftFluid: {
    const float dt = mgr.FluidPlaneManager()->GetLastSplashDeltaTime(GetUniqueId());
    if (dt >= 0.2f) {
      const float energy = 0.5f * GetMass() * GetVelocityWR().MagSquared();
      if (energy > 500.f) {
        const float intensity =
            0.1f + 0.4f * (CMath::Min(energy, kMaxSplashEnergy) - 500.f) / 29500.f;
        const float surfaceZ = water.GetSurfaceZ();
        const CVector3f& translation = GetTranslation();
        CVector3f pos(translation.GetX(), translation.GetY(), surfaceZ);
        mgr.FluidPlaneManager()->CreateSplash(GetUniqueId(), mgr, water, pos, intensity, true);
      }
    }
  }
  // Fall through: entering and leaving also create ripples.
  case kFS_InFluid: {
    const float dt = mgr.FluidPlaneManager()->GetLastRippleDeltaTime(GetUniqueId());
    if (dt >= (HealthInfo(mgr)->GetHP() > 0.f ? 0.2f : 0.7f)) {
      const float surfaceZ = water.GetSurfaceZ();
      const CVector3f& translation = GetTranslation();
      CVector3f center(translation.GetX(), translation.GetY(), surfaceZ);
      const CPlane& surface = water.GetWRSurfacePlane();
      const float mass = GetMass();
      water.FluidPlane().AddRipple(mass, GetUniqueId(), center, GetVelocityWR(), water, mgr,
                                   surface.GetNormal());
    }
    break;
  }
  default:
    break;
  }
}

void CAi::CreateFuncLookup(CAiFuncMap* map) { mFuncMap = map; }

const CAiStateFunc CAi::GetStateFunc(const char* state) { return mFuncMap->GetStateFunc(state); }
const CAiTriggerFunc CAi::GetTriggerFunc(const char* trigger) {
  return mFuncMap->GetTriggerFunc(trigger);
}

CStateMachine* CAi::GetStateMachine() {
  if (!mStateMachine.TryCache()) {
    return nullptr;
  }
  return mStateMachine.GetObject();
}

EWeaponCollisionResponseTypes CAi::GetCollisionResponseType(const CVector3f&, const CVector3f&,
                                                            const CWeaponMode&, int) const {
  return kWCR_EnemyNormal;
}

void CAi::Patrol(CStateManager&, EStateMsg, float) {}
void CAi::FollowPattern(CStateManager&, EStateMsg, float) {}
void CAi::Dead(CStateManager&, EStateMsg, float) {}
void CAi::PathFind(CStateManager&, EStateMsg, float) {}
void CAi::Start(CStateManager&, EStateMsg, float) {}
void CAi::SelectTarget(CStateManager&, EStateMsg, float) {}
void CAi::TargetPatrol(CStateManager&, EStateMsg, float) {}
void CAi::TargetPlayer(CStateManager&, EStateMsg, float) {}
void CAi::TargetCover(CStateManager&, EStateMsg, float) {}
void CAi::Halt(CStateManager&, EStateMsg, float) {}
void CAi::Walk(CStateManager&, EStateMsg, float) {}
void CAi::Run(CStateManager&, EStateMsg, float) {}
void CAi::Generate(CStateManager&, EStateMsg, float) {}
void CAi::Deactivate(CStateManager&, EStateMsg, float) {}
void CAi::Attack(CStateManager&, EStateMsg, float) {}
void CAi::LoopedAttack(CStateManager&, EStateMsg, float) {}
void CAi::JumpBack(CStateManager&, EStateMsg, float) {}
void CAi::DoubleSnap(CStateManager&, EStateMsg, float) {}
void CAi::Shuffle(CStateManager&, EStateMsg, float) {}
void CAi::TurnAround(CStateManager&, EStateMsg, float) {}
void CAi::Skid(CStateManager&, EStateMsg, float) {}
void CAi::Active(CStateManager&, EStateMsg, float) {}
void CAi::InActive(CStateManager&, EStateMsg, float) {}
void CAi::CoverAttack(CStateManager&, EStateMsg, float) {}
void CAi::Crouch(CStateManager&, EStateMsg, float) {}
void CAi::FadeIn(CStateManager&, EStateMsg, float) {}
void CAi::FadeOut(CStateManager&, EStateMsg, float) {}
void CAi::GetUp(CStateManager&, EStateMsg, float) {}
void CAi::Taunt(CStateManager&, EStateMsg, float) {}
void CAi::Suck(CStateManager&, EStateMsg, float) {}
void CAi::Flee(CStateManager&, EStateMsg, float) {}
void CAi::Lurk(CStateManager&, EStateMsg, float) {}
void CAi::ProjectileAttack(CStateManager&, EStateMsg, float) {}
void CAi::Flinch(CStateManager&, EStateMsg, float) {}
void CAi::Hurled(CStateManager&, EStateMsg, float) {}
void CAi::TelegraphAttack(CStateManager&, EStateMsg, float) {}
void CAi::Jump(CStateManager&, EStateMsg, float) {}
void CAi::Explode(CStateManager&, EStateMsg, float) {}
void CAi::Dodge(CStateManager&, EStateMsg, float) {}
void CAi::Retreat(CStateManager&, EStateMsg, float) {}
void CAi::Cover(CStateManager&, EStateMsg, float) {}
void CAi::Approach(CStateManager&, EStateMsg, float) {}
void CAi::WallHang(CStateManager&, EStateMsg, float) {}
void CAi::WallDetach(CStateManager&, EStateMsg, float) {}
void CAi::Enraged(CStateManager&, EStateMsg, float) {}
void CAi::SpecialAttack(CStateManager&, EStateMsg, float) {}
void CAi::Growth(CStateManager&, EStateMsg, float) {}
void CAi::Faint(CStateManager&, EStateMsg, float) {}
void CAi::Land(CStateManager&, EStateMsg, float) {}
void CAi::Bounce(CStateManager&, EStateMsg, float) {}
void CAi::PathFindEx(CStateManager&, EStateMsg, float) {}
void CAi::Dizzy(CStateManager&, EStateMsg, float) {}
void CAi::CallForBackup(CStateManager&, EStateMsg, float) {}
void CAi::BulbAttack(CStateManager&, EStateMsg, float) {}
void CAi::PodAttack(CStateManager&, EStateMsg, float) {}
bool CAi::InAttackPosition(CStateManager&, float) { return false; }
bool CAi::Leash(CStateManager&, float) { return false; }
bool CAi::OffLine(CStateManager&, float) { return false; }
bool CAi::Attacked(CStateManager&, float) { return false; }
bool CAi::PathShagged(CStateManager&, float) { return false; }
bool CAi::PathOver(CStateManager&, float) { return false; }
bool CAi::PathFound(CStateManager&, float) { return false; }
bool CAi::TooClose(CStateManager&, float) { return false; }
bool CAi::InRange(CStateManager&, float) { return false; }
bool CAi::InMaxRange(CStateManager&, float) { return false; }
bool CAi::InDetectionRange(CStateManager&, float) { return false; }
bool CAi::SpotPlayer(CStateManager&, float) { return false; }
bool CAi::PlayerSpot(CStateManager&, float) { return false; }
bool CAi::PatternOver(CStateManager&, float) { return false; }
bool CAi::PatternShagged(CStateManager&, float) { return false; }
bool CAi::HasAttackPattern(CStateManager&, float) { return false; }
bool CAi::HasPatrolPath(CStateManager&, float) { return false; }
bool CAi::HasRetreatPattern(CStateManager&, float) { return false; }
bool CAi::Delay(CStateManager&, float) { return false; }
bool CAi::RandomDelay(CStateManager&, float) { return false; }
bool CAi::FixedDelay(CStateManager&, float) { return false; }
bool CAi::Default(CStateManager&, float) { return false; }
bool CAi::AnimOver(CStateManager&, float) { return false; }
bool CAi::ShouldAttack(CStateManager&, float) { return false; }
bool CAi::ShouldDoubleSnap(CStateManager&, float) { return false; }
bool CAi::InPosition(CStateManager&, float) { return false; }
bool CAi::ShouldTurn(CStateManager&, float) { return false; }
bool CAi::HitSomething(CStateManager&, float) { return false; }
bool CAi::ShouldJumpBack(CStateManager&, float) { return false; }
bool CAi::Stuck(CStateManager&, float) { return false; }
bool CAi::NoPathNodes(CStateManager&, float) { return false; }
bool CAi::Landed(CStateManager&, float) { return false; }
bool CAi::HearShot(CStateManager&, float) { return false; }
bool CAi::HearPlayer(CStateManager&, float) { return false; }
bool CAi::CoverCheck(CStateManager&, float) { return false; }
bool CAi::CoverFind(CStateManager&, float) { return false; }
bool CAi::CoverBlown(CStateManager&, float) { return false; }
bool CAi::CoverNearlyBlown(CStateManager&, float) { return false; }
bool CAi::CoveringFire(CStateManager&, float) { return false; }
bool CAi::GotUp(CStateManager&, float) { return false; }
bool CAi::LineOfSight(CStateManager&, float) { return false; }
bool CAi::AggressionCheck(CStateManager&, float) { return false; }
bool CAi::AttackOver(CStateManager&, float) { return false; }
bool CAi::ShouldTaunt(CStateManager&, float) { return false; }
bool CAi::Inside(CStateManager&, float) { return false; }
bool CAi::ShouldFire(CStateManager&, float) { return false; }
bool CAi::ShouldFlinch(CStateManager&, float) { return false; }
bool CAi::PatrolPathOver(CStateManager&, float) { return false; }
bool CAi::ShouldDodge(CStateManager&, float) { return false; }
bool CAi::ShouldRetreat(CStateManager&, float) { return false; }
bool CAi::ShouldCrouch(CStateManager&, float) { return false; }
bool CAi::ShouldMove(CStateManager&, float) { return false; }
bool CAi::ShotAt(CStateManager&, float) { return false; }
bool CAi::HasTargetingPoint(CStateManager&, float) { return false; }
bool CAi::ShouldWallHang(CStateManager&, float) { return false; }
bool CAi::SetAIStage(CStateManager&, float) { return false; }
bool CAi::AIStage(CStateManager&, float) { return false; }
bool CAi::StartAttack(CStateManager&, float) { return false; }
bool CAi::BreakAttack(CStateManager&, float) { return false; }
bool CAi::ShouldStrafe(CStateManager&, float) { return false; }
bool CAi::ShouldSpecialAttack(CStateManager&, float) { return false; }
bool CAi::LostInterest(CStateManager&, float) { return false; }
bool CAi::CodeTrigger(CStateManager&, float) { return false; }
bool CAi::BounceFind(CStateManager&, float) { return false; }
bool CAi::Random(CStateManager&, float) { return false; }
bool CAi::FixedRandom(CStateManager&, float) { return false; }
bool CAi::IsDizzy(CStateManager&, float) { return false; }
bool CAi::ShouldCallForBackup(CStateManager&, float) { return false; }
