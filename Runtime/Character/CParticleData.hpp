#ifndef __PSHAG_CPARTICLEDATA_HPP__
#define __PSHAG_CPARTICLEDATA_HPP__

#include "IOStreams.hpp"
#include "RetroTypes.hpp"

namespace urde
{

enum class EParentedMode
{
    Initial,
    ContinuousEmitter,
    ContinuousSystem
};

class CParticleData
{
    u32 x0_duration;
    SObjectTag x4_particle;
    std::string xc_boneName;
    float x1c_scale;
    EParentedMode x20_parentMode;
public:
    CParticleData(CInputStream& in);
};

}

#endif // __PSHAG_CPARTICLEDATA_HPP__
