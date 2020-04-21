#include "Runtime/Character/CBoneTracking.hpp"

#include "Runtime/CStateManager.hpp"
#include "Runtime/Character/CAnimData.hpp"
#include "Runtime/Character/CBodyController.hpp"
#include "Runtime/Character/CHierarchyPoseBuilder.hpp"
#include "Runtime/World/CPatterned.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde {

CBoneTracking::CBoneTracking(const CAnimData& animData, std::string_view bone,
                             float maxTrackingAngle, float angSpeed, EBoneTrackingFlags flags)
: x14_segId(animData.GetCharLayoutInfo().GetSegIdFromString(bone))
, x1c_maxTrackingAngle(maxTrackingAngle)
, x20_angSpeed(angSpeed)
, x36_26_noParent(True(flags & EBoneTrackingFlags::NoParent))
, x36_27_noParentOrigin(True(flags & EBoneTrackingFlags::NoParentOrigin))
, x36_28_noHorizontalAim(True(flags & EBoneTrackingFlags::NoHorizontalAim))
, x36_29_parentIk(True(flags & EBoneTrackingFlags::ParentIk)) {}

void CBoneTracking::Update(float dt) { x18_time += dt; }

void CBoneTracking::PreRender(const CStateManager& mgr, CAnimData& animData, const zeus::CTransform& xf,
                              const zeus::CVector3f& vec, const CBodyController& bodyController) {
  TCastToPtr<CPatterned> patterned = bodyController.GetOwner();

  PreRender(mgr, animData, xf, vec,
            (bodyController.GetBodyStateInfo().ApplyHeadTracking() && patterned && patterned->ApplyBoneTracking()));
}

void CBoneTracking::PreRender(const CStateManager& mgr, CAnimData& animData, const zeus::CTransform& worldXf,
                              const zeus::CVector3f& localOffsetScale, bool tracking) {
  if (x14_segId == 0)
    return;
  CHierarchyPoseBuilder& pb = animData.PoseBuilder();
  TCastToConstPtr<CActor> targetAct = mgr.GetObjectById(x34_target);
  if (x36_24_active && tracking && (targetAct || x24_targetPosition)) {
    x36_25_hasTrackedRotation = true;
    const auto& layoutInfo = pb.CharLayoutInfo();
    CSegId bone;
    if (x36_26_noParent)
      bone = x14_segId;
    else
      bone = layoutInfo->GetRootNode()->GetBoneMap()[x14_segId].x0_parentId;
    zeus::CTransform parentBoneXf;
    pb.BuildTransform(bone, parentBoneXf);
    zeus::CVector3f pos = parentBoneXf.origin;
    if (x36_27_noParentOrigin && !x36_26_noParent) {
      zeus::CTransform thisBoneXf;
      pb.BuildTransform(x14_segId, thisBoneXf);
      pos = thisBoneXf.origin;
    }
    parentBoneXf.origin = pos * localOffsetScale;
    zeus::CTransform finalXf = worldXf * parentBoneXf;
    zeus::CVector3f localDir = finalXf.transposeRotate(
      (targetAct ? targetAct->GetAimPosition(mgr, 0.f) : *x24_targetPosition) - finalXf.origin).normalized();
    if (x36_28_noHorizontalAim)
      localDir = zeus::CVector3f(0.f, localDir.toVec2f().magnitude(), localDir.z());
    if (x36_29_parentIk) {
      float negElev = -parentBoneXf.basis[1].z();
      zeus::CVector3f ikBase(0.f, std::sqrt(1.f - negElev * negElev), negElev);
      float angle = zeus::CVector3f::getAngleDiff(ikBase, localDir);
      angle = std::min(angle, x1c_maxTrackingAngle);
      localDir = zeus::CVector3f::slerp(ikBase, localDir, angle);
    } else {
      float angle = zeus::CVector3f::getAngleDiff(zeus::skForward, localDir);
      angle = std::min(angle, x1c_maxTrackingAngle);
      localDir = zeus::CVector3f::slerp(zeus::skForward, localDir, angle);
    }
    float angle = zeus::CVector3f::getAngleDiff(x0_curRotation.transform(zeus::skForward), localDir);
    float clampedAngle = std::min(angle, x18_time * x20_angSpeed);
    if (clampedAngle > 1.0e-05f) {
      x0_curRotation = zeus::CQuaternion::slerpShort(x0_curRotation,
      zeus::CQuaternion::lookAt(zeus::skForward, zeus::CUnitVector3f(localDir), 2.f * M_PIF),
      clampedAngle / angle);
    }
    pb.GetTreeMap()[x14_segId].x4_rotation = x0_curRotation;
    animData.MarkPoseDirty();
  } else if (x36_25_hasTrackedRotation) {
    zeus::CQuaternion qb = pb.GetTreeMap()[x14_segId].x4_rotation;
    float angle = zeus::CVector3f::getAngleDiff(x0_curRotation.transform(zeus::skForward),
                                                qb.transform(zeus::skForward));
    float maxAngDelta = x18_time * x20_angSpeed;
    float clampedAngle = std::min(angle, maxAngDelta);
    if (clampedAngle > 0.5f * maxAngDelta) {
      x0_curRotation = zeus::CQuaternion::slerpShort(x0_curRotation, qb, clampedAngle / angle);
      pb.GetTreeMap()[x14_segId].x4_rotation = x0_curRotation;
      animData.MarkPoseDirty();
    } else {
      x36_25_hasTrackedRotation = false;
      x0_curRotation = qb;
    }
  } else {
    x0_curRotation = pb.GetTreeMap()[x14_segId].x4_rotation;
  }
  x18_time = 0.f;
}

void CBoneTracking::SetActive(bool active) { x36_24_active = active; }

void CBoneTracking::SetTarget(TUniqueId target) { x34_target = target; }

void CBoneTracking::UnsetTarget() { x34_target = kInvalidUniqueId; }

void CBoneTracking::SetTargetPosition(const zeus::CVector3f& targetPos) { x24_targetPosition = targetPos; }

void CBoneTracking::SetNoHorizontalAim(bool b) { x36_28_noHorizontalAim = b; }
} // namespace urde
