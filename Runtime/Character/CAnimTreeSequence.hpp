#pragma once

#include <memory>
#include <vector>

#include "Runtime/GCNTypes.hpp"
#include "Runtime/Character/CAnimSysContext.hpp"
#include "Runtime/Character/CAnimTreeSingleChild.hpp"
#include "Runtime/Character/CSequenceHelper.hpp"

namespace metaforce {
class IMetaAnim;
class CTransitionDatabaseGame;

class CAnimTreeSequence : public CAnimTreeSingleChild {
  CAnimSysContext x18_animCtx;
  std::vector<std::shared_ptr<IMetaAnim>> x28_sequence;
  u32 x38_curIdx = 0;
  CSequenceFundamentals x3c_fundamentals;
  CCharAnimTime x94_curTime;

public:
  CAnimTreeSequence(std::vector<std::shared_ptr<IMetaAnim>> seq, CAnimSysContext animSys, std::string_view name);
  CAnimTreeSequence(const std::shared_ptr<CAnimTreeNode>& curNode, std::vector<std::shared_ptr<IMetaAnim>> metaAnims,
                    CAnimSysContext animSys, std::string_view name, CSequenceFundamentals fundamentals,
                    const CCharAnimTime& time);

  CAnimTreeEffectiveContribution VGetContributionOfHighestInfluence() const override;
  std::shared_ptr<IAnimReader> VGetBestUnblendedChild() const override;
  bool VSupportsReverseView() const { return false; }

  SAdvancementResults VAdvanceView(const CCharAnimTime& dt) override;
  CCharAnimTime VGetTimeRemaining() const override;
  CSteadyStateAnimInfo VGetSteadyStateAnimInfo() const override;
  size_t VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut, size_t capacity, size_t iterator, u32) const override;
  size_t VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut, size_t capacity, size_t iterator,
                          u32) const override;
  size_t VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut, size_t capacity, size_t iterator,
                             u32) const override;
  size_t VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut, size_t capacity, size_t iterator,
                          u32) const override;
  std::unique_ptr<IAnimReader> VClone() const override;
};

} // namespace metaforce
