#ifndef _DNAMP1_CTWEAKPARTICLE_HPP_
#define _DNAMP1_CTWEAKPARTICLE_HPP_

#include "../../DNACommon/DNACommon.hpp"

namespace Retro
{
namespace DNAMP1
{

struct CTweakParticle : BigYAML
{
    DECL_YAML
    String<-1> m_particle;
    String<-1> m_powerBeam;
    String<-1> m_genThrust;

    CTweakParticle(Athena::io::IStreamReader& reader) { this->read(reader); }
};

}
}

#endif // _DNAMP1_CTWEAKPARTICLE_HPP_
