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
};

}

#endif // __URDE_CPARTICLEDATABASE_HPP__
