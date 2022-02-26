#pragma once

#include <memory>

#include "Runtime/Factory/CFactoryMgr.hpp"
#include "Runtime/CToken.hpp"
#include "Runtime/Streams/IOStreams.hpp"
#include "Runtime/Factory/IObj.hpp"
#include "Runtime/RetroTypes.hpp"

namespace metaforce {
class CSwooshDescription;
class CSimplePool;
class CParticleSwooshDataFactory {
  static std::unique_ptr<CSwooshDescription> CreateGeneratorDescription(CInputStream& in, CSimplePool* resPool);
  static bool CreateWPSM(CSwooshDescription* desc, CInputStream& in, CSimplePool* resPool);

public:
  static std::unique_ptr<CSwooshDescription> GetGeneratorDesc(CInputStream& in, CSimplePool* resPool);
};

CFactoryFnReturn FParticleSwooshDataFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& vparms,
                                            CObjectReference*);
} // namespace metaforce
