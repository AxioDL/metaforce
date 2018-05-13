#ifndef __URDE_CPARTICLEPOINODE_HPP__
#define __URDE_CPARTICLEPOINODE_HPP__

#include "CPOINode.hpp"
#include "CParticleData.hpp"

namespace urde
{
class IAnimSourceInfo;

class CParticlePOINode : public CPOINode
{
    CParticleData x38_data;
public:
    CParticlePOINode();
    CParticlePOINode(CInputStream& in);
    const CParticleData& GetParticleData() const { return x38_data; }

    static CParticlePOINode CopyNodeMinusStartTime(const CParticlePOINode& node,
                                                   const CCharAnimTime& startTime);
};

}

#endif // __URDE_CPARTICLEPOINODE_HPP__
