#pragma once

#include "../../DNACommon/Tweaks/ITweakParticle.hpp"

namespace DataSpec::DNAMP1 {

struct CTweakParticle final : ITweakParticle {
  AT_DECL_DNA_YAML
  String<-1> m_particle;
  String<-1> m_powerBeam;
  String<-1> m_genThrust;

  CTweakParticle() = default;
  CTweakParticle(athena::io::IStreamReader& reader) { this->read(reader); }
};

} // namespace DataSpec::DNAMP1
