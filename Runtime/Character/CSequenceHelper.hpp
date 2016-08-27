#ifndef __URDE_CSEQUENCEHELPER_HPP__
#define __URDE_CSEQUENCEHELPER_HPP__

#include "CAnimTreeNode.hpp"
#include "CBoolPOINode.hpp"
#include "CInt32POINode.hpp"
#include "CParticlePOINode.hpp"
#include "CSoundPOINode.hpp"

namespace urde
{
class CAnimSysContext;
class IMetaAnim;
class CTransitionDatabaseGame;

class CSequenceFundamentals
{
    CSteadyStateAnimInfo x0_ssInfo;
    std::vector<CBoolPOINode> x18_boolNodes;
    std::vector<CInt32POINode> x28_int32Nodes;
    std::vector<CParticlePOINode> x38_particleNodes;
    std::vector<CSoundPOINode> x48_soundNodes;
public:
    CSequenceFundamentals(const CSteadyStateAnimInfo& ssInfo,
                          const std::vector<CBoolPOINode>& boolNodes,
                          const std::vector<CInt32POINode>& int32Nodes,
                          const std::vector<CParticlePOINode>& particleNodes,
                          const std::vector<CSoundPOINode>& soundNodes);

    const CSteadyStateAnimInfo& GetSteadyStateAnimInfo() const { return x0_ssInfo; }
    const std::vector<CBoolPOINode>& GetBoolPointsOfInterest() const { return x18_boolNodes; }
    const std::vector<CInt32POINode>& GetInt32PointsOfInterest() const { return x28_int32Nodes; }
    const std::vector<CParticlePOINode>& GetParticlePointsOfInterest() const { return x38_particleNodes; }
    const std::vector<CSoundPOINode>& GetSoundPointsOfInterest() const { return x48_soundNodes; }
};

class CSequenceHelper
{
    TLockedToken<CTransitionDatabaseGame> x0_transDB;
    std::vector<std::shared_ptr<CAnimTreeNode>> x10_treeNodes;
    std::vector<bool> x20_;
public:
    CSequenceHelper(const std::shared_ptr<CAnimTreeNode>& a,
                    const std::shared_ptr<CAnimTreeNode>& b,
                    const CAnimSysContext& animCtx);
    CSequenceHelper(const std::vector<std::shared_ptr<IMetaAnim>>& nodes,
                    const CAnimSysContext& animCtx);
    CSequenceFundamentals ComputeSequenceFundamentals();
};

}

#endif // __URDE_CSEQUENCEHELPER_HPP__
