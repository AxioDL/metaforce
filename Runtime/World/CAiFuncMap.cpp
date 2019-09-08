#include "CAiFuncMap.hpp"
#include "CAi.hpp"
#include "CStateManager.hpp"

namespace urde {
CAiFuncMap::CAiFuncMap() {
  /* Ai States */
  x0_stateFuncs.emplace("Patrol", &CAi::Patrol);
  x0_stateFuncs.emplace("FollowPattern", &CAi::FollowPattern);
  x0_stateFuncs.emplace("Dead", &CAi::Dead);
  x0_stateFuncs.emplace("PathFind", &CAi::PathFind);
  x0_stateFuncs.emplace("Start", &CAi::Start);
  x0_stateFuncs.emplace("SelectTarget", &CAi::SelectTarget);
  x0_stateFuncs.emplace("TargetPatrol", &CAi::TargetPatrol);
  x0_stateFuncs.emplace("TargetPlayer", &CAi::TargetPlayer);
  x0_stateFuncs.emplace("TargetCover", &CAi::TargetCover);
  x0_stateFuncs.emplace("Halt", &CAi::Halt);
  x0_stateFuncs.emplace("Walk", &CAi::Walk);
  x0_stateFuncs.emplace("Run", &CAi::Run);
  x0_stateFuncs.emplace("Generate", &CAi::Generate);
  x0_stateFuncs.emplace("Deactivate", &CAi::Deactivate);
  x0_stateFuncs.emplace("Attack", &CAi::Attack);
  x0_stateFuncs.emplace("LoopedAttack", &CAi::LoopedAttack);
  x0_stateFuncs.emplace("JumpBack", &CAi::JumpBack);
  x0_stateFuncs.emplace("DoubleSnap", &CAi::DoubleSnap);
  x0_stateFuncs.emplace("Shuffle", &CAi::Shuffle);
  x0_stateFuncs.emplace("TurnAround", &CAi::TurnAround);
  x0_stateFuncs.emplace("Skid", &CAi::Skid);
  x0_stateFuncs.emplace("Active", &CAi::Active);
  x0_stateFuncs.emplace("InActive", &CAi::InActive);
  x0_stateFuncs.emplace("CoverAttack", &CAi::CoverAttack);
  x0_stateFuncs.emplace("Crouch", &CAi::Crouch);
  x0_stateFuncs.emplace("FadeIn", &CAi::FadeIn);
  x0_stateFuncs.emplace("FadeOut", &CAi::FadeOut);
  x0_stateFuncs.emplace("GetUp", &CAi::GetUp);
  x0_stateFuncs.emplace("Taunt", &CAi::Taunt);
  x0_stateFuncs.emplace("Suck", &CAi::Suck);
  x0_stateFuncs.emplace("Flee", &CAi::Flee);
  x0_stateFuncs.emplace("Lurk", &CAi::Lurk);
  x0_stateFuncs.emplace("ProjectileAttack", &CAi::ProjectileAttack);
  x0_stateFuncs.emplace("Flinch", &CAi::Flinch);
  x0_stateFuncs.emplace("Hurled", &CAi::Hurled);
  x0_stateFuncs.emplace("TelegraphAttack", &CAi::TelegraphAttack);
  x0_stateFuncs.emplace("Jump", &CAi::Jump);
  x0_stateFuncs.emplace("Explode", &CAi::Explode);
  x0_stateFuncs.emplace("Dodge", &CAi::Dodge);
  x0_stateFuncs.emplace("Retreat", &CAi::Retreat);
  x0_stateFuncs.emplace("Cover", &CAi::Cover);
  x0_stateFuncs.emplace("Approach", &CAi::Approach);
  x0_stateFuncs.emplace("WallHang", &CAi::WallHang);
  x0_stateFuncs.emplace("WallDetach", &CAi::WallDetach);
  x0_stateFuncs.emplace("Enraged", &CAi::Enraged);
  x0_stateFuncs.emplace("SpecialAttack", &CAi::SpecialAttack);
  x0_stateFuncs.emplace("Growth", &CAi::Growth);
  x0_stateFuncs.emplace("Faint", &CAi::Faint);
  x0_stateFuncs.emplace("Land", &CAi::Land);
  x0_stateFuncs.emplace("Bounce", &CAi::Bounce);
  x0_stateFuncs.emplace("PathFindEx", &CAi::PathFindEx);
  x0_stateFuncs.emplace("Dizzy", &CAi::Dizzy);
  x0_stateFuncs.emplace("CallForBackup", &CAi::CallForBackup);
  x0_stateFuncs.emplace("BulbAttack", &CAi::BulbAttack);
  x0_stateFuncs.emplace("PodAttack", &CAi::PodAttack);

  /* Ai Triggers */
  x10_triggerFuncs.emplace("InAttackPosition", &CAi::InAttackPosition);
  x10_triggerFuncs.emplace("Leash", &CAi::Leash);
  x10_triggerFuncs.emplace("OffLine", &CAi::OffLine);
  x10_triggerFuncs.emplace("Attacked", &CAi::Attacked);
  x10_triggerFuncs.emplace("PathShagged", &CAi::PathShagged);
  x10_triggerFuncs.emplace("PathOver", &CAi::PathOver);
  x10_triggerFuncs.emplace("PathFound", &CAi::PathFound);
  x10_triggerFuncs.emplace("TooClose", &CAi::TooClose);
  x10_triggerFuncs.emplace("InRange", &CAi::InRange);
  x10_triggerFuncs.emplace("InMaxRange", &CAi::InMaxRange);
  x10_triggerFuncs.emplace("InDetectionRange", &CAi::InDetectionRange);
  x10_triggerFuncs.emplace("SpotPlayer", &CAi::SpotPlayer);
  x10_triggerFuncs.emplace("PlayerSpot", &CAi::PlayerSpot);
  x10_triggerFuncs.emplace("PatternOver", &CAi::PatternOver);
  x10_triggerFuncs.emplace("PatternShagged", &CAi::PatternShagged);
  x10_triggerFuncs.emplace("HasAttackPattern", &CAi::HasAttackPattern);
  x10_triggerFuncs.emplace("HasPatrolPath", &CAi::HasPatrolPath);
  x10_triggerFuncs.emplace("HasRetreatPattern", &CAi::HasRetreatPattern);
  x10_triggerFuncs.emplace("Delay", &CAi::Delay);
  x10_triggerFuncs.emplace("RandomDelay", &CAi::RandomDelay);
  x10_triggerFuncs.emplace("FixedDelay", &CAi::FixedDelay);
  x10_triggerFuncs.emplace("Default", &CAi::Default);
  x10_triggerFuncs.emplace("AnimOver", &CAi::AnimOver);
  x10_triggerFuncs.emplace("ShouldAttack", &CAi::ShouldAttack);
  x10_triggerFuncs.emplace("ShouldDoubleSnap", &CAi::ShouldDoubleSnap);
  x10_triggerFuncs.emplace("InPosition", &CAi::InPosition);
  x10_triggerFuncs.emplace("ShouldTurn", &CAi::ShouldTurn);
  x10_triggerFuncs.emplace("HitSomething", &CAi::HitSomething);
  x10_triggerFuncs.emplace("ShouldJumpBack", &CAi::ShouldJumpBack);
  x10_triggerFuncs.emplace("Stuck", &CAi::Stuck);
  x10_triggerFuncs.emplace("NoPathNodes", &CAi::NoPathNodes);
  x10_triggerFuncs.emplace("Landed", &CAi::Landed);
  x10_triggerFuncs.emplace("HearShot", &CAi::HearShot);
  x10_triggerFuncs.emplace("HearPlayer", &CAi::HearPlayer);
  x10_triggerFuncs.emplace("CoverCheck", &CAi::CoverCheck);
  x10_triggerFuncs.emplace("CoverFind", &CAi::CoverFind);
  x10_triggerFuncs.emplace("CoverBlown", &CAi::CoverBlown);
  x10_triggerFuncs.emplace("CoverNearlyBlown", &CAi::CoverNearlyBlown);
  x10_triggerFuncs.emplace("CoveringFire", &CAi::CoveringFire);
  x10_triggerFuncs.emplace("GotUp", &CAi::GotUp);
  x10_triggerFuncs.emplace("LineOfSight", &CAi::LineOfSight);
  x10_triggerFuncs.emplace("AggressionCheck", &CAi::AggressionCheck);
  x10_triggerFuncs.emplace("AttackOver", &CAi::AttackOver);
  x10_triggerFuncs.emplace("ShouldTaunt", &CAi::ShouldTaunt);
  x10_triggerFuncs.emplace("Inside", &CAi::Inside);
  x10_triggerFuncs.emplace("ShouldFire", &CAi::ShouldFire);
  x10_triggerFuncs.emplace("ShouldFlinch", &CAi::ShouldFlinch);
  x10_triggerFuncs.emplace("PatrolPathOver", &CAi::PatrolPathOver);
  x10_triggerFuncs.emplace("ShouldDodge", &CAi::ShouldDodge);
  x10_triggerFuncs.emplace("ShouldRetreat", &CAi::ShouldRetreat);
  x10_triggerFuncs.emplace("ShouldCrouch", &CAi::ShouldCrouch);
  x10_triggerFuncs.emplace("ShouldMove", &CAi::ShouldMove);
  x10_triggerFuncs.emplace("ShotAt", &CAi::ShotAt);
  x10_triggerFuncs.emplace("HasTargetingPoint", &CAi::HasTargetingPoint);
  x10_triggerFuncs.emplace("ShouldWallHang", &CAi::ShouldWallHang);
  x10_triggerFuncs.emplace("SetAIStage", &CAi::SetAIStage);
  x10_triggerFuncs.emplace("AIStage", &CAi::AIStage);
  x10_triggerFuncs.emplace("StartAttack", &CAi::StartAttack);
  x10_triggerFuncs.emplace("BreakAttack", &CAi::BreakAttack);
  x10_triggerFuncs.emplace("ShouldStrafe", &CAi::ShouldStrafe);
  x10_triggerFuncs.emplace("ShouldSpecialAttack", &CAi::ShouldSpecialAttack);
  x10_triggerFuncs.emplace("LostInterest", &CAi::LostInterest);
  x10_triggerFuncs.emplace("CodeTrigger", &CAi::CodeTrigger);
  x10_triggerFuncs.emplace("BounceFind", &CAi::BounceFind);
  x10_triggerFuncs.emplace("Random", &CAi::Random);
  x10_triggerFuncs.emplace("FixedRandom", &CAi::FixedRandom);
  x10_triggerFuncs.emplace("IsDizzy", &CAi::IsDizzy);
  x10_triggerFuncs.emplace("ShouldCallForBackup", &CAi::ShouldCallForBackup);

  CAi::CreateFuncLookup(this);
}

CAiStateFunc CAiFuncMap::GetStateFunc(const char* func) {
  if (x0_stateFuncs.find(func) == x0_stateFuncs.end())
    return nullptr;
  return x0_stateFuncs[func];
}

CAiTriggerFunc CAiFuncMap::GetTriggerFunc(const char* func) {
  if (x10_triggerFuncs.find(func) == x10_triggerFuncs.end())
    return nullptr;
  return x10_triggerFuncs[func];
}
} // namespace urde
