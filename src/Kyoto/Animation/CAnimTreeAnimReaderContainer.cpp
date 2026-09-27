#include "Kyoto/Animation/CAnimTreeAnimReaderContainer.hpp"

CAdvancementResults CAnimTreeAnimReaderContainer::VAdvanceView(const CCharAnimTime& time) {
  return mReader->AdvanceView(time);
}

CCharAnimTime CAnimTreeAnimReaderContainer::VGetTimeRemaining() const {
  return mReader->GetTimeRemaining();
}

CSteadyStateAnimInfo CAnimTreeAnimReaderContainer::VGetSteadyStateAnimInfo() const {
  return mReader->GetSteadyStateAnimInfo();
}

bool CAnimTreeAnimReaderContainer::VHasOffset(const CSegId& seg) const {
  return mReader->HasOffset(seg);
}

CVector3f CAnimTreeAnimReaderContainer::VGetOffset(const CSegId& seg) const {
  return mReader->GetOffset(seg);
}

CQuaternion CAnimTreeAnimReaderContainer::VGetRotation(const CSegId& seg) const {
  return mReader->GetRotation(seg);
}

uint CAnimTreeAnimReaderContainer::VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut,
                                                   uint capacity, uint iterator, int unk) const {
  return mReader->GetBoolPOIList(time, listOut, capacity, iterator, unk);
}

uint CAnimTreeAnimReaderContainer::VGetInt32POIList(const CCharAnimTime& time,
                                                    CInt32POINode* listOut, uint capacity,
                                                    uint iterator, int unk) const {
  return mReader->GetInt32POIList(time, listOut, capacity, iterator, unk);
}

uint CAnimTreeAnimReaderContainer::VGetParticlePOIList(const CCharAnimTime& time,
                                                       CParticlePOINode* listOut, uint capacity,
                                                       uint iterator, int unk) const {
  return mReader->GetParticlePOIList(time, listOut, capacity, iterator, unk);
}

uint CAnimTreeAnimReaderContainer::VGetSoundPOIList(const CCharAnimTime& time,
                                                    CSoundPOINode* listOut, uint capacity,
                                                    uint iterator, int unk) const {
  return mReader->GetSoundPOIList(time, listOut, capacity, iterator, unk);
}

bool CAnimTreeAnimReaderContainer::VGetBoolPOIState(const char* name) const {
  return mReader->GetBoolPOIState(name);
}

s32 CAnimTreeAnimReaderContainer::VGetInt32POIState(const char* name) const {
  return mReader->GetInt32POIState(name);
}

CParticleData::EParentedMode
CAnimTreeAnimReaderContainer::VGetParticlePOIState(const char* name) const {
  return mReader->GetParticlePOIState(name);
}

void CAnimTreeAnimReaderContainer::VGetSegStatementSet(const CSegIdList& list,
                                                       CSegStatementSet& setOut) const {
  mReader->GetSegStatementSet(list, setOut);
}

void CAnimTreeAnimReaderContainer::VGetSegStatementSet(const CSegIdList& list,
                                                       CSegStatementSet& setOut,
                                                       const CCharAnimTime& time) const {
  mReader->GetSegStatementSet(list, setOut, time);
}

rstl::ownership_transfer< IAnimReader > CAnimTreeAnimReaderContainer::VClone() const {
  return rs_new CAnimTreeAnimReaderContainer(mReader->Clone(), mName, mAnimDbIdx);
}

CAnimTreeEffectiveContribution
CAnimTreeAnimReaderContainer::VGetContributionOfHighestInfluence() const {
  return CAnimTreeEffectiveContribution(1.f, mName, mReader->GetSteadyStateAnimInfo(),
                                        mReader->GetTimeRemaining(), mAnimDbIdx);
}

rstl::optional_object< rstl::ownership_transfer< IAnimReader > >
CAnimTreeAnimReaderContainer::VSimplified() {
  return rstl::optional_object_null();
}

void CAnimTreeAnimReaderContainer::VSetPhase(float phase) { mReader->SetPhase(phase); }

CAdvancementResults
CAnimTreeAnimReaderContainer::VGetAdvancementResults(const CCharAnimTime& a,
                                                     const CCharAnimTime& b) const {
  return mReader->GetAdvancementResults(a, b);
}

rstl::rc_ptr< CAnimTreeNode > CAnimTreeAnimReaderContainer::VGetBestUnblendedChild() const {
  return rstl::rc_ptr< CAnimTreeNode >();
}

void CAnimTreeAnimReaderContainer::VGetWeightedReaders(
    float w, rstl::reserved_vector< rstl::pair< float, IAnimReader* >, 16 >& out) const {
  out.push_back(rstl::pair< float, IAnimReader* >(w, const_cast< IAnimReader* >(&*mReader)));
}
