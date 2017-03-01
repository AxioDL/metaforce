#ifndef __URDE_CPARTICLEDATABASE_HPP__
#define __URDE_CPARTICLEDATABASE_HPP__

#include "CCharacterInfo.hpp"
#include "CParticleGenInfo.hpp"
#include "zeus/CFrustum.hpp"
#include "CToken.hpp"
#include <map>

namespace urde
{
class CPoseAsTransforms;
class CCharLayoutInfo;
class CGenDescription;
class CSwooshDescription;
class CElectricDescription;

class CParticleDatabase
{
    std::map<ResId, std::shared_ptr<TLockedToken<CGenDescription>>> x0_particleDescs;
    std::map<ResId, std::shared_ptr<TLockedToken<CSwooshDescription>>> x14_swooshDescs;
    std::map<ResId, std::shared_ptr<TLockedToken<CElectricDescription>>> x28_electricDescs;
    std::map<std::string, std::unique_ptr<CParticleGenInfo>> x3c_;
    std::map<std::string, std::unique_ptr<CParticleGenInfo>> x50_;
    std::map<std::string, std::unique_ptr<CParticleGenInfo>> x64_;
    std::map<std::string, std::unique_ptr<CParticleGenInfo>> x78_;
    std::map<std::string, std::unique_ptr<CParticleGenInfo>> x8c_;
    std::map<std::string, std::unique_ptr<CParticleGenInfo>> xa0_;

public:
    void CacheParticleDesc(const CCharacterInfo::CParticleResData& desc);
    void SetModulationColorAllActiveEffects(const zeus::CColor& color);
    void SuspendAllActiveEffects(CStateManager& stateMgr);
    void StartEffect(const std::string& name, u32 flags, const CParticleData& data, const zeus::CVector3f& scale,
                     CStateManager& stateMgr, TAreaId aid, u32 unk1);
    void Update(float dt, const CPoseAsTransforms& pose, const CCharLayoutInfo& charInfo, const zeus::CTransform& xf,
                const zeus::CVector3f& vec, CStateManager& stateMgr);
    void AddToRendererClipped(const zeus::CFrustum& frustum);
    void GetActiveParticleLightIds(std::vector<TUniqueId>&);
    void GetActiveParticleLightIdsFromParticleDB(
        std::vector<TUniqueId>&,
        const std::map<std::string, std::unique_ptr<CParticleGenInfo>, std::less<std::string>>&);
};
}

#endif // __URDE_CPARTICLEDATABASE_HPP__
