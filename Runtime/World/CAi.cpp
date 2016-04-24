#include "CAi.hpp"
#include "Character/CModelData.hpp"
#include "CStateManager.hpp"

namespace urde
{

CAiFuncMap::CAiFuncMap()
{
    /* Ai States */
    x0_stateFuncs["Patrol"] = &CAi::Patrol;
    x0_stateFuncs["FollowPattern"] = &CAi::FollowPattern;
    x0_stateFuncs["Dead"] = &CAi::Dead;
    x0_stateFuncs["PathFind"] = &CAi::PathFind;
    x0_stateFuncs["Start"] = &CAi::Start;
    x0_stateFuncs["SelectTarget"] = &CAi::SelectTarget;
    x0_stateFuncs["TargetPatrol"] = &CAi::TargetPatrol;
    x0_stateFuncs["TargetPlayer"] = &CAi::TargetPlayer;
    x0_stateFuncs["TargetCover"] = &CAi::TargetCover;
    x0_stateFuncs["Halt"] = &CAi::Halt;
    x0_stateFuncs["Walk"] = &CAi::Walk;
    x0_stateFuncs["Run"] = &CAi::Run;
    x0_stateFuncs["Generate"] = &CAi::Generate;
    x0_stateFuncs["Deactivate"] = &CAi::Deactivate;
    x0_stateFuncs["Attack"] = &CAi::Attack;
    x0_stateFuncs["LoopedAttack"] = &CAi::LoopedAttack;
    x0_stateFuncs["JumpBack"] = &CAi::JumpBack;
    x0_stateFuncs["DoubleSnap"] = &CAi::DoubleSnap;
    x0_stateFuncs["Shuffle"] = &CAi::Shuffle;
    x0_stateFuncs["TurnAround"] = &CAi::TurnAround;
    x0_stateFuncs["Skid"] = &CAi::Skid;
    x0_stateFuncs["Active"] = &CAi::Active;
    x0_stateFuncs["InActive"] = &CAi::InActive;
    x0_stateFuncs["CoverAttack"] = &CAi::CoverAttack;
    x0_stateFuncs["Crouch"] = &CAi::Crouch;
    x0_stateFuncs["FadeIn"] = &CAi::FadeIn;
    x0_stateFuncs["FadeOut"] = &CAi::FadeOut;
    x0_stateFuncs["GetUp"] = &CAi::GetUp;
    x0_stateFuncs["Taunt"] = &CAi::Taunt;
    x0_stateFuncs["Suck"] = &CAi::Suck;
    x0_stateFuncs["Flee"] = &CAi::Flee;
    x0_stateFuncs["Lurk"] = &CAi::Lurk;
    x0_stateFuncs["ProjectileAttack"] = &CAi::ProjectileAttack;
    x0_stateFuncs["Flinch"] = &CAi::Flinch;
    x0_stateFuncs["Hurled"] = &CAi::Hurled;
    x0_stateFuncs["TelegraphAttack"] = &CAi::TelegraphAttack;
    x0_stateFuncs["Jump"] = &CAi::Jump;
    x0_stateFuncs["Explode"] = &CAi::Explode;
    x0_stateFuncs["Dodge"] = &CAi::Dodge;
    x0_stateFuncs["Retreat"] = &CAi::Retreat;
    x0_stateFuncs["Cover"] = &CAi::Cover;
    x0_stateFuncs["Approach"] = &CAi::Approach;
    x0_stateFuncs["WallHang"] = &CAi::WallHang;
    x0_stateFuncs["WallDetach"] = &CAi::WallDetach;
    x0_stateFuncs["Enraged"] = &CAi::Enraged;
    x0_stateFuncs["SpecialAttack"] = &CAi::SpecialAttack;
    x0_stateFuncs["Growth"] = &CAi::Growth;
    x0_stateFuncs["Faint"] = &CAi::Faint;
    x0_stateFuncs["Land"] = &CAi::Land;
    x0_stateFuncs["Bounce"] = &CAi::Bounce;
    x0_stateFuncs["PathFindEx"] = &CAi::PathFindEx;
    x0_stateFuncs["Dizzy"] = &CAi::Dizzy;
    x0_stateFuncs["CallForBackup"] = &CAi::CallForBackup;
    x0_stateFuncs["BulbAttack"] = &CAi::BulbAttack;
    x0_stateFuncs["PodAttack"] = &CAi::PodAttack;

    /* Ai Triggers */
    x10_triggerFuncs["InAttackPosition"] = &CAi::InAttackPosition;
    x10_triggerFuncs["Leash"] = &CAi::Leash;
    x10_triggerFuncs["OffLine"] = &CAi::OffLine;
    x10_triggerFuncs["Attacked"] = &CAi::Attacked;
    x10_triggerFuncs["PathShagged"] = &CAi::PathShagged;
    x10_triggerFuncs["TooClose"] = &CAi::TooClose;
    x10_triggerFuncs["InRange"] = &CAi::InRange;
    x10_triggerFuncs["InMaxRange"] = &CAi::InMaxRange;
    x10_triggerFuncs["InDetectionRange"] = &CAi::InDetectionRange;
    x10_triggerFuncs["SpotPlayer"] = &CAi::SpotPlayer;
    x10_triggerFuncs["PlayerSpot"] = &CAi::PlayerSpot;
    x10_triggerFuncs["PatternOver"] = &CAi::PatternOver;
    x10_triggerFuncs["PatternShagged"] = &CAi::PatternShagged;
    x10_triggerFuncs["HasAttackPattern"] = &CAi::HasAttackPattern;
    x10_triggerFuncs["HasPatrolPath"] = &CAi::HasPatrolPath;
    x10_triggerFuncs["HasRetreatPattern"] = &CAi::HasRetreatPattern;
    x10_triggerFuncs["Delay"] = &CAi::Delay;
    x10_triggerFuncs["RandomDelay"] = &CAi::RandomDelay;
    x10_triggerFuncs["FixedDelay"] = &CAi::FixedDelay;
    x10_triggerFuncs["Default"] = &CAi::Default;
    x10_triggerFuncs["AnimOver"] = &CAi::AnimOver;
    x10_triggerFuncs["ShouldAttack"] = &CAi::ShouldAttack;
    x10_triggerFuncs["ShouldDoubleSnap"] = &CAi::ShouldDoubleSnap;
    x10_triggerFuncs["InPosition"] = &CAi::InPosition;
    x10_triggerFuncs["ShouldTurn"] = &CAi::ShouldTurn;
    x10_triggerFuncs["HitSomething"] = &CAi::HitSomething;
    x10_triggerFuncs["ShouldJumpBack"] = &CAi::ShouldJumpBack;
    x10_triggerFuncs["Stuck"] = &CAi::Stuck;
    x10_triggerFuncs["NoPathNodes"] = &CAi::NoPathNodes;
    x10_triggerFuncs["Landed"] = &CAi::Landed;
    x10_triggerFuncs["HearShot"] = &CAi::HearShot;
    x10_triggerFuncs["HearPlayer"] = &CAi::HearPlayer;
    x10_triggerFuncs["CoverCheck"] = &CAi::CoverCheck;
    x10_triggerFuncs["CoverFind"] = &CAi::CoverFind;
    x10_triggerFuncs["CoverBlown"] = &CAi::CoverBlown;
    x10_triggerFuncs["CoverNearlyBlown"] = &CAi::CoverNearlyBlown;
    x10_triggerFuncs["CoveringFire"] = &CAi::CoveringFire;
    x10_triggerFuncs["GotUp"] = &CAi::GotUp;
    x10_triggerFuncs["LineOfSight"] = &CAi::LineOfSight;
    x10_triggerFuncs["AggressionCheck"] = &CAi::AggressionCheck;
    x10_triggerFuncs["AttackOver"] = &CAi::AttackOver;
    x10_triggerFuncs["ShouldTaunt"] = &CAi::ShouldTaunt;
    x10_triggerFuncs["Inside"] = &CAi::Inside;
    x10_triggerFuncs["ShouldFire"] = &CAi::ShouldFire;
    x10_triggerFuncs["ShouldFlinch"] = &CAi::ShouldFlinch;
    x10_triggerFuncs["PatrolPathOver"] = &CAi::PatrolPathOver;
    x10_triggerFuncs["ShouldDodge"] = &CAi::ShouldDodge;
    x10_triggerFuncs["ShouldRetreat"] = &CAi::ShouldRetreat;
    x10_triggerFuncs["ShouldCrouch"] = &CAi::ShouldCrouch;
    x10_triggerFuncs["ShouldMove"] = &CAi::ShouldMove;
    x10_triggerFuncs["ShotAt"] = &CAi::ShotAt;
    x10_triggerFuncs["HasTargetingPoint"] = &CAi::HasTargetingPoint;
    x10_triggerFuncs["ShouldWallHang"] = &CAi::ShouldWallHang;
    x10_triggerFuncs["SetAIStage"] = &CAi::SetAIStage;
    x10_triggerFuncs["AIStage"] = &CAi::AIStage;
    x10_triggerFuncs["StartAttack"] = &CAi::StartAttack;
    x10_triggerFuncs["BreakAttack"] = &CAi::BreakAttack;
    x10_triggerFuncs["ShouldStrafe"] = &CAi::ShouldStrafe;
    x10_triggerFuncs["ShouldSpecialAttack"] = &CAi::ShouldSpecialAttack;
    x10_triggerFuncs["LostInterest"] = &CAi::LostInterest;
    x10_triggerFuncs["CodeTrigger"] = &CAi::CodeTrigger;
    x10_triggerFuncs["BounceFind"] = &CAi::BounceFind;
    x10_triggerFuncs["Random"] = &CAi::Random;
    x10_triggerFuncs["FixedRandom"] = &CAi::FixedRandom;
    x10_triggerFuncs["IsDizzy"] = &CAi::IsDizzy;
    x10_triggerFuncs["ShouldCallForBackup"] = &CAi::ShouldCallForBackup;

    CAi::CreateFuncLookup(this);
}

CAiStateFunc CAiFuncMap::GetStateFunc(const char* func)
{
    if (x0_stateFuncs.find(func) == x0_stateFuncs.end())
        return nullptr;
    return x0_stateFuncs[func];
}

CAiTriggerFunc CAiFuncMap::GetTriggerFunc(const char* func)
{
    if (x10_triggerFuncs.find(func) == x10_triggerFuncs.end())
        return nullptr;
    return x10_triggerFuncs[func];
}
static CMaterialList MakeAiMaterialList(const CMaterialList& in)
{
    CMaterialList ret = in;
    ret.Add(EMaterialTypes::FourtyEight);
    ret.Add(EMaterialTypes::TwentyOne);
    return ret;
}

CAi::CAi(TUniqueId uid, bool active, const std::string& name, const CEntityInfo& info, const zeus::CTransform& xf,
         CModelData&& mData, const zeus::CAABox& box, float f1, const CHealthInfo& hInfo, const CDamageVulnerability& dmgVuln,
         const CMaterialList& list, ResId, const CActorParameters& actorParams, float f2, float f3)
    : CPhysicsActor(uid, active, name, info, xf, std::move(mData), MakeAiMaterialList(list), box, SMoverData(f1), actorParams,
                    f2, f3),
      x258_healthInfo(hInfo),
      x260_damageVulnerability(dmgVuln)
{
}

CAiStateFunc CAi::GetStateFunc(const char* func)
{
    return m_FuncMap->GetStateFunc(func);
}

CAiTriggerFunc CAi::GetTrigerFunc(const char* func)
{
    return m_FuncMap->GetTriggerFunc(func);
}
void CAi::CreateFuncLookup(CAiFuncMap* funcMap)
{
    m_FuncMap = funcMap;
}
CAiFuncMap* CAi::m_FuncMap = nullptr;

}
