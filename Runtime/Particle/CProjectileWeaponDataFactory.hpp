#pragma once

#include "RetroTypes.hpp"
#include "CFactoryMgr.hpp"
#include "IObj.hpp"
#include "CToken.hpp"
#include "IOStreams.hpp"

namespace urde {
class CWeaponDescription;
class CSimplePool;
class CProjectileWeaponDataFactory {
  static CWeaponDescription* CreateGeneratorDescription(CInputStream& in, CSimplePool* resPool);
  static bool CreateWPSM(CWeaponDescription* desc, CInputStream& in, CSimplePool* resPool);

public:
  static CWeaponDescription* GetGeneratorDesc(CInputStream& in, CSimplePool* resPool);
};

CFactoryFnReturn FProjectileWeaponDataFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& vparms,
                                              CObjectReference*);
} // namespace urde
