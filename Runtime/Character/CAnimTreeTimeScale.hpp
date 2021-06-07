#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Runtime/GCNTypes.hpp"
#include "Runtime/Character/CAnimTreeSingleChild.hpp"
#include "Runtime/Character/CTimeScaleFunctions.hpp"

namespace metaforce {

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

  CAnimTreeEffectiveContribution VGetContributionOfHighestInfluence() const override;
  std::shared_ptr<IAnimReader> VGetBestUnblendedChild() const override;
  std::unique_ptr<IAnimReader> VClone() const override;
  CSteadyStateAnimInfo VGetSteadyStateAnimInfo() const override;
  CCharAnimTime VGetTimeRemaining() const override;
  SAdvancementResults VAdvanceView(const CCharAnimTime& dt) override;
};

} // namespace metaforce
