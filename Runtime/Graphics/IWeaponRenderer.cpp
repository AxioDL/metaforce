#include "Runtime/Graphics/IWeaponRenderer.hpp"

#include "Runtime/Particle/CParticleGen.hpp"

namespace metaforce {

void CDefaultWeaponRenderer::AddParticleGen(CParticleGen& gen) { gen.Render(); }

} // namespace metaforce
