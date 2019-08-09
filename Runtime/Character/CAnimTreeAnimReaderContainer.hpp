#pragma once

#include "CAnimTreeNode.hpp"

namespace urde {

class CAnimTreeAnimReaderContainer : public CAnimTreeNode {
  std::shared_ptr<IAnimReader> x14_reader;
  u32 x1c_animDbIdx;

public:
  CAnimTreeAnimReaderContainer(std::string_view name, std::shared_ptr<IAnimReader> reader, u32 animDbIdx);

  u32 Depth() const override;
  CAnimTreeEffectiveContribution VGetContributionOfHighestInfluence() const override;
  u32 VGetNumChildren() const override;
  std::shared_ptr<IAnimReader> VGetBestUnblendedChild() const override;
  void VGetWeightedReaders(rstl::reserved_vector<std::pair<float, std::weak_ptr<IAnimReader>>, 16>& out,
                           float w) const override;

  SAdvancementResults VAdvanceView(const CCharAnimTime& a) override;
  CCharAnimTime VGetTimeRemaining() const override;
  CSteadyStateAnimInfo VGetSteadyStateAnimInfo() const override;
  bool VHasOffset(const CSegId& seg) const override;
  zeus::CVector3f VGetOffset(const CSegId& seg) const override;
  zeus::CQuaternion VGetRotation(const CSegId& seg) const override;
  u32 VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut, u32 capacity, u32 iterator, u32) const override;
  u32 VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut, u32 capacity, u32 iterator,
                       u32) const override;
  u32 VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut, u32 capacity, u32 iterator,
                          u32) const override;
  u32 VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut, u32 capacity, u32 iterator,
                       u32) const override;
  bool VGetBoolPOIState(const char*) const override;
  s32 VGetInt32POIState(const char*) const override;
  CParticleData::EParentedMode VGetParticlePOIState(const char*) const override;
  void VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut) const override;
  void VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut, const CCharAnimTime& time) const override;
  std::unique_ptr<IAnimReader> VClone() const override;
  std::optional<std::unique_ptr<IAnimReader>> VSimplified() override;
  void VSetPhase(float) override;
  SAdvancementResults VGetAdvancementResults(const CCharAnimTime& a, const CCharAnimTime& b) const override;
};

} // namespace urde
