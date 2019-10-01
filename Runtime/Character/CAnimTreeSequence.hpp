#pragma once

#include <memory>
#include <vector>

#include "Runtime/GCNTypes.hpp"
#include "Runtime/Character/CAnimSysContext.hpp"
#include "Runtime/Character/CAnimTreeSingleChild.hpp"
#include "Runtime/Character/CSequenceHelper.hpp"

namespace urde {
class IMetaAnim;
class CTransitionDatabaseGame;

class CAnimTreeSequence : public CAnimTreeSingleChild {
  CAnimSysContext x18_animCtx;
  std::vector<std::shared_ptr<IMetaAnim>> x28_sequence;
  u32 x38_curIdx = 0;
  CSequenceFundamentals x3c_fundamentals;
  CCharAnimTime x94_curTime;

public:
  CAnimTreeSequence(const std::vector<std::shared_ptr<IMetaAnim>>& seq, const CAnimSysContext& animSys,
                    std::string_view name);
  CAnimTreeSequence(const std::shared_ptr<CAnimTreeNode>& curNode,
                    const std::vector<std::shared_ptr<IMetaAnim>>& metaAnims, const CAnimSysContext& animSys,
                    std::string_view name, const CSequenceFundamentals& fundamentals, const CCharAnimTime& time);

  CAnimTreeEffectiveContribution VGetContributionOfHighestInfluence() const override;
  std::shared_ptr<IAnimReader> VGetBestUnblendedChild() const override;
  bool VSupportsReverseView() const { return false; }

  SAdvancementResults VAdvanceView(const CCharAnimTime& dt) override;
  CCharAnimTime VGetTimeRemaining() const override;
  CSteadyStateAnimInfo VGetSteadyStateAnimInfo() const override;
  u32 VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut, u32 capacity, u32 iterator, u32) const override;
  u32 VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut, u32 capacity, u32 iterator,
                       u32) const override;
  u32 VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut, u32 capacity, u32 iterator,
                          u32) const override;
  u32 VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut, u32 capacity, u32 iterator,
                       u32) const override;
  std::unique_ptr<IAnimReader> VClone() const override;
};

} // namespace urde
