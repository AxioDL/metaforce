#ifndef _CSEQUENCEFUNDAMENTALS
#define _CSEQUENCEFUNDAMENTALS
#include "Kyoto/Animation/CBoolPOINode.hpp"
#include "Kyoto/Animation/CInt32POINode.hpp"
#include "Kyoto/Animation/CParticlePOINode.hpp"
#include "Kyoto/Animation/CSoundPOINode.hpp"
#include "Kyoto/Animation/CSteadyStateAnimInfo.hpp"

class CSequenceFundamentals {
public:
  CSequenceFundamentals(const CSteadyStateAnimInfo& info,
                        const rstl::vector< CBoolPOINode >& boolNodes,
                        const rstl::vector< CInt32POINode >& int32Nodes,
                        const rstl::vector< CParticlePOINode >& particleNodes,
                        const rstl::vector< CSoundPOINode >& soundNodes)
  : mSteadyStateAnimInfo(info)
  , mBoolPOINodes(boolNodes)
  , mInt32POINodes(int32Nodes)
  , mParticlePOINodes(particleNodes)
  , mSoundPOINodes(soundNodes) {}

  const CSteadyStateAnimInfo& GetSteadyStateAnimInfo() const { return mSteadyStateAnimInfo; }
  const rstl::vector< CBoolPOINode >& GetBoolPointsOfInterest() const { return mBoolPOINodes; }
  const rstl::vector< CInt32POINode >& GetInt32PointsOfInterest() const {
    return mInt32POINodes;
  }
  const rstl::vector< CParticlePOINode >& GetParticlePointsOfInterest() const {
    return mParticlePOINodes;
  }
  const rstl::vector< CSoundPOINode >& GetSoundPointsOfInterest() const {
    return mSoundPOINodes;
  }

private:
  CSteadyStateAnimInfo mSteadyStateAnimInfo;
  rstl::vector< CBoolPOINode > mBoolPOINodes;
  rstl::vector< CInt32POINode > mInt32POINodes;
  rstl::vector< CParticlePOINode > mParticlePOINodes;
  rstl::vector< CSoundPOINode > mSoundPOINodes;
};
CHECK_SIZEOF(CSequenceFundamentals, 0x58)
#endif // _CSEQUENCEFUNDAMENTALS
