#include "CSequenceHelper.hpp"
#include "CAnimSysContext.hpp"
#include "CBoolPOINode.hpp"
#include "CInt32POINode.hpp"
#include "CParticlePOINode.hpp"
#include "CSoundPOINode.hpp"
#include "IMetaAnim.hpp"

namespace urde
{

CSequenceFundamentals::CSequenceFundamentals(const CSteadyStateAnimInfo& ssInfo,
                                             const std::vector<CBoolPOINode>& boolNodes,
                                             const std::vector<CInt32POINode>& int32Nodes,
                                             const std::vector<CParticlePOINode>& particleNodes,
                                             const std::vector<CSoundPOINode>& soundNodes)
: x0_ssInfo(ssInfo), x18_boolNodes(boolNodes), x28_int32Nodes(int32Nodes), x38_particleNodes(particleNodes),
  x48_soundNodes(soundNodes)
{
}

CSequenceHelper::CSequenceHelper(const std::shared_ptr<CAnimTreeNode>& a,
                                 const std::shared_ptr<CAnimTreeNode>& b,
                                 const CAnimSysContext& animCtx)
: x0_transDB(animCtx.x0_transDB)
{
    x10_treeNodes.reserve(2);
    x10_treeNodes.push_back(a);
    x10_treeNodes.push_back(b);
}

CSequenceHelper::CSequenceHelper(const std::vector<std::shared_ptr<IMetaAnim>>& nodes,
                                 const CAnimSysContext& animCtx)
: x0_transDB(animCtx.x0_transDB)
{
    x10_treeNodes.reserve(nodes.size());
    for (const std::shared_ptr<IMetaAnim>& meta : nodes)
        x10_treeNodes.push_back(meta->GetAnimationTree(animCtx, CMetaAnimTreeBuildOrders::NoSpecialOrders()));
}

CSequenceFundamentals CSequenceHelper::ComputeSequenceFundamentals()
{
}

}
