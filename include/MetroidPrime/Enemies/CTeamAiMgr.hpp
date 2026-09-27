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
  : mOwnerId(ownerId)
  , mRoleA(kTAR_Invalid)
  , mRoleB(kTAR_Invalid)
  , mRoleC(kTAR_Invalid)
  , mCurRole(kTAR_Invalid)
  , mRoleIndex(-1)
  , mCaptainPriority(0)
  , mPosition(CVector3f::Zero()) {}

  CTeamAiRole(TUniqueId ownerId, ETeamAiRole roleA, ETeamAiRole roleB, ETeamAiRole roleC)
  : mOwnerId(ownerId)
  , mRoleA(roleA)
  , mRoleB(roleB)
  , mRoleC(roleC)
  , mCurRole(kTAR_Invalid)
  , mRoleIndex(-1)
  , mCaptainPriority(0)
  , mPosition(CVector3f::Zero()) {}

  bool AllowsRole(ETeamAiRole role) const {
    return mRoleA == role || mRoleB == role || mRoleC == role;
  }

  bool operator<(const CTeamAiRole& other) const {
    return mOwnerId.Value() < other.mOwnerId.Value();
  }
  void __swap(const CTeamAiRole& other);

  TUniqueId GetOwnerId() const { return mOwnerId; }
  ETeamAiRole GetTeamAiRole() const { return mCurRole; }
  void SetTeamAiRole(ETeamAiRole role) { mCurRole = role; }
  bool HasTeamAiRole() const {
    return mCurRole != kTAR_Initial && mCurRole >= kTAR_Initial &&
           mCurRole <= kTAR_Unassigned;
  }
  int GetRoleIndex() const { return mRoleIndex; }
  void SetRoleIndex(int idx) { mRoleIndex = idx; }
  const CVector3f& GetTeamPosition() const { return mPosition; }
  void SetTeamPosition(const CVector3f& pos) { mPosition = pos; }

private:
  TUniqueId mOwnerId;
  ETeamAiRole mRoleA;
  ETeamAiRole mRoleB;
  ETeamAiRole mRoleC;
  ETeamAiRole mCurRole;
  int mRoleIndex;
  int mCaptainPriority;
  CVector3f mPosition;

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
    uint mAiCount;
    uint mMeleeCount;
    uint mProjectileCount;
    uint mUnknownCount;
    uint mMaxMeleeAttackerCount;
    uint mMaxProjectileAttackerCount;
    uint mPositionMode;
    float mMeleeTimeInterval;
    float mProjectileTimeInterval;

    static const int kNumProperties;
  };

  CTeamAiMgr(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
             const CUnknown& data);

  // CEntity
  void Think(float dt, CStateManager& mgr) override;
  DECLARE_TYPES_MATCH_OR_ACCEPT;
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

  uint GetProjectileRoleCount() const { return mData.mProjectileCount; }
  uint GetMaxMeleeAttackerCount() const { return mData.mMaxMeleeAttackerCount; }
  uint GetMaxProjectileAttackerCount() const { return mData.mMaxProjectileAttackerCount; }
  bool HasMeleeAttackers() const { return mMeleeAttackers.size() != 0u; }
  bool HasProjectileAttackers() const { return mProjectileAttackers.size() != 0u; }
  const rstl::vector< TUniqueId >& GetProjectileAttackers() const {
    return mProjectileAttackers;
  }

  rstl::vector< CTeamAiRole >& GetTeamAiRoles() { return mRoles; }
  const rstl::vector< CTeamAiRole >& GetTeamAiRoles() const { return mRoles; }

  size_t GetNumRoles() const { return mRoles.size(); }

private:
  CUnknown mData;
  rstl::vector< CTeamAiRole > mRoles;
  rstl::vector< TUniqueId > mMeleeAttackers;
  rstl::vector< TUniqueId > mProjectileAttackers;
  float mTimeDirty;
  TUniqueId mTeamCaptainId;
  float mTimeSinceMelee;
  float mTimeSinceProjectile;
};
typedef CTeamAiMgr::CUnknown CTeamAiMgr_CUnknown;
CHECK_SIZEOF(CTeamAiMgr_CUnknown, 0x24)
CHECK_SIZEOF(CTeamAiMgr, 0x98)

#endif
