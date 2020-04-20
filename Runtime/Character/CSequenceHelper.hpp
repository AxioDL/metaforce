#pragma once

#include <memory>
#include <vector>

#include "Runtime/Character/CAnimSysContext.hpp"
#include "Runtime/Character/CAnimTreeNode.hpp"
#include "Runtime/Character/CBoolPOINode.hpp"
#include "Runtime/Character/CInt32POINode.hpp"
#include "Runtime/Character/CParticlePOINode.hpp"
#include "Runtime/Character/CSoundPOINode.hpp"
#include "Runtime/Character/CTransitionDatabaseGame.hpp"

namespace urde {
class IMetaAnim;

class CSequenceFundamentals {
  CSteadyStateAnimInfo x0_ssInfo;
  std::vector<CBoolPOINode> x18_boolNodes;
  std::vector<CInt32POINode> x28_int32Nodes;
  std::vector<CParticlePOINode> x38_particleNodes;
  std::vector<CSoundPOINode> x48_soundNodes;

public:
  CSequenceFundamentals(const CSteadyStateAnimInfo& ssInfo, std::vector<CBoolPOINode> boolNodes,
                        std::vector<CInt32POINode> int32Nodes, std::vector<CParticlePOINode> particleNodes,
                        std::vector<CSoundPOINode> soundNodes);

  const CSteadyStateAnimInfo& GetSteadyStateAnimInfo() const { return x0_ssInfo; }
  const std::vector<CBoolPOINode>& GetBoolPointsOfInterest() const { return x18_boolNodes; }
  const std::vector<CInt32POINode>& GetInt32PointsOfInterest() const { return x28_int32Nodes; }
  const std::vector<CParticlePOINode>& GetParticlePointsOfInterest() const { return x38_particleNodes; }
  const std::vector<CSoundPOINode>& GetSoundPointsOfInterest() const { return x48_soundNodes; }
};

class CSequenceHelper {
  CAnimSysContext x0_animCtx;
  std::vector<std::shared_ptr<CAnimTreeNode>> x10_treeNodes;
  std::vector<bool> x20_;

public:
  CSequenceHelper(const std::shared_ptr<CAnimTreeNode>& a, const std::shared_ptr<CAnimTreeNode>& b,
                  CAnimSysContext animCtx);
  CSequenceHelper(const std::vector<std::shared_ptr<IMetaAnim>>& nodes, CAnimSysContext animCtx);
  CSequenceFundamentals ComputeSequenceFundamentals();
};

} // namespace urde
