#ifndef _CTEAMAIMGR_HPP
#define _CTEAMAIMGR_HPP

#include "Kyoto/Math/CVector3f.hpp"
#include "MetroidPrime/CEntity.hpp"
#include "rstl/vector.hpp"

class CAi;
class CInputStream;
class CStateManager;
class IVisitor;

class CTeamAiRole {
public:
  enum ETeamAiRole {
    kTAR_Invalid = -1,
    kTAR_Initial = 0,
    kTAR_Melee = 1,
    kTAR_Projectile = 2,
    kTAR_Unknown = 3,
    kTAR_Unassigned = 4,
  };

  CTeamAiRole(TUniqueId ownerId)
  : x0_ownerId(ownerId)
  , x4_roleA(kTAR_Invalid)
  , x8_roleB(kTAR_Invalid)
  , xc_roleC(kTAR_Invalid)
  , x10_curRole(kTAR_Invalid)
  , x14_roleIndex(-1)
  , x18_captainPriority(0)
  , x1c_position(CVector3f::Zero()) {}

  CTeamAiRole(TUniqueId ownerId, ETeamAiRole roleA, ETeamAiRole roleB, ETeamAiRole roleC)
  : x0_ownerId(ownerId)
  , x4_roleA(roleA)
  , x8_roleB(roleB)
  , xc_roleC(roleC)
  , x10_curRole(kTAR_Invalid)
  , x14_roleIndex(-1)
  , x18_captainPriority(0)
  , x1c_position(CVector3f::Zero()) {}

  bool AllowsRole(ETeamAiRole role) const {
    return x4_roleA == role || x8_roleB == role || xc_roleC == role;
  }

  bool operator<(const CTeamAiRole& other) const {
    return x0_ownerId.Value() < other.x0_ownerId.Value();
  }
  void __swap(const CTeamAiRole& other);

  TUniqueId GetOwnerId() const { return x0_ownerId; }
  ETeamAiRole GetTeamAiRole() const { return x10_curRole; }
  void SetTeamAiRole(ETeamAiRole role) { x10_curRole = role; }
  bool HasTeamAiRole() const {
    return x10_curRole != kTAR_Initial && x10_curRole >= kTAR_Initial &&
           x10_curRole <= kTAR_Unassigned;
  }
  int GetRoleIndex() const { return x14_roleIndex; }
  void SetRoleIndex(int idx) { x14_roleIndex = idx; }
  const CVector3f& GetTeamPosition() const { return x1c_position; }
  void SetTeamPosition(const CVector3f& pos) { x1c_position = pos; }

private:
  TUniqueId x0_ownerId;
  ETeamAiRole x4_roleA;
  ETeamAiRole x8_roleB;
  ETeamAiRole xc_roleC;
  ETeamAiRole x10_curRole;
  int x14_roleIndex;
  int x18_captainPriority;
  CVector3f x1c_position;

  friend class CTeamAiMgr;
};
CHECK_SIZEOF(CTeamAiRole, 0x28)

enum EAttackType {
  kAT_Melee,
  kAT_Projectile,
};

class CTeamAiMgr : public CEntity {
public:
  class CUnknown {
    friend class CTeamAiMgr;

  public:
    CUnknown(CInputStream& in, int propCount);

    static int GetNumProperties() { return kNumProperties; }

  private:
    uint x0_aiCount;
    uint x4_meleeCount;
    uint x8_projectileCount;
    uint xc_unknownCount;
    uint x10_maxMeleeAttackerCount;
    uint x14_maxProjectileAttackerCount;
    uint x18_positionMode;
    float x1c_meleeTimeInterval;
    float x20_projectileTimeInterval;

    static const int kNumProperties;
  };

  CTeamAiMgr(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
             const CUnknown& data);
  ~CTeamAiMgr() override {}

  // CEntity
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& mgr) override;

  void UpdateTeamCaptain();
  void SpacingSort(CStateManager& mgr, const CVector3f& pos);
  void PositionTeam(CStateManager& mgr);
  void AssignRoles(CTeamAiRole::ETeamAiRole role, uint count);
  void ResetRoles(CStateManager& mgr);
  void UpdateRoles(CStateManager& mgr);
  bool ShouldUpdateRoles(float dt);
  void RemoveProjectileAttacker(TUniqueId id);
  bool AddProjectileAttacker(TUniqueId id);
  bool CanAcceptProjectileAttacker(TUniqueId id) const;
  bool IsProjectileAttacker(TUniqueId id) const;
  void RemoveMeleeAttacker(TUniqueId id);
  bool AddMeleeAttacker(TUniqueId id);
  bool CanAcceptMeleeAttacker(TUniqueId id) const;
  bool IsMeleeAttacker(TUniqueId id) const;
  bool IsPartOfTeam(TUniqueId id) const;
  bool HasTeamAiRole(TUniqueId id) const;
  void ClearTeamAiRole(TUniqueId id);
  const CTeamAiRole* GetTeamAiRole(TUniqueId id) const;
  int GetNumAssignedOfRole(CTeamAiRole::ETeamAiRole role) const;
  int GetNumAssignedAiRoles() const;
  void RemoveTeamAiRole(TUniqueId id);
  bool AssignTeamAiRole(const CAi& ai, int roleA, int roleB, int roleC);

  static void ResetTeamAiRole(EAttackType type, CStateManager& mgr, TUniqueId mgrId, TUniqueId aiId,
                              bool clearRole);
  static bool AddAttacker(EAttackType type, CStateManager& mgr, TUniqueId mgrId, TUniqueId aiId);
  static bool CanAcceptAttacker(EAttackType type, CStateManager& mgr, TUniqueId mgrId,
                                TUniqueId aiId);
  static const CTeamAiRole* GetTeamAiRole(const CStateManager& mgr, TUniqueId mgrId,
                                          TUniqueId aiId);
  static TUniqueId GetTeamAiMgr(const CAi& ai, const CStateManager& mgr);

  uint GetProjectileRoleCount() const { return x34_data.x8_projectileCount; }
  uint GetMaxMeleeAttackerCount() const { return x34_data.x10_maxMeleeAttackerCount; }
  uint GetMaxProjectileAttackerCount() const { return x34_data.x14_maxProjectileAttackerCount; }
  bool HasMeleeAttackers() const { return x68_meleeAttackers.size() != 0u; }
  bool HasProjectileAttackers() const { return x78_projectileAttackers.size() != 0u; }
  const rstl::vector< TUniqueId >& GetProjectileAttackers() const {
    return x78_projectileAttackers;
  }

  rstl::vector< CTeamAiRole >& GetTeamAiRoles() { return x58_roles; }
  const rstl::vector< CTeamAiRole >& GetTeamAiRoles() const { return x58_roles; }

  size_t GetNumRoles() const { return x58_roles.size(); }

private:
  CUnknown x34_data;
  rstl::vector< CTeamAiRole > x58_roles;
  rstl::vector< TUniqueId > x68_meleeAttackers;
  rstl::vector< TUniqueId > x78_projectileAttackers;
  float x88_timeDirty;
  TUniqueId x8c_teamCaptainId;
  float x90_timeSinceMelee;
  float x94_timeSinceProjectile;
};
typedef CTeamAiMgr::CUnknown CTeamAiMgr_CUnknown;
CHECK_SIZEOF(CTeamAiMgr_CUnknown, 0x24)
CHECK_SIZEOF(CTeamAiMgr, 0x98)

#endif
