#pragma once

#include <list>
#include "Audio/CSfxManager.hpp"
#include "CToken.hpp"
#include "zeus/CTransform.hpp"
#include "Particle/CParticleElectric.hpp"
#include "Particle/CParticleSwoosh.hpp"
#include "Graphics/CRainSplashGenerator.hpp"

namespace urde
{
class CStateManager;
class CEntity;
class CElementGen;
class CTexture;
class CGenDescription;
class CActor;
class CScriptPlayerActor;

class CActorModelParticles
{
public:
    enum class EDependency
    {
        OnFire,
        Ash,
        IceBreak,
        FirePop,
        IcePop,
        Electric
    };

    class CItem
    {
        friend class CActorModelParticles;
        TUniqueId x0_id;
        TAreaId x4_areaId;
        rstl::reserved_vector<std::pair<std::unique_ptr<CElementGen>, u32>, 8> x8_thermalHotParticles;
        float x6c_onFireDelayTimer = 0.f;
        bool x70_onFire = false;
        CSfxHandle x74_sfx;
        std::unique_ptr<CElementGen> x78_;
        u32 x80_ = 0;
        s32 x84_ = -1;
        u32 x88_seed1 = 99;
        rstl::reserved_vector<std::unique_ptr<CElementGen>, 4> x8c_thermalColdParticles;
        s32 xb0_ = -1;
        u32 xb4_seed2 = 99;
        std::unique_ptr<CElementGen> xb8_;
        std::unique_ptr<CParticleElectric> xc0_particleElectric;
        u32 xc8_ = 0;
        u32 xcc_seed3 = 99;
        zeus::CColor xd0_;
        std::unique_ptr<CRainSplashGenerator> xd4_rainSplashGenerator;
        TToken<CTexture> xdc_ashy;
        std::unique_ptr<CElementGen> xe4_;
        zeus::CVector3f xec_ = zeus::CVector3f::skOne;
        zeus::CTransform xf8_;
        CActorModelParticles& x128_parent;
        union
        {
            struct
            {
                bool x12c_24_thermalCold : 1;
                bool x12c_25_thermalHot : 1;
            };
            u16 _dummy = 0;
        };
        float x130_ = 10.f;
        u8 x134_lockDeps = 0;
        void sub_801e59a8(EDependency i);
        bool UpdateOnFire(float dt, CActor* actor, CStateManager& mgr);
        bool UpdateAsh(float dt, CActor* actor, CStateManager& mgr);
        bool sub_801e65bc(float dt, CActor* actor, CStateManager& mgr);
        bool UpdateFirePop(float dt, CActor* actor, CStateManager& mgr);
        bool UpdateElectric(float dt, CActor* actor, CStateManager& mgr);
        bool sub_801e69f0(float dt, CActor* actor, CStateManager& mgr);
        bool sub_801e5e98(float dt, CActor* actor, CStateManager& mgr);
        bool UpdateIcePop(float dt, CActor* actor, CStateManager& mgr);
    public:
        CItem(const CEntity& ent, CActorModelParticles& parent);
        void GeneratePoints(const std::vector<std::pair<zeus::CVector3f, zeus::CVector3f>>& vn);
        bool Update(float dt, CStateManager& mgr);
        void Lock(EDependency i);
        void Unlock(EDependency i);
    };

private:
    friend class CItem;
    std::list<CItem> x0_items;
    TToken<CGenDescription> x18_onFire;
    TToken<CGenDescription> x20_ash;
    TToken<CGenDescription> x28_iceBreak;
    TToken<CGenDescription> x30_firePop;
    TToken<CGenDescription> x38_icePop;
    TToken<CElectricDescription> x40_electric;
    TToken<CTexture> x48_ashy;
    struct Dependency
    {
        std::vector<CToken> x0_tokens;
        int x10_refCount = 0;
        bool x14_loaded = false;
        void Increment()
        {
            ++x10_refCount;
            if (x10_refCount == 1)
                Load();
        }
        void Decrement()
        {
            --x10_refCount;
            if (x10_refCount <= 0)
                Unload();
        }
        void Load()
        {
            bool loading = false;
            for (CToken& tok : x0_tokens)
            {
                tok.Lock();
                if (!tok.IsLoaded())
                    loading = true;
            }
            if (!loading)
                x14_loaded = true;
        }
        void Unload()
        {
            for (CToken& tok : x0_tokens)
                tok.Unlock();
            x14_loaded = false;
        }
    };
    rstl::reserved_vector<Dependency, 6> x50_dgrps;
    u8 xe4_bits = 0;
    u8 xe5_bits1 = 0;
    u8 xe6_bits2 = 0;

    Dependency GetParticleDGRPTokens(const char* name);
    void LoadParticleDGRPs();

    std::unique_ptr<CElementGen> MakeOnFireGen() const;
    std::unique_ptr<CElementGen> MakeAshGen() const;
    std::unique_ptr<CElementGen> MakeIceGen() const;
    std::unique_ptr<CElementGen> MakeFirePopGen() const;
    std::unique_ptr<CElementGen> MakeIcePopGen() const;
    std::unique_ptr<CParticleElectric> MakeElectricGen() const;

    void DecrementDependency(EDependency d);
    void IncrementDependency(EDependency d);
public:
    CActorModelParticles();
    static void PointGenerator(void* item, const std::vector<std::pair<zeus::CVector3f, zeus::CVector3f>>& vn);
    void AddStragglersToRenderer(const CStateManager& mgr);
    void Update(float dt, CStateManager& mgr);
    void SetupHook(TUniqueId uid);
    std::list<CItem>::const_iterator FindSystem(TUniqueId uid) const;
    std::list<CItem>::iterator FindOrCreateSystem(CActor& act);
    void StartIce(CActor& actor);
    void AddRainSplashGenerator(CActor& act, CStateManager& mgr, u32 maxSplashes,
                                u32 genRate, float minZ);
    void RemoveRainSplashGenerator(CActor& act);
    void Render(const CActor& actor) const;
    void StartElectric(CActor& act);
    void StopElectric(CActor& act);
    void sub_801e51d0(CActor& act);
    void StopThermalHotParticles(CActor& act);
    void StartBurnDeath(CActor& act);
    void EnsureElectricLoaded(CActor& act);
    void EnsureFirePopLoaded(CActor& act);
    void EnsureIceBreakLoaded(CActor& act);
    void LightDudeOnFire(CActor& act);
    const CTexture* GetAshyTexture(const CActor& act);
};
}

