#pragma once

#include <memory>

#include "Runtime/CFactoryMgr.hpp"
#include "Runtime/CToken.hpp"
#include "Runtime/IOStreams.hpp"
#include "Runtime/IObj.hpp"
#include "Runtime/RetroTypes.hpp"

namespace metaforce {
class CSimplePool;
class CWeaponDescription;

class CProjectileWeaponDataFactory {
  static std::unique_ptr<CWeaponDescription> CreateGeneratorDescription(CInputStream& in, CSimplePool* resPool);
  static bool CreateWPSM(CWeaponDescription* desc, CInputStream& in, CSimplePool* resPool);

public:
  static std::unique_ptr<CWeaponDescription> GetGeneratorDesc(CInputStream& in, CSimplePool* resPool);
};

CFactoryFnReturn FProjectileWeaponDataFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& vparms,
                                              CObjectReference*);
} // namespace metaforce
