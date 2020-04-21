#include "Runtime/World/CWallWalker.hpp"

#include "Runtime/CStateManager.hpp"
#include "Runtime/Collision/CGameCollision.hpp"
#include "Runtime/Collision/CMetroidAreaCollider.hpp"
#include "Runtime/World/CPatternedInfo.hpp"
#include "Runtime/World/CScriptWaypoint.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde {
CWallWalker::CWallWalker(ECharacter chr, TUniqueId uid, std::string_view name, EFlavorType flavType,
                         const CEntityInfo& eInfo, const zeus::CTransform& xf, CModelData&& mData,
                         const CPatternedInfo& pInfo, EMovementType mType, EColliderType colType, EBodyType bType,
                         const CActorParameters& aParms, float collisionCloseMargin, float alignAngVel,
                         EKnockBackVariant kbVariant, float advanceWpRadius, EWalkerType wType,
                         float playerObstructionMinDist, bool disableMove)
: CPatterned(chr, uid, name, flavType, eInfo, xf, std::move(mData), pInfo, mType, colType, bType, aParms, kbVariant)
, x590_colSphere(zeus::CSphere(zeus::skZero3f, pInfo.GetHalfExtent()), x68_material)
, x5b0_collisionCloseMargin(collisionCloseMargin)
, x5b4_alignAngVel(alignAngVel)
, x5c0_advanceWpRadius(advanceWpRadius)
, x5c4_playerObstructionMinDist(playerObstructionMinDist)
, x5cc_bendingHackAnim(
      GetModelData()->GetAnimationData()->GetCharacterInfo().GetAnimationIndex("BendingAnimationHack"sv))
, x5d0_walkerType(wType)
, x5d6_27_disableMove(disableMove) {}

void CWallWalker::OrientToSurfaceNormal(const zeus::CVector3f& normal, float clampAngle) {
  float dot = x34_transform.basis[2].dot(normal);
  if (zeus::close_enough(dot, 1.f) || dot < -0.999f)
    return;
  zeus::CQuaternion q = zeus::CQuaternion::clampedRotateTo(x34_transform.basis[2], normal, zeus::degToRad(clampAngle));
  q.setImaginary(x34_transform.transposeRotate(q.getImaginary()));
  SetTransform((zeus::CQuaternion(x34_transform.basis) * q).normalized().toTransform(GetTranslation()));
}

bool CWallWalker::PointOnSurface(const CCollisionSurface& surf, const zeus::CVector3f& point) {
  zeus::CVector3f normal = surf.GetNormal();
  zeus::CVector3f projPt = ProjectPointToPlane(point, surf.GetVert(0), normal);
  for (int i = 0; i < 3; ++i) {
    zeus::CVector3f projDelta = projPt - surf.GetVert(i);
    zeus::CVector3f edge = surf.GetVert((i + 2) % 3) - surf.GetVert(i);
    if (projDelta.cross(edge).dot(normal) < 0.f)
      return false;
  }
  return true;
}

void CWallWalker::AlignToFloor(CStateManager& mgr, float radius, const zeus::CVector3f& newPos, float dt) {
  bool hasSurface = false;
  float margin = radius + x5b0_collisionCloseMargin;
  zeus::CAABox aabb(newPos - margin, newPos + margin);
  CAreaCollisionCache ccache(aabb);
  CGameCollision::BuildAreaCollisionCache(mgr, ccache);
  if (x5d6_25_hasAlignSurface)
    x5d6_25_hasAlignSurface = PointOnSurface(x568_alignNormal, newPos);
  if (!x5d6_25_hasAlignSurface || !(x5d4_thinkCounter & 0x3)) {
    for (auto& leaf : ccache) {
      for (auto& node : leaf) {
        CAreaOctTree::TriListReference triArr = node.GetTriangleArray();
        for (u16 i = 0; i < triArr.GetSize(); ++i) {
          u16 triIdx = triArr.GetAt(i);
          CCollisionSurface surf = leaf.GetOctTree().GetMasterListTriangle(triIdx);
          float dist = std::fabs(surf.GetPlane().pointToPlaneDist(newPos));
          if (dist < margin && PointOnSurface(surf, newPos)) {
            margin = dist;
            x568_alignNormal = surf;
            hasSurface = true;
          }
        }
      }
    }
    x5d6_25_hasAlignSurface = hasSurface;
  }
  if (x5d6_25_hasAlignSurface) {
    OrientToSurfaceNormal(x568_alignNormal.GetNormal(), x5b4_alignAngVel * dt);
    x5b8_tumbleAngle = 0.f;
    x5d6_28_addBendingWeight = false;
  } else {
    float angDelta = zeus::radToDeg(x138_velocity.magnitude()) / x590_colSphere.GetSphere().radius * dt;
    OrientToSurfaceNormal(x34_transform.basis[1], angDelta);
    if (x450_bodyController->HasBodyState(pas::EAnimationState::Step)) {
      x450_bodyController->GetCommandMgr().DeliverCmd(CBCStepCmd(pas::EStepDirection::Down, pas::EStepType::Normal));
    } else {
      x5d6_28_addBendingWeight = true;
    }
    x5b8_tumbleAngle += angDelta;
  }
}

void CWallWalker::GotoNextWaypoint(CStateManager& mgr) {
  if (TCastToPtr<CScriptWaypoint> wp = mgr.ObjectById(x2dc_destObj)) {
    zeus::CVector3f wpPos = wp->GetTranslation();
    if ((wpPos - GetTranslation()).magSquared() < x5c0_advanceWpRadius * x5c0_advanceWpRadius) {
      x2dc_destObj = wp->NextWaypoint(mgr);
      if (!zeus::close_enough(wp->GetPause(), 0.f)) {
        x5bc_patrolPauseRemTime = wp->GetPause();
        if (x5d0_walkerType == EWalkerType::Parasite)
          x450_bodyController->SetLocomotionType(pas::ELocomotionType::Relaxed);
      }
      mgr.SendScriptMsg(wp, GetUniqueId(), EScriptObjectMessage::Arrived);
    }
    SetDestPos(wpPos);
  }
}

void CWallWalker::PreThink(float dt, CStateManager& mgr) {
  CPatterned::PreThink(dt, mgr);
  if (!GetActive() || x5d6_26_playerObstructed || x5bc_patrolPauseRemTime > 0.f || x5d6_27_disableMove ||
      x450_bodyController->IsFrozen() || !x5d6_24_alignToFloor)
    return;

  zeus::CQuaternion quat(GetTransform().buildMatrix3f());
  AddMotionState(PredictMotion(dt));
  zeus::CQuaternion quat2(GetTransform().buildMatrix3f());
  ClearForcesAndTorques();
  if (x5d6_25_hasAlignSurface) {
    zeus::CPlane plane = x568_alignNormal.GetPlane();
    const float futureDt = (10.f * dt);
    SetTranslation(zeus::CVector3f::lerp(
        GetTranslation(),
        GetTranslation() -
            (plane.pointToPlaneDist(GetTranslation()) - x590_colSphere.GetSphere().radius - 0.01f) * plane.normal(),
        futureDt));
  }
  MoveCollisionPrimitive(zeus::skZero3f);
}

void CWallWalker::Think(float dt, CStateManager& mgr) {
  if (!x450_bodyController->GetActive())
    x450_bodyController->Activate(mgr);
  CPatterned::Think(dt, mgr);

  if (x5cc_bendingHackAnim == -1)
    return;

  if (x5d6_28_addBendingWeight) {
    if (x5c8_bendingHackWeight < 1.f) {
      x5c8_bendingHackWeight += (dt * x138_velocity.magnitude()) / 0.6f;
      if (x5c8_bendingHackWeight >= 1.f)
        x5c8_bendingHackWeight = 1.f;
    }
  } else if (x5c8_bendingHackWeight > 0.f) {
    x5c8_bendingHackWeight -= (dt * x138_velocity.magnitude()) / 1.5f;
    if (x5c8_bendingHackWeight < 0.f)
      x5c8_bendingHackWeight = 0.f;
  }

  if (x5c8_bendingHackWeight <= 0.f && !x5d6_29_applyBendingHack)
    return;

  if (x5c8_bendingHackWeight > 0.0001f) {
    GetModelData()->GetAnimationData()->AddAdditiveAnimation(x5cc_bendingHackAnim, x5c8_bendingHackWeight, true, false);
    x5d6_29_applyBendingHack = true;
  } else {
    GetModelData()->GetAnimationData()->DelAdditiveAnimation(x5cc_bendingHackAnim);
    x5d6_29_applyBendingHack = false;
  }
}

void CWallWalker::Render(CStateManager& mgr) { CPatterned::Render(mgr); }

void CWallWalker::UpdateWPDestination(CStateManager& mgr) {
  if (TCastToPtr<CScriptWaypoint> wp = mgr.ObjectById(x2dc_destObj)) {
    zeus::CVector3f wpPos = wp->GetTranslation();
    if ((wpPos - GetTranslation()).magSquared() < x5c0_advanceWpRadius * x5c0_advanceWpRadius) {
      x2dc_destObj = wp->NextWaypoint(mgr);
      if (std::fabs(wp->GetPause()) > 0.00001f) {
        x5bc_patrolPauseRemTime = wp->GetPause();
        if (x5d0_walkerType == EWalkerType::Parasite)
          x450_bodyController->SetLocomotionType(pas::ELocomotionType::Relaxed);
        mgr.SendScriptMsg(wp, GetUniqueId(), EScriptObjectMessage::Arrived);
      }
    }

    SetDestPos(wpPos);
  }
}

} // namespace urde
