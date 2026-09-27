#include "Kyoto/Animation/CAnimTreeSingleChild.hpp"
#include "Kyoto/Math/CQuaternion.hpp"

CAdvancementResults CAnimTreeSingleChild::VAdvanceView(const CCharAnimTime& dt) {
  return mChild->VAdvanceView(dt);
}

CCharAnimTime CAnimTreeSingleChild::VGetTimeRemaining() const {
  return mChild->VGetTimeRemaining();
}

bool CAnimTreeSingleChild::VHasOffset(const CSegId& seg) const {
  return mChild->VHasOffset(seg);
}

CVector3f CAnimTreeSingleChild::VGetOffset(const CSegId& seg) const {
  return mChild->VGetOffset(seg);
}

CQuaternion CAnimTreeSingleChild::VGetRotation(const CSegId& seg) const {
  return mChild->VGetRotation(seg);
}

void CAnimTreeSingleChild::VGetSegStatementSet(const CSegIdList& list,
                                               CSegStatementSet& setOut) const {
  return mChild->VGetSegStatementSet(list, setOut);
}

void CAnimTreeSingleChild::VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut,
                                               const CCharAnimTime& time) const {
  return mChild->VGetSegStatementSet(list, setOut, time);
}

uint CAnimTreeSingleChild::VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut,
                                           uint capacity, uint iterator, int unk) const {
  return mChild->GetBoolPOIList(time, listOut, capacity, iterator, unk);
}

uint CAnimTreeSingleChild::VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut,
                                            uint capacity, uint iterator, int unk) const {
  return mChild->GetInt32POIList(time, listOut, capacity, iterator, unk);
}

uint CAnimTreeSingleChild::VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut,
                                               uint capacity, uint iterator, int unk) const {
  return mChild->GetParticlePOIList(time, listOut, capacity, iterator, unk);
}

uint CAnimTreeSingleChild::VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut,
                                            uint capacity, uint iterator, int unk) const {
  return mChild->GetSoundPOIList(time, listOut, capacity, iterator, unk);
}

bool CAnimTreeSingleChild::VGetBoolPOIState(const char* name) const {
  return mChild->VGetBoolPOIState(name);
}

s32 CAnimTreeSingleChild::VGetInt32POIState(const char* name) const {
  return mChild->VGetInt32POIState(name);
}

CParticleData::EParentedMode CAnimTreeSingleChild::VGetParticlePOIState(const char* name) const {
  return mChild->VGetParticlePOIState(name);
}

uint CAnimTreeSingleChild::VGetNumChildren() const { return mChild->VGetNumChildren() + 1; }

void CAnimTreeSingleChild::VSetPhase(float phase) { mChild->VSetPhase(phase); }

CAdvancementResults CAnimTreeSingleChild::VGetAdvancementResults(const CCharAnimTime& a,
                                                                 const CCharAnimTime& b) const {
  return mChild->VGetAdvancementResults(a, b);
}

void CAnimTreeSingleChild::VGetWeightedReaders(
    float w, rstl::reserved_vector< rstl::pair< float, IAnimReader* >, 16 >& out) const {
  return mChild->VGetWeightedReaders(w, out);
}
