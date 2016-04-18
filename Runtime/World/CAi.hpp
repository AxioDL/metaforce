#ifndef __URDE_CAI_HPP__
#define __URDE_CAI_HPP__

#include "GCNTypes.hpp"
#include "ScriptObjectSupport.hpp"
#include "RetroTypes.hpp"
#include "CEntity.hpp"
#include "CPhysicsActor.hpp"

#include "zeus/zeus.hpp"

namespace urde
{

/* TODO: Figure out what all this is for and move it somewhere appropriate */
enum class EStateMsg
{
};

enum class EListenNoiseType
{
};

class CAiFuncMap
{
    /* TODO: Figure out return type, I think it's a string */
    void GetStateFunc(const char*);
    void GetTriggerFunc(const char*);
};

/* TODO: Move these */
class CHealthInfo
{
    float x0_;
    float x4_;
public:
    CHealthInfo(CInputStream& in) : x0_(in.readFloatBig()), x4_(in.readFloatBig()) {}
};

enum class EVulnerability
{
};

class CDamageVulnerability
{
    EVulnerability x0_power;
    EVulnerability x4_ice;
    EVulnerability x8_wave;
    EVulnerability xc_plasma;
    EVulnerability x10_bomb;
    EVulnerability x14_powerbomb;
    EVulnerability x18_missile;
    EVulnerability x1c_boostBall;
    EVulnerability x20_phazon;
    EVulnerability x24_enemyWp1;
    EVulnerability x28_enemyWp2Poison;
    EVulnerability x2c_enemyWp3Lava;
    EVulnerability x30_enemyWp4;
    EVulnerability x34_unk1;
    EVulnerability x38_unk2;
    EVulnerability x3c_unk3;

#if 0
    struct ChargedBeams : BigYAML
    {
        DECL_YAML
        Value<atUint32> propertyCount;
        Value<atUint32> power;
        Value<atUint32> ice;
        Value<atUint32> wave;
        Value<atUint32> plasma;
        Value<atUint32> phazon;
    } chargedBeams;

    struct BeamCombos : BigYAML
    {
        DECL_YAML
        Value<atUint32> propertyCount;
        Value<atUint32> superMissiles;
        Value<atUint32> iceSpreader;
        Value<atUint32> wavebuster;
        Value<atUint32> flameThrower;
        Value<atUint32> phazonCombo;
    } beamCombos;
#endif

public:
    CDamageVulnerability(CInputStream& in) {}
};

class CStateManager;
class CAi : public CPhysicsActor
{
    CHealthInfo          x240_healthInfo;
    CDamageVulnerability x248_damageVulnerability;
public:

    void CreateFuncLookup(CAiFuncMap* funcMap) { }
    void GetStateFunc(const char*) {}
    void GetTrigerFunc(const char*) {}
    void GetStateMachine() {}

    /* TODO: Figure out the return types, if any, and fix these prototypes */
    virtual void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) {}
    virtual CHealthInfo HealthInfo(CStateManager&) { return x240_healthInfo; }
    virtual CHealthInfo GetHealthInfo(const CStateManager&) { return x240_healthInfo; }
    virtual CDamageVulnerability GetDamageVulnerability()  { return x248_damageVulnerability; }

    virtual void TakeDamage(const zeus::CVector3f&, float) {}
    virtual bool CanBeShot(const CStateManager&, int) { return true; }
    virtual bool IsListening() { return false; }
    virtual int Listen(const zeus::CVector3f&, EListenNoiseType) { return 0; }

    virtual void Patrol(CStateManager&, EStateMsg, float) {}
    virtual void FollowPattern(CStateManager&, EStateMsg, float) {}
    virtual void Dead(CStateManager&, EStateMsg, float) {}
    virtual void PathFind(CStateManager&, EStateMsg, float) {}
    virtual void Start(CStateManager&, EStateMsg, float) {}
    virtual void SelectTarget(CStateManager&, EStateMsg, float) {}
    virtual void TargetPatrol(CStateManager&, EStateMsg, float) {}
    virtual void TargetPlayer(CStateManager&, EStateMsg, float) {}
    virtual void TargetCover(CStateManager&, EStateMsg, float) {}
    virtual void Halt(CStateManager&, EStateMsg, float) {}
    virtual void Walk(CStateManager&, EStateMsg, float) {}
    virtual void Run(CStateManager&, EStateMsg, float) {}
    virtual void Generate(CStateManager&, EStateMsg, float) {}
    virtual void Deactivate(CStateManager&, EStateMsg, float) {}
    virtual void Attack(CStateManager&, EStateMsg, float) {}
    virtual void LoopedAttack(CStateManager&, EStateMsg, float) {}
    virtual void JumpBack(CStateManager&, EStateMsg, float) {}
    virtual void DoubleSnap(CStateManager&, EStateMsg, float) {}
    virtual void Shuffle(CStateManager&, EStateMsg, float) {}
    virtual void TurnAround(CStateManager&, EStateMsg, float) {}
    virtual void Skid(CStateManager&, EStateMsg, float) {}
    virtual void Active(CStateManager&, EStateMsg, float) {}
    virtual void InActive(CStateManager&, EStateMsg, float) {}
    virtual void CoverAttack(CStateManager&, EStateMsg, float) {}
    virtual void Crouch(CStateManager&, EStateMsg, float) {}
    virtual void FadeIn(CStateManager&, EStateMsg, float) {}
    virtual void FadeOut(CStateManager&, EStateMsg, float) {}
    virtual void GetUp(CStateManager&, EStateMsg, float) {}
    virtual void Taunt(CStateManager&, EStateMsg, float) {}
    virtual void Suck(CStateManager&, EStateMsg, float) {}
    virtual void Flee(CStateManager&, EStateMsg, float) {}
    virtual void Lurk(CStateManager&, EStateMsg, float) {}
    virtual void ProjectileAttack(CStateManager&, EStateMsg, float) {}
    virtual void Flinch(CStateManager&, EStateMsg, float) {}
    virtual void Hurled(CStateManager&, EStateMsg, float) {}
    virtual void TelegraphAttack(CStateManager&, EStateMsg, float) {}
    virtual void Jump(CStateManager&, EStateMsg, float) {}
    virtual void Explode(CStateManager&, EStateMsg, float) {}
    virtual void Dodge(CStateManager&, EStateMsg, float) {}
    virtual void Retreat(CStateManager&, EStateMsg, float) {}
    virtual void Cover(CStateManager&, EStateMsg, float) {}
    virtual void Approach(CStateManager&, EStateMsg, float) {}
    virtual void WallHang(CStateManager&, EStateMsg, float) {}
    virtual void WallDetach(CStateManager&, EStateMsg, float) {}
    virtual void Enraged(CStateManager&, EStateMsg, float) {}
    virtual void SpecialAttack(CStateManager&, EStateMsg, float) {}
    virtual void Growth(CStateManager&, EStateMsg, float) {}
    virtual void Faint(CStateManager&, EStateMsg, float) {}
    virtual void BulbAttack(CStateManager&, EStateMsg, float) {}
    virtual void PodAttack(CStateManager&, EStateMsg, float) {}

    virtual void InAttackPosition(CStateManager&, float) {}
    virtual void Leash(CStateManager&, float) {}
    virtual void OffLine(CStateManager&,float) {}
    virtual void Attacked(CStateManager&, float) {}
    virtual void PathShagged(CStateManager&,float) {}
    virtual void PathOver(CStateManager&,float) {}
    virtual void TooClose(CStateManager&,float) {}
    virtual void InRange(CStateManager&,float) {}
    virtual void InMaxRange(CStateManager&,float) {}
    virtual void InDetectionRange(CStateManager&,float) {}
    virtual void SpotPlayer(CStateManager&,float) {}
    virtual void PlayerSpot(CStateManager&,float) {}
    virtual void PatternOver(CStateManager&,float) {}
    virtual void PatternedShagged(CStateManager&,float) {}
    virtual void HasAttackPattern(CStateManager&,float) {}
    virtual void HasPatrolPath(CStateManager&,float) {}
    virtual void HasRetreatPattern(CStateManager&,float) {}
    virtual void Delay(CStateManager&,float) {}
    virtual void RandomDelay(CStateManager&,float) {}
    virtual void FixedDelay(CStateManager&,float) {}
    virtual void AnimOver(CStateManager&, float) {}
    virtual void ShouldAttack(CStateManager&,float) {}
    virtual void ShouldDoubleSnap(CStateManager&,float) {}
    virtual void InPosition(CStateManager&,float) {}
    virtual void ShouldTurn(CStateManager&,float) {}
    virtual void HitSomething(CStateManager&,float) {}
    virtual void ShouldJumpBack(CStateManager&,float) {}
    virtual void Stuck(CStateManager&,float) {}
    virtual void NoPathNodes(CStateManager&,float) {}
    virtual void Landed(CStateManager&,float) {}
    virtual void HearShot(CStateManager&,float) {}
    virtual void HearPlayer(CStateManager&,float) {}
    virtual void CoverCheck(CStateManager&, float) {}
    virtual void CoverFind(CStateManager&, float) {}
    virtual void CoverBlown(CStateManager&, float) {}
    virtual void CoverNearlyBlown(CStateManager&, float) {}
    virtual void CoveringFire(CStateManager&, float) {}
    virtual void GotUp(CStateManager&,float) {}
    virtual void LineOfSight(CStateManager&,float) {}
    virtual void AggressionCheck(CStateManager&, float) {}
    virtual void AttackOver(CStateManager&, float) {}
    virtual void ShouldTaunt(CStateManager&,float) {}
    virtual void Inside(CStateManager&,float) {}
    virtual void ShouldFire(CStateManager&,float) {}
    virtual void ShouldFlinch(CStateManager&,float) {}
    virtual void PatrolPathOver(CStateManager&,float) {}
    virtual void ShouldDodge(CStateManager&,float) {}
    virtual void ShouldRetreat(CStateManager&,float) {}
    virtual void ShouldCrouch(CStateManager&,float) {}
    virtual void ShouldMove(CStateManager&,float) {}
    virtual void ShotAt(CStateManager&,float) {}
    virtual void HasTargettingPoint(CStateManager&,float) {}
    virtual void ShouldWallHang(CStateManager&,float) {}
    virtual void SetAIStage(CStateManager&,float) {}
    virtual void AIStage(CStateManager&,float) {}
    virtual void StartAttack(CStateManager&,float) {}
    virtual void BreakAttack(CStateManager&, float) {}
    virtual void ShoulStrafe(CStateManager&,float) {}
    virtual void ShouldSpecialAttack(CStateManager&,float) {}
    virtual void LostInterest(CStateManager&,float) {}
};

}

#endif // __URDE_CAI_HPP__
