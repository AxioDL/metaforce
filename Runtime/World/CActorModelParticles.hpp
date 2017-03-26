#ifndef __URDE_CACTORMODELPARTICLES_HPP__
#define __URDE_CACTORMODELPARTICLES_HPP__

#include <list>
#include "Audio/CSfxManager.hpp"
#include "CToken.hpp"
#include "zeus/CTransform.hpp"

namespace urde
{
class CStateManager;
class CEntity;
class CElementGen;
class CTexture;
class CElectricDescription;
class CGenDescription;
class CParticleElectric;

class CActorModelParticles
{
public:
    class CItem
    {
        friend class CActorModelParticles;
        TUniqueId x0_id;
        TAreaId x4_areaId;
        rstl::reserved_vector<std::pair<std::unique_ptr<CElementGen>, u32>, 8> x8_;
        float x6c_ = 0.f;
        CSfxHandle x74_sfx;
        bool x70_ = false;
        std::unique_ptr<CElementGen> x78_;
        u32 x80_ = 0;
        u32 x84_ = -1;
        u32 x88_ = 99;
        rstl::reserved_vector<std::unique_ptr<CElementGen>, 4> x8c_;
        u32 xb0_ = -1;
        u32 xb4_ = 99;
        std::unique_ptr<CElementGen> xb8_;
        std::unique_ptr<CElementGen> xc0_;
        u32 xc8_ = 0;
        u32 xcc_ = 99;
        zeus::CColor xd0_;
        std::unique_ptr<u32> xd4_;
        TToken<CTexture> xdc_ashy;
        std::unique_ptr<CElementGen> xe4_;
        float xec_ = 1.f;
        float xf0_ = 1.f;
        float xf4_ = 1.f;
        zeus::CTransform xf8_;
        CActorModelParticles& x128_parent;
        union
        {
            struct
            {
                bool x12c_24_ : 1;
                bool x12c_25_ : 1;
            };
            u16 _dummy = 0;
        };
        float x130_ = 10.f;
        u8 x134_bits = 0;
    public:
        CItem(const CEntity& ent, CActorModelParticles& parent);
        void GeneratePoints(const zeus::CVector3f* v1, const zeus::CVector3f* v2, int w1);
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
    static void PointGenerator(void* item, const zeus::CVector3f* v1, const zeus::CVector3f* v2, int w1);
    void AddStragglersToRenderer(const CStateManager& mgr);
    void Update(float dt, CStateManager& mgr);
    void SetupHook(TUniqueId uid);
    std::list<CItem>::const_iterator FindSystem(TUniqueId uid) const;
};
}

#endif // __URDE_CACTORMODELPARTICLES_HPP__
