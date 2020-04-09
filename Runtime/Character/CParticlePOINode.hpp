#pragma once

#include "Runtime/Character/CParticleData.hpp"
#include "Runtime/Character/CPOINode.hpp"

namespace urde {
class IAnimSourceInfo;

class CParticlePOINode : public CPOINode {
  CParticleData x38_data;

public:
  explicit CParticlePOINode();
  explicit CParticlePOINode(CInputStream& in);
  const CParticleData& GetParticleData() const { return x38_data; }

  static CParticlePOINode CopyNodeMinusStartTime(const CParticlePOINode& node, const CCharAnimTime& startTime);
};

} // namespace urde
