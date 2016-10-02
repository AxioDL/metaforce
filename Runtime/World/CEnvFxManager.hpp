#ifndef __URDE_CENVFXMANAGER_HPP__
#define __URDE_CENVFXMANAGER_HPP__

#include "RetroTypes.hpp"
#include "CToken.hpp"
#include "zeus/CAABox.hpp"

namespace urde
{
class CStateManager;
class CTexture;

enum class EEnvFxType
{
    None,
    Rain,
    Snow
};

enum class EPhazonType
{
    None,
    Blue,
    Orange
};

class CEnvFxManager
{
    zeus::CAABox x0_ = zeus::CAABox(-63.5, 63.5);
    zeus::CVector3f x18_ = zeus::CVector3f::skZero;
    u8 x24_ = 0;
    float x28_ = 0.f;
    u32 x2c_ = -1;
    float x30_ = 0.f;
    float x34_fxDensity = 0.f;
    float x38_ = 0.f;
    u8 x3c = 0;

    void SetupSnowTevs();
    void SetupRainTevs();
public:
    CEnvFxManager();
    void AsyncLoadResources(CStateManager& mgr);

    void Update(float, float, EEnvFxType, const CStateManager&);
    void SetFxDensity(s32, float);
    void MoveWrapCells(s32, s32);
    void GetParticleBoundsToWorldScale() const;

};

}

#endif // __URDE_CENVFXMANAGER_HPP__
