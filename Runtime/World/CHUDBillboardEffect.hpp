#pragma once

#include "RetroTypes.hpp"
#include "World/CEffect.hpp"
#include "CToken.hpp"
#include "Particle/CElementGen.hpp"
#include "Particle/CParticleElectric.hpp"
#include "Particle/CParticleSwoosh.hpp"

namespace urde
{
class CGenDescription;
class CElectricDescription;

class CHUDBillboardEffect : public CEffect
{
    std::unique_ptr<CParticleGen> xe8_generator;
    zeus::CVector3f xec_v2;
    zeus::CVector3f xf8_;
    bool x104_24_ : 1;
    bool x104_25_ : 1;
    bool x104_26_ : 1;
    bool x104_27_ : 1;
    float x108_ = 0.f;
    static u32 g_IndirectTexturedBillboardCount;
    static u32 g_BillboardCount;
public:
    CHUDBillboardEffect(const std::experimental::optional<TToken<CGenDescription>>& particle,
                        const std::experimental::optional<TToken<CElectricDescription>>& electric,
                        TUniqueId uid, bool active, std::string_view name, float, const zeus::CVector3f& v0,
                        const zeus::CColor& color, const zeus::CVector3f& v1, const zeus::CVector3f& v2);
    ~CHUDBillboardEffect();
    void Accept(IVisitor& visitor);
    bool GetX104_26() const { return x104_26_; }
    void SetX104_27(bool b) { x104_27_ = b; }
    CParticleGen* GetParticleGen() const { return xe8_generator.get(); }

    static float GetNearClipDistance(CStateManager& mgr);
    static zeus::CVector3f GetScaleForPOV(CStateManager& mgr);
};

}

