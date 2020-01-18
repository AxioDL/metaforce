#include "Runtime/Graphics/IWeaponRenderer.hpp"

#include "Runtime/Particle/CParticleGen.hpp"

namespace urde {

void CDefaultWeaponRenderer::AddParticleGen(const CParticleGen& gen) { ((CParticleGen&)gen).Render(); }

} // namespace urde
