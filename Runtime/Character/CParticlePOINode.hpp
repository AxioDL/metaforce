#ifndef __PSHAG_CPARTICLEPOINODE_HPP__
#define __PSHAG_CPARTICLEPOINODE_HPP__

#include "CPOINode.hpp"
#include "CParticleData.hpp"

namespace urde
{

class CParticlePOINode : public CPOINode
{
    CParticleData x38_data;
public:
    CParticlePOINode(CInputStream& in);
};

}

#endif // __PSHAG_CPARTICLEPOINODE_HPP__
