#pragma once

#include <memory>

#include "Runtime/GCNTypes.hpp"
#include "Runtime/rstl.hpp"
#include "Runtime/Character/CAnimTreeNode.hpp"

namespace metaforce {

class CAnimTreeSingleChild : public CAnimTreeNode {
protected:
  std::shared_ptr<CAnimTreeNode> x14_child;

public:
  CAnimTreeSingleChild(const std::weak_ptr<CAnimTreeNode>& node, std::string_view name);

  SAdvancementResults VAdvanceView(const CCharAnimTime& dt) override;
  CCharAnimTime VGetTimeRemaining() const override;
  bool VHasOffset(const CSegId& seg) const override;
  zeus::CVector3f VGetOffset(const CSegId& seg) const override;
  zeus::CQuaternion VGetRotation(const CSegId& seg) const override;
  size_t VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut, size_t capacity, size_t iterator,
                         u32) const override;
  size_t VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut, size_t capacity, size_t iterator,
                          u32) const override;
  size_t VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut, size_t capacity, size_t iterator,
                             u32) const override;
  size_t VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut, size_t capacity, size_t iterator,
                          u32) const override;
  bool VGetBoolPOIState(std::string_view name) const override;
  s32 VGetInt32POIState(std::string_view name) const override;
  CParticleData::EParentedMode VGetParticlePOIState(std::string_view name) const override;
  void VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut) const override;
  void VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut, const CCharAnimTime& time) const override;
  void VSetPhase(float) override;
  SAdvancementResults VGetAdvancementResults(const CCharAnimTime& a, const CCharAnimTime& b) const override;
  u32 Depth() const override;
  u32 VGetNumChildren() const override;
  void VGetWeightedReaders(rstl::reserved_vector<std::pair<float, std::weak_ptr<IAnimReader>>, 16>& out,
                           float w) const override {
    x14_child->VGetWeightedReaders(out, w);
  }
};

} // namespace metaforce
