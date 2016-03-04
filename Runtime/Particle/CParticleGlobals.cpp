#include "CParticleGlobals.hpp"

namespace urde
{

int CParticleGlobals::g_EmitterTime = 0;
float CParticleGlobals::g_EmitterTimeReal = 0.0;

int CParticleGlobals::g_ParticleLifetime = 0;
float CParticleGlobals::g_ParticleLifetimeReal = 0.0;

int CParticleGlobals::g_ParticleLifetimePercentage = 0;
float CParticleGlobals::g_ParticleLifetimePercentageReal = 0.0;
float CParticleGlobals::g_ParticleLifetimePercentageRemainder = 0.0;

float CParticleGlobals::g_papValues[8] = { 0.f };
CParticleGlobals::SParticleSystem* CParticleGlobals::g_currentParticleSystem = nullptr;
}
