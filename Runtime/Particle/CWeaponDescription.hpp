#pragma once

#include <memory>

#include "Runtime/RetroTypes.hpp"
#include "Runtime/Particle/CColorElement.hpp"
#include "Runtime/Particle/CIntElement.hpp"
#include "Runtime/Particle/CModVectorElement.hpp"
#include "Runtime/Particle/CParticleDataFactory.hpp"
#include "Runtime/Particle/CRealElement.hpp"
#include "Runtime/Particle/CVectorElement.hpp"

namespace urde {
class CCollisionResponseData;

struct SCollisionResponseData {
  TToken<CCollisionResponseData> m_res;
  bool m_found = false;
  SCollisionResponseData() = default;
  SCollisionResponseData(CToken&& tok, bool found) : m_res(std::move(tok)), m_found(found) {}
  explicit operator bool() const { return m_found; }
};

class CWeaponDescription {
public:
  std::unique_ptr<CVectorElement> x0_IORN;
  std::unique_ptr<CVectorElement> x4_IVEC;
  std::unique_ptr<CVectorElement> x8_PSOV;
  std::unique_ptr<CModVectorElement> xc_PSVM;
  bool x10_VMD2 = false;
  std::unique_ptr<CIntElement> x14_PSLT;
  std::unique_ptr<CVectorElement> x18_PSCL;
  std::unique_ptr<CColorElement> x1c_PCOL;
  std::unique_ptr<CVectorElement> x20_POFS;
  std::unique_ptr<CVectorElement> x24_OFST;
  bool x28_APSO = false;
  bool x29_HOMG = false;
  bool x2a_AP11 = false;
  bool x2b_AP21 = false;
  bool x2c_AS11 = false;
  bool x2d_AS12 = false;
  bool x2e_AS13 = false;
  std::unique_ptr<CRealElement> x30_TRAT;
  SChildGeneratorDesc x34_APSM;
  SChildGeneratorDesc x44_APS2;
  SSwooshGeneratorDesc x54_ASW1;
  SSwooshGeneratorDesc x64_ASW2;
  SSwooshGeneratorDesc x74_ASW3;
  SParticleModel x84_OHEF;
  SCollisionResponseData x94_COLR;
  bool xa4_EWTR = true;
  bool xa5_LWTR = true;
  bool xa6_SWTR = true;
  s32 xa8_PJFX = -1;
  std::unique_ptr<CRealElement> xac_RNGE;
  std::unique_ptr<CRealElement> xb0_FOFF;
  // PAL/RS5
  bool x28_SPS1 = false;
  bool x29_SPS2 = false;
  bool x29_FC60 = false;
};
} // namespace urde
