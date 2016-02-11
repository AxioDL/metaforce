#ifndef __RETRO_CPARTICLESWOOSH_HPP__
#define __RETRO_CPARTICLESWOOSH_HPP__

#include "CElementGen.hpp"
#include "CToken.hpp"

namespace Retro
{
class CSwooshDescription;

class CParticleSwoosh : public CElementGen
{
public:
    CParticleSwoosh(const TToken<CSwooshDescription>& desc, int);
};

}

#endif // __RETRO_CPARTICLESWOOSH_HPP__
