#ifndef __RETRO_CPARTICLEGLOBALS_HPP__
#define __RETRO_CPARTICLEGLOBALS_HPP__

#include "CVector3f.hpp"

namespace Retro
{

class CParticleGlobals
{
public:
    static int g_emitterTimeInt;
    static float g_emitterTimeFloat;
    static void SetEmitterTime(int frame)
    {
        g_emitterTimeInt = frame;
        g_emitterTimeFloat = frame;
    }

    static int g_particleLifetimeInt;
    static float g_particleLifetimeFloat;
    static void SetParticleLifetime(int frame)
    {
        g_particleLifetimeInt = frame;
        g_particleLifetimeFloat = frame;
    }

    static int g_particleLifetimePercentTweenInt;
    static float g_particleLifetimePercentTweenFloat;
    static float g_particleLifetimePercentTweenIntFloatRem;
    static void UpdateParticleLifetimeTweenValues(int frame)
    {
        float lt = g_particleLifetimeInt != 0.0f ? g_particleLifetimeInt : 1.0f;
        g_particleLifetimePercentTweenFloat = 100.0f * frame / lt;
        g_particleLifetimePercentTweenInt = g_particleLifetimePercentTweenFloat;
        g_particleLifetimePercentTweenIntFloatRem = g_particleLifetimePercentTweenFloat - g_particleLifetimePercentTweenInt;
    }

    static float* g_papValues;

    struct SParticleMetrics
    {
        Zeus::CVector3f x0_ploc;
        Zeus::CVector3f x10_plco;
        Zeus::CVector3f x1c_pvel;
        float x2c_psll;
        float x30_prlw;
    };
    static SParticleMetrics* g_particleMetrics;
};

}

#endif // __RETRO_CPARTICLEGLOBALS_HPP__
