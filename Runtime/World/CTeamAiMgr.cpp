#include "Runtime/World/CTeamAiMgr.hpp"

#include <algorithm>

#include "Runtime/CStateManager.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "Runtime/World/CPatterned.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace metaforce {

struct TeamAiRoleSorter {
  enum class Type {
    OwnerID,
    Distance,
    TeamAIRole,
  };

  zeus::CVector3f x0_pos;
  Type xc_type;

  bool operator()(const CTeamAiRole& a, const CTeamAiRole& b) const {
    const float aDist = (x0_pos - a.GetTeamPosition()).magSquared();
    const float bDist = (x0_pos - b.GetTeamPosition()).magSquared();
    switch (xc_type) {
    case Type::OwnerID:
      return a.GetOwnerId() < b.GetOwnerId();
    case Type::Distance:
      return aDist < bDist;
    default:
      if (a.GetTeamAiRole() == b.GetTeamAiRole())
        return aDist < bDist;
      else
        return a.GetTeamAiRole() < b.GetTeamAiRole();
    }
  }
  TeamAiRoleSorter(const zeus::CVector3f& pos, Type type) : x0_pos(pos), xc_type(type) {}
};

CTeamAiData::CTeamAiData(CInputStream& in, s32 propCount)
: x0_aiCount(in.ReadLong())
, x4_meleeCount(in.ReadLong())
, x8_rangedCount(in.ReadLong())
, xc_unknownCount(in.ReadLong())
, x10_maxMeleeAttackerCount(in.ReadLong())
, x14_maxRangedAttackerCount(in.ReadLong())
, x18_positionMode(in.ReadLong())
, x1c_meleeTimeInterval(propCount > 8 ? in.ReadFloat() : 0.f)
, x20_rangedTimeInterval(propCount > 8 ? in.ReadFloat() : 0.f) {}

CTeamAiMgr::CTeamAiMgr(TUniqueId uid, std::string_view name, const CEntityInfo& info, const CTeamAiData& data)
: CEntity(uid, info, true, name), x34_data(data) {
  if (x34_data.x0_aiCount)
    x58_roles.reserve(x34_data.x0_aiCount);
  if (x34_data.x4_meleeCount)
    x68_meleeAttackers.reserve(x34_data.x4_meleeCount);
  if (x34_data.x8_rangedCount)
    x78_rangedAttackers.reserve(x34_data.x8_rangedCount);
}

void CTeamAiMgr::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CTeamAiMgr::UpdateTeamCaptain() {
  int maxPriority = INT_MIN;
  x8c_teamCaptainId = kInvalidUniqueId;
  for (const auto& role : x58_roles) {
    if (role.x18_captainPriority > maxPriority) {
      maxPriority = role.x18_captainPriority;
      x8c_teamCaptainId = role.GetOwnerId();
    }
  }
}

bool CTeamAiMgr::ShouldUpdateRoles(float dt) {
  if (x58_roles.empty()) {
    return false;
  }

  x88_timeDirty += dt;
  if (x88_timeDirty >= 1.5f) {
    return true;
  }

  return std::any_of(x58_roles.cbegin(), x58_roles.cend(), [](const auto& role) {
    return role.GetTeamAiRole() <= CTeamAiRole::ETeamAiRole::Initial ||
           role.GetTeamAiRole() > CTeamAiRole::ETeamAiRole::Unassigned;
  });
}

void CTeamAiMgr::ResetRoles(CStateManager& mgr) {
  for (auto& role : x58_roles) {
    role.x10_curRole = CTeamAiRole::ETeamAiRole::Initial;
    role.x14_roleIndex = 0;
    if (const CAi* ai = static_cast<const CAi*>(mgr.GetObjectById(role.GetOwnerId())))
      role.x1c_position = ai->GetTranslation();
  }
}

void CTeamAiMgr::SpacingSort(CStateManager& mgr, const zeus::CVector3f& pos) {
  const TeamAiRoleSorter sorter(pos, TeamAiRoleSorter::Type::TeamAIRole);
  std::sort(x58_roles.begin(), x58_roles.end(), sorter);
  float tierStagger = 4.5f;
  for (const auto& role : x58_roles) {
    if (const TCastToConstPtr<CPatterned> ai = mgr.ObjectById(role.GetOwnerId())) {
      const float length = (ai->GetBaseBoundingBox().max.y() - ai->GetBaseBoundingBox().min.y()) * 1.5f;
      if (length > tierStagger) {
        tierStagger = length;
      }
    }
  }
  float curTierDist = tierStagger;
  int tierTeamSize = 0;
  int maxTierTeamSize = 3;
  for (auto& role : x58_roles) {
    if (const TCastToConstPtr<CPatterned> ai = mgr.ObjectById(role.GetOwnerId())) {
      zeus::CVector3f delta = ai->GetTranslation() - pos;
      zeus::CVector3f newPos;
      if (delta.canBeNormalized()) {
        newPos = pos + delta.normalized() * curTierDist;
      } else {
        newPos = pos + ai->GetTransform().basis[1] * curTierDist;
      }
      role.x1c_position = newPos;
      role.x1c_position.z() = ai->GetTranslation().z();
      tierTeamSize += 1;
      if (tierTeamSize > maxTierTeamSize) {
        curTierDist += tierStagger;
        tierTeamSize = 0;
        maxTierTeamSize += 1;
      }
    }
  }
  const TeamAiRoleSorter sorter2(pos, TeamAiRoleSorter::Type::OwnerID);
  std::sort(x58_roles.begin(), x58_roles.end(), sorter2);
}

void CTeamAiMgr::PositionTeam(CStateManager& mgr) {
  zeus::CVector3f aimPos = mgr.GetPlayer().GetAimPosition(mgr, 0.f);
  switch (x34_data.x18_positionMode) {
  case 1:
    SpacingSort(mgr, aimPos);
    break;
  default:
    for (auto& role : x58_roles) {
      if (const TCastToConstPtr<CPatterned> ai = mgr.ObjectById(role.GetOwnerId())) {
        role.x1c_position = ai->GetOrigin(mgr, role, aimPos);
      }
    }
    break;
  }
}

void CTeamAiMgr::AssignRoles(CTeamAiRole::ETeamAiRole assRole, s32 count) {
  if (count == 0) {
    return;
  }

  s32 lastIdx = 0;
  for (auto& role : x58_roles) {
    if (role.GetTeamAiRole() == CTeamAiRole::ETeamAiRole::Initial) {
      if (role.x4_roleA == assRole || role.x8_roleB == assRole || role.xc_roleC == assRole) {
        role.x10_curRole = assRole;
        role.x14_roleIndex = lastIdx++;
        if (lastIdx == count) {
          return;
        }
      }
    }
  }
}

void CTeamAiMgr::UpdateRoles(CStateManager& mgr) {
  ResetRoles(mgr);

  const zeus::CVector3f aimPos = mgr.GetPlayer().GetAimPosition(mgr, 0.f);
  const TeamAiRoleSorter sorter(aimPos, TeamAiRoleSorter::Type::Distance);
  std::sort(x58_roles.begin(), x58_roles.end(), sorter);

  AssignRoles(CTeamAiRole::ETeamAiRole::Melee, x34_data.x4_meleeCount);
  AssignRoles(CTeamAiRole::ETeamAiRole::Ranged, x34_data.x8_rangedCount);
  AssignRoles(CTeamAiRole::ETeamAiRole::Unknown, x34_data.xc_unknownCount);

  for (auto& role : x58_roles) {
    if (role.GetTeamAiRole() <= CTeamAiRole::ETeamAiRole::Initial ||
        role.GetTeamAiRole() > CTeamAiRole::ETeamAiRole::Unassigned) {
      role.SetTeamAiRole(CTeamAiRole::ETeamAiRole::Unassigned);
    }
  }

  const TeamAiRoleSorter sorter2(aimPos, TeamAiRoleSorter::Type::OwnerID);
  std::sort(x58_roles.begin(), x58_roles.end(), sorter2);
  x88_timeDirty = 0.f;
}

void CTeamAiMgr::Think(float dt, CStateManager& mgr) {
  CEntity::Think(dt, mgr);

  if (ShouldUpdateRoles(dt)) {
    UpdateRoles(mgr);
  }

  PositionTeam(mgr);
  x90_timeSinceMelee += dt;
  x94_timeSinceRanged += dt;
}

void CTeamAiMgr::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& mgr) {
  CEntity::AcceptScriptMsg(msg, objId, mgr);
}

CTeamAiRole* CTeamAiMgr::GetTeamAiRole(TUniqueId aiId) {
  auto search =
      rstl::binary_find(x58_roles.begin(), x58_roles.end(), aiId, [](const auto& obj) { return obj.GetOwnerId(); });
  return search != x58_roles.end() ? &*search : nullptr;
}

bool CTeamAiMgr::IsPartOfTeam(TUniqueId aiId) const {
  auto search =
      rstl::binary_find(x58_roles.begin(), x58_roles.end(), aiId, [](const auto& obj) { return obj.GetOwnerId(); });
  return search != x58_roles.end();
}

bool CTeamAiMgr::HasTeamAiRole(TUniqueId aiId) const {
  auto search =
      rstl::binary_find(x58_roles.begin(), x58_roles.end(), aiId, [](const auto& obj) { return obj.GetOwnerId(); });
  return (search != x58_roles.end() && search->GetTeamAiRole() > CTeamAiRole::ETeamAiRole::Initial &&
          search->GetTeamAiRole() <= CTeamAiRole::ETeamAiRole::Unassigned);
}

bool CTeamAiMgr::IsMeleeAttacker(TUniqueId aiId) const {
  auto search = rstl::binary_find(x68_meleeAttackers.begin(), x68_meleeAttackers.end(), aiId);
  return search != x68_meleeAttackers.end();
}

bool CTeamAiMgr::CanAcceptMeleeAttacker(TUniqueId aiId) const {
  if (x90_timeSinceMelee >= x34_data.x1c_meleeTimeInterval &&
      x68_meleeAttackers.size() < x34_data.x10_maxMeleeAttackerCount)
    return true;
  return IsMeleeAttacker(aiId);
}

bool CTeamAiMgr::AddMeleeAttacker(TUniqueId aiId) {
  if (x90_timeSinceMelee >= x34_data.x1c_meleeTimeInterval &&
      x68_meleeAttackers.size() < x34_data.x10_maxMeleeAttackerCount && HasTeamAiRole(aiId)) {
    auto search = rstl::binary_find(x68_meleeAttackers.begin(), x68_meleeAttackers.end(), aiId);
    if (search == x68_meleeAttackers.end()) {
      x68_meleeAttackers.insert(std::lower_bound(x68_meleeAttackers.begin(), x68_meleeAttackers.end(), aiId), aiId);
      x90_timeSinceMelee = 0.f;
    }
    return true;
  }
  return false;
}

void CTeamAiMgr::RemoveMeleeAttacker(TUniqueId aiId) {
  auto search = rstl::binary_find(x68_meleeAttackers.begin(), x68_meleeAttackers.end(), aiId);
  if (search != x68_meleeAttackers.end())
    x68_meleeAttackers.erase(search);
}

bool CTeamAiMgr::IsRangedAttacker(TUniqueId aiId) const {
  auto search = rstl::binary_find(x78_rangedAttackers.begin(), x78_rangedAttackers.end(), aiId);
  return search != x78_rangedAttackers.end();
}

bool CTeamAiMgr::CanAcceptRangedAttacker(TUniqueId aiId) const {
  if (x94_timeSinceRanged >= x34_data.x20_rangedTimeInterval &&
      x78_rangedAttackers.size() < x34_data.x14_maxRangedAttackerCount)
    return true;
  return IsRangedAttacker(aiId);
}

bool CTeamAiMgr::AddRangedAttacker(TUniqueId aiId) {
  if (x94_timeSinceRanged >= x34_data.x20_rangedTimeInterval &&
      x78_rangedAttackers.size() < x34_data.x14_maxRangedAttackerCount && HasTeamAiRole(aiId)) {
    auto search = rstl::binary_find(x78_rangedAttackers.begin(), x78_rangedAttackers.end(), aiId);
    if (search == x78_rangedAttackers.end()) {
      x78_rangedAttackers.insert(std::lower_bound(x78_rangedAttackers.begin(), x78_rangedAttackers.end(), aiId), aiId);
      x94_timeSinceRanged = 0.f;
    }
    return true;
  }
  return false;
}

void CTeamAiMgr::RemoveRangedAttacker(TUniqueId aiId) {
  auto search = rstl::binary_find(x78_rangedAttackers.begin(), x78_rangedAttackers.end(), aiId);
  if (search != x78_rangedAttackers.end())
    x78_rangedAttackers.erase(search);
}

bool CTeamAiMgr::AssignTeamAiRole(const CAi& ai, CTeamAiRole::ETeamAiRole roleA, CTeamAiRole::ETeamAiRole roleB,
                                  CTeamAiRole::ETeamAiRole roleC) {
  CTeamAiRole newRole(ai.GetUniqueId(), roleA, roleB, roleC);
  auto search = rstl::binary_find(x58_roles.begin(), x58_roles.end(), newRole);
  if (search == x58_roles.end()) {
    if (x58_roles.size() >= x58_roles.capacity())
      return false;
    x58_roles.insert(std::lower_bound(x58_roles.begin(), x58_roles.end(), newRole), newRole);
  } else {
    *search = newRole;
  }
  UpdateTeamCaptain();
  return true;
}

void CTeamAiMgr::RemoveTeamAiRole(TUniqueId aiId) {
  if (IsMeleeAttacker(aiId))
    RemoveMeleeAttacker(aiId);
  if (IsRangedAttacker(aiId))
    RemoveRangedAttacker(aiId);
  auto search =
      rstl::binary_find(x58_roles.begin(), x58_roles.end(), aiId, [](const auto& obj) { return obj.GetOwnerId(); });
  x58_roles.erase(search);
  UpdateTeamCaptain();
}

void CTeamAiMgr::ClearTeamAiRole(TUniqueId aiId) {
  auto search =
      rstl::binary_find(x58_roles.begin(), x58_roles.end(), aiId, [](const auto& obj) { return obj.GetOwnerId(); });
  if (search != x58_roles.end())
    search->x10_curRole = CTeamAiRole::ETeamAiRole::Initial;
}

s32 CTeamAiMgr::GetNumAssignedOfRole(CTeamAiRole::ETeamAiRole testRole) const {
  return static_cast<s32>(std::count_if(x58_roles.cbegin(), x58_roles.cend(),
                                        [testRole](const auto& role) { return role.GetTeamAiRole() == testRole; }));
}

s32 CTeamAiMgr::GetNumAssignedAiRoles() const {
  return static_cast<s32>(std::count_if(x58_roles.cbegin(), x58_roles.cend(), [](const auto& role) {
    const auto aiRole = role.GetTeamAiRole();
    return aiRole > CTeamAiRole::ETeamAiRole::Initial && aiRole <= CTeamAiRole::ETeamAiRole::Unassigned;
  }));
}

CTeamAiRole* CTeamAiMgr::GetTeamAiRole(CStateManager& mgr, TUniqueId mgrId, TUniqueId aiId) {
  if (TCastToPtr<CTeamAiMgr> aimgr = mgr.ObjectById(mgrId))
    return aimgr->GetTeamAiRole(aiId);
  return nullptr;
}

void CTeamAiMgr::ResetTeamAiRole(EAttackType type, CStateManager& mgr, TUniqueId mgrId, TUniqueId aiId,
                                 bool clearRole) {
  if (TCastToPtr<CTeamAiMgr> tmgr = mgr.ObjectById(mgrId)) {
    if (tmgr->HasTeamAiRole(aiId)) {
      if (type == EAttackType::Melee) {
        if (tmgr->IsMeleeAttacker(aiId))
          tmgr->RemoveMeleeAttacker(aiId);
      } else if (type == EAttackType::Ranged) {
        if (tmgr->IsRangedAttacker(aiId))
          tmgr->RemoveRangedAttacker(aiId);
      }
      if (clearRole)
        tmgr->ClearTeamAiRole(aiId);
    }
  }
}

bool CTeamAiMgr::CanAcceptAttacker(EAttackType type, CStateManager& mgr, TUniqueId mgrId, TUniqueId aiId) {
  if (TCastToPtr<CTeamAiMgr> tmgr = mgr.ObjectById(mgrId)) {
    if (tmgr->HasTeamAiRole(aiId)) {
      if (type == EAttackType::Melee)
        return tmgr->CanAcceptMeleeAttacker(aiId);
      else if (type == EAttackType::Ranged)
        return tmgr->CanAcceptRangedAttacker(aiId);
    }
  }
  return false;
}

bool CTeamAiMgr::AddAttacker(EAttackType type, CStateManager& mgr, TUniqueId mgrId, TUniqueId aiId) {
  if (TCastToPtr<CTeamAiMgr> tmgr = mgr.ObjectById(mgrId)) {
    if (tmgr->HasTeamAiRole(aiId)) {
      if (type == EAttackType::Melee)
        return tmgr->AddMeleeAttacker(aiId);
      else if (type == EAttackType::Ranged)
        return tmgr->AddRangedAttacker(aiId);
    }
  }
  return false;
}

TUniqueId CTeamAiMgr::GetTeamAiMgr(const CAi& ai, const CStateManager& mgr) {
  for (const auto& conn : ai.GetConnectionList()) {
    if (conn.x0_state == EScriptObjectState::Active && conn.x4_msg == EScriptObjectMessage::Play) {
      const TUniqueId id = mgr.GetIdForScript(conn.x8_objId);
      if (const TCastToConstPtr<CTeamAiMgr> aimgr = mgr.GetObjectById(id)) {
        return aimgr->GetUniqueId();
      }
    }
  }
  return kInvalidUniqueId;
}

} // namespace metaforce
