#ifndef __URDE_CENVFXMANAGER_HPP__
#define __URDE_CENVFXMANAGER_HPP__

#include "RetroTypes.hpp"
#include "CToken.hpp"
#include "zeus/CAABox.hpp"
#include "Particle/CGenDescription.hpp"

namespace urde
{
class CStateManager;
class CTexture;
class CActor;

enum class EEnvFxType
{
    None,
    Snow,
    Rain
};

enum class EPhazonType
{
    None,
    Blue,
    Orange
};

class CVectorFixed8_8
{
    //u16 x0_[3];
};

class CEnvFxManagerGrid
{
    friend class CEnvFxManager;
    bool x0_24_ = true;
    zeus::CVector2i x4_;
    zeus::CVector2i xc_;
    std::pair<bool, float> x14_ = {false, FLT_MAX};
    std::vector<CVectorFixed8_8> x1c_;
public:
    CEnvFxManagerGrid(const zeus::CVector2i& a, const zeus::CVector2i& b,
                      const std::vector<CVectorFixed8_8>& vec, int reserve)
    : x4_(a), xc_(b), x1c_(vec)
    {
        x1c_.reserve(reserve);
    }
};

class CEnvFxManager
{
    zeus::CAABox x0_ = zeus::CAABox(-63.5, 63.5);
    zeus::CVector3f x18_ = zeus::CVector3f::skZero;
    bool x24_ = false;
    float x28_ = 0.f;
    u32 x2c_ = -1;
    float x30_rainMagnitude = 0.f;
    float x34_fxDensity = 0.f;
    float x38_ = 0.f;
    u8 x3c = 0;

    rstl::reserved_vector<CEnvFxManagerGrid, 64> x50_grids;

    float xb54_;
    TLockedToken<CGenDescription> xb58_envRainSplash;
    bool xb64_ = true;
    TUniqueId xb68_envRainSplashId = kInvalidUniqueId;
    bool xb6a_ = false;
    u32 xb6c_ = 0;
    u32 xb70_ = 0;

    void SetupSnowTevs();
    void SetupRainTevs();
public:
    CEnvFxManager();
    void AsyncLoadResources(CStateManager& mgr);

    void Update(float, const CStateManager&);
    void Render(const CStateManager& mgr);
    void SetFxDensity(s32, float);
    void MoveWrapCells(s32, s32);
    void GetParticleBoundsToWorldScale() const;
    void AreaLoaded();
    void SetXB54(float f) { xb54_ = f; }
    bool GetX24() const { return x24_; }
    float GetRainMagnitude() const { return x30_rainMagnitude; }
    void Cleanup();
};

}

#endif // __URDE_CENVFXMANAGER_HPP__
