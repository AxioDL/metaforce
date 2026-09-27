#include "MetroidPrime/Enemies/CTeamAiMgr.hpp"

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/Enemies/CAi.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "rstl/algorithm.hpp"

const int CTeamAiMgr::CUnknown::kNumProperties = 8;

struct CRoleSorter {
  CVector3f mPos;
  int mType;

  CRoleSorter(const CVector3f& pos, int type) : mPos(pos), mType(type) {}

  bool operator()(const CTeamAiRole& a, const CTeamAiRole& b) const;
};

bool CRoleSorter::operator()(const CTeamAiRole& a, const CTeamAiRole& b) const {
  const float aDist = (mPos - a.GetTeamPosition()).MagSquared();
  const float bDist = (mPos - b.GetTeamPosition()).MagSquared();

  switch (mType) {
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
: mAiCount(in.ReadLong())
, mMeleeCount(in.ReadLong())
, mProjectileCount(in.ReadLong())
, mUnknownCount(in.ReadLong())
, mMaxMeleeAttackerCount(in.ReadLong())
, mMaxProjectileAttackerCount(in.ReadLong())
, mPositionMode(in.ReadLong())
, mMeleeTimeInterval(propCount > 8 ? in.ReadFloat() : 0.f)
, mProjectileTimeInterval(propCount > 8 ? in.ReadFloat() : 0.f) {}

CTeamAiMgr::CTeamAiMgr(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                       const CUnknown& data)
: CEntity(uid, info, true, name)
, mData(data)
, mRoles()
, mMeleeAttackers()
, mProjectileAttackers()
, mTimeDirty(0.f)
, mTeamCaptainId(kInvalidUniqueId)
, mTimeSinceMelee(data.mMeleeTimeInterval)
, mTimeSinceProjectile(data.mProjectileTimeInterval) {
  if (mData.mAiCount != 0) {
    mRoles.reserve(mData.mAiCount);
  }
  if (mData.mMeleeCount != 0) {
    mMeleeAttackers.reserve(mData.mMeleeCount);
  }
  if (mData.mProjectileCount != 0) {
    mProjectileAttackers.reserve(mData.mProjectileCount);
  }
}

TUniqueId CTeamAiMgr::GetTeamAiMgr(const CAi& ai, const CStateManager& mgr) {
  rstl::vector< SConnection >::const_iterator conn = ai.GetConnectionList().begin();
  for (; conn != ai.GetConnectionList().end(); ++conn) {
    if (conn->mState == kSS_Active && conn->mMsg == kSM_Play) {
      if (const CTeamAiMgr* teamMgr = TCastToConstPtr< CTeamAiMgr >(
              mgr.GetObjectById(mgr.GetIdForScript(conn->mObjId)))) {
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
  mTimeSinceMelee += dt;
  mTimeSinceProjectile += dt;
}

bool CTeamAiMgr::AssignTeamAiRole(const CAi& ai, int roleA, int roleB, int roleC) {
  CTeamAiRole searchRole(ai.GetUniqueId(), static_cast< CTeamAiRole::ETeamAiRole >(roleA),
                         static_cast< CTeamAiRole::ETeamAiRole >(roleB),
                         static_cast< CTeamAiRole::ETeamAiRole >(roleC));
  rstl::vector< CTeamAiRole >::iterator search =
      rstl::binary_find(mRoles.begin(), mRoles.end(), searchRole);

  if (search == mRoles.end()) {
    if (mRoles.size() < mRoles.capacity()) {
      rstl::vector< CTeamAiRole >::iterator insertPos =
          rstl::lower_bound(mRoles.begin(), mRoles.end(), searchRole);
      mRoles.insert(insertPos, searchRole);
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
      rstl::binary_find(mRoles.begin(), mRoles.end(), searchRole);
  mRoles.erase(search);

  UpdateTeamCaptain();
}

int CTeamAiMgr::GetNumAssignedAiRoles() const {
  rstl::vector< CTeamAiRole >::const_iterator cur = mRoles.begin();
  rstl::vector< CTeamAiRole >::const_iterator end = mRoles.end();
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
  rstl::vector< CTeamAiRole >::const_iterator cur = mRoles.begin();
  rstl::vector< CTeamAiRole >::const_iterator end = mRoles.end();
  for (; cur != end; ++cur) {
    if (role == cur->mCurRole) {
      ++count;
    }
  }
  return count;
}

const CTeamAiRole* CTeamAiMgr::GetTeamAiRole(TUniqueId id) const {
  CTeamAiRole searchRole(id);
  rstl::vector< CTeamAiRole >::const_iterator search =
      rstl::binary_find(mRoles.begin(), mRoles.end(), searchRole);
  return search != mRoles.end() ? search.operator->() : 0;
}

void CTeamAiMgr::ClearTeamAiRole(TUniqueId id) {
  CTeamAiRole searchRole(id);
  rstl::vector< CTeamAiRole >::iterator search =
      rstl::binary_find(mRoles.begin(), mRoles.end(), searchRole);
  if (search != mRoles.end()) {
    search->SetTeamAiRole(CTeamAiRole::kTAR_Initial);
  }
}

bool CTeamAiMgr::HasTeamAiRole(TUniqueId id) const {
  CTeamAiRole searchRole(id);
  rstl::vector< CTeamAiRole >::const_iterator search =
      rstl::binary_find(mRoles.begin(), mRoles.end(), searchRole);
  if (search != mRoles.end()) {
    return search->HasTeamAiRole();
  }
  return false;
}

bool CTeamAiMgr::IsPartOfTeam(TUniqueId id) const {
  CTeamAiRole searchRole(id);
  rstl::vector< CTeamAiRole >::const_iterator search =
      rstl::binary_find(mRoles.begin(), mRoles.end(), searchRole);
  return search != mRoles.end();
}

bool CTeamAiMgr::IsMeleeAttacker(TUniqueId id) const {
  rstl::vector< TUniqueId >::const_iterator search =
      rstl::binary_find(mMeleeAttackers.begin(), mMeleeAttackers.end(), id);
  return search != mMeleeAttackers.end();
}

bool CTeamAiMgr::CanAcceptMeleeAttacker(TUniqueId id) const {
  if (mTimeSinceMelee >= mData.mMeleeTimeInterval &&
      mMeleeAttackers.size() < mData.mMaxMeleeAttackerCount) {
    return true;
  }
  rstl::vector< TUniqueId >::const_iterator search =
      rstl::binary_find(mMeleeAttackers.begin(), mMeleeAttackers.end(), id);
  if (search != mMeleeAttackers.end()) {
    return true;
  }
  return false;
}

bool CTeamAiMgr::AddMeleeAttacker(TUniqueId id) {
  if (mTimeSinceMelee >= mData.mMeleeTimeInterval &&
      mMeleeAttackers.size() < mData.mMaxMeleeAttackerCount && HasTeamAiRole(id)) {
    rstl::vector< TUniqueId >::iterator search =
        rstl::binary_find(mMeleeAttackers.begin(), mMeleeAttackers.end(), id);
    if (search == mMeleeAttackers.end()) {
      mMeleeAttackers.reserve(mMeleeAttackers.size() + 1);
      rstl::vector< TUniqueId >::iterator insertPos =
          rstl::lower_bound(mMeleeAttackers.begin(), mMeleeAttackers.end(), id);
      mMeleeAttackers.insert(insertPos, id);
      mTimeSinceMelee = 0.f;
    }
    return true;
  }
  return false;
}

void CTeamAiMgr::RemoveMeleeAttacker(TUniqueId id) {
  rstl::vector< TUniqueId >::iterator search =
      rstl::binary_find(mMeleeAttackers.begin(), mMeleeAttackers.end(), id);
  if (search != mMeleeAttackers.end()) {
    mMeleeAttackers.erase(search);
  }
}

bool CTeamAiMgr::IsProjectileAttacker(TUniqueId id) const {
  rstl::vector< TUniqueId >::const_iterator search =
      rstl::binary_find(mProjectileAttackers.begin(), mProjectileAttackers.end(), id);
  return search != mProjectileAttackers.end();
}

bool CTeamAiMgr::CanAcceptProjectileAttacker(TUniqueId id) const {
  if (mTimeSinceProjectile >= mData.mProjectileTimeInterval &&
      mProjectileAttackers.size() < mData.mMaxProjectileAttackerCount) {
    return true;
  }
  rstl::vector< TUniqueId >::const_iterator search =
      rstl::binary_find(mProjectileAttackers.begin(), mProjectileAttackers.end(), id);
  if (search != mProjectileAttackers.end()) {
    return true;
  }
  return false;
}

bool CTeamAiMgr::AddProjectileAttacker(TUniqueId id) {
  if (mTimeSinceProjectile >= mData.mProjectileTimeInterval &&
      mProjectileAttackers.size() < mData.mMaxProjectileAttackerCount &&
      HasTeamAiRole(id)) {
    rstl::vector< TUniqueId >::iterator search =
        rstl::binary_find(mProjectileAttackers.begin(), mProjectileAttackers.end(), id);
    if (search == mProjectileAttackers.end()) {
      mProjectileAttackers.reserve(mProjectileAttackers.size() + 1);
      rstl::vector< TUniqueId >::iterator insertPos =
          rstl::lower_bound(mProjectileAttackers.begin(), mProjectileAttackers.end(), id);
      mProjectileAttackers.insert(insertPos, id);
      mTimeSinceProjectile = 0.f;
    }
    return true;
  }
  return false;
}

void CTeamAiMgr::RemoveProjectileAttacker(TUniqueId id) {
  rstl::vector< TUniqueId >::iterator search =
      rstl::binary_find(mProjectileAttackers.begin(), mProjectileAttackers.end(), id);
  if (search != mProjectileAttackers.end()) {
    mProjectileAttackers.erase(search);
  }
}

bool CTeamAiMgr::ShouldUpdateRoles(float dt) {
  if (mRoles.size() > 0) {
    mTimeDirty += dt;
    if (mTimeDirty >= 1.5f) {
      return true;
    }

    rstl::vector< CTeamAiRole >::const_iterator role = mRoles.begin();
    for (; role != mRoles.end(); ++role) {
      bool valid = false;
      const CTeamAiRole::ETeamAiRole curRole = role->mCurRole;
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
  rstl::sort(mRoles.begin(), mRoles.end(), CRoleSorter(aimPos, 1));

  AssignRoles(CTeamAiRole::kTAR_Melee, mData.mMeleeCount);
  AssignRoles(CTeamAiRole::kTAR_Projectile, mData.mProjectileCount);
  AssignRoles(CTeamAiRole::kTAR_Unknown, mData.mUnknownCount);

  rstl::vector< CTeamAiRole >::iterator role = mRoles.begin();
  for (; role != mRoles.end(); ++role) {
    if (!role->HasTeamAiRole()) {
      role->mCurRole = CTeamAiRole::kTAR_Unassigned;
    }
  }

  rstl::sort(mRoles.begin(), mRoles.end(), CRoleSorter(aimPos, 0));
  mTimeDirty = 0.f;
}

void CTeamAiMgr::ResetRoles(CStateManager& mgr) {
  rstl::vector< CTeamAiRole >::iterator role = mRoles.begin();
  for (; role != mRoles.end(); ++role) {
    role->mCurRole = CTeamAiRole::kTAR_Initial;
    role->mRoleIndex = 0;

    if (const CAi* ai = static_cast< const CAi* >(mgr.GetObjectById(role->GetOwnerId()))) {
      role->mPosition = ai->GetTranslation();
    }
  }
}

void CTeamAiMgr::AssignRoles(CTeamAiRole::ETeamAiRole role, uint count) {
  if (count == 0) {
    return;
  }

  uint roleIndex = 0;
  for (rstl::vector< CTeamAiRole >::iterator cur = mRoles.begin(); cur != mRoles.end();
       ++cur) {
    if (cur->mCurRole == CTeamAiRole::kTAR_Initial) {
      if (cur->AllowsRole(role)) {
        cur->mCurRole = role;
        cur->mRoleIndex = roleIndex;
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
  switch (static_cast< int >(mData.mPositionMode)) {
  case 1:
    SpacingSort(mgr, aimPos);
    break;
  case 0:
  default:
    rstl::vector< CTeamAiRole >::iterator role = mRoles.begin();
    for (; role != mRoles.end(); ++role) {
      if (CPatterned* ai = TCastToPtr< CPatterned >(mgr.ObjectById(role->GetOwnerId()))) {
        role->mPosition = ai->GetOrigin(mgr, *role, aimPos);
      }
    }
    break;
  }
}

void CTeamAiMgr::SpacingSort(CStateManager& mgr, const CVector3f& pos) {
  rstl::sort(mRoles.begin(), mRoles.end(), CRoleSorter(pos, 2));

  float tierStagger = 4.5f;
  for (rstl::vector< CTeamAiRole >::iterator role = mRoles.begin();
       role != mRoles.end(); ++role) {
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
  for (rstl::vector< CTeamAiRole >::iterator role = mRoles.begin();
       role != mRoles.end(); ++role) {
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

  rstl::sort(mRoles.begin(), mRoles.end(), CRoleSorter(pos, 0));
}

void CTeamAiMgr::UpdateTeamCaptain() {
  int captainPriority = -0x80000000;
  mTeamCaptainId = kInvalidUniqueId;
  rstl::vector< CTeamAiRole >::iterator role = mRoles.begin();
  for (; role != mRoles.end(); ++role) {
    if (role->mCaptainPriority > captainPriority) {
      captainPriority = role->mCaptainPriority;
      mTeamCaptainId = role->GetOwnerId();
    }
  }
}
