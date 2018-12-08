#include "CAnimTreeAnimReaderContainer.hpp"
#include "CFBStreamedAnimReader.hpp"

namespace urde {

CAnimTreeAnimReaderContainer::CAnimTreeAnimReaderContainer(std::string_view name, std::shared_ptr<IAnimReader> reader,
                                                           u32 dbIdx)
: CAnimTreeNode(name), x14_reader(reader), x1c_animDbIdx(dbIdx) {}

u32 CAnimTreeAnimReaderContainer::Depth() const { return 1; }

CAnimTreeEffectiveContribution CAnimTreeAnimReaderContainer::VGetContributionOfHighestInfluence() const {
  return {1.f, x4_name, VGetSteadyStateAnimInfo(), VGetTimeRemaining(), x1c_animDbIdx};
}

u32 CAnimTreeAnimReaderContainer::VGetNumChildren() const { return 0; }

std::shared_ptr<IAnimReader> CAnimTreeAnimReaderContainer::VGetBestUnblendedChild() const { return {}; }

SAdvancementResults CAnimTreeAnimReaderContainer::VAdvanceView(const CCharAnimTime& dt) {
  return x14_reader->VAdvanceView(dt);
}

CCharAnimTime CAnimTreeAnimReaderContainer::VGetTimeRemaining() const { return x14_reader->VGetTimeRemaining(); }

CSteadyStateAnimInfo CAnimTreeAnimReaderContainer::VGetSteadyStateAnimInfo() const {
  return x14_reader->VGetSteadyStateAnimInfo();
}

bool CAnimTreeAnimReaderContainer::VHasOffset(const CSegId& seg) const { return x14_reader->VHasOffset(seg); }

zeus::CVector3f CAnimTreeAnimReaderContainer::VGetOffset(const CSegId& seg) const {
  return x14_reader->VGetOffset(seg);
}

zeus::CQuaternion CAnimTreeAnimReaderContainer::VGetRotation(const CSegId& seg) const {
  return x14_reader->VGetRotation(seg);
}

u32 CAnimTreeAnimReaderContainer::VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut, u32 capacity,
                                                  u32 iterator, u32 unk) const {
  return x14_reader->GetBoolPOIList(time, listOut, capacity, iterator, unk);
}

u32 CAnimTreeAnimReaderContainer::VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut, u32 capacity,
                                                   u32 iterator, u32 unk) const {
  return x14_reader->GetInt32POIList(time, listOut, capacity, iterator, unk);
}

u32 CAnimTreeAnimReaderContainer::VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut,
                                                      u32 capacity, u32 iterator, u32 unk) const {
  return x14_reader->GetParticlePOIList(time, listOut, capacity, iterator, unk);
}

u32 CAnimTreeAnimReaderContainer::VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut, u32 capacity,
                                                   u32 iterator, u32 unk) const {
  return x14_reader->GetSoundPOIList(time, listOut, capacity, iterator, unk);
}

bool CAnimTreeAnimReaderContainer::VGetBoolPOIState(const char* name) const {
  return x14_reader->VGetBoolPOIState(name);
}

s32 CAnimTreeAnimReaderContainer::VGetInt32POIState(const char* name) const {
  return x14_reader->VGetBoolPOIState(name);
}

CParticleData::EParentedMode CAnimTreeAnimReaderContainer::VGetParticlePOIState(const char* name) const {
  return x14_reader->VGetParticlePOIState(name);
}

void CAnimTreeAnimReaderContainer::VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut) const {
  return x14_reader->VGetSegStatementSet(list, setOut);
}

void CAnimTreeAnimReaderContainer::VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut,
                                                       const CCharAnimTime& time) const {
  return x14_reader->VGetSegStatementSet(list, setOut, time);
}

std::unique_ptr<IAnimReader> CAnimTreeAnimReaderContainer::VClone() const {
  return std::make_unique<CAnimTreeAnimReaderContainer>(x4_name, x14_reader->Clone(), x1c_animDbIdx);
}

std::experimental::optional<std::unique_ptr<IAnimReader>> CAnimTreeAnimReaderContainer::VSimplified() { return {}; }

void CAnimTreeAnimReaderContainer::VSetPhase(float ph) { x14_reader->VSetPhase(ph); }

SAdvancementResults CAnimTreeAnimReaderContainer::VGetAdvancementResults(const CCharAnimTime& a,
                                                                         const CCharAnimTime& b) const {
  return x14_reader->VGetAdvancementResults(a, b);
}

void CAnimTreeAnimReaderContainer::VGetWeightedReaders(
    rstl::reserved_vector<std::pair<float, std::weak_ptr<IAnimReader>>, 16>& out, float w) const {
  out.emplace_back(std::make_pair(w, x14_reader));
}

} // namespace urde
