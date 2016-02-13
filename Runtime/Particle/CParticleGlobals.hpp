#ifndef __PSHAG_CPARTICLEGLOBALS_HPP__
#define __PSHAG_CPARTICLEGLOBALS_HPP__

#include "CVector3f.hpp"

namespace pshag
{

class CParticleGlobals
{
public:
    static int g_EmitterTime;
    static float g_EmitterTimeReal;
    static void SetEmitterTime(int frame)
    {
        g_EmitterTime = frame;
        g_EmitterTimeReal = frame;
    }

    static int g_ParticleLifetime;
    static float g_ParticleLifetimeReal;
    static void SetParticleLifetime(int frame)
    {
        g_ParticleLifetime = frame;
        g_ParticleLifetimeReal = frame;
    }

    static int g_ParticleLifetimePercentage;
    static float g_ParticleLifetimePercentageReal;
    static float g_ParticleLifetimePercentageRemainder;
    static void UpdateParticleLifetimeTweenValues(int frame)
    {
        float lt = g_ParticleLifetime != 0.0f ? g_ParticleLifetime : 1.0f;
        g_ParticleLifetimePercentageReal = 100.0f * frame / lt;
        g_ParticleLifetimePercentage = g_ParticleLifetimePercentageReal;
        g_ParticleLifetimePercentageRemainder = g_ParticleLifetimePercentageReal - g_ParticleLifetimePercentage;
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

#endif // __PSHAG_CPARTICLEGLOBALS_HPP__
