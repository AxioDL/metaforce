#ifndef __URDE_CSCRIPEFFECT_HPP__
#define __URDE_CSCRIPEFFECT_HPP__

#include "CActor.hpp"

namespace urde
{
class CElementGen;
class CParticleElectric;
class CScriptEffect : public CActor
{
    static u32 g_NumParticlesUpdating;
    static u32 g_NumParticlesRendered;
    TLockedToken<CElectricDescription> xe8_electricToken;
    std::unique_ptr<CParticleElectric> xf4_electric;
    TLockedToken<CGenDescription> xf8_particleSystemToken;
    std::unique_ptr<CElementGen> x104_particleSystem;
    TUniqueId x108_lightId = kInvalidUniqueId;
    CAssetId x10c_partId;
    bool x110_24_ : 1;
    bool x110_25_ : 1;
    bool x110_26_ : 1;
    bool x110_27_ : 1;
    bool x110_28_ : 1;
    bool x110_29_ : 1;
    bool x110_30_ : 1;
    bool x110_31_ : 1;
    bool x111_24_ : 1;
    bool x111_25_ : 1;
    bool x111_26_canRender : 1;
    float x114_;
    float x118_;
    float x11c_;
    float x120_;
    float x124_;
    float x128_;
    float x12c_;
    float x130_;
    float x134_;
    std::unique_ptr<CActorLights> x138_actorLights;
    TUniqueId x13c_triggerId = kInvalidUniqueId;
    float x140_ = 0.f;
public:
    CScriptEffect(TUniqueId, std::string_view name, const CEntityInfo& info,
                  const zeus::CTransform& xf, const zeus::CVector3f& scale,
                  CAssetId partId, CAssetId elscId, bool, bool, bool, bool active,
                  bool, float, float, float, float, bool, float, float, float,
                  bool, bool, bool, const CLightParameters& lParms, bool);

    void Accept(IVisitor& visitor);
    void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);
    void PreRender(CStateManager&, const zeus::CFrustum&);
    void AddToRenderer(const zeus::CFrustum&, const CStateManager&) const;
    void Render(const CStateManager&) const;
    void Think(float, CStateManager&);
    bool CanRenderUnsorted(const CStateManager&) const { return false; }
    void CalculateRenderBounds();
    zeus::CAABox GetSortingBounds(const CStateManager&) const;
    bool AreBothSystemsDeleteable();
    static void ResetParticleCounts()
    {
        g_NumParticlesUpdating = 0;
        g_NumParticlesRendered = 0;
    }
};

}

#endif // __URDE_CSCRIPEFFECT_HPP__
