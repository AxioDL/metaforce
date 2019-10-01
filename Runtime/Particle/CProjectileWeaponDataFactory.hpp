#pragma once

#include "Runtime/CFactoryMgr.hpp"
#include "Runtime/CToken.hpp"
#include "Runtime/IOStreams.hpp"
#include "Runtime/IObj.hpp"
#include "Runtime/RetroTypes.hpp"

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
