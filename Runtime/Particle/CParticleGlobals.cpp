#include "CParticleGlobals.hpp"

namespace Retro
{

int CParticleGlobals::g_EmitterTime = 0;
float CParticleGlobals::g_EmitterTimeReal = 0.0;

int CParticleGlobals::g_ParticleLifetime = 0;
float CParticleGlobals::g_ParticleLifetimeReal = 0.0;

int CParticleGlobals::g_ParticleLifetimePercentage = 0;
float CParticleGlobals::g_ParticleLifetimePercentageReal = 0.0;
float CParticleGlobals::g_ParticleLifetimePercentageRemainder = 0.0;

float* CParticleGlobals::g_papValues = nullptr;
CParticleGlobals::SParticleMetrics* CParticleGlobals::g_particleMetrics = nullptr;

}
