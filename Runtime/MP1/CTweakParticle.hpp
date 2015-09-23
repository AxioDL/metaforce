#ifndef __RETRO_CTWEAKPARTICLE_HPP__
#define __RETRO_CTWEAKPARTICLE_HPP__

#include "IOStreams.hpp"
#include "DataSpec/DNAMP1/Tweaks/CTweakParticle.hpp"

namespace Retro
{
namespace MP1
{

class CTweakParticle : DNAMP1::CTweakParticle
{
public:
    CTweakParticle(CInputStream& reader);
};

}
}

#endif // __RETRO_CTWEAKPARTICLE_HPP__
