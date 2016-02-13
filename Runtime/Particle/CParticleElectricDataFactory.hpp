#ifndef __RETRO_CPARTICLEELECTRICDATAFACTORY_HPP__
#define __RETRO_CPARTICLEELECTRICDATAFACTORY_HPP__

#include "RetroTypes.hpp"
#include "IObj.hpp"
#include "CToken.hpp"
#include "IOStreams.hpp"

namespace Retro
{
class CElectricDescription;
class CSimplePool;
class CParticleElectricDataFactory
{
    static CElectricDescription* CreateElectricDescription(CInputStream& in, CSimplePool* resPool);
    static bool CreateELSM(CElectricDescription* desc, CInputStream& in, CSimplePool* resPool);
    static bool LoadELSMTokens(CElectricDescription* desc);
public:
    static CElectricDescription* GetGeneratorDesc(CInputStream& in, CSimplePool* resPool);
};

std::unique_ptr<IObj> FParticleElectricDataFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& vparms);
}

#endif // __RETRO_CPARTICLEELECTRICDATAFACTORY_HPP__
