#ifndef __RETRO_CPARTICLESWOOSHDATAFACTORY_HPP__
#define __RETRO_CPARTICLESWOOSHDATAFACTORY_HPP__

#include "RetroTypes.hpp"
#include "IObj.hpp"
#include "CToken.hpp"
#include "IOStreams.hpp"

namespace Retro
{
class CSwooshDescription;
class CSimplePool;
class CParticleSwooshDataFactory
{
public:
    static CSwooshDescription* GetGeneratorDesc(CInputStream& in, CSimplePool* resPool);
    static CSwooshDescription* CreateGeneratorDescription(CInputStream& in, CSimplePool* resPool);
    static bool CreateWPSM(CSwooshDescription* desc, CInputStream& in, CSimplePool* resPool);
};

std::unique_ptr<IObj> FParticleSwooshDataFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& vparms);
}

#endif // __RETRO_CPARTICLESWOOSHDATAFACTORY_HPP__
