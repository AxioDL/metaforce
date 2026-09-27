#include "Kyoto/Animation/CAnimTreeTransition.hpp"

#include "Kyoto/Animation/CCharAnimMemoryMetrics.hpp"
#include "Kyoto/Math/CloseEnough.hpp"

CAnimTreeTransition::CAnimTreeTransition(const bool b1, const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                         const rstl::ncrc_ptr< CAnimTreeNode >& b,
                                         const CCharAnimTime& transDur, bool runA, int flags,
                                         const rstl::string& name)
: CAnimTreeTweenBase(b1, a, b, flags, name)
, mTransDur(transDur)
, mTimeInTrans(0.f)
, mRunA(runA)
, mLoopA(a->GetBoolPOIState("Loop"))
, mInitialized(false) {
  CCharAnimMemoryMetrics::AddToTotalSize(19, CCharAnimMemoryMetrics::kASS_Two);
}

CAnimTreeTransition::CAnimTreeTransition(const bool b1, const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                         const rstl::ncrc_ptr< CAnimTreeNode >& b,
                                         const CCharAnimTime& transDur,
                                         const CCharAnimTime& timeInTrans, bool runA, bool loopA,
                                         int flags, const rstl::string& name, bool initialized)
: CAnimTreeTweenBase(b1, a, b, flags, name)
, mTransDur(transDur)
, mTimeInTrans(timeInTrans)
, mRunA(runA)
, mLoopA(loopA)
, mInitialized(initialized) {
  CCharAnimMemoryMetrics::AddToTotalSize(19, CCharAnimMemoryMetrics::kASS_Two);
}

CAnimTreeTransition::~CAnimTreeTransition() {
  CCharAnimMemoryMetrics::SubtractFromTotalSize(19, CCharAnimMemoryMetrics::kASS_Two);
}

rstl::optional_object< rstl::ownership_transfer< IAnimReader > >
CAnimTreeTransition::VSimplified() {
  if (close_enough(GetBlendingWeight(), 1.f)) {
    rstl::optional_object< rstl::ownership_transfer< IAnimReader > > simp = mB->Simplified();
    if (simp)
      return simp;
    return mB->Clone();
  }
  return CAnimTreeTweenBase::VSimplified();
}

rstl::optional_object< rstl::ownership_transfer< IAnimReader > >
CAnimTreeTransition::VReverseSimplified() {
  if (close_enough(GetBlendingWeight(), 0.f))
    return mA->Clone();
  return CAnimTreeTweenBase::VReverseSimplified();
}

rstl::pair< CCharAnimTime, CAdvancementDeltas >
CAnimTreeTransition::AdvanceViewForTransitionalPeriod(const CCharAnimTime& time) {
  IncAdvancementDepth();
  CDoubleChildAdvancementResult res = AdvanceViewBothChildren(time, mRunA, mLoopA);
  DecAdvancementDepth();
  const CCharAnimTime& trueAdvancement = res.GetTrueAdvancement();
  if (trueAdvancement.EqualsZero())
    return rstl::pair< CCharAnimTime, CAdvancementDeltas >(
        CCharAnimTime::ZeroFlat(),
        CAdvancementDeltas(CVector3f::Zero(), CQuaternion::NoRotation()));
  float oldWeight = GetBlendingWeight();
  mTimeInTrans += trueAdvancement;
  float newWeight = GetBlendingWeight();
  if (ShouldCullTree()) {
    if (newWeight < 0.5f)
      mCullSelector = 1;
    else
      mCullSelector = 2;
  }
  const CAdvancementDeltas& leftDeltas = res.GetLeftAdvancementDeltas();
  const CAdvancementDeltas& rightDeltas = res.GetRightAdvancementDeltas();
  if (GetBlendRoot() & kBlendRoot_Offset)
    return rstl::pair< CCharAnimTime, CAdvancementDeltas >(
        res.GetTrueAdvancement(),
        CAdvancementDeltas::Interpolate(leftDeltas, rightDeltas, oldWeight, newWeight));
  return rstl::pair< CCharAnimTime, CAdvancementDeltas >(res.GetTrueAdvancement(), rightDeltas);
}

CAdvancementResults CAnimTreeTransition::VAdvanceView(const CCharAnimTime& time) {
  if (time.EqualsZero()) {
    IncAdvancementDepth();
    mB->AdvanceView(time);
    if (mRunA)
      mA->AdvanceView(time);
    DecAdvancementDepth();
    if (ShouldCullTree())
      mCullSelector = 1;
    return CAdvancementResults(CCharAnimTime::ZeroFlat(),
                               CAdvancementDeltas(CVector3f::Zero(), CQuaternion::NoRotation()));
  }
  if (!mInitialized)
    mInitialized = true;
  if (mTimeInTrans + time < mTransDur) {
    rstl::pair< CCharAnimTime, CAdvancementDeltas > res = AdvanceViewForTransitionalPeriod(time);
    return CAdvancementResults(time - res.first, res.second);
  }
  CCharAnimTime transTimeRem = mTransDur - mTimeInTrans;
  rstl::pair< CCharAnimTime, CAdvancementDeltas > res(
      CCharAnimTime(0.f), CAdvancementDeltas(CVector3f::Zero(), CQuaternion::NoRotation()));
  if (transTimeRem.GreaterThanZero()) {
    res = AdvanceViewForTransitionalPeriod(transTimeRem);
    if (res.first != transTimeRem)
      return CAdvancementResults(res.first, res.second);
  }
  CCharAnimTime remainder = time - transTimeRem;
  return CAdvancementResults(remainder, res.second);
}

rstl::ownership_transfer< IAnimReader > CAnimTreeTransition::VClone() const {
  return rs_new CAnimTreeTransition(CharacterSpaceBlend(), Cast(mA->Clone()),
                                    Cast(mB->Clone()), mTransDur, mTimeInTrans, mRunA,
                                    mLoopA, GetBlendRoot(), mName, mInitialized);
}

float CAnimTreeTransition::VGetBlendingWeight() const {
  if (mTransDur.GreaterThanZero())
    return (1.f / mTransDur.GetSeconds()) * mTimeInTrans.GetSeconds();
  return 1.f;
}

CCharAnimTime CAnimTreeTransition::VGetTimeRemaining() const {
  return rstl::max_val(mB->GetTimeRemaining(), mTransDur - mTimeInTrans);
}

CSteadyStateAnimInfo CAnimTreeTransition::VGetSteadyStateAnimInfo() const {
  CSteadyStateAnimInfo bInfo = mB->GetSteadyStateAnimInfo();
  return CSteadyStateAnimInfo(bInfo.IsLooping(), rstl::max_val(mTransDur, bInfo.GetDuration()),
                              bInfo.GetOffset());
}

rstl::string CAnimTreeTransition::CreatePrimitiveName(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                                      const rstl::ncrc_ptr< CAnimTreeNode >& b,
                                                      float duration) {
  return rstl::string_l("");
}

void CAnimTreeTransition::SetBlendingWeight(float weight) {
  rstl::rc_ptr< CAnimTreeNode > right = GetRightChild();
  static_cast< CAnimTreeTweenBase* >(right.GetPtr())->SetBlendingWeight(weight);
}

rstl::rc_ptr< CAnimTreeNode > CAnimTreeTransition::VGetBestUnblendedChild() const {
  rstl::rc_ptr< CAnimTreeNode > right = GetRightChild();
  rstl::rc_ptr< CAnimTreeNode > child = right->GetBestUnblendedChild();
  if (!child)
    return right;
  return child;
}

const int CAnimTreeTweenBase::kBlendRoot_Offset = 1;

const int CAnimTreeTweenBase::kBlendRoot_Rotation = 2;
