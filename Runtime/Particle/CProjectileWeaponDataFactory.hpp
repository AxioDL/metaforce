#ifndef __PSHAG_CPROJECTILEWEAPONDATAFACTORY_HPP__
#define __PSHAG_CPROJECTILEWEAPONDATAFACTORY_HPP__

#include "RetroTypes.hpp"
#include "IObj.hpp"
#include "CToken.hpp"
#include "IOStreams.hpp"

namespace pshag
{
class CWeaponDescription;
class CSimplePool;
class CProjectileWeaponDataFactory
{
    static CWeaponDescription* CreateGeneratorDescription(CInputStream& in, CSimplePool* resPool);
    static bool CreateWPSM(CWeaponDescription* desc, CInputStream& in, CSimplePool* resPool);
public:
    static CWeaponDescription* GetGeneratorDesc(CInputStream& in, CSimplePool* resPool);
};

std::unique_ptr<IObj> FProjectileWeaponDataFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& vparms);
}

#endif // __PSHAG_CPROJECTILEWEAPONDATAFACTORY_HPP__
