#include "CSequenceHelper.hpp"
#include "CAnimSysContext.hpp"
#include "CBoolPOINode.hpp"
#include "CInt32POINode.hpp"
#include "CParticlePOINode.hpp"
#include "CSoundPOINode.hpp"
#include "IMetaAnim.hpp"
#include "CTreeUtils.hpp"

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
: x0_animCtx(animCtx)
{
    x10_treeNodes.reserve(2);
    x10_treeNodes.push_back(a);
    x10_treeNodes.push_back(b);
}

CSequenceHelper::CSequenceHelper(const std::vector<std::shared_ptr<IMetaAnim>>& nodes,
                                 const CAnimSysContext& animCtx)
: x0_animCtx(animCtx)
{
    x10_treeNodes.reserve(nodes.size());
    for (const std::shared_ptr<IMetaAnim>& meta : nodes)
        x10_treeNodes.push_back(meta->GetAnimationTree(animCtx, CMetaAnimTreeBuildOrders::NoSpecialOrders()));
}

CSequenceFundamentals CSequenceHelper::ComputeSequenceFundamentals()
{
    CCharAnimTime duration;
    zeus::CVector3f offset;
    std::vector<CBoolPOINode> boolNodes;
    std::vector<CInt32POINode> int32Nodes;
    std::vector<CParticlePOINode> particleNodes;
    std::vector<CSoundPOINode> soundNodes;
    if (x10_treeNodes.size() > 0)
    {
        std::shared_ptr<CAnimTreeNode> node = CAnimTreeNode::Cast(x10_treeNodes[0]->Clone());
        for (int i=0 ; i<x10_treeNodes.size() ; ++i)
        {
            CBoolPOINode boolNodeArr[64];
            u32 numBools = node->GetBoolPOIList(CCharAnimTime::Infinity(), boolNodeArr, 64, 0, 0);
            boolNodes.reserve(boolNodes.size() + numBools);
            for (int j=0 ; j<numBools ; ++j)
            {
                CBoolPOINode& n = boolNodeArr[j];
                n.SetTime(n.GetTime() + duration);
                boolNodes.push_back(n);
            }

            CInt32POINode int32NodeArr[64];
            u32 numInt32s = node->GetInt32POIList(CCharAnimTime::Infinity(), int32NodeArr, 64, 0, 0);
            int32Nodes.reserve(int32Nodes.size() + numInt32s);
            for (int j=0 ; j<numInt32s ; ++j)
            {
                CInt32POINode& n = int32NodeArr[j];
                n.SetTime(n.GetTime() + duration);
                int32Nodes.push_back(n);
            }

            CParticlePOINode particleNodeArr[64];
            u32 numParticles = node->GetParticlePOIList(CCharAnimTime::Infinity(), particleNodeArr, 64, 0, 0);
            particleNodes.reserve(particleNodes.size() + numParticles);
            for (int j=0 ; j<numParticles ; ++j)
            {
                CParticlePOINode& n = particleNodeArr[j];
                n.SetTime(n.GetTime() + duration);
                particleNodes.push_back(n);
            }

            CSoundPOINode soundNodeArr[64];
            u32 numSounds = node->GetSoundPOIList(CCharAnimTime::Infinity(), soundNodeArr, 64, 0, 0);
            soundNodes.reserve(soundNodes.size() + numSounds);
            for (int j=0 ; j<numSounds ; ++j)
            {
                CSoundPOINode& n = soundNodeArr[j];
                n.SetTime(n.GetTime() + duration);
                soundNodes.push_back(n);
            }

            duration += node->VGetTimeRemaining();

            CCharAnimTime remTime = node->VGetTimeRemaining();
            while (!remTime.EqualsZero() && !remTime.EpsilonZero())
            {
                SAdvancementResults res = node->VAdvanceView(remTime);
                auto simp = node->Simplified();
                if (simp)
                    node = CAnimTreeNode::Cast(std::move(*simp));
                //CCharAnimTime prevRemTime = remTime;
                remTime = res.x0_remTime;
                /* This was originally accumulating uninitialized register values (stack variable misuse?) */
                offset += res.x8_deltas.x0_posDelta;
            }

            if (i < x10_treeNodes.size()-1)
            {
                node = CTreeUtils::GetTransitionTree(node,
                    CAnimTreeNode::Cast(std::move(x10_treeNodes[i+1]->Clone())), x0_animCtx);
            }
        }
    }

    return {{false, duration, offset}, boolNodes, int32Nodes, particleNodes, soundNodes};
}

}
