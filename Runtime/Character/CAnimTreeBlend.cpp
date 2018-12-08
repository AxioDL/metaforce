#include "CAnimTreeBlend.hpp"

namespace urde {

std::string CAnimTreeBlend::CreatePrimitiveName(const std::shared_ptr<CAnimTreeNode>& a,
                                                const std::shared_ptr<CAnimTreeNode>& b, float scale) {
  return "";
}

CAnimTreeBlend::CAnimTreeBlend(bool b1, const std::shared_ptr<CAnimTreeNode>& a,
                               const std::shared_ptr<CAnimTreeNode>& b, float blendWeight, std::string_view name)
: CAnimTreeTweenBase(b1, a, b, 1 | 2, name), x24_blendWeight(blendWeight) {}

SAdvancementResults CAnimTreeBlend::VAdvanceView(const CCharAnimTime& dt) {
  IncAdvancementDepth();
  SAdvancementResults resA = x14_a->VAdvanceView(dt);
  SAdvancementResults resB = x18_b->VAdvanceView(dt);
  DecAdvancementDepth();
  if (ShouldCullTree()) {
    if (GetBlendingWeight() < 0.5f)
      x20_25_cullSelector = 1;
    else
      x20_25_cullSelector = 2;
  }

  const SAdvancementResults& maxRemTime = (resA.x0_remTime < resB.x0_remTime) ? resB : resA;
  if (x1c_flags & 0x1) {
    return {maxRemTime.x0_remTime, SAdvancementDeltas::Blend(resA.x8_deltas, resB.x8_deltas, GetBlendingWeight())};
  } else {
    return resB;
  }
}

CCharAnimTime CAnimTreeBlend::VGetTimeRemaining() const {
  CCharAnimTime remA = x14_a->VGetTimeRemaining();
  CCharAnimTime remB = x18_b->VGetTimeRemaining();
  return (remA < remB) ? remB : remA;
}

CSteadyStateAnimInfo CAnimTreeBlend::VGetSteadyStateAnimInfo() const {
  CSteadyStateAnimInfo ssA = x14_a->VGetSteadyStateAnimInfo();
  CSteadyStateAnimInfo ssB = x18_b->VGetSteadyStateAnimInfo();
  zeus::CVector3f resOffset;
  if (ssA.GetDuration() < ssB.GetDuration()) {
    resOffset = ssA.GetOffset() * (ssB.GetDuration() / ssA.GetDuration()) * x24_blendWeight +
                ssB.GetOffset() * (1.f - x24_blendWeight);
  } else if (ssB.GetDuration() < ssA.GetDuration()) {
    resOffset = ssA.GetOffset() * x24_blendWeight +
                ssB.GetOffset() * (ssA.GetDuration() / ssB.GetDuration()) * (1.f - x24_blendWeight);
  } else {
    resOffset = ssA.GetOffset() + ssB.GetOffset();
  }

  return {ssA.IsLooping(), (ssA.GetDuration() < ssB.GetDuration()) ? ssB.GetDuration() : ssA.GetDuration(), resOffset};
}

std::unique_ptr<IAnimReader> CAnimTreeBlend::VClone() const {
  return std::make_unique<CAnimTreeBlend>(x20_24_b1, CAnimTreeNode::Cast(x14_a->Clone()),
                                          CAnimTreeNode::Cast(x18_b->Clone()), x24_blendWeight, x4_name);
}

void CAnimTreeBlend::SetBlendingWeight(float w) { x24_blendWeight = w; }

float CAnimTreeBlend::VGetBlendingWeight() const { return x24_blendWeight; }

} // namespace urde
