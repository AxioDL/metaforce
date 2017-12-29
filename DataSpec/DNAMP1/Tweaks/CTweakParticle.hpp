#ifndef _DNAMP1_CTWEAKPARTICLE_HPP_
#define _DNAMP1_CTWEAKPARTICLE_HPP_

#include "../../DNACommon/Tweaks/ITweakParticle.hpp"

namespace DataSpec::DNAMP1
{

struct CTweakParticle final : ITweakParticle
{
    DECL_YAML
    String<-1> m_particle;
    String<-1> m_powerBeam;
    String<-1> m_genThrust;

    CTweakParticle()=default;
    CTweakParticle(athena::io::IStreamReader& reader) { this->read(reader); }
};

}

#endif // _DNAMP1_CTWEAKPARTICLE_HPP_
