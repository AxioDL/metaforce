#include "CParticleGlobals.hpp"

namespace Retro
{

int CParticleGlobals::g_emitterTimeInt = 0;
float CParticleGlobals::g_emitterTimeFloat = 0.0;

int CParticleGlobals::g_particleLifetimeInt = 0;
float CParticleGlobals::g_particleLifetimeFloat = 0.0;

int CParticleGlobals::g_particleLifetimePercentTweenInt = 0;
float CParticleGlobals::g_particleLifetimePercentTweenFloat = 0.0;
float CParticleGlobals::g_particleLifetimePercentTweenIntFloatRem = 0.0;

float* CParticleGlobals::g_papValues = nullptr;
CParticleGlobals::SParticleMetrics* CParticleGlobals::g_particleMetrics = nullptr;

}
