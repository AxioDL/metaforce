#include "Kyoto/Animation/CAnimTreeBlend.hpp"

rstl::ownership_transfer< IAnimReader > CAnimTreeBlend::VClone() const {
  return rs_new CAnimTreeBlend(CharacterSpaceBlend(), Cast(mA->Clone()), Cast(mB->Clone()),
                               mBlendWeight, mName);
}

float CAnimTreeBlend::VGetBlendingWeight() const { return mBlendWeight; }

CCharAnimTime CAnimTreeBlend::VGetTimeRemaining() const {
  return rstl::max_val(mA->GetTimeRemaining(), mB->GetTimeRemaining());
}

CSteadyStateAnimInfo CAnimTreeBlend::VGetSteadyStateAnimInfo() const {
  CSteadyStateAnimInfo infoA = mA->GetSteadyStateAnimInfo();
  CSteadyStateAnimInfo infoB = mB->GetSteadyStateAnimInfo();
  CVector3f offsetA = infoA.GetOffset();
  CVector3f offsetB = infoB.GetOffset();
  CCharAnimTime durationA = infoA.GetDuration();
  CCharAnimTime durationB = infoB.GetDuration();
  CVector3f offset;
  if (durationA < durationB) {
    float scale = durationB / durationA;
    offset = offsetA * scale * mBlendWeight + offsetB * (1.f - mBlendWeight);
  } else if (durationB < durationA) {
    float scale = durationA / durationB;
    offset = offsetA * mBlendWeight + offsetB * scale * (1.f - mBlendWeight);
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

void CAnimTreeBlend::SetBlendingWeight(float weight) { mBlendWeight = weight; }

CAdvancementResults CAnimTreeBlend::VAdvanceView(const CCharAnimTime& time) {
  IncAdvancementDepth();
#if VERSION >= VERSION_GM8E_02
  CAdvancementResults resA = mA->AdvanceView(time);
  CAdvancementResults resB = mB->AdvanceView(time);
#else
  CAdvancementResults resA = mA->AdvanceView(time);
  const CAdvancementDeltas& deltasA = resA.GetAdvancementDeltas();
  CAdvancementResults resB = mB->AdvanceView(time);
  const CAdvancementDeltas& deltasB = resB.GetAdvancementDeltas();
#endif
  DecAdvancementDepth();
  if (ShouldCullTree()) {
    if (GetBlendingWeight() < 0.5f)
      mCullSelector = 1;
    else
      mCullSelector = 2;
  }
  CCharAnimTime remainder = rstl::max_val(resA.GetRemainder(), resB.GetRemainder());
#if VERSION >= VERSION_GM8E_02
  const CAdvancementDeltas& deltasA = resA.GetAdvancementDeltas();
  const CAdvancementDeltas& deltasB = resB.GetAdvancementDeltas();
#endif
  if (GetBlendRoot() & kBlendRoot_Offset)
    return CAdvancementResults(remainder,
                               CAdvancementDeltas::Blend(deltasA, deltasB, GetBlendingWeight()));
  return resB;
}
