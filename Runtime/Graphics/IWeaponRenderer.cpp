#include "Runtime/Graphics/IWeaponRenderer.hpp"

#include "Runtime/Particle/CParticleGen.hpp"

namespace urde {

void CDefaultWeaponRenderer::AddParticleGen(CParticleGen& gen) { gen.Render(); }

} // namespace urde
