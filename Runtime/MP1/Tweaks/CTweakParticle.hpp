#pragma once

#include "Runtime/Tweaks/ITweakParticle.hpp"

namespace metaforce::MP1 {

struct CTweakParticle final : Tweaks::ITweakParticle {
  std::string m_particle;
  std::string m_powerBeam;
  std::string m_genThrust;

  CTweakParticle() = default;
  CTweakParticle(CInputStream& reader);
};

} // namespace DataSpec::DNAMP1
