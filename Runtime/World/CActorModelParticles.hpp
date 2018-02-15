#ifndef __URDE_CACTORMODELPARTICLES_HPP__
#define __URDE_CACTORMODELPARTICLES_HPP__

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

class CActorModelParticles
{
public:
    class CItem
    {
        friend class CActorModelParticles;
        TUniqueId x0_id;
        TAreaId x4_areaId;
        rstl::reserved_vector<std::pair<std::unique_ptr<CElementGen>, u32>, 8> x8_thermalHotParticles;
        float x6c_ = 0.f;
        bool x70_ = false;
        CSfxHandle x74_sfx;
        std::unique_ptr<CElementGen> x78_;
        u32 x80_ = 0;
        u32 x84_ = -1;
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
        u8 x134_bits = 0;
    public:
        CItem(const CEntity& ent, CActorModelParticles& parent);
        void GeneratePoints(const std::vector<std::pair<zeus::CVector3f, zeus::CVector3f>>& vn);
        void Update(float, CStateManager&);
    };

private:
    std::list<CItem> x0_items;
    TToken<CGenDescription> x18_onFire;
    TToken<CGenDescription> x20_ash;
    TToken<CGenDescription> x28_iceBreak;
    TToken<CGenDescription> x30_firePop;
    TToken<CGenDescription> x38_icePop;
    TToken<CElectricDescription> x40_electric;
    TToken<CTexture> x48_ashy;
    rstl::reserved_vector<std::pair<std::vector<CToken>, bool>, 6> x50_dgrps;

    std::pair<std::vector<CToken>, bool> GetParticleDGRPTokens(const char* name);
    void LoadParticleDGRPs();

    std::unique_ptr<CElementGen> MakeOnFireGen() const;
    std::unique_ptr<CElementGen> MakeAshGen() const;
    std::unique_ptr<CElementGen> MakeIceGen() const;
    std::unique_ptr<CElementGen> MakeFirePopGen() const;
    std::unique_ptr<CElementGen> MakeIcePopGen() const;
    std::unique_ptr<CParticleElectric> MakeElectricGen() const;

public:
    CActorModelParticles();
    static void PointGenerator(void* item, const std::vector<std::pair<zeus::CVector3f, zeus::CVector3f>>& vn);
    void AddStragglersToRenderer(const CStateManager& mgr);
    void Update(float dt, CStateManager& mgr);
    void SetupHook(TUniqueId uid);
    std::list<CItem>::const_iterator FindSystem(TUniqueId uid) const;
    void StartIce(CActor& actor, CStateManager& mgr);
    void Render(const CActor& actor) const;
};
}

#endif // __URDE_CACTORMODELPARTICLES_HPP__
