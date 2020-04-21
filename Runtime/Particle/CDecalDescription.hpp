#pragma once

#include <memory>

#include "Runtime/Particle/CColorElement.hpp"
#include "Runtime/Particle/CIntElement.hpp"
#include "Runtime/Particle/CParticleDataFactory.hpp"
#include "Runtime/Particle/CRealElement.hpp"
#include "Runtime/Particle/CUVElement.hpp"
#include "Runtime/Particle/CVectorElement.hpp"

namespace urde {

struct SQuadDescr {
  std::unique_ptr<CIntElement> x0_LFT;
  std::unique_ptr<CRealElement> x4_SZE;
  std::unique_ptr<CRealElement> x8_ROT;
  std::unique_ptr<CVectorElement> xc_OFF;
  std::unique_ptr<CColorElement> x10_CLR;
  std::unique_ptr<CUVElement> x14_TEX;
  bool x18_ADD = false;
};

class CDecalDescription {
public:
  CDecalDescription() = default;

  SQuadDescr x0_Quads[2];
  SParticleModel x38_DMDL;
  std::unique_ptr<CIntElement> x48_DLFT;
  std::unique_ptr<CVectorElement> x4c_DMOP;
  std::unique_ptr<CVectorElement> x50_DMRT;
  std::unique_ptr<CVectorElement> x54_DMSC;
  std::unique_ptr<CColorElement> x58_DMCL;
  bool x5c_24_DMAB : 1 = false;
  bool x5c_25_DMOO : 1 = false;
};

} // namespace urde
