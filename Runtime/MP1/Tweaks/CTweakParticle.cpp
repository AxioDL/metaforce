#include "Runtime/MP1/Tweaks/CTweakParticle.hpp"
#include "Runtime/Streams/IOStreams.hpp"

namespace metaforce::MP1 {
CTweakParticle::CTweakParticle(CInputStream& in) {
  m_particle = in.Get<std::string>();
  m_powerBeam = in.Get<std::string>();
  m_genThrust = in.Get<std::string>();
}
} // namespace metaforce::MP1