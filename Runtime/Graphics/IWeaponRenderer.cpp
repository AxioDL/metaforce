#include "IWeaponRenderer.hpp"
#include "Particle/CParticleGen.hpp"

namespace urde
{

void CDefaultWeaponRenderer::AddParticleGen(const CParticleGen& gen)
{
    ((CParticleGen&)gen).Render();
}

}
