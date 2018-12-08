#pragma once

#include "CAnimTreeNode.hpp"

namespace urde {

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
  SAdvancementResults VAdvanceView(const CCharAnimTime& a);
  u32 VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut, u32 capacity, u32 iterator, u32) const;
  u32 VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut, u32 capacity, u32 iterator, u32) const;
  u32 VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut, u32 capacity, u32 iterator, u32) const;
  u32 VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut, u32 capacity, u32 iterator, u32) const;
  bool VGetBoolPOIState(const char* name) const;
  s32 VGetInt32POIState(const char* name) const;
  CParticleData::EParentedMode VGetParticlePOIState(const char* name) const;
  void VSetPhase(float);
  SAdvancementResults VGetAdvancementResults(const CCharAnimTime& a, const CCharAnimTime& b) const;
  u32 Depth() const;
  CAnimTreeEffectiveContribution VGetContributionOfHighestInfluence() const;
  u32 VGetNumChildren() const;
  std::shared_ptr<IAnimReader> VGetBestUnblendedChild() const;
  void VGetWeightedReaders(rstl::reserved_vector<std::pair<float, std::weak_ptr<IAnimReader>>, 16>& out, float w) const;

  virtual float VGetRightChildWeight() const = 0;
  float GetRightChildWeight() const { return VGetRightChildWeight(); }

  const std::shared_ptr<CAnimTreeNode>& GetLeftChild() const { return x14_a; }
  const std::shared_ptr<CAnimTreeNode>& GetRightChild() const { return x18_b; }
};
} // namespace urde
