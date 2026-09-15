#include "Kyoto/Animation/CAnimTreeBlend.hpp"

rstl::ownership_transfer< IAnimReader > CAnimTreeBlend::VClone() const {
  return rs_new CAnimTreeBlend(CharacterSpaceBlend(), Cast(x14_a->Clone()), Cast(x18_b->Clone()),
                               x24_blendWeight, x4_name);
}

float CAnimTreeBlend::VGetBlendingWeight() const { return x24_blendWeight; }

CCharAnimTime CAnimTreeBlend::VGetTimeRemaining() const {
  return rstl::max_val(x14_a->GetTimeRemaining(), x18_b->GetTimeRemaining());
}

CSteadyStateAnimInfo CAnimTreeBlend::VGetSteadyStateAnimInfo() const {
  CSteadyStateAnimInfo infoA = x14_a->GetSteadyStateAnimInfo();
  CSteadyStateAnimInfo infoB = x18_b->GetSteadyStateAnimInfo();
  CVector3f offsetA = infoA.GetOffset();
  CVector3f offsetB = infoB.GetOffset();
  CCharAnimTime durationA = infoA.GetDuration();
  CCharAnimTime durationB = infoB.GetDuration();
  CVector3f offset;
  if (durationA < durationB) {
    float scale = durationB / durationA;
    offset = offsetA * scale * x24_blendWeight + offsetB * (1.f - x24_blendWeight);
  } else if (durationB < durationA) {
    float scale = durationA / durationB;
    offset = offsetA * x24_blendWeight + offsetB * scale * (1.f - x24_blendWeight);
  } else {
    offset = offsetA + offsetB;
  }
  return CSteadyStateAnimInfo(infoB.IsLooping(),
                              rstl::max_val(infoA.GetDuration(), infoB.GetDuration()), offset);
}

rstl::string CAnimTreeBlend::CreatePrimitiveName(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                                 const rstl::ncrc_ptr< CAnimTreeNode >& b,
                                                 float weight) {
  return rstl::string_l("");
}

void CAnimTreeBlend::SetBlendingWeight(float weight) { x24_blendWeight = weight; }

CAdvancementResults CAnimTreeBlend::VAdvanceView(const CCharAnimTime& time) {
  IncAdvancementDepth();
#if VERSION >= VERSION_GM8P_00
  CAdvancementResults resA = x14_a->AdvanceView(time);
  CAdvancementResults resB = x18_b->AdvanceView(time);
#else
  CAdvancementResults resA = x14_a->AdvanceView(time);
  const CAdvancementDeltas& deltasA = resA.GetAdvancementDeltas();
  CAdvancementResults resB = x18_b->AdvanceView(time);
  const CAdvancementDeltas& deltasB = resB.GetAdvancementDeltas();
#endif
  DecAdvancementDepth();
  if (ShouldCullTree()) {
    if (GetBlendingWeight() < 0.5f)
      x20_25_cullSelector = 1;
    else
      x20_25_cullSelector = 2;
  }
  CCharAnimTime remainder = rstl::max_val(resA.GetRemainder(), resB.GetRemainder());
#if VERSION >= VERSION_GM8P_00
  const CAdvancementDeltas& deltasA = resA.GetAdvancementDeltas();
  const CAdvancementDeltas& deltasB = resB.GetAdvancementDeltas();
#endif
  if (GetBlendRoot() & kBlendRoot_Offset)
    return CAdvancementResults(remainder,
                               CAdvancementDeltas::Blend(deltasA, deltasB, GetBlendingWeight()));
  return resB;
}
