#ifndef __URDE_CPROJECTILEWEAPON_HPP__
#define __URDE_CPROJECTILEWEAPON_HPP__

#include "RetroTypes.hpp"
#include "CRandom16.hpp"
#include "CToken.hpp"
#include "zeus/CVector3f.hpp"
#include "Particle/CElementGen.hpp"
#include "Particle/CParticleSwoosh.hpp"
#include "Particle/CGenDescription.hpp"
#include "Particle/CSwooshDescription.hpp"
#include "Particle/CWeaponDescription.hpp"

namespace urde
{
class CModel;
class CProjectileWeapon
{
    static u16 g_GlobalSeed;
    TLockedToken<CWeaponDescription> x4_weaponDesc;
    CRandom16 x10_random;
    zeus::CTransform x14_localToWorldXf;
    zeus::CTransform x44_localXf;
    zeus::CVector3f x74_ = zeus::CVector3f::skZero;
    zeus::CVector3f x80_ = zeus::CVector3f::skZero;
    zeus::CVector3f x8c_ = zeus::CVector3f::skZero;
    zeus::CVector3f x98_ = zeus::CVector3f::skOne;
    zeus::CVector3f xa4_ = zeus::CVector3f::skZero;
    zeus::CVector3f xb0_ = zeus::CVector3f::skZero;
    zeus::CVector3f xbc_ = zeus::CVector3f::skZero;
    zeus::CColor xc8_ = zeus::CColor::skWhite;
    double xd0_ = 0.0;
    double xd8_ = 0.0;
    float xe0_maxTurnRate;
    u32 xe4_;
    u32 xe8_lifetime = 0x7FFFFF;
    u32 xec_ = 0;
    u32 xf0_ = 0;
    u32 xf4_ = 0;
    u32 xf8_ = 0;
    u32 xfc_ = 0;
    std::unique_ptr<CElementGen> x104_particle1;
    std::unique_ptr<CElementGen> x104_particle2;
    TToken<CModel> x108_model;
    bool x114_hasModel = false;
    std::unique_ptr<CParticleSwoosh> x118_swoosh1;
    std::unique_ptr<CParticleSwoosh> x11c_swoosh2;
    std::unique_ptr<CParticleSwoosh> x120_swoosh3;
    union
    {
        struct
        {
            bool x124_24_active : 1;
            bool x124_25_ : 1;
            bool x124_26_ap11 : 1;
            bool x124_27_ap21 : 1;
            bool x124_28_as11 : 1;
            bool x124_29_as12 : 1;
            bool x124_30_as13 : 1;
        };
        u32 _dummy = 0;
    };

public:
    CProjectileWeapon(const TToken<CWeaponDescription>& wDesc, const zeus::CVector3f& pos,
                      const zeus::CTransform& orient, const zeus::CVector3f& scale, s32);
    virtual ~CProjectileWeapon() = default;
    bool IsProjectileActive() const { return x124_24_active; }
    virtual zeus::CTransform GetTransform() const;
    virtual zeus::CVector3f GetTranslation() const;
    float GetMaxTurnRate() const { return xe0_maxTurnRate; }
    TLockedToken<CWeaponDescription> GetWeaponDescription() const { return x4_weaponDesc; }
    virtual void RenderParticles() const;
    virtual void Update(float);
    void UpdateParticleFx();
    void UpdateChildParticleSystems(float);
    void SetWorldSpaceOrientation(const zeus::CTransform& xf);
    void SetRelativeOrientation(const zeus::CTransform& xf) { x44_localXf = xf; }
    static void SetGlobalSeed(u16 seed) { g_GlobalSeed = seed; }
};
}

#endif // __URDE_CPROJECTILEWEAPON_HPP__
