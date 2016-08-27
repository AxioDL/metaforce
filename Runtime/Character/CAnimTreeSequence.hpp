#ifndef __URDE_CANIMTREESEQUENCE_HPP__
#define __URDE_CANIMTREESEQUENCE_HPP__

#include "CAnimTreeSingleChild.hpp"
#include "CAnimSysContext.hpp"
#include "CSequenceHelper.hpp"

namespace urde
{
class IMetaAnim;
class CTransitionDatabaseGame;

class CAnimTreeSequence : public CAnimTreeSingleChild
{
    CAnimSysContext x18_;
    std::vector<std::shared_ptr<IMetaAnim>> x28_;
    u32 x38_curIdx = 0;
    CSequenceFundamentals x3c_fundamentals;
    CCharAnimTime x94_curTime;
public:
    CAnimTreeSequence(const std::vector<std::shared_ptr<IMetaAnim>>& seq,
                      const CAnimSysContext& animSys,
                      const std::string& name);
    CAnimTreeSequence(const std::shared_ptr<CAnimTreeNode>& curNode,
                      const std::vector<std::shared_ptr<IMetaAnim>>& metaAnims,
                      const CAnimSysContext& animSys,
                      const std::string& name,
                      const CSequenceFundamentals& fundamentals,
                      const CCharAnimTime& time);

    CAnimTreeEffectiveContribution VGetContributionOfHighestInfluence() const;
    std::shared_ptr<IAnimReader> VGetBestUnblendedChild() const;
    void VGetWeightedReaders(std::vector<std::pair<float, std::weak_ptr<IAnimReader>>>& out, float w) const;

    SAdvancementResults VAdvanceView(const CCharAnimTime& a);
    CCharAnimTime VGetTimeRemaining() const;
    CSteadyStateAnimInfo VGetSteadyStateAnimInfo() const;
    u32 VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut, u32 capacity, u32 iterator, u32) const;
    u32 VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut, u32 capacity, u32 iterator, u32) const;
    u32 VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut, u32 capacity, u32 iterator, u32) const;
    u32 VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut, u32 capacity, u32 iterator, u32) const;
    std::shared_ptr<IAnimReader> VClone() const;
};

}

#endif // __URDE_CANIMTREESEQUENCE_HPP__
