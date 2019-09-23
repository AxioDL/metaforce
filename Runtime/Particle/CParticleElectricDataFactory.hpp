#pragma once

#include "Runtime/CFactoryMgr.hpp"
#include "Runtime/CToken.hpp"
#include "Runtime/IOStreams.hpp"
#include "Runtime/IObj.hpp"
#include "Runtime/RetroTypes.hpp"

namespace urde {
class CElectricDescription;
class CSimplePool;
class CParticleElectricDataFactory {
  static CElectricDescription* CreateElectricDescription(CInputStream& in, CSimplePool* resPool);
  static bool CreateELSM(CElectricDescription* desc, CInputStream& in, CSimplePool* resPool);
  static void LoadELSMTokens(CElectricDescription* desc);

public:
  static CElectricDescription* GetGeneratorDesc(CInputStream& in, CSimplePool* resPool);
};

CFactoryFnReturn FParticleElectricDataFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& vparms,
                                              CObjectReference*);
} // namespace urde
