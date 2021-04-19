#pragma once

#include <memory>
#include <string_view>

#include "Runtime/rstl.hpp"
#include "Runtime/Character/CAnimTreeNode.hpp"

namespace metaforce {

class CAnimTreeDoubleChild : public CAnimTreeNode {
public:
  class CDoubleChildAdvancementResult {
    CCharAnimTime x0_trueAdvancement;
    SAdvancementDeltas x8_leftDeltas;
    SAdvancementDeltas x24_rightDeltas;

  public:
    CDoubleChildAdvancementResult(const CCharAnimTime& trueAdvancement, const SAdvancementDeltas& leftDeltas,
                                  const SAdvancementDeltas& rightDeltas)
    : x0_trueAdvancement(trueAdvancement), x8_leftDeltas(leftDeltas), x24_rightDeltas(rightDeltas) {}
    const SAdvancementDeltas& GetLeftAdvancementDeltas() const { return x8_leftDeltas; }
    const SAdvancementDeltas& GetRightAdvancementDeltas() const { return x24_rightDeltas; }
    const CCharAnimTime& GetTrueAdvancement() const { return x0_trueAdvancement; }
  };

protected:
  std::shared_ptr<CAnimTreeNode> x14_a;
  std::shared_ptr<CAnimTreeNode> x18_b;

  CDoubleChildAdvancementResult AdvanceViewBothChildren(const CCharAnimTime& time, bool runLeft, bool loopLeft);

public:
  CAnimTreeDoubleChild(const std::weak_ptr<CAnimTreeNode>& a, const std::weak_ptr<CAnimTreeNode>& b,
                       std::string_view name);
  SAdvancementResults VAdvanceView(const CCharAnimTime& a) override;
  size_t VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut, size_t capacity, size_t iterator, u32) const override;
  size_t VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut, size_t capacity, size_t iterator,
                          u32) const override;
  size_t VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut, size_t capacity, size_t iterator,
                             u32) const override;
  size_t VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut, size_t capacity, size_t iterator,
                          u32) const override;
  bool VGetBoolPOIState(std::string_view name) const override;
  s32 VGetInt32POIState(std::string_view name) const override;
  CParticleData::EParentedMode VGetParticlePOIState(std::string_view name) const override;
  void VSetPhase(float) override;
  SAdvancementResults VGetAdvancementResults(const CCharAnimTime& a, const CCharAnimTime& b) const override;
  u32 Depth() const override;
  CAnimTreeEffectiveContribution VGetContributionOfHighestInfluence() const override;
  u32 VGetNumChildren() const override;
  std::shared_ptr<IAnimReader> VGetBestUnblendedChild() const override;
  void VGetWeightedReaders(rstl::reserved_vector<std::pair<float, std::weak_ptr<IAnimReader>>, 16>& out,
                           float w) const override;

  virtual float VGetRightChildWeight() const = 0;
  float GetRightChildWeight() const { return VGetRightChildWeight(); }

  const std::shared_ptr<CAnimTreeNode>& GetLeftChild() const { return x14_a; }
  const std::shared_ptr<CAnimTreeNode>& GetRightChild() const { return x18_b; }
};
} // namespace metaforce
