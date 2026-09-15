#include "MetroidPrime/Enemies/CTeamAiMgr.hpp"

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/Enemies/CAi.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "rstl/algorithm.hpp"

const int CTeamAiMgr::CUnknown::kNumProperties = 8;

struct CRoleSorter {
  CVector3f x0_pos;
  int xc_type;

  CRoleSorter(const CVector3f& pos, int type) : x0_pos(pos), xc_type(type) {}

  bool operator()(const CTeamAiRole& a, const CTeamAiRole& b) const;
};

bool CRoleSorter::operator()(const CTeamAiRole& a, const CTeamAiRole& b) const {
  const float aDist = (x0_pos - a.GetTeamPosition()).MagSquared();
  const float bDist = (x0_pos - b.GetTeamPosition()).MagSquared();

  switch (xc_type) {
  case 0:
    return a.GetOwnerId().Value() < b.GetOwnerId().Value();
  case 1:
    return aDist < bDist;
  case 2:
  default:
    if (a.GetTeamAiRole() == b.GetTeamAiRole()) {
      return aDist < bDist;
    }
    return a.GetTeamAiRole() < b.GetTeamAiRole();
  }
}

CTeamAiMgr::CUnknown::CUnknown(CInputStream& in, int propCount)
: x0_aiCount(in.ReadLong())
, x4_meleeCount(in.ReadLong())
, x8_projectileCount(in.ReadLong())
, xc_unknownCount(in.ReadLong())
, x10_maxMeleeAttackerCount(in.ReadLong())
, x14_maxProjectileAttackerCount(in.ReadLong())
, x18_positionMode(in.ReadLong())
, x1c_meleeTimeInterval(propCount > 8 ? in.ReadFloat() : 0.f)
, x20_projectileTimeInterval(propCount > 8 ? in.ReadFloat() : 0.f) {}

CTeamAiMgr::CTeamAiMgr(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                       const CUnknown& data)
: CEntity(uid, info, true, name)
, x34_data(data)
, x58_roles()
, x68_meleeAttackers()
, x78_projectileAttackers()
, x88_timeDirty(0.f)
, x8c_teamCaptainId(kInvalidUniqueId)
, x90_timeSinceMelee(data.x1c_meleeTimeInterval)
, x94_timeSinceProjectile(data.x20_projectileTimeInterval) {
  if (x34_data.x0_aiCount != 0) {
    x58_roles.reserve(x34_data.x0_aiCount);
  }
  if (x34_data.x4_meleeCount != 0) {
    x68_meleeAttackers.reserve(x34_data.x4_meleeCount);
  }
  if (x34_data.x8_projectileCount != 0) {
    x78_projectileAttackers.reserve(x34_data.x8_projectileCount);
  }
}

TUniqueId CTeamAiMgr::GetTeamAiMgr(const CAi& ai, const CStateManager& mgr) {
  rstl::vector< SConnection >::const_iterator conn = ai.GetConnectionList().begin();
  for (; conn != ai.GetConnectionList().end(); ++conn) {
    if (conn->x0_state == kSS_Active && conn->x4_msg == kSM_Play) {
      if (const CTeamAiMgr* teamMgr = TCastToConstPtr< CTeamAiMgr >(
              mgr.GetObjectById(mgr.GetIdForScript(conn->x8_objId)))) {
        return teamMgr->GetUniqueId();
      }
    }
  }
  return kInvalidUniqueId;
}

const CTeamAiRole* CTeamAiMgr::GetTeamAiRole(const CStateManager& mgr, TUniqueId mgrId,
                                             TUniqueId aiId) {
  if (const CTeamAiMgr* teamMgr = TCastToConstPtr< CTeamAiMgr >(mgr.GetObjectById(mgrId))) {
    return teamMgr->GetTeamAiRole(aiId);
  }
  return 0;
}

bool CTeamAiMgr::CanAcceptAttacker(EAttackType type, CStateManager& mgr, TUniqueId mgrId,
                                   TUniqueId aiId) {
  if (CTeamAiMgr* teamMgr = TCastToPtr< CTeamAiMgr >(mgr.ObjectById(mgrId))) {
    if (teamMgr->HasTeamAiRole(aiId)) {
      if (type == kAT_Melee) {
        return teamMgr->CanAcceptMeleeAttacker(aiId);
      }
      if (type == kAT_Projectile) {
        return teamMgr->CanAcceptProjectileAttacker(aiId);
      }
    }
  }
  return false;
}

bool CTeamAiMgr::AddAttacker(EAttackType type, CStateManager& mgr, TUniqueId mgrId,
                             TUniqueId aiId) {
  if (CTeamAiMgr* teamMgr = TCastToPtr< CTeamAiMgr >(mgr.ObjectById(mgrId))) {
    if (teamMgr->HasTeamAiRole(aiId)) {
      if (type == kAT_Melee) {
        return teamMgr->AddMeleeAttacker(aiId);
      }
      if (type == kAT_Projectile) {
        return teamMgr->AddProjectileAttacker(aiId);
      }
    }
  }
  return false;
}

void CTeamAiMgr::ResetTeamAiRole(EAttackType type, CStateManager& mgr, TUniqueId mgrId,
                                 TUniqueId aiId, bool clearRole) {
  if (CTeamAiMgr* teamMgr = TCastToPtr< CTeamAiMgr >(mgr.ObjectById(mgrId))) {
    if (teamMgr->HasTeamAiRole(aiId)) {
      if (type == kAT_Melee) {
        if (teamMgr->IsMeleeAttacker(aiId)) {
          teamMgr->RemoveMeleeAttacker(aiId);
        }
      } else if (type == kAT_Projectile) {
        if (teamMgr->IsProjectileAttacker(aiId)) {
          teamMgr->RemoveProjectileAttacker(aiId);
        }
      }
      if (clearRole) {
        teamMgr->ClearTeamAiRole(aiId);
      }
    }
  }
}

void CTeamAiMgr::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& mgr) {
  CEntity::AcceptScriptMsg(msg, objId, mgr);
}

ENTITY_ACCEPT_IMPL(CTeamAiMgr)

void CTeamAiMgr::Think(float dt, CStateManager& mgr) {
  CEntity::Think(dt, mgr);
  if (ShouldUpdateRoles(dt)) {
    UpdateRoles(mgr);
  }
  PositionTeam(mgr);
  x90_timeSinceMelee += dt;
  x94_timeSinceProjectile += dt;
}

bool CTeamAiMgr::AssignTeamAiRole(const CAi& ai, int roleA, int roleB, int roleC) {
  CTeamAiRole searchRole(ai.GetUniqueId(), static_cast< CTeamAiRole::ETeamAiRole >(roleA),
                         static_cast< CTeamAiRole::ETeamAiRole >(roleB),
                         static_cast< CTeamAiRole::ETeamAiRole >(roleC));
  rstl::vector< CTeamAiRole >::iterator search =
      rstl::binary_find(x58_roles.begin(), x58_roles.end(), searchRole);

  if (search == x58_roles.end()) {
    if (x58_roles.size() < x58_roles.capacity()) {
      rstl::vector< CTeamAiRole >::iterator insertPos =
          rstl::lower_bound(x58_roles.begin(), x58_roles.end(), searchRole);
      x58_roles.insert(insertPos, searchRole);
    } else {
      return false;
    }
  } else {
    *search = searchRole;
  }

  UpdateTeamCaptain();
  return true;
}

void CTeamAiMgr::RemoveTeamAiRole(TUniqueId id) {
  if (IsMeleeAttacker(id)) {
    RemoveMeleeAttacker(id);
  }
  if (IsProjectileAttacker(id)) {
    RemoveProjectileAttacker(id);
  }

  CTeamAiRole searchRole(id);
  rstl::vector< CTeamAiRole >::iterator search =
      rstl::binary_find(x58_roles.begin(), x58_roles.end(), searchRole);
  x58_roles.erase(search);

  UpdateTeamCaptain();
}

int CTeamAiMgr::GetNumAssignedAiRoles() const {
  rstl::vector< CTeamAiRole >::const_iterator cur = x58_roles.begin();
  rstl::vector< CTeamAiRole >::const_iterator end = x58_roles.end();
  int count = 0;
  for (; cur != end; ++cur) {
    if (cur->HasTeamAiRole()) {
      ++count;
    }
  }
  return count;
}

int CTeamAiMgr::GetNumAssignedOfRole(CTeamAiRole::ETeamAiRole role) const {
  int count = 0;
  rstl::vector< CTeamAiRole >::const_iterator cur = x58_roles.begin();
  rstl::vector< CTeamAiRole >::const_iterator end = x58_roles.end();
  for (; cur != end; ++cur) {
    if (role == cur->x10_curRole) {
      ++count;
    }
  }
  return count;
}

const CTeamAiRole* CTeamAiMgr::GetTeamAiRole(TUniqueId id) const {
  CTeamAiRole searchRole(id);
  rstl::vector< CTeamAiRole >::const_iterator search =
      rstl::binary_find(x58_roles.begin(), x58_roles.end(), searchRole);
  return search != x58_roles.end() ? search.operator->() : 0;
}

void CTeamAiMgr::ClearTeamAiRole(TUniqueId id) {
  CTeamAiRole searchRole(id);
  rstl::vector< CTeamAiRole >::iterator search =
      rstl::binary_find(x58_roles.begin(), x58_roles.end(), searchRole);
  if (search != x58_roles.end()) {
    search->SetTeamAiRole(CTeamAiRole::kTAR_Initial);
  }
}

bool CTeamAiMgr::HasTeamAiRole(TUniqueId id) const {
  CTeamAiRole searchRole(id);
  rstl::vector< CTeamAiRole >::const_iterator search =
      rstl::binary_find(x58_roles.begin(), x58_roles.end(), searchRole);
  if (search != x58_roles.end()) {
    return search->HasTeamAiRole();
  }
  return false;
}

bool CTeamAiMgr::IsPartOfTeam(TUniqueId id) const {
  CTeamAiRole searchRole(id);
  rstl::vector< CTeamAiRole >::const_iterator search =
      rstl::binary_find(x58_roles.begin(), x58_roles.end(), searchRole);
  return search != x58_roles.end();
}

bool CTeamAiMgr::IsMeleeAttacker(TUniqueId id) const {
  rstl::vector< TUniqueId >::const_iterator search =
      rstl::binary_find(x68_meleeAttackers.begin(), x68_meleeAttackers.end(), id);
  return search != x68_meleeAttackers.end();
}

bool CTeamAiMgr::CanAcceptMeleeAttacker(TUniqueId id) const {
  if (x90_timeSinceMelee >= x34_data.x1c_meleeTimeInterval &&
      x68_meleeAttackers.size() < x34_data.x10_maxMeleeAttackerCount) {
    return true;
  }
  rstl::vector< TUniqueId >::const_iterator search =
      rstl::binary_find(x68_meleeAttackers.begin(), x68_meleeAttackers.end(), id);
  if (search != x68_meleeAttackers.end()) {
    return true;
  }
  return false;
}

bool CTeamAiMgr::AddMeleeAttacker(TUniqueId id) {
  if (x90_timeSinceMelee >= x34_data.x1c_meleeTimeInterval &&
      x68_meleeAttackers.size() < x34_data.x10_maxMeleeAttackerCount && HasTeamAiRole(id)) {
    rstl::vector< TUniqueId >::iterator search =
        rstl::binary_find(x68_meleeAttackers.begin(), x68_meleeAttackers.end(), id);
    if (search == x68_meleeAttackers.end()) {
      x68_meleeAttackers.reserve(x68_meleeAttackers.size() + 1);
      rstl::vector< TUniqueId >::iterator insertPos =
          rstl::lower_bound(x68_meleeAttackers.begin(), x68_meleeAttackers.end(), id);
      x68_meleeAttackers.insert(insertPos, id);
      x90_timeSinceMelee = 0.f;
    }
    return true;
  }
  return false;
}

void CTeamAiMgr::RemoveMeleeAttacker(TUniqueId id) {
  rstl::vector< TUniqueId >::iterator search =
      rstl::binary_find(x68_meleeAttackers.begin(), x68_meleeAttackers.end(), id);
  if (search != x68_meleeAttackers.end()) {
    x68_meleeAttackers.erase(search);
  }
}

bool CTeamAiMgr::IsProjectileAttacker(TUniqueId id) const {
  rstl::vector< TUniqueId >::const_iterator search =
      rstl::binary_find(x78_projectileAttackers.begin(), x78_projectileAttackers.end(), id);
  return search != x78_projectileAttackers.end();
}

bool CTeamAiMgr::CanAcceptProjectileAttacker(TUniqueId id) const {
  if (x94_timeSinceProjectile >= x34_data.x20_projectileTimeInterval &&
      x78_projectileAttackers.size() < x34_data.x14_maxProjectileAttackerCount) {
    return true;
  }
  rstl::vector< TUniqueId >::const_iterator search =
      rstl::binary_find(x78_projectileAttackers.begin(), x78_projectileAttackers.end(), id);
  if (search != x78_projectileAttackers.end()) {
    return true;
  }
  return false;
}

bool CTeamAiMgr::AddProjectileAttacker(TUniqueId id) {
  if (x94_timeSinceProjectile >= x34_data.x20_projectileTimeInterval &&
      x78_projectileAttackers.size() < x34_data.x14_maxProjectileAttackerCount &&
      HasTeamAiRole(id)) {
    rstl::vector< TUniqueId >::iterator search =
        rstl::binary_find(x78_projectileAttackers.begin(), x78_projectileAttackers.end(), id);
    if (search == x78_projectileAttackers.end()) {
      x78_projectileAttackers.reserve(x78_projectileAttackers.size() + 1);
      rstl::vector< TUniqueId >::iterator insertPos =
          rstl::lower_bound(x78_projectileAttackers.begin(), x78_projectileAttackers.end(), id);
      x78_projectileAttackers.insert(insertPos, id);
      x94_timeSinceProjectile = 0.f;
    }
    return true;
  }
  return false;
}

void CTeamAiMgr::RemoveProjectileAttacker(TUniqueId id) {
  rstl::vector< TUniqueId >::iterator search =
      rstl::binary_find(x78_projectileAttackers.begin(), x78_projectileAttackers.end(), id);
  if (search != x78_projectileAttackers.end()) {
    x78_projectileAttackers.erase(search);
  }
}

bool CTeamAiMgr::ShouldUpdateRoles(float dt) {
  if (x58_roles.size() > 0) {
    x88_timeDirty += dt;
    if (x88_timeDirty >= 1.5f) {
      return true;
    }

    rstl::vector< CTeamAiRole >::const_iterator role = x58_roles.begin();
    for (; role != x58_roles.end(); ++role) {
      bool valid = false;
      const CTeamAiRole::ETeamAiRole curRole = role->x10_curRole;
      if (curRole != CTeamAiRole::kTAR_Initial && curRole >= CTeamAiRole::kTAR_Initial &&
          curRole <= CTeamAiRole::kTAR_Unassigned) {
        valid = true;
      }
      if (!valid) {
        return true;
      }
    }
  }

  return false;
}

void CTeamAiMgr::UpdateRoles(CStateManager& mgr) {
  ResetRoles(mgr);

  CVector3f aimPos = mgr.GetPlayer()->GetAimPosition(mgr, 0.f);
  rstl::sort(x58_roles.begin(), x58_roles.end(), CRoleSorter(aimPos, 1));

  AssignRoles(CTeamAiRole::kTAR_Melee, x34_data.x4_meleeCount);
  AssignRoles(CTeamAiRole::kTAR_Projectile, x34_data.x8_projectileCount);
  AssignRoles(CTeamAiRole::kTAR_Unknown, x34_data.xc_unknownCount);

  rstl::vector< CTeamAiRole >::iterator role = x58_roles.begin();
  for (; role != x58_roles.end(); ++role) {
    if (!role->HasTeamAiRole()) {
      role->x10_curRole = CTeamAiRole::kTAR_Unassigned;
    }
  }

  rstl::sort(x58_roles.begin(), x58_roles.end(), CRoleSorter(aimPos, 0));
  x88_timeDirty = 0.f;
}

void CTeamAiMgr::ResetRoles(CStateManager& mgr) {
  rstl::vector< CTeamAiRole >::iterator role = x58_roles.begin();
  for (; role != x58_roles.end(); ++role) {
    role->x10_curRole = CTeamAiRole::kTAR_Initial;
    role->x14_roleIndex = 0;

    if (const CAi* ai = static_cast< const CAi* >(mgr.GetObjectById(role->GetOwnerId()))) {
      role->x1c_position = ai->GetTranslation();
    }
  }
}

void CTeamAiMgr::AssignRoles(CTeamAiRole::ETeamAiRole role, uint count) {
  if (count == 0) {
    return;
  }

  uint roleIndex = 0;
  for (rstl::vector< CTeamAiRole >::iterator cur = x58_roles.begin(); cur != x58_roles.end();
       ++cur) {
    if (cur->x10_curRole == CTeamAiRole::kTAR_Initial) {
      if (cur->AllowsRole(role)) {
        cur->x10_curRole = role;
        cur->x14_roleIndex = roleIndex;
        roleIndex += 1;
        if (roleIndex == count) {
          return;
        }
      }
    }
  }
}

void CTeamAiMgr::PositionTeam(CStateManager& mgr) {
  CVector3f aimPos = mgr.GetPlayer()->GetAimPosition(mgr, 0.f);
  switch (static_cast< int >(x34_data.x18_positionMode)) {
  case 1:
    SpacingSort(mgr, aimPos);
    break;
  case 0:
  default:
    rstl::vector< CTeamAiRole >::iterator role = x58_roles.begin();
    for (; role != x58_roles.end(); ++role) {
      if (CPatterned* ai = TCastToPtr< CPatterned >(mgr.ObjectById(role->GetOwnerId()))) {
        role->x1c_position = ai->GetOrigin(mgr, *role, aimPos);
      }
    }
    break;
  }
}

void CTeamAiMgr::SpacingSort(CStateManager& mgr, const CVector3f& pos) {
  rstl::sort(x58_roles.begin(), x58_roles.end(), CRoleSorter(pos, 2));

  float tierStagger = 4.5f;
  for (rstl::vector< CTeamAiRole >::iterator role = x58_roles.begin();
       role != x58_roles.end(); ++role) {
    if (CPatterned* ai = TCastToPtr< CPatterned >(mgr.ObjectById(role->GetOwnerId()))) {
      const CAABox& aabb = ai->GetBaseBoundingBox();
      const float length = (aabb.GetMaxPoint().GetY() - aabb.GetMinPoint().GetY()) * 1.5f;
      if (length > tierStagger) {
        tierStagger = length;
      }
    }
  }

  float curTierDist = tierStagger;
  int tierTeamSize = 0;
  int maxTierTeamSize = 3;
  for (rstl::vector< CTeamAiRole >::iterator role = x58_roles.begin();
       role != x58_roles.end(); ++role) {
    CPatterned* const ai = TCastToPtr< CPatterned >(mgr.ObjectById(role->GetOwnerId()));
    if (ai) {
      CVector3f delta = ai->GetTranslation() - pos;
      delta.SetZ(0.f);
      const CVector3f& newPos = delta.CanBeNormalized()
                                    ? pos + curTierDist * delta.AsNormalized()
                                    : pos + curTierDist * ai->GetTransform().GetForward();
      CVector3f finalPos = newPos;
      finalPos.SetZ(ai->GetTranslation().GetZ());
      role->SetTeamPosition(finalPos);
      tierTeamSize += 1;
      if (tierTeamSize > maxTierTeamSize) {
        curTierDist += tierStagger;
        tierTeamSize = 0;
        maxTierTeamSize += 1;
      }
    }
  }

  rstl::sort(x58_roles.begin(), x58_roles.end(), CRoleSorter(pos, 0));
}

void CTeamAiMgr::UpdateTeamCaptain() {
  int captainPriority = -0x80000000;
  x8c_teamCaptainId = kInvalidUniqueId;
  rstl::vector< CTeamAiRole >::iterator role = x58_roles.begin();
  for (; role != x58_roles.end(); ++role) {
    if (role->x18_captainPriority > captainPriority) {
      captainPriority = role->x18_captainPriority;
      x8c_teamCaptainId = role->GetOwnerId();
    }
  }
}
