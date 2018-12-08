#pragma once

#include "World/CPatterned.hpp"
#include "Weapon/CProjectileInfo.hpp"
#include "Character/CBoneTracking.hpp"
#include "Character/CIkChain.hpp"
#include "Character/CRagDoll.hpp"
#include "World/CPathFindSearch.hpp"
#include "Weapon/CBurstFire.hpp"

namespace urde::MP1
{
class CSpacePirate;

class CPirateRagDoll : public CRagDoll
{
    CSpacePirate* x6c_spacePirate;
    u16 x70_thudSfx;
    float x74_sfxTimer = 0.f;
    zeus::CVector3f x78_lastSFXPos;
    zeus::CVector3f x84_torsoImpulse;
    rstl::reserved_vector<TUniqueId, 4> x90_waypoints;
    rstl::reserved_vector<u32, 4> x9c_wpParticleIdxs;
    bool xb0_24_initSfx : 1;
public:
    CPirateRagDoll(CStateManager& mgr, CSpacePirate* sp, u16 thudSfx, u32 flags);

    void PreRender(const zeus::CVector3f& v, CModelData& mData);
    void Update(CStateManager& mgr, float dt, float waterTop);
    void Prime(CStateManager& mgr, const zeus::CTransform& xf, CModelData& mData);
    zeus::CVector3f& TorsoImpulse() { return x84_torsoImpulse; }
};

class CSpacePirate : public CPatterned
{
    friend class CPirateRagDoll;
public:
    DEFINE_PATTERNED(SpacePirate)
private:
    class CSpacePirateData
    {
        friend class CSpacePirate;
        float x0_;
        float x4_;
        float x8_;
        float xc_;
        float x10_;
        float x14_hearNoiseRange;
        u32 x18_flags;
        bool x1c_;
        CProjectileInfo x20_mainProjectileInfo;
        u16 x48_;
        CDamageInfo x4c_;
        float x68_;
        CProjectileInfo x6c_;
        float x94_;
        u16 x98_ragdollThudSfx;
        float x9c_averageNextShotTime;
        float xa0_nextShotTimeVariation;
        u16 xa4_;
        float xa8_aimDelayTime;
        u32 xac_firstBurstCount;
        float xb0_minCloakAlpha;
        float xb4_maxCloakAlpha;
        float xb8_;
        float xbc_;
        u16 xc0_;
        u16 xc2_;
        float xc4_;
        float xc8_;
    public:
        CSpacePirateData(CInputStream&, u32);
    };

    CSpacePirateData x568_pirateData;
    union
    {
        struct
        {
            bool x634_24_pendingAmbush : 1;
            bool x634_25_ceilingAmbush : 1;
            bool x634_26_ : 1;
            bool x634_27_melee : 1;
            bool x634_28_ : 1;
            bool x634_29_onlyAttackInRange : 1;
            bool x634_30_ : 1;
            bool x634_31_ : 1;
            bool x635_24_ : 1;
            bool x635_25_ : 1;
            bool x635_26_ : 1;
            bool x635_27_shadowPirate : 1;
            bool x635_28_ : 1;
            bool x635_29_ : 1;
            bool x635_30_floatingCorpse : 1;
            bool x635_31_ragdollNoAiCollision : 1;
            bool x636_24_trooper : 1;
            bool x636_25_hearNoise : 1;
            bool x636_26_ : 1;
            bool x636_27_ : 1;
            bool x636_28_ : 1;
            bool x636_29_ : 1;
            bool x636_30_ : 1;
            bool x636_31_ : 1;
            bool x637_24_ : 1;
            bool x637_25_ : 1;
            bool x637_26_hearPlayerFire : 1;
            bool x637_27_inProjectilePath : 1;
            bool x637_28_ : 1;
            bool x637_29_ : 1;
            bool x637_30_ : 1;
            bool x637_31_prevInCineCam : 1;
            bool x638_24_pendingFrenzyChance : 1;
            bool x638_25_ : 1;
            bool x638_26_ : 1;
            bool x638_27_ : 1;
            bool x638_28_ : 1;
            bool x638_29_ : 1;
            bool x638_30_ragdollOver : 1;
            bool x638_31_ : 1;
            bool x639_24_ : 1;
            bool x639_25_ : 1;
            bool x639_26_started : 1;
            bool x639_27_ : 1;
            bool x639_28_ : 1;
            bool x639_29_ : 1;
            bool x639_30_ : 1;
            bool x639_31_ : 1;
            bool x63a_24_ : 1;
        };

        u64 _dummy = 0;
    };

    s32 x63c_frenzyFrames = 0;
    TUniqueId x640_ = kInvalidUniqueId;
    TUniqueId x642_ = kInvalidUniqueId;
    float x644_ = 1.f;
    zeus::CVector3f x648_ = zeus::CVector3f::skForward;
    zeus::CVector3f x654_;
    CPathFindSearch x660_pathFindSearch;
    float x744_ = 0.f;
    float x748_ = 0.f;
    u32 x74c_ = 0;
    float x750_initialHP;
    float x754_ = 0.f;
    CSegId x758_headSeg;
    u32 x75c_ = 0;
    s32 x760_ = -1;
    CBoneTracking x764_boneTracking;
    s32 x79c_ = -1;
    float x7a4_intoJumpDist = 1.f;
    float x7a8_ = 2.f;
    float x7ac_ = 0.f;
    u32 x7b0_ = 0;
    TUniqueId x7b4_ = kInvalidUniqueId;
    CSegId x7b6_gunSeg;
    CSegId x7b7_elbowSeg;
    CSegId x7b8_wristSeg;
    CSegId x7b9_swooshSeg;
    float x7bc_ = 1.f;
    TUniqueId x7c0_targetId = kInvalidUniqueId;
    CBurstFire x7c4_burstFire;
    float x824_ = 3.f;
    zeus::CVector3f x828_;
    s32 x834_ = -1;
    float x838_ = 0.f;
    s32 x83c_ = -1;
    TUniqueId x840_ = kInvalidUniqueId;
    s32 x844_ = -1;
    float x848_dodgeDist = 3.f;
    float x84c_breakDodgeDist = 3.f;
    float x850_timeSinceHitByPlayer = FLT_MAX;
    float x854_lowHealthFrenzyTimer = FLT_MAX;
    float x858_ragdollDelayTimer = 0.f;
    std::unique_ptr<CPirateRagDoll> x85c_ragDoll;
    CIkChain x860_ikChain;
    float x8a8_cloakDelayTimer = 0.f;
    float x8ac_electricParticleTimer = 0.f;
    float x8b0_cloakStepTime = 0.f;
    float x8b4_shadowPirateAlpha = 0.5f;
    float x8b8_minCloakAlpha;
    float x8bc_maxCloakAlpha;
    float x8c0_;
    float x8c4_aimDelayTimer;
    TUniqueId x8c8_teamAiMgrId = kInvalidUniqueId;
    zeus::CColor x8cc_trooperColor = zeus::CColor::skWhite;
    zeus::CVector2f x8d0_;
    float x8d8_ = 0.f;
    float x8dc_ = 0.f;

    static const SBurst skBurstsSeatedOOV[];
    static const SBurst skBurstsInjuredOOV[];
    static const SBurst skBurstsJumpingOOV[];
    static const SBurst skBurstsFrenziedOOV[];
    static const SBurst skBurstsStandardOOV[];
    static const SBurst skBurstsQuickOOV[];
    static const SBurst skBurstsSeated[];
    static const SBurst skBurstsInjured[];
    static const SBurst skBurstsJumping[];
    static const SBurst skBurstsFrenzied[];
    static const SBurst skBurstsStandard[];
    static const SBurst skBurstsQuick[];
    static const SBurst* skBursts[];

    static std::list<TUniqueId> mChargePlayerList;

    void UpdateCloak(float dt, CStateManager& mgr);
    bool ShouldFrenzy(CStateManager& mgr);
    void ResetTeamAiRole(CStateManager& mgr);
    void AssignTeamAiRole(CStateManager& mgr);
    void RemoveTeamAiRole(CStateManager& mgr);
    bool CheckTargetable(CStateManager& mgr);
    void FireProjectile(float dt, CStateManager& mgr);
    void UpdateAttacks(float dt, CStateManager& mgr);
    zeus::CVector3f GetTargetPos(CStateManager& mgr);
    void UpdateAimBodyState(float dt, CStateManager& mgr);
    void SetCinematicCollision(CStateManager& mgr);
    void SetNonCinematicCollision(CStateManager& mgr);
    void CheckForProjectiles(CStateManager& mgr);
    void SetEyeParticleActive(CStateManager& mgr, bool active);
    void SetVelocityForJump();

public:
    CSpacePirate(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&, CModelData&&,
                 const CActorParameters&, const CPatternedInfo&, CInputStream&, u32);

    void Accept(IVisitor &visitor);
    void Think(float dt, CStateManager& mgr);
    void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr);
    void PreRender(CStateManager& mgr, const zeus::CFrustum& frustum);
    void Render(const CStateManager& mgr) const;

    void CalculateRenderBounds();
    void Touch(CActor& other, CStateManager& mgr);
    zeus::CAABox GetSortingBounds(const CStateManager& mgr) const;
    void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type, float dt);
    void Death(CStateManager& mgr, const zeus::CVector3f& direction, EScriptObjectState state);
    void KnockBack(const zeus::CVector3f&, CStateManager&, const CDamageInfo& info,
                   EKnockBackType type, bool inDeferred, float magnitude);
    bool IsListening() const;
    bool Listen(const zeus::CVector3f&, EListenNoiseType);
    zeus::CVector3f GetOrigin(const CStateManager& mgr, const CTeamAiRole& role) const;

    void Patrol(CStateManager&, EStateMsg, float);
    void Dead(CStateManager&, EStateMsg, float);
    void PathFind(CStateManager&, EStateMsg, float);
    void TargetPatrol(CStateManager&, EStateMsg, float);
    void TargetCover(CStateManager&, EStateMsg, float);
    void Halt(CStateManager&, EStateMsg, float);
    void Run(CStateManager&, EStateMsg, float);
    void Generate(CStateManager&, EStateMsg, float);
    void Deactivate(CStateManager&, EStateMsg, float);
    void Attack(CStateManager&, EStateMsg, float);
    void JumpBack(CStateManager&, EStateMsg, float);
    void DoubleSnap(CStateManager&, EStateMsg, float);
    void Shuffle(CStateManager&, EStateMsg, float);
    void TurnAround(CStateManager&, EStateMsg, float);
    void Skid(CStateManager&, EStateMsg, float);
    void CoverAttack(CStateManager&, EStateMsg, float);
    void Crouch(CStateManager&, EStateMsg, float);
    void GetUp(CStateManager&, EStateMsg, float);
    void Taunt(CStateManager&, EStateMsg, float);
    void Flee(CStateManager&, EStateMsg, float);
    void Lurk(CStateManager&, EStateMsg, float);
    void Jump(CStateManager&, EStateMsg, float);
    void Dodge(CStateManager&, EStateMsg, float);
    void Cover(CStateManager&, EStateMsg, float);
    void Approach(CStateManager&, EStateMsg, float);
    void WallHang(CStateManager&, EStateMsg, float);
    void WallDetach(CStateManager&, EStateMsg, float);
    void Enraged(CStateManager&, EStateMsg, float);
    void SpecialAttack(CStateManager&, EStateMsg, float);
    void Bounce(CStateManager&, EStateMsg, float);
    void PathFindEx(CStateManager&, EStateMsg, float);

    bool Leash(CStateManager&, float);
    bool OffLine(CStateManager&, float);
    bool Attacked(CStateManager&, float);
    bool InRange(CStateManager&, float);
    bool SpotPlayer(CStateManager&, float);
    bool PatternOver(CStateManager&, float);
    bool PatternShagged(CStateManager&, float);
    bool AnimOver(CStateManager&, float);
    bool ShouldAttack(CStateManager&, float);
    bool ShouldJumpBack(CStateManager&, float);
    bool Stuck(CStateManager&, float);
    bool Landed(CStateManager&, float);
    bool HearShot(CStateManager&, float);
    bool HearPlayer(CStateManager&, float);
    bool CoverCheck(CStateManager&, float);
    bool CoverFind(CStateManager&, float);
    bool CoverBlown(CStateManager&, float);
    bool CoverNearlyBlown(CStateManager&, float);
    bool CoveringFire(CStateManager&, float);
    bool LineOfSight(CStateManager&, float);
    bool AggressionCheck(CStateManager&, float);
    bool ShouldDodge(CStateManager&, float);
    bool ShouldRetreat(CStateManager&, float);
    bool ShouldCrouch(CStateManager&, float);
    bool ShouldMove(CStateManager&, float);
    bool ShotAt(CStateManager&, float);
    bool HasTargetingPoint(CStateManager&, float);
    bool ShouldWallHang(CStateManager&, float);
    bool StartAttack(CStateManager&, float);
    bool BreakAttack(CStateManager&, float);
    bool ShouldStrafe(CStateManager&, float);
    bool ShouldSpecialAttack(CStateManager&, float);
    bool LostInterest(CStateManager&, float);
    bool BounceFind(CStateManager&, float);

    CPathFindSearch* GetSearchPath();
    u8 GetModelAlphau8(const CStateManager& mgr) const;
    float GetGravityConstant() const;
    CProjectileInfo* GetProjectileInfo();
};
}
