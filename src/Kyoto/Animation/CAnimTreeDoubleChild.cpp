#include "Kyoto/Animation/CAnimTreeDoubleChild.hpp"
#include "Kyoto/Animation/CAnimTreeNode.hpp"
#include "Kyoto/Animation/CBoolPOINode.hpp"
#include "Kyoto/Animation/CCharAnimMemoryMetrics.hpp"
#include "Kyoto/Animation/CInt32POINode.hpp"
#include "Kyoto/Animation/CPOINode.hpp"
#include "Kyoto/Animation/CParticlePOINode.hpp"
#include "Kyoto/Animation/CSoundPOINode.hpp"
#include "Kyoto/Animation/IAnimReader.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include <stdlib.h>

CAnimTreeDoubleChild::CAnimTreeDoubleChild(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                           const rstl::ncrc_ptr< CAnimTreeNode >& b,
                                           const rstl::string& name)
: CAnimTreeNode(name), mA(a), mB(b) {
  CCharAnimMemoryMetrics::AddToTotalSize(8, CCharAnimMemoryMetrics::kASS_Two);
}

CAdvancementResults CAnimTreeDoubleChild::VAdvanceView(const CCharAnimTime& dt) {
  CAdvancementResults resA = mA->AdvanceView(dt);
  CAdvancementResults resB = mB->AdvanceView(dt);
  return resA.GetRemainder() > resB.GetRemainder() ? resA : resB;
}

CAnimTreeDoubleChild::~CAnimTreeDoubleChild() {
  CCharAnimMemoryMetrics::SubtractFromTotalSize(8, CCharAnimMemoryMetrics::kASS_Two);
}

uint CAnimTreeDoubleChild::VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut,
                                           uint capacity, uint iterator, int unk) const {
  int x = mA->GetBoolPOIList(time, listOut, capacity, iterator, unk);
  x += mB->GetBoolPOIList(time, listOut, capacity, x + iterator, unk);
  if (x > capacity)
    x = capacity;
  qsort(listOut, x, sizeof(CBoolPOINode), CPOINode::compare);
  return x;
}

uint CAnimTreeDoubleChild::VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut,
                                            uint capacity, uint iterator, int unk) const {
  int x = mA->GetInt32POIList(time, listOut, capacity, iterator, unk);
  x += mB->GetInt32POIList(time, listOut, capacity, x + iterator, unk);
  if (x > capacity)
    x = capacity;
  qsort(listOut, x, sizeof(CInt32POINode), CPOINode::compare);
  return x;
}

uint CAnimTreeDoubleChild::VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut,
                                               uint capacity, uint iterator, int unk) const {
  int x = mA->GetParticlePOIList(time, listOut, capacity, iterator, unk);
  x += mB->GetParticlePOIList(time, listOut, capacity, x + iterator, unk);
  if (x > capacity)
    x = capacity;
  qsort(listOut, x, sizeof(CParticlePOINode), CPOINode::compare);
  return x;
}

uint CAnimTreeDoubleChild::VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut,
                                            uint capacity, uint iterator, int unk) const {
  int x = mA->GetSoundPOIList(time, listOut, capacity, iterator, unk);
  x += mB->GetSoundPOIList(time, listOut, capacity, x + iterator, unk);
  if (x > capacity)
    x = capacity;
  qsort(listOut, x, sizeof(CSoundPOINode), CPOINode::compare);
  return x;
}

bool CAnimTreeDoubleChild::VGetBoolPOIState(const char* name) const {
  return mB->VGetBoolPOIState(name);
}

s32 CAnimTreeDoubleChild::VGetInt32POIState(const char* name) const {
  return mB->VGetInt32POIState(name);
}

CParticleData::EParentedMode CAnimTreeDoubleChild::VGetParticlePOIState(const char* name) const {
  return mB->VGetParticlePOIState(name);
}

CAnimTreeEffectiveContribution CAnimTreeDoubleChild::VGetContributionOfHighestInfluence() const {
  CAnimTreeEffectiveContribution a = mA->GetContributionOfHighestInfluence();
  CAnimTreeEffectiveContribution b = mB->GetContributionOfHighestInfluence();
  float leftWeight = a.GetContributionWeight() * GetLeftChildWeight();
  float rightWeight = b.GetContributionWeight() * GetRightChildWeight();
  return leftWeight > rightWeight
             ? CAnimTreeEffectiveContribution(leftWeight, a.GetPrimitiveName(),
                                              a.GetSteadyStateAnimInfo(), a.GetTimeRemaining(),
                                              a.GetAnimDatabaseIndex())
             : CAnimTreeEffectiveContribution(rightWeight, b.GetPrimitiveName(),
                                              b.GetSteadyStateAnimInfo(), b.GetTimeRemaining(),
                                              b.GetAnimDatabaseIndex());
}

uint CAnimTreeDoubleChild::VGetNumChildren() const {
  int num_children = mB->VGetNumChildren();
  num_children += mA->VGetNumChildren() + 2;
  return num_children;
}

CAnimTreeDoubleChild::CDoubleChildAdvancementResult::CDoubleChildAdvancementResult(
    const CCharAnimTime& trueAdvancement, const CAdvancementDeltas& leftDeltas,
    const CAdvancementDeltas& rightDeltas)
: mTrueAdvancement(trueAdvancement), mLeftDeltas(leftDeltas), mRightDeltas(rightDeltas) {}

CAnimTreeDoubleChild::CDoubleChildAdvancementResult
CAnimTreeDoubleChild::AdvanceViewBothChildren(const CCharAnimTime& time, bool runLeft,
                                              bool loopLeft) {
  CCharAnimTime leftRemaining = time;
  CCharAnimTime totalTime = !runLeft   ? CCharAnimTime::ZeroFlat()
                            : loopLeft ? CCharAnimTime::Infinity()
                                       : mA->GetTimeRemaining();
  CVector3f leftOffset(0.f, 0.f, 0.f);
  CQuaternion leftRotation = CQuaternion::NoRotation();
  CCharAnimTime rightRemaining = time;
  CVector3f rightOffset(0.f, 0.f, 0.f);
  CQuaternion rightRotation = CQuaternion::NoRotation();
  if (time.GreaterThanZero()) {
    while (leftRemaining.GreaterThanZero() && !close_enough(leftRemaining.GetSeconds(), 0.f) &&
           totalTime.GreaterThanZero() &&
           (loopLeft || !close_enough(totalTime.GetSeconds(), 0.f))) {
      CAdvancementResults result = mA->AdvanceView(leftRemaining);
      rstl::optional_object< rstl::ownership_transfer< IAnimReader > > simplified =
          mA->Simplified();
      if (simplified.valid())
        mA = Cast(*simplified);
      CAdvancementDeltas deltas = result.GetAdvancementDeltas();
      leftOffset += deltas.GetOffsetDelta();
      CQuaternion rotation = deltas.GetOrientationDelta();
      leftRotation *= rotation;
      if (!loopLeft)
        totalTime = mA->GetTimeRemaining();
      leftRemaining = result.GetRemainder();
    }
    while (rightRemaining.GreaterThanZero() && !close_enough(rightRemaining.GetSeconds(), 0.f)) {
      CAdvancementResults result = mB->AdvanceView(rightRemaining);
      rstl::optional_object< rstl::ownership_transfer< IAnimReader > > simplified =
          mB->Simplified();
      if (simplified.valid())
        mB = Cast(*simplified);
      CAdvancementDeltas deltas = result.GetAdvancementDeltas();
      rightOffset += deltas.GetOffsetDelta();
      CQuaternion rotation = deltas.GetOrientationDelta();
      rightRotation *= rotation;
      rightRemaining = result.GetRemainder();
    }
  }
  return CDoubleChildAdvancementResult(time, CAdvancementDeltas(leftOffset, leftRotation),
                                       CAdvancementDeltas(rightOffset, rightRotation));
}

void CAnimTreeDoubleChild::VSetPhase(float phase) {
  mA->VSetPhase(phase);
  mB->VSetPhase(phase);
}

CAdvancementResults CAnimTreeDoubleChild::VGetAdvancementResults(const CCharAnimTime& a,
                                                                 const CCharAnimTime& b) const {
  CAdvancementResults resA = mA->GetAdvancementResults(a, b);
  CAdvancementResults resB = mB->GetAdvancementResults(a, b);
  return resA.GetRemainder() > resB.GetRemainder() ? resA : resB;
}

rstl::rc_ptr< CAnimTreeNode > CAnimTreeDoubleChild::VGetBestUnblendedChild() const {
  rstl::rc_ptr< CAnimTreeNode > child = GetRightChildWeight() > 0.5f ? mB : mA;
  if (!child)
    return child;
  rstl::rc_ptr< CAnimTreeNode > best = child->GetBestUnblendedChild();
  if (!best)
    return child;
  return best;
}

void CAnimTreeDoubleChild::VGetWeightedReaders(
    float w, rstl::reserved_vector< rstl::pair< float, IAnimReader* >, 16 >& out) const {
  mA->VGetWeightedReaders(w, out);
  mB->VGetWeightedReaders(w, out);
}
