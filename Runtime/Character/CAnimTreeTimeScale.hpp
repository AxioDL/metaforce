#pragma once

#include "CAnimTreeSingleChild.hpp"
#include "CTimeScaleFunctions.hpp"

namespace urde {

class CAnimTreeTimeScale : public CAnimTreeSingleChild {
  std::unique_ptr<IVaryingAnimationTimeScale> x18_timeScale;
  CCharAnimTime x20_curAccelTime;
  CCharAnimTime x28_targetAccelTime;
  CCharAnimTime x30_initialTime;

public:
  CAnimTreeTimeScale(const std::weak_ptr<CAnimTreeNode>& node, float timeScale, std::string_view name);
  CAnimTreeTimeScale(const std::weak_ptr<CAnimTreeNode>& node, std::unique_ptr<IVaryingAnimationTimeScale>&& timeScale,
                     const CCharAnimTime& time, std::string_view name);

  static std::string CreatePrimitiveName(const std::weak_ptr<CAnimTreeNode>&, float, const CCharAnimTime&, float);

  CCharAnimTime GetRealLifeTime(const CCharAnimTime&) const;
  void VSetPhase(float) override;
  std::optional<std::unique_ptr<IAnimReader>> VSimplified() override;

  u32 VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut, u32 capacity, u32 iterator, u32) const override;
  u32 VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut, u32 capacity, u32 iterator,
                       u32) const override;
  u32 VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut, u32 capacity, u32 iterator,
                          u32) const override;
  u32 VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut, u32 capacity, u32 iterator,
                       u32) const override;
  bool VGetBoolPOIState(const char* name) const override;
  s32 VGetInt32POIState(const char* name) const override;
  CParticleData::EParentedMode VGetParticlePOIState(const char* name) const override;

  CAnimTreeEffectiveContribution VGetContributionOfHighestInfluence() const override;
  std::shared_ptr<IAnimReader> VGetBestUnblendedChild() const override;
  std::unique_ptr<IAnimReader> VClone() const override;
  CSteadyStateAnimInfo VGetSteadyStateAnimInfo() const override;
  CCharAnimTime VGetTimeRemaining() const override;
  SAdvancementResults VAdvanceView(const CCharAnimTime& dt) override;
};

} // namespace urde
