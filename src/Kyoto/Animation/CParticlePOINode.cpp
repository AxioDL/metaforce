#include "Kyoto/Animation/CParticlePOINode.hpp"

CParticlePOINode::CParticlePOINode(CInputStream& in) : CPOINode(in), mData(in) {}

CParticlePOINode CParticlePOINode::CopyNodeMinusStartTime(const CParticlePOINode& node,
                                                          const CCharAnimTime& startTime) {
  return CParticlePOINode(node.GetString(), node.GetPoiType(), node.GetTime() - startTime,
                          node.GetIndex(), node.GetSaveState(), node.GetWeight(),
                          node.GetCharacterIndex(), node.GetFlags(), node.GetParticleData());
}
