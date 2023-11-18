#pragma once

#include <memory>

#include "Runtime/CFactoryMgr.hpp"
#include "Runtime/CToken.hpp"
#include "Runtime/Streams/IOStreams.hpp"
#include "Runtime/IObj.hpp"
#include "Runtime/RetroTypes.hpp"

namespace metaforce {
class CElectricDescription;
class CSimplePool;
class CParticleElectricDataFactory {
  static std::unique_ptr<CElectricDescription> CreateElectricDescription(CInputStream& in, CSimplePool* resPool);
  static bool CreateELSM(CElectricDescription* desc, CInputStream& in, CSimplePool* resPool);
  static void LoadELSMTokens(CElectricDescription* desc);

public:
  static std::unique_ptr<CElectricDescription> GetGeneratorDesc(CInputStream& in, CSimplePool* resPool);
};

CFactoryFnReturn FParticleElectricDataFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& vparms,
                                              CObjectReference*);
} // namespace metaforce
