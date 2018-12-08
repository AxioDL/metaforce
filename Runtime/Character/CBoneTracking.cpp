#include "Character/CBoneTracking.hpp"
#include "Character/CBodyController.hpp"
#include "Character/CAnimData.hpp"
#include "Character/CHierarchyPoseBuilder.hpp"
#include "World/CPatterned.hpp"
#include "CStateManager.hpp"

#include "TCastTo.hpp"

namespace urde {

CBoneTracking::CBoneTracking(const CAnimData& animData, std::string_view bone, float f1, float f2, bool b1)
: x14_segId(animData.GetCharLayoutInfo().GetSegIdFromString(bone))
, x1c_(f1)
, x20_(f2)
, x36_24_active(false)
, x36_25_(false)
, x36_26_(b1)
, x36_27_(b1)
, x36_28_(b1)
, x36_29_(b1) {}

void CBoneTracking::Update(float dt) { x18_time += dt; }

void CBoneTracking::PreRender(const CStateManager& mgr, CAnimData& animData, const zeus::CTransform& xf,
                              const zeus::CVector3f& vec, const CBodyController& bodyController) {
  TCastToPtr<CPatterned> patterned = bodyController.GetOwner();

  PreRender(mgr, animData, xf, vec,
            (bodyController.GetBodyStateInfo().ApplyHeadTracking() && patterned && patterned->ApplyBoneTracking()));
}

void CBoneTracking::PreRender(const CStateManager& mgr, CAnimData& animData, const zeus::CTransform& xf,
                              const zeus::CVector3f& vec, bool b) {
  if (x14_segId == 0)
    return;

  x18_time = 0.f;
}

void CBoneTracking::SetActive(bool) { x36_24_active = true; }

void CBoneTracking::SetTarget(TUniqueId target) { x34_target = target; }

void CBoneTracking::SetTargetPosition(const zeus::CVector3f& targetPos) { x24_targetPosition = targetPos; }
} // namespace urde