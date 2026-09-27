#include "MetroidPrime/Enemies/CWallWalker.hpp"

#include "Kyoto/Animation/CVertexMorphEffect.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Enemies/CPatternedInfo.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWaypoint.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "WorldFormat/CMetroidAreaCollider.hpp"

static const char* const skBendingAnimation = "BendingAnimationHack";

CWallWalker::CWallWalker(const EPatternedAI chr, const TUniqueId uid, const rstl::string& name,
                         const EFlavorType flavorType, const CEntityInfo& info,
                         const CTransform4f& xf, const CModelData& mData,
                         const CPatternedInfo& pInfo, const EMovementType moveType,
                         const EColliderType colType, const EBodyType bodyType,
                         const CActorParameters& actParms, const ECreatureSize kbVariant,
                         const float collisionCloseMargin, const EType walkerType,
                         const bool disableMove, const float alignAngVel,
                         const float advanceWpRadius, const float playerObstructionMinDist)
: CPatterned(chr, uid, name, flavorType, info, xf, mData, pInfo, moveType, colType, bodyType,
             actParms, kbVariant)
, mAlignNormal(CVector3f::Zero(), CVector3f::Right(), CVector3f::Forward(), 0xffffffff)
, mColSphere(CSphere(CVector3f::Zero(), pInfo.GetHalfExtent()), GetMaterialList())
, mCollisionCloseMargin(collisionCloseMargin)
, mAlignAngVel(alignAngVel)
, mTumbleAngle(0.f)
, mPatrolPauseRemTime(0.f)
, mAdvanceWpRadius(advanceWpRadius)
, mPlayerObstructionMinDist(playerObstructionMinDist)
, mBendingHackWeight(0.f)
, mBendingHackAnim(
      GetAnimationData()->GetCharacterInfo().GetAnimationIndex(rstl::string_l(skBendingAnimation)))
, mWalkerType(walkerType)
, mThinkCounter(0)
, mAlignToFloor(false)
, mHasAlignSurface(false)
, mPlayerObstructed(false)
, mDisableMove(disableMove)
, mAddBendingWeight(false)
, mApplyBendingHack(false) {}

CVector3f CWallWalker::ProjectPointToPlane(const CVector3f& point, const CVector3f& planePoint,
                                           const CVector3f& normal) {
  return point - CVector3f::Dot(point - planePoint, normal) * normal;
}

CVector3f CWallWalker::ProjectVectorToPlane(const CVector3f& vec, const CVector3f& planeDir) {
  return vec - CVector3f::Dot(vec, planeDir) * planeDir;
}

bool CWallWalker::PointOnSurface(const CCollisionSurface& surface, const CVector3f& point) {
  const CVector3f projected = ProjectPointToPlane(point, surface.GetVert(0), surface.GetNormal());
  const CVector3f normal = surface.GetNormal();
  for (int i = 0; i < 3; ++i) {
    const CVector3f edge = surface.GetVert((i + 2) % 3) - surface.GetVert(i);
    const CVector3f delta = projected - surface.GetVert(i);
    const CVector3f cross = CVector3f::Cross(delta, edge);
    if (CVector3f::Dot(normal, cross) < 0.f) {
      return false;
    }
  }
  return true;
}

void CWallWalker::OrientToSurfaceNormal(const CVector3f& normal, float clampAngle) {
  const CVector3f& up = GetTransform().GetUp();
  const float dot = CVector3f::Dot(up, normal);
  if (close_enough(dot, 1.f)) {
    return;
  }
  if (dot < -0.999f) {
    return;
  }
  const CQuaternion rotation = CQuaternion::ClampedRotateTo(GetTransform().GetUp(), normal,
                                                            CRelAngle::FromDegrees(clampAngle));
  const CQuaternion localRotation(rotation.GetScalar(),
                                  GetTransform().TransposeRotate(rotation.GetVector()));
  SetRotation((CQuaternion::FromMatrix(GetTransform()) * localRotation).BuildNormalized());
}

void CWallWalker::AlignToFloor(CStateManager& mgr, float radius, const CVector3f& newPos,
                               float dt) {
  bool hasSurface = false;
  const CVector3f extent(radius + mCollisionCloseMargin, radius + mCollisionCloseMargin,
                         radius + mCollisionCloseMargin);
  const CAABox bounds(newPos - extent, newPos + extent);
  CAreaCollisionCache cache(bounds);
  CGameCollision::BuildAreaCollisionCache(mgr, cache);
  float margin = radius + mCollisionCloseMargin;
  if (mHasAlignSurface) {
    mHasAlignSurface = PointOnSurface(mAlignNormal, newPos);
  }
  if (!mHasAlignSurface || !(mThinkCounter & 3)) {
    for (int i = 0; i < static_cast< int >(cache.GetNumCaches()); ++i) {
      const CMetroidAreaCollider::COctreeLeafCache& leaf = cache.GetOctreeLeafCache(i);
      for (int j = 0; j < leaf.GetNumLeaves(); ++j) {
        const CAreaOctTree::Node& node = leaf.GetLeaf(j);
        const CAreaOctTree::TriListReference triangles = node.GetTriangleArray();
        const CAreaOctTree& tree = node.GetOwner();
        const int triangleCount = triangles.GetSize();
        for (int k = 0; k < triangleCount; ++k) {
          const CCollisionSurface& surface = tree.GetMasterListTriangle(triangles.GetAt(k));
          const float planeConstant = surface.GetPlane().GetConstant();
          const float dist =
              CMath::AbsF(CVector3f::Dot(surface.GetNormal(), newPos) - planeConstant);
          if (dist < margin && PointOnSurface(surface, newPos)) {
            margin = dist;
            mAlignNormal = surface;
            hasSurface = true;
          }
        }
      }
    }
    mHasAlignSurface = hasSurface;
  }
  if (mHasAlignSurface) {
    OrientToSurfaceNormal(CVector3f(mAlignNormal.GetNormal()), mAlignAngVel * dt);
    mTumbleAngle = 0.f;
    mAddBendingWeight = false;
  } else {
    const float angularVelocity =
        CMath::Rad2Deg(GetVelocityWR().Magnitude()) / mColSphere.GetSphere().GetRadius();
    const float angle = angularVelocity * dt;
    OrientToSurfaceNormal(CVector3f(GetTransform().GetForward()), angle);
    if (mBodyController->HasBodyState(pas::kAS_Step)) {
      mBodyController->CommandMgr().DeliverCmd(CBCStepCmd(pas::kSD_Down, pas::kStep_Normal));
    } else {
      mAddBendingWeight = true;
    }
    mTumbleAngle += angle;
  }
}

void CWallWalker::PreThink(float dt, CStateManager& mgr) {
  CPatterned::PreThink(dt, mgr);
  if (GetActive() && !mPlayerObstructed && mPatrolPauseRemTime <= 0.f &&
      !mDisableMove && close_enough(mBodyController->GetPercentageFrozen(), 0.f) &&
      mAlignToFloor) {
    // Retail constructs both orientation snapshots around the motion update.
    const CQuaternion oldOrientation = CQuaternion::FromMatrix(GetTransform());
    const CMotionState motion = PredictMotion(dt);
    AddMotionState(motion);
    const CQuaternion newOrientation = CQuaternion::FromMatrix(GetTransform());
    ClearForcesAndTorques();
    if (mHasAlignSurface) {
      const CPlane plane = mAlignNormal.GetPlane();
      const CVector3f position = GetTranslation();
      const CVector3f projected = position - (plane.GetHeight(GetTranslation()) -
                                              mColSphere.GetSphere().GetRadius() - 0.01f) *
                                                 plane.GetNormal();
      SetTranslation(CVector3f::Lerp(position, projected, 10.f * dt));
    }
    MoveCollisionPrimitive(CVector3f::Zero());
  }
}

void CWallWalker::GotoNextWaypoint(CStateManager& mgr) {
  if (CScriptWaypoint* waypoint = TCastToPtr< CScriptWaypoint >(mgr.ObjectById(mDestObj))) {
    const CVector3f position = waypoint->GetTranslation();
    const CVector3f& delta = position - GetTranslation();
    if (delta.MagSquared() < mAdvanceWpRadius * mAdvanceWpRadius) {
      mDestObj = waypoint->NextWaypoint(mgr);
      if (!close_enough(waypoint->GetPause(), 0.f)) {
        mPatrolPauseRemTime = waypoint->GetPause();
        if (mWalkerType == kWT_Parasite) {
          mBodyController->SetLocomotionType(pas::kLT_Relaxed);
        }
      }
      mgr.DeliverScriptMsg(waypoint, GetUniqueId(), kSM_Arrived);
    }
    SetDestPos(position);
  }
}

void CWallWalker::Render(const CStateManager& mgr) const { CPatterned::Render(mgr); }

void CWallWalker::Think(float dt, CStateManager& mgr) {
  CPatterned::Think(dt, mgr);
  if (mBendingHackAnim == -1) {
    return;
  }
  if (mAddBendingWeight) {
    if (mBendingHackWeight < 1.f) {
      const float speed = GetVelocityWR().Magnitude();
      const float weight = dt * speed / 0.6f;
      mBendingHackWeight += weight;
      if (mBendingHackWeight >= 1.f) {
        mBendingHackWeight = 1.f;
      }
    }
  } else if (mBendingHackWeight > 0.f) {
    const float speed = GetVelocityWR().Magnitude();
    const float weight = dt * speed / 1.5f;
    mBendingHackWeight -= weight;
    if (mBendingHackWeight <= 0.f) {
      mBendingHackWeight = 0.f;
    }
  }
  if (mBendingHackWeight > 0.f || mApplyBendingHack) {
    if (mBendingHackWeight > 0.0001f) {
      AnimationData()->AddAdditiveAnimation(mBendingHackAnim, mBendingHackWeight, true,
                                            false);
      mApplyBendingHack = true;
    } else {
      AnimationData()->DelAdditiveAnimation(mBendingHackAnim);
      mApplyBendingHack = false;
    }
  }
}
