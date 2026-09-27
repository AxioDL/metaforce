#include "Kyoto/Animation/CAnimTreeSequence.hpp"

#include "Kyoto/Animation/CBoolPOINode.hpp"
#include "Kyoto/Animation/CInt32POINode.hpp"
#include "Kyoto/Animation/CParticlePOINode.hpp"
#include "Kyoto/Animation/CSoundPOINode.hpp"
#include "Kyoto/Animation/CTreeUtils.hpp"
#include "Kyoto/Animation/IMetaAnim.hpp"

template < class T >
uint _getPOIList(const CCharAnimTime& time, T* listOut, uint capacity, uint iterator, int additive,
                rstl::vector< T > stream, const CCharAnimTime& curTime) {
  CCharAnimTime curTimeCopy(curTime);
  uint count = stream.size();
  CCharAnimTime tmpTime = curTime + time;
  uint ret = 0;
  uint it = iterator;
  while (it < count) {
    T node(stream[it]);
    if (node.GetTime() > tmpTime)
      break;
    if (node.GetTime() >= curTimeCopy) {
      T adjustedNode = T::CopyNodeMinusStartTime(node, curTimeCopy);
      uint idx = iterator + ret;
      if (idx < capacity) {
        ++ret;
        T& dest = listOut[idx];
        dest = adjustedNode;
      }
    }
    ++it;
  }
  return ret;
}

CAnimTreeSequence::CAnimTreeSequence(const rstl::vector< rstl::rc_ptr< IMetaAnim > >& seq,
                                     const CAnimSysContext& animSys, const rstl::string& name)
: CAnimTreeSingleChild(
      seq[0]->GetAnimationTree(animSys, CMetaAnimTreeBuildOrders::NoSpecialOrders()), name)
, mAnimCtx(animSys)
, mSequence(seq)
, mCurIdx(0)
, mFundamentals(CSequenceHelper(seq, animSys).ComputeSequenceFundamentals())
, mCurTime(0.f) {}

CAnimTreeSequence::CAnimTreeSequence(const rstl::ncrc_ptr< CAnimTreeNode >& curNode,
                                     const rstl::vector< rstl::rc_ptr< IMetaAnim > >& metaAnims,
                                     const CAnimSysContext& animSys, int curIdx,
                                     const rstl::string& name,
                                     const CSequenceFundamentals& fundamentals,
                                     const CCharAnimTime& time)
: CAnimTreeSingleChild(curNode, name)
, mAnimCtx(animSys)
, mSequence(metaAnims)
, mCurIdx(curIdx)
, mFundamentals(fundamentals)
, mCurTime(time) {
  mChild->VGetTimeRemaining();
}

CAdvancementResults CAnimTreeSequence::VAdvanceView(const CCharAnimTime& dt) {
  CCharAnimTime totalDelta(0.f);
  CVector3f posDelta(0.f, 0.f, 0.f);
  CQuaternion rotDelta = CQuaternion::NoRotation();

  rstl::ncrc_ptr< CAnimTreeNode > curChild(mChild);
  int seqSize = mSequence.size();

  if (mCurIdx >= seqSize && curChild->VGetTimeRemaining().EqualsZero()) {
    mFundamentals = CSequenceHelper(mSequence, mAnimCtx).ComputeSequenceFundamentals();
    mCurIdx = 0;
    mCurTime = CCharAnimTime::ZeroFlat();
    rstl::ncrc_ptr< CAnimTreeNode > newChild =
        CTreeUtils::GetTransitionTree(curChild,
                                      mSequence[mCurIdx]->GetAnimationTree(
                                          mAnimCtx, CMetaAnimTreeBuildOrders::NoSpecialOrders()),
                                      mAnimCtx);
    mChild = newChild;
    curChild = mChild;
  }

  CCharAnimTime remTime = dt;
  while (remTime.GreaterThanZero() && mCurIdx < seqSize) {
    CCharAnimTime chRem = curChild->VGetTimeRemaining();
    if (chRem.EqualsZero()) {
      ++mCurIdx;
      if (mCurIdx < seqSize) {
        rstl::ncrc_ptr< CAnimTreeNode > newChild = CTreeUtils::GetTransitionTree(
            curChild,
            mSequence[mCurIdx]->GetAnimationTree(mAnimCtx,
                                                       CMetaAnimTreeBuildOrders::NoSpecialOrders()),
            mAnimCtx);
        mChild = newChild;
      }
    }
    curChild = mChild;
    if (mCurIdx < seqSize) {
      CAdvancementResults res = curChild->VAdvanceView(remTime);
      rstl::optional_object< rstl::ownership_transfer< IAnimReader > > simp =
          curChild->Simplified();
      if (simp) {
        curChild = Cast(*simp);
        mChild = curChild;
        curChild = mChild;
      }
      CCharAnimTime prevRemTime = remTime;
      remTime = res.mRemTime;
      totalDelta += prevRemTime - remTime;
      const CAdvancementDeltas& deltas = res.mDeltas;
      posDelta += deltas.mPosDelta;
      rotDelta *= deltas.mRotDelta;
    }
  }

  mCurTime += totalDelta;
  return CAdvancementResults(dt - totalDelta, CAdvancementDeltas(posDelta, rotDelta));
}

CCharAnimTime CAnimTreeSequence::VGetTimeRemaining() const {
  if (mCurIdx == mSequence.size() - 1)
    return mChild->VGetTimeRemaining();
  CCharAnimTime dur = mFundamentals.GetSteadyStateAnimInfo().GetDuration();
  return dur - mCurTime;
}

CSteadyStateAnimInfo CAnimTreeSequence::VGetSteadyStateAnimInfo() const {
  return mFundamentals.GetSteadyStateAnimInfo();
}

uint CAnimTreeSequence::VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut,
                                        uint capacity, uint iterator, int additive) const {
  return _getPOIList(time, listOut, capacity, iterator, additive,
                     mFundamentals.GetBoolPointsOfInterest(), mCurTime);
}

uint CAnimTreeSequence::VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut,
                                         uint capacity, uint iterator, int additive) const {
  return _getPOIList(time, listOut, capacity, iterator, additive,
                     mFundamentals.GetInt32PointsOfInterest(), mCurTime);
}

uint CAnimTreeSequence::VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut,
                                            uint capacity, uint iterator, int additive) const {
  return _getPOIList(time, listOut, capacity, iterator, additive,
                     mFundamentals.GetParticlePointsOfInterest(), mCurTime);
}

uint CAnimTreeSequence::VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut,
                                         uint capacity, uint iterator, int additive) const {
  return _getPOIList(time, listOut, capacity, iterator, additive,
                     mFundamentals.GetSoundPointsOfInterest(), mCurTime);
}

rstl::ownership_transfer< IAnimReader > CAnimTreeSequence::VClone() const {
  return rs_new CAnimTreeSequence(Cast(mChild->Clone()), mSequence, mAnimCtx, mCurIdx,
                                  mName, mFundamentals, mCurTime);
}

rstl::rc_ptr< CAnimTreeNode > CAnimTreeSequence::VGetBestUnblendedChild() const {
  rstl::rc_ptr< CAnimTreeNode > ch = mChild->GetBestUnblendedChild();
  if (ch)
    return rs_new CAnimTreeSequence(Cast(ch->Clone()), mSequence, mAnimCtx, mCurIdx,
                                    mName, mFundamentals, mCurTime);
  return ch;
}

bool CAnimTreeSequence::VSupportsReverseView() const { return false; }

CAnimTreeEffectiveContribution CAnimTreeSequence::VGetContributionOfHighestInfluence() const {
  return mChild->GetContributionOfHighestInfluence();
}

rstl::string CAnimTreeSequence::CreatePrimitiveName(const rstl::vector< rstl::string >& names) {
  return rstl::string_l("");
}
