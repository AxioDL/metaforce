#ifndef __RETRO_CTWEAKS_HPP__
#define __RETRO_CTWEAKS_HPP__

#include "CTweakParticle.hpp"

namespace Retro
{

class CTweaks
{
    TOneStatic<CTweakParticle> m_particle;
public:
    void RegisterTweaks();
    void RegisterResourceTweaks();
};

}

#endif // __RETRO_CTWEAKS_HPP__
