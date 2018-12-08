#pragma once

#include "CEntity.hpp"
#include "zeus/CVector3f.hpp"

namespace urde
{
class CStateManager;
class CAi;

class CTeamAiRole
{
    friend class CTeamAiMgr;
public:
    enum class ETeamAiRole
    {
        Invalid = -1,
        Initial,
        Melee,
        Projectile,
        Unknown,
        Unassigned
    };
private:
    TUniqueId x0_ownerId;
    ETeamAiRole x4_roleA = ETeamAiRole::Invalid;
    ETeamAiRole x8_roleB = ETeamAiRole::Invalid;
    ETeamAiRole xc_roleC = ETeamAiRole::Invalid;
    ETeamAiRole x10_curRole = ETeamAiRole::Invalid;
    s32 x14_roleIndex = -1;
    s32 x18_captainPriority = 0;
    zeus::CVector3f x1c_position;
public:
    CTeamAiRole(TUniqueId ownerId, ETeamAiRole a, ETeamAiRole b, ETeamAiRole c)
    : x0_ownerId(ownerId), x4_roleA(a), x8_roleB(b), xc_roleC(c) {}
    TUniqueId GetOwnerId() const { return x0_ownerId; }
    bool HasTeamAiRole() const { return false; }
    ETeamAiRole GetTeamAiRole() const { return x10_curRole; }
    void SetTeamAiRole(ETeamAiRole role) { x10_curRole = role; }
    s32 GetRoleIndex() const { return x14_roleIndex; }
    void SetRoleIndex(s32 idx) { x14_roleIndex = idx; }
    const zeus::CVector3f& GetTeamPosition() const { return x1c_position; }
    void SetTeamPosition(const zeus::CVector3f& pos) { x1c_position = pos; }
    bool operator<(const CTeamAiRole& other) const { return x0_ownerId < other.x0_ownerId; }
};

class CTeamAiData
{
    friend class CTeamAiMgr;
    u32 x0_aiCount;
    u32 x4_meleeCount;
    u32 x8_projectileCount;
    u32 xc_unknownCount;
    u32 x10_maxMeleeAttackerCount;
    u32 x14_maxProjectileAttackerCount;
    u32 x18_positionMode;
    float x1c_meleeTimeInterval;
    float x20_projectileTimeInterval;
public:
    CTeamAiData(CInputStream& in, s32 propCount);
};

class CTeamAiMgr : public CEntity
{
public:
    enum class EAttackType
    {
        Melee,
        Projectile
    };
private:
    CTeamAiData x34_data;
    std::vector<CTeamAiRole> x58_roles;
    std::vector<TUniqueId> x68_meleeAttackers;
    std::vector<TUniqueId> x78_projectileAttackers;
    float x88_timeDirty = 0.f;
    TUniqueId x8c_teamCaptainId = kInvalidUniqueId;
    float x90_timeSinceMelee;
    float x94_timeSinceProjectile;

    void UpdateTeamCaptain();
    bool ShouldUpdateRoles(float dt);
    void ResetRoles(CStateManager& mgr);
    void AssignRoles(CTeamAiRole::ETeamAiRole role, s32 count);
    void UpdateRoles(CStateManager& mgr);
    void SpacingSort(CStateManager& mgr, const zeus::CVector3f& pos);
    void PositionTeam(CStateManager& mgr);
    bool IsMeleeAttacker(TUniqueId aiId) const;
    bool CanAcceptMeleeAttacker(TUniqueId aiId) const;
    bool AddMeleeAttacker(TUniqueId aiId);
    void RemoveMeleeAttacker(TUniqueId aiId);
    bool IsProjectileAttacker(TUniqueId aiId) const;
    bool CanAcceptProjectileAttacker(TUniqueId aiId) const;
    bool AddProjectileAttacker(TUniqueId aiId);
    void RemoveProjectileAttacker(TUniqueId aiId);

public:
    CTeamAiMgr(TUniqueId uid, std::string_view name, const CEntityInfo& info, const CTeamAiData& data);

    void Accept(IVisitor&);
    void Think(float dt, CStateManager& mgr);
    void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& mgr);
    CTeamAiRole* GetTeamAiRole(TUniqueId aiId);
    bool IsPartOfTeam(TUniqueId aiId) const;
    bool HasTeamAiRole(TUniqueId aiId) const;
    bool AssignTeamAiRole(const CAi& ai, CTeamAiRole::ETeamAiRole roleA,
                          CTeamAiRole::ETeamAiRole roleB, CTeamAiRole::ETeamAiRole roleC);
    void RemoveTeamAiRole(TUniqueId aiId);
    void ClearTeamAiRole(TUniqueId aiId);
    s32 GetNumAssignedOfRole(CTeamAiRole::ETeamAiRole role) const;
    s32 GetNumAssignedAiRoles() const;

    static CTeamAiRole* GetTeamAiRole(CStateManager& mgr, TUniqueId mgrId, TUniqueId aiId);
    static void ResetTeamAiRole(EAttackType type, CStateManager& mgr, TUniqueId mgrId, TUniqueId aiId, bool clearRole);
    static bool CanAcceptAttacker(EAttackType type, CStateManager& mgr, TUniqueId mgrId, TUniqueId aiId);
    static bool AddAttacker(EAttackType type, CStateManager& mgr, TUniqueId mgrId, TUniqueId aiId);

    static TUniqueId GetTeamAiMgr(CAi& ai, CStateManager& mgr);
};
}

