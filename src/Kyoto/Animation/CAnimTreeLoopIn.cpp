#include "Kyoto/Animation/CAnimTreeLoopIn.hpp"

#include "Kyoto/Animation/CSequenceHelper.hpp"
#include "Kyoto/Animation/CTreeUtils.hpp"
#include "Kyoto/Math/CloseEnough.hpp"

template < class T >
uint _getPOIList(const CCharAnimTime& time, T* listOut, uint capacity, uint iterator, int additive,
                 const rstl::vector< T >& stream, const CCharAnimTime& curTime) {
  uint ret = 0;
  CCharAnimTime curTimeCopy(curTime);
  uint count = stream.size();
  CCharAnimTime tmpTime = curTime + time;
  uint it = iterator;
  while (it < count) {
    T node(stream[it]);
    if (node.GetTime() > tmpTime)
      break;
    if (node.GetTime() >= curTimeCopy) {
      if (iterator + ret < capacity) {
        T adjustedNode = T::CopyNodeMinusStartTime(node, curTimeCopy);
        listOut[iterator + ret] = adjustedNode;
        ++ret;
      }
    }
    ++it;
  }
  return ret;
}

CAnimTreeLoopIn::CAnimTreeLoopIn(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                 const rstl::ncrc_ptr< CAnimTreeNode >& b,
                                 const rstl::ncrc_ptr< CAnimTreeNode >& c,
                                 const CAnimSysContext& animCtx, const rstl::string& name)
: CAnimTreeSingleChild(CTreeUtils::GetTransitionTree(a, c, animCtx), name)
, mNextAnim(b)
, mDidLoopIn(false)
, mAnimCtx(animCtx)
, mFundamentals(CSequenceHelper(mChild, mNextAnim, animCtx).ComputeSequenceFundamentals())
, mCurTime(0.f) {}

CAnimTreeLoopIn::CAnimTreeLoopIn(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                 const rstl::ncrc_ptr< CAnimTreeNode >& b, bool didLoopIn,
                                 const CAnimSysContext& animCtx, const rstl::string& name,
                                 const CSequenceFundamentals& fundamentals,
                                 const CCharAnimTime& time)
: CAnimTreeSingleChild(a, name)
, mNextAnim(b)
, mDidLoopIn(didLoopIn)
, mAnimCtx(animCtx)
, mFundamentals(fundamentals)
, mCurTime(time) {}

CAdvancementResults CAnimTreeLoopIn::VAdvanceView(const CCharAnimTime& dt) {
  rstl::ncrc_ptr< CAnimTreeNode > origChild = mChild;
  CAdvancementResults res = origChild->AdvanceView(dt);
  CCharAnimTime remainder = res.GetRemainder();
  mCurTime += dt - remainder;
  CCharAnimTime remTime = origChild->GetTimeRemaining();
  if ((close_enough(remTime.GetSeconds(), 0.f) ||
       close_enough(dt.GetSeconds() - remainder.GetSeconds(), 0.f)) &&
      !mDidLoopIn) {
    rstl::ncrc_ptr< CAnimTreeNode > newChild =
        CTreeUtils::GetTransitionTree(origChild, mNextAnim, mAnimCtx);
    ReplaceChild(newChild);
    mDidLoopIn = true;
  }
  return res;
}

CCharAnimTime CAnimTreeLoopIn::VGetTimeRemaining() const {
  CCharAnimTime duration = mFundamentals.GetSteadyStateAnimInfo().GetDuration();
  return duration - mCurTime;
}

CSteadyStateAnimInfo CAnimTreeLoopIn::VGetSteadyStateAnimInfo() const {
  return mFundamentals.GetSteadyStateAnimInfo();
}

uint CAnimTreeLoopIn::VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut,
                                      uint capacity, uint iterator, int unk) const {
  return _getPOIList(time, listOut, capacity, iterator, unk,
                     mFundamentals.GetBoolPointsOfInterest(), mCurTime);
}

uint CAnimTreeLoopIn::VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut,
                                       uint capacity, uint iterator, int unk) const {
  return _getPOIList(time, listOut, capacity, iterator, unk,
                     mFundamentals.GetInt32PointsOfInterest(), mCurTime);
}

uint CAnimTreeLoopIn::VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut,
                                          uint capacity, uint iterator, int unk) const {
  return _getPOIList(time, listOut, capacity, iterator, unk,
                     mFundamentals.GetParticlePointsOfInterest(), mCurTime);
}

uint CAnimTreeLoopIn::VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut,
                                       uint capacity, uint iterator, int unk) const {
  return _getPOIList(time, listOut, capacity, iterator, unk,
                     mFundamentals.GetSoundPointsOfInterest(), mCurTime);
}

rstl::ownership_transfer< IAnimReader > CAnimTreeLoopIn::VClone() const {
  return rs_new CAnimTreeLoopIn(Cast(mChild->Clone()), mNextAnim, mDidLoopIn, mAnimCtx,
                                mName, mFundamentals, mCurTime);
}

rstl::rc_ptr< CAnimTreeNode > CAnimTreeLoopIn::VGetBestUnblendedChild() const {
  rstl::rc_ptr< CAnimTreeNode > child = mChild->GetBestUnblendedChild();
  if (child)
    return rs_new CAnimTreeLoopIn(Cast(child->Clone()), mNextAnim, mDidLoopIn, mAnimCtx,
                                  mName, mFundamentals, mCurTime);
  return child;
}

rstl::optional_object< rstl::ownership_transfer< IAnimReader > > CAnimTreeLoopIn::VSimplified() {
  CCharAnimTime remTime = mChild->GetTimeRemaining();
  if (remTime.GreaterThanZero() && !close_enough(remTime.GetSeconds(), 0.f)) {
    rstl::ncrc_ptr< CAnimTreeNode > child = mChild;
    rstl::optional_object< rstl::ownership_transfer< IAnimReader > > simp = child->Simplified();
    if (simp)
      ReplaceChild(Cast(*simp));
  } else if (mDidLoopIn && mChild->GetTimeRemaining().EqualsZero()) {
    return mChild->Clone();
  }
  return rstl::optional_object_null();
}

bool CAnimTreeLoopIn::VSupportsReverseView() const { return false; }

CAnimTreeEffectiveContribution CAnimTreeLoopIn::VGetContributionOfHighestInfluence() const {
  return mChild->GetContributionOfHighestInfluence();
}

rstl::string CAnimTreeLoopIn::CreatePrimitiveName(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                                  const rstl::ncrc_ptr< CAnimTreeNode >& b,
                                                  const rstl::ncrc_ptr< CAnimTreeNode >& c) {
  return rstl::string_l("");
}
