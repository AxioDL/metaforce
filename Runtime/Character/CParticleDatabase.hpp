#ifndef __URDE_CPARTICLEDATABASE_HPP__
#define __URDE_CPARTICLEDATABASE_HPP__

#include "CCharacterInfo.hpp"
#include "CParticleGenInfo.hpp"
#include <map>

namespace urde
{

class CParticleDatabase
{
    std::map<std::string, std::unique_ptr<CParticleGenInfo>> x3c_;
public:
    void CacheParticleDesc(const CCharacterInfo::CParticleResData& desc);
    void SetModulationColorAllActiveEffects(const zeus::CColor& color);
    void SuspendAllActiveEffects(CStateManager& stateMgr);
    void StartEffect(const std::string& name, u32 flags, const CParticleData& data,
                     const zeus::CVector3f& scale, CStateManager& stateMgr, TAreaId aid, u32 unk1);
};

}

#endif // __URDE_CPARTICLEDATABASE_HPP__
