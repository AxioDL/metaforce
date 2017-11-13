#ifndef __URDE_CPARTICLEDATABASE_HPP__
#define __URDE_CPARTICLEDATABASE_HPP__

#include "CCharacterInfo.hpp"
#include "CParticleGenInfo.hpp"
#include "zeus/CFrustum.hpp"
#include "CToken.hpp"
#include "Particle/CGenDescription.hpp"
#include "Particle/CSwooshDescription.hpp"
#include "Particle/CElectricDescription.hpp"
#include <map>

namespace urde
{
class CPoseAsTransforms;
class CCharLayoutInfo;

class CParticleDatabase
{
    std::map<CAssetId, std::shared_ptr<TLockedToken<CGenDescription>>> x0_particleDescs;
    std::map<CAssetId, std::shared_ptr<TLockedToken<CSwooshDescription>>> x14_swooshDescs;
    std::map<CAssetId, std::shared_ptr<TLockedToken<CElectricDescription>>> x28_electricDescs;
    std::map<std::string, std::unique_ptr<CParticleGenInfo>> x3c_rendererDrawLoop;
    std::map<std::string, std::unique_ptr<CParticleGenInfo>> x50_firstDrawLoop;
    std::map<std::string, std::unique_ptr<CParticleGenInfo>> x64_lastDrawLoop;
    std::map<std::string, std::unique_ptr<CParticleGenInfo>> x78_rendererDraw;
    std::map<std::string, std::unique_ptr<CParticleGenInfo>> x8c_firstDraw;
    std::map<std::string, std::unique_ptr<CParticleGenInfo>> xa0_lastDraw;
    bool xb4_24_active : 1;
    bool xb4_25_drawingEnds : 1;

    static void SetModulationColorAllActiveEffectsForParticleDB(const zeus::CColor& color,
        std::map<std::string, std::unique_ptr<CParticleGenInfo>>& map);
    static void SuspendAllActiveEffectsForParticleDB(CStateManager& mgr,
        std::map<std::string, std::unique_ptr<CParticleGenInfo>>& map);
    static void DeleteAllLightsForParticleDB(CStateManager& mgr,
        std::map<std::string, std::unique_ptr<CParticleGenInfo>>& map);
    static void RenderParticleGenMap(const std::map<std::string, std::unique_ptr<CParticleGenInfo>>& map);
    static void RenderParticleGenMapMasked(const std::map<std::string, std::unique_ptr<CParticleGenInfo>>& map,
                                           int mask, int target);
    static void AddToRendererClippedParticleGenMap(const std::map<std::string, std::unique_ptr<CParticleGenInfo>>& map,
                                                   const zeus::CFrustum& frustum);
    static void AddToRendererClippedParticleGenMapMasked(const std::map<std::string, std::unique_ptr<CParticleGenInfo>>& map,
                                                         const zeus::CFrustum& frustum, int mask, int target);
    static void UpdateParticleGenDB(float dt, const CPoseAsTransforms& pose, const CCharLayoutInfo& charInfo,
                                    const zeus::CTransform& xf, const zeus::CVector3f& vec, CStateManager& stateMgr,
                                    std::map<std::string, std::unique_ptr<CParticleGenInfo>>& map, bool deleteIfDone);
public:
    CParticleDatabase();
    void CacheParticleDesc(const CCharacterInfo::CParticleResData& desc);
    void SetModulationColorAllActiveEffects(const zeus::CColor& color);
    void SuspendAllActiveEffects(CStateManager& stateMgr);
    void DeleteAllLights(CStateManager& stateMgr);
    void Update(float dt, const CPoseAsTransforms& pose, const CCharLayoutInfo& charInfo, const zeus::CTransform& xf,
                const zeus::CVector3f& scale, CStateManager& stateMgr);
    void RenderSystemsToBeDrawnLastMasked(int mask, int target) const;
    void RenderSystemsToBeDrawnLast() const;
    void RenderSystemsToBeDrawnFirstMasked(int mask, int target) const;
    void RenderSystemsToBeDrawnFirst() const;
    void AddToRendererClippedMasked(const zeus::CFrustum& frustum, int mask, int target) const;
    void AddToRendererClipped(const zeus::CFrustum& frustum) const;
    CParticleGenInfo* GetParticleEffect(std::string_view name) const;
    void SetParticleEffectState(std::string_view name, bool active, CStateManager& mgr);
    void SetCEXTValue(std::string_view name, int idx, float value);
    void AddAuxiliaryParticleEffect(std::string_view name, int flags, const CAuxiliaryParticleData& data,
                                    const zeus::CVector3f& scale, CStateManager& mgr, TAreaId aid, int lightId);
    void AddParticleEffect(std::string_view name, int flags, const CParticleData& data,
                           const zeus::CVector3f& scale, CStateManager& mgr, TAreaId aid, bool oneShot, int lightId);
    void InsertParticleGen(bool oneShot, int flags, std::string_view name,
                           std::unique_ptr<CParticleGenInfo>&& gen);
};
}

#endif // __URDE_CPARTICLEDATABASE_HPP__
