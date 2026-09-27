#ifndef _CPARTICLEPOINODE
#define _CPARTICLEPOINODE

#include "Kyoto/Animation/CPOINode.hpp"
#include "Kyoto/Particles/CParticleData.hpp"

class CParticlePOINode : public CPOINode {
public:
  CParticlePOINode(rstl::string name = rstl::string_l(""), ushort type = kPT_Particle,
                   const CCharAnimTime& time = CCharAnimTime(), int index = -1, bool unique = false,
                   float weight = 1.f, int charIdx = -1, int flags = 0,
                   const CParticleData& data = CParticleData())
  : CPOINode(name, type, time, index, unique, weight, charIdx, flags), mData(data) {}

  explicit CParticlePOINode(CInputStream& in);

  const CParticleData& GetParticleData() const { return mData; }

  static CParticlePOINode CopyNodeMinusStartTime(const CParticlePOINode& node,
                                                 const CCharAnimTime& startTime);

private:
  CParticleData mData;
};

#endif // _CPARTICLEPOINODE
