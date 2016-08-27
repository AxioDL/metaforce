#include "CAnimTreeSequence.hpp"
#include "IMetaAnim.hpp"
#include "CAnimSysContext.hpp"

namespace urde
{

CAnimTreeSequence::CAnimTreeSequence(const std::vector<std::shared_ptr<IMetaAnim>>& seq,
                                     const CAnimSysContext& animSys,
                                     const std::string& name)
: CAnimTreeSingleChild(seq[0]->GetAnimationTree(animSys, CMetaAnimTreeBuildOrders::NoSpecialOrders()), name),
  x18_(animSys), x3c_fundamentals(CSequenceHelper(seq, animSys).ComputeSequenceFundamentals()), x94_curTime(0.f)
{
}

CAnimTreeSequence::CAnimTreeSequence(const std::shared_ptr<CAnimTreeNode>& curNode,
                                     const std::vector<std::shared_ptr<IMetaAnim>>& metaAnims,
                                     const CAnimSysContext& animSys,
                                     const std::string& name,
                                     const CSequenceFundamentals& fundamentals,
                                     const CCharAnimTime& time)
: CAnimTreeSingleChild(curNode, name), x18_(animSys), x28_(metaAnims), x3c_fundamentals(fundamentals), x94_curTime(time)
{
}

CAnimTreeEffectiveContribution CAnimTreeSequence::VGetContributionOfHighestInfluence() const
{
    return x14_child->GetContributionOfHighestInfluence();
}

std::shared_ptr<IAnimReader> CAnimTreeSequence::VGetBestUnblendedChild() const
{
    std::shared_ptr<IAnimReader> ch = x14_child->GetBestUnblendedChild();
    if (!ch)
        return ch;
    return std::make_shared<CAnimTreeSequence>(std::static_pointer_cast<CAnimTreeNode>(ch->Clone()),
                                               x28_, x18_, x4_name, x3c_fundamentals, x94_curTime);
}

void CAnimTreeSequence::VGetWeightedReaders
(std::vector<std::pair<float, std::weak_ptr<IAnimReader>>>& out, float w) const
{
    x14_child->VGetWeightedReaders(out, w);
}

SAdvancementResults CAnimTreeSequence::VAdvanceView(const CCharAnimTime& a)
{
}

CCharAnimTime CAnimTreeSequence::VGetTimeRemaining() const
{
    if (x38_curIdx == x28_.size() - 1)
        return x14_child->VGetTimeRemaining();
    return x3c_fundamentals.GetSteadyStateAnimInfo().GetDuration() - x94_curTime;
}

CSteadyStateAnimInfo CAnimTreeSequence::VGetSteadyStateAnimInfo() const
{
    return x3c_fundamentals.GetSteadyStateAnimInfo();
}

u32 CAnimTreeSequence::VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut,
                                       u32 capacity, u32 iterator, u32 unk) const
{
    return _getPOIList(time, listOut, capacity, iterator, unk,
                       x3c_fundamentals.GetBoolPointsOfInterest(), x94_curTime);
}

u32 CAnimTreeSequence::VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut,
                                        u32 capacity, u32 iterator, u32 unk) const
{
    return _getPOIList(time, listOut, capacity, iterator, unk,
                       x3c_fundamentals.GetInt32PointsOfInterest(), x94_curTime);
}

u32 CAnimTreeSequence::VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut,
                                           u32 capacity, u32 iterator, u32 unk) const
{
    return _getPOIList(time, listOut, capacity, iterator, unk,
                       x3c_fundamentals.GetParticlePointsOfInterest(), x94_curTime);
}

u32 CAnimTreeSequence::VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut,
                                        u32 capacity, u32 iterator, u32 unk) const
{
    return _getPOIList(time, listOut, capacity, iterator, unk,
                       x3c_fundamentals.GetSoundPointsOfInterest(), x94_curTime);
}

std::shared_ptr<IAnimReader> CAnimTreeSequence::VClone() const
{
    return std::make_shared<CAnimTreeSequence>(std::static_pointer_cast<CAnimTreeNode>(x14_child->Clone()),
                                               x28_, x18_, x4_name, x3c_fundamentals, x94_curTime);
}

}
