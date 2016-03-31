#ifndef __PSHAG_CPARTICLESWOOSHDATAFACTORY_HPP__
#define __PSHAG_CPARTICLESWOOSHDATAFACTORY_HPP__

#include "RetroTypes.hpp"
#include "CFactoryMgr.hpp"
#include "IObj.hpp"
#include "CToken.hpp"
#include "IOStreams.hpp"

namespace urde
{
class CSwooshDescription;
class CSimplePool;
class CParticleSwooshDataFactory
{
    static CSwooshDescription* CreateGeneratorDescription(CInputStream& in, CSimplePool* resPool);
    static bool CreateWPSM(CSwooshDescription* desc, CInputStream& in, CSimplePool* resPool);
public:
    static CSwooshDescription* GetGeneratorDesc(CInputStream& in, CSimplePool* resPool);
};

CFactoryFnReturn FParticleSwooshDataFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& vparms);
}

#endif // __PSHAG_CPARTICLESWOOSHDATAFACTORY_HPP__
