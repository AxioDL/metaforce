#ifndef __PSHAG_CDECALDATAFACTORY_HPP__
#define __PSHAG_CDECALDATAFACTORY_HPP__

#include "RetroTypes.hpp"
#include "IObj.hpp"
#include "CToken.hpp"
#include "IOStreams.hpp"


namespace pshag
{
class CDecalDescription;
class CSimplePool;

class CDecalDataFactory
{
public:
    static CDecalDescription* GetGeneratorDesc(CInputStream& in,CSimplePool* resPool);
    static CDecalDescription* CreateGeneratorDescription(CInputStream& in, CSimplePool* resPool);
    static bool CreateDPSM(CDecalDescription* desc,CInputStream& in,CSimplePool* resPool);
};

std::unique_ptr<IObj> FDecalDataFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& vparms);
}
#endif // __PSHAG_CDECALDATAFACTORY_HPP__
