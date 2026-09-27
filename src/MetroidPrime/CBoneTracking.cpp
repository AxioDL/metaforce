#include "MetroidPrime/CBoneTracking.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/TGameTypes.hpp"

#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"

CBoneTracking::CBoneTracking(const CAnimData& animData, const rstl::string& bone,
                             float maxTrackingAngle, float angSpeed, EBoneTrackingFlags flags)
: mRotation(CQuaternion::NoRotation())
, x10_(0.f)
, mSegId(animData.GetCharLayoutInfo()->GetSegIdFromString(bone))
, mTime(0.f)
, mMaxTrackingAngle(maxTrackingAngle)
, mAngSpeed(angSpeed)
, mTarget(kInvalidUniqueId)
, mActive(false)
, mHasTrackedRotation(false)
, mNoParent(flags & kBTF_NoParent)
, mNoParentOrigin(flags & kBTF_NoParentOrigin)
, mNoHorizontalAim(flags & kBTF_NoHorizontalAim)
, mParentIk(flags & kBTF_ParentIk) {}

void CBoneTracking::Update(float dt) { mTime += dt; }

void CBoneTracking::PreRender(const CStateManager& mgr, CAnimData& animData, const CTransform4f& xf,
                              const CVector3f& scale, const CBodyController& controller) {
  CPatterned* patterned = TCastToPtr< CPatterned >(controller.GetOwner());
  PreRender(mgr, animData, xf, scale,
            controller.GetBodyStateInfo().ApplyHeadTracking() &&
                (!patterned || patterned->ApplyBoneTracking()));
}

void CBoneTracking::PreRender(const CStateManager& mgr, CAnimData& animData, const CTransform4f& xf,
                              const CVector3f& scale, const bool tracking) {
  if (mSegId != CSegId::Null()) {
    CHierarchyPoseBuilder& poseBuilder = animData.PoseBuilder();
    const CActor* target = TCastToConstPtr< CActor >(mgr.GetObjectById(mTarget));
    if (mActive && tracking && (target || mTargetPosition.valid())) {
      mHasTrackedRotation = true;
      const CCharLayoutInfo* layout = *poseBuilder.CharLayoutInfo();
      const CSegId bone = mNoParent ? mSegId : layout->GetOriginalParent(mSegId);
      CTransform4f parentXf = CTransform4f::Identity();
      poseBuilder.BuildTransform(bone, parentXf);
      CVector3f position = parentXf.GetTranslation();
      if (mNoParentOrigin && !mNoParent) {
        CTransform4f boneXf = CTransform4f::Identity();
        poseBuilder.BuildTransform(mSegId, boneXf);
        position = boneXf.GetTranslation();
      }
      parentXf.SetTranslation(CVector3f::ByElementMultiply(scale, position));
      CTransform4f finalXf = xf * parentXf;
      const CVector3f& targetPosition =
          target ? target->GetAimPosition(mgr, 0.f) : *mTargetPosition;
      CVector3f localDir = finalXf.TransposeMultiply(targetPosition).AsNormalized();
      if (mNoHorizontalAim) {
        const float horizontalMagnitude =
            CMath::SqrtF(localDir.GetX() * localDir.GetX() + localDir.GetY() * localDir.GetY());
        localDir = CVector3f(0.f, horizontalMagnitude, localDir.GetZ());
      }
      if (mParentIk) {
        const float negativeElevation = -parentXf.GetForward().GetZ();
        const CVector3f ikBase(0.f, CMath::SqrtF(1.f - negativeElevation * negativeElevation),
                               negativeElevation);
        float angle = CVector3f::GetAngleDiff(ikBase, localDir);
        angle = CMath::Min(angle, mMaxTrackingAngle);
        localDir = CVector3f::Slerp(ikBase, localDir, CRelAngle::FromRadians(angle));
      } else {
        float angle = CVector3f::GetAngleDiff(CVector3f::Forward(), localDir);
        angle = CMath::Min(angle, mMaxTrackingAngle);
        localDir = CVector3f::Slerp(CVector3f::Forward(), localDir, CRelAngle::FromRadians(angle));
      }
      const CVector3f currentDir = mRotation.Transform(CVector3f::Forward());
      const float angle = CVector3f::GetAngleDiff(currentDir, localDir);
      const float maxAngleDelta = mTime * mAngSpeed;
      const float clampedAngle = CMath::Min(angle, maxAngleDelta);
      if (clampedAngle > 1.e-5f) {
        const CVector3f& forward = CVector3f::Forward();
        const CQuaternion rotation =
            CQuaternion::LookAt(forward, localDir, CRelAngle::FromDegrees(360.f));
        mRotation = CQuaternion::SlerpLocal(mRotation, rotation, clampedAngle / angle);
      }
      poseBuilder.Insert(mSegId, mRotation);
      animData.SetPoseBuilderValid(false);
    } else if (mHasTrackedRotation) {
      const CVector3f currentDir = mRotation.Transform(CVector3f::Forward());
      const CQuaternion rotation = poseBuilder.GetSegRotation(mSegId);
      const CVector3f animationDir = rotation.Transform(CVector3f::Forward());
      const float angle = CVector3f::GetAngleDiff(currentDir, animationDir);
      const float maxAngleDelta = mTime * mAngSpeed;
      const float clampedAngle = CMath::Min(angle, maxAngleDelta);
      if (clampedAngle > 0.5f * maxAngleDelta) {
        mRotation = CQuaternion::SlerpLocal(mRotation, rotation, clampedAngle / angle);
        poseBuilder.Insert(mSegId, mRotation);
        animData.SetPoseBuilderValid(false);
      } else {
        mHasTrackedRotation = false;
        mRotation = rotation;
      }
    } else {
      mRotation = poseBuilder.GetSegRotation(mSegId);
    }
  }
  mTime = 0.f;
}

void CBoneTracking::SetActive(bool v) { mActive = v; }
void CBoneTracking::SetTarget(const TUniqueId target) { mTarget = target; }
void CBoneTracking::UnsetTarget() { mTarget = kInvalidUniqueId; }
void CBoneTracking::SetTargetPosition(const CVector3f& target) { mTargetPosition = target; }
void CBoneTracking::SetNoHorizontalAim(const bool aim) { mNoHorizontalAim = aim; }
