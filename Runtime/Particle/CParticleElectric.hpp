#ifndef __RETRO_CPARTICLEELECTRIC_HPP__
#define __RETRO_CPARTICLEELECTRIC_HPP__

#include "CElementGen.hpp"

namespace Retro
{
class CElectricDescription;

class CParticleElectric : public CElementGen
{
public:
    CParticleElectric(const TToken<CElectricDescription>& desc);
};

}

#endif // __RETRO_CPARTICLEELECTRIC_HPP__
