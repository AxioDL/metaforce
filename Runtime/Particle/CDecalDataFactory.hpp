#pragma once

#include <memory>

#include "Runtime/CFactoryMgr.hpp"
#include "Runtime/CToken.hpp"
#include "Runtime/IObj.hpp"
#include "Runtime/IOStreams.hpp"
#include "Runtime/RetroTypes.hpp"
#include "Runtime/Particle/CDecalDescription.hpp"

namespace metaforce {
class CSimplePool;

class CDecalDataFactory {
  static bool CreateDPSM(CDecalDescription* desc, CInputStream& in, CSimplePool* resPool);
  static std::unique_ptr<CDecalDescription> CreateGeneratorDescription(CInputStream& in, CSimplePool* resPool);
  static void GetQuadDecalInfo(CInputStream& in, CSimplePool* resPool, FourCC clsId, SQuadDescr& quad);

public:
  static std::unique_ptr<CDecalDescription> GetGeneratorDesc(CInputStream& in, CSimplePool* resPool);
};

CFactoryFnReturn FDecalDataFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& vparms,
                                   CObjectReference*);
} // namespace metaforce
